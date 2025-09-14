#!/usr/bin/env python3
"""
QA CLI: Lightweight command handlers to support the Quinn QA workflow without
activating any external agent runtime. Reads repo-local config and writes
outputs to docs/qa per .bmad-core/core-config.yaml.

Commands:
  help                          Show available commands
  gate STORY --status S [--reason TEXT]
                                Create/update a quality gate file for a story

Notes:
- STORY can be one of:
    • Numeric id like 1.4
    • Slugged id like 1.4.single-sonicator-control
    • A direct path to a story file (e.g., docs/agile/stories/1.4.single-sonicator-control.md)
- Gate statuses: PASS | CONCERNS | FAIL | WAIVED
- Only gate is implemented end-to-end initially; other commands can be added later.
"""
from __future__ import annotations
import argparse
import datetime as dt
import pathlib
import re
import sys
from typing import Dict, Optional, Tuple

try:
    import yaml  # PyYAML
    from jinja2 import Template
except Exception as e:
    print("ERROR: Missing dependencies. Please install with:\n  pip install -r requirements.txt", file=sys.stderr)
    raise

REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
CONFIG_PATH = REPO_ROOT / ".bmad-core" / "core-config.yaml"
GATE_TEMPLATE_PATH = REPO_ROOT / ".bmad-core" / "templates" / "qa-gate-tmpl.yaml"


# ----------------------------- Utilities ----------------------------------

def load_core_config() -> Dict:
    if not CONFIG_PATH.exists():
        raise FileNotFoundError(f"Core config not found: {CONFIG_PATH}")
    with open(CONFIG_PATH, "r", encoding="utf-8") as f:
        cfg = yaml.safe_load(f) or {}
    return cfg


def get_paths_from_config(cfg: Dict) -> Tuple[pathlib.Path, pathlib.Path]:
    qa_loc = (cfg.get("qa", {}) or {}).get("qaLocation", "docs/qa")
    story_loc = cfg.get("devStoryLocation", "docs/agile/stories")
    return (REPO_ROOT / qa_loc, REPO_ROOT / story_loc)


def normalize_story_input(story_arg: str) -> str:
    """Return a normalized base name used to search for a story file.
    Examples:
      '1.4' -> '1.4'
      '1.4.single-sonicator-control' -> '1.4.single-sonicator-control'
      'docs/agile/stories/1.4.single-sonicator-control.md' -> '1.4.single-sonicator-control'
    """
    p = pathlib.Path(story_arg)
    if p.suffix == ".md":
        base = p.name
    else:
        base = p.name if "." in story_arg or "/" not in story_arg else p.name
    base = base[:-3] if base.endswith(".md") else base
    return base


def find_story_file(story_arg: str, story_dir: pathlib.Path) -> pathlib.Path:
    base = normalize_story_input(story_arg)
    # If a direct path was given and exists, use it
    direct = REPO_ROOT / story_arg
    if direct.exists() and direct.is_file():
        return direct
    # Try exact match first
    candidate = story_dir / f"{base}.md"
    if candidate.exists():
        return candidate
    # Try prefix match (e.g., '1.4' matches '1.4.single-sonicator-control.md')
    matches = sorted(story_dir.glob(f"{base}*.md"))
    if len(matches) == 1:
        return matches[0]
    if len(matches) > 1:
        raise ValueError(f"Ambiguous story id '{story_arg}'. Matches: {[m.name for m in matches]}")
    raise FileNotFoundError(f"Story not found for '{story_arg}' in {story_dir}")


def parse_frontmatter_and_title(story_path: pathlib.Path) -> Dict:
    """Parse YAML frontmatter and the first markdown H1 title if present."""
    text = story_path.read_text(encoding="utf-8")
    frontmatter: Dict = {}
    title_md: Optional[str] = None

    # YAML frontmatter between leading --- lines
    if text.startswith("---\n"):
        end = text.find("\n---\n", 4)
        if end != -1:
            fm_text = text[4:end]
            try:
                frontmatter = yaml.safe_load(fm_text) or {}
            except Exception:
                frontmatter = {}
            rest = text[end + 5 :]
        else:
            rest = text
    else:
        rest = text

    # First '# ' title in the remaining content
    for line in rest.splitlines():
        if line.strip().startswith("# "):
            title_md = line.strip()[2:].strip()
            break

    # Prefer frontmatter title
    story_title = frontmatter.get("title") or title_md or story_path.stem
    return {"frontmatter": frontmatter, "title": story_title}


def split_story_id_and_slug(stem: str) -> Tuple[str, str, str]:
    """Return epic_num, story_num, slug from a filename stem like '1.4.single-sonicator-control'."""
    # Expect forms like '1.4.*' or '0.3-*'
    m = re.match(r"^(\d+)\.(\d+)[.-](.+)$", stem)
    if not m:
        # If no slug part, try to split only numbers
        m2 = re.match(r"^(\d+)\.(\d+)$", stem)
        if not m2:
            raise ValueError(f"Cannot parse story id/slug from '{stem}'")
        epic, story = m2.group(1), m2.group(2)
        slug = stem
    else:
        epic, story, slug = m.group(1), m.group(2), m.group(3)
        slug = slug.replace(" ", "-").replace("_", "-")
    return epic, story, slug


# ---------------------------- Gate command ---------------------------------

def render_gate_yaml(context: Dict) -> str:
    template_text = GATE_TEMPLATE_PATH.read_text(encoding="utf-8") if GATE_TEMPLATE_PATH.exists() else None
    if template_text:
        # Render and then minimize to essential fields to avoid shipping template metadata
        rendered = Template(template_text).render(**context)
        try:
            data = yaml.safe_load(rendered) or {}
        except Exception:
            data = {}
        # Build a minimal, clean gate document
        out = {
            "schema": 1,
            "story": f"{context['epic_num']}.{context['story_num']}",
            "story_title": context["story_title"],
            "gate": context["gate_status"],
            "status_reason": context["status_reason"],
            "reviewer": "Quinn (Test Architect)",
            "updated": context["iso_timestamp"],
            "waiver": {"active": context["gate_status"] == "WAIVED"},
            "top_issues": [],
            "risk_summary": {"totals": {"critical": 0, "high": 0, "medium": 0, "low": 0}, "recommendations": {"must_fix": [], "monitor": []}},
        }
        return yaml.safe_dump(out, sort_keys=False)
    # Fallback minimal gate file
    fallback = {
        "schema": 1,
        "story": f"{context['epic_num']}.{context['story_num']}",
        "story_title": context["story_title"],
        "gate": context["gate_status"],
        "status_reason": context["status_reason"],
        "reviewer": "Quinn (Test Architect)",
        "updated": context["iso_timestamp"],
        "waiver": {"active": context["gate_status"] == "WAIVED"},
        "top_issues": [],
    }
    return yaml.safe_dump(fallback, sort_keys=False)


def cmd_gate(args: argparse.Namespace) -> int:
    cfg = load_core_config()
    qa_dir, stories_dir = get_paths_from_config(cfg)

    story_path = find_story_file(args.story, stories_dir)
    stem = story_path.stem
    epic_num, story_num, slug = split_story_id_and_slug(stem)
    meta = parse_frontmatter_and_title(story_path)

    now_iso = dt.datetime.utcnow().replace(microsecond=0).isoformat() + "Z"

    context = {
        "epic_num": epic_num,
        "story_num": story_num,
        "story_slug": slug,
        "story_title": meta["title"],
        "gate_status": args.status,
        "status_reason": args.reason or "",
        "iso_timestamp": now_iso,
    }

    out_dir = qa_dir / "gates"
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / f"{epic_num}.{story_num}-{slug}.yml"

    rendered = render_gate_yaml(context)
    out_path.write_text(rendered, encoding="utf-8")

    print(f"✅ Gate file written: {out_path.relative_to(REPO_ROOT)}")
    return 0


# ------------------------------ CLI wiring ---------------------------------

def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(description="Quinn QA CLI (lightweight)", add_help=True)
    sub = p.add_subparsers(dest="cmd")

    # help command (default)
    help_p = sub.add_parser("help", help="Show available commands")
    help_p.set_defaults(func=lambda a: print_commands() or 0)

    # gate command
    gate_p = sub.add_parser("gate", help="Create/update a quality gate file for a story")
    gate_p.add_argument("story", help="Story id or path (e.g., 1.4 or 1.4.single-sonicator-control)")
    gate_p.add_argument("--status", required=True, choices=["PASS", "CONCERNS", "FAIL", "WAIVED"], help="Gate status")
    gate_p.add_argument("--reason", default="", help="Short rationale for the gate decision")
    gate_p.set_defaults(func=cmd_gate)

    return p


def print_commands() -> None:
    cmds = [
        "help                          Show available commands",
        "gate {story} --status S [--reason TEXT]  Create/update quality gate file",
        # Placeholders for future expansion (not yet implemented):
        "nfr-assess {story}             [planned] NFR assessment",
        "review {story}                 [planned] Comprehensive QA review",
        "risk-profile {story}           [planned] Risk assessment",
        "test-design {story}            [planned] Test scenario design",
        "trace {story}                  [planned] Requirements traceability",
    ]
    print("Available commands:")
    for i, c in enumerate(cmds, 1):
        print(f"  {i:>2}) {c}")


def main(argv: Optional[list[str]] = None) -> int:
    parser = build_parser()
    # If no args, default to help
    if argv is None:
        argv = sys.argv[1:]
    if not argv:
        print_commands()
        return 0
    args = parser.parse_args(argv)
    if not hasattr(args, "func"):
        print_commands()
        return 0
    try:
        return int(args.func(args))
    except KeyboardInterrupt:
        print("\nAborted.")
        return 130
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

