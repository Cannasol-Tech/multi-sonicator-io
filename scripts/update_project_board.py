#!/usr/bin/env python3
"""
Auto-updates project board markdown files from story files and QA gates.

Updates:
- docs/agile/project-board.md: Last Updated timestamp; table rows per story (Status, QA Gate, QA Gate Timestamp)
- docs/agile/stories/STORY_STATUS_DASHBOARD.md: Last Updated date; table rows per story (Status, QA Gate)

Scope: Non-destructive update of known cells only. Leaves other columns intact.
"""
from __future__ import annotations

import re
import sys
import pathlib
import datetime as dt
from typing import Dict, Optional

import yaml

REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
STORIES_DIR = REPO_ROOT / "docs" / "agile" / "stories"
GATES_DIR = REPO_ROOT / "docs" / "qa" / "gates"

PROJECT_BOARD = REPO_ROOT / "docs" / "agile" / "project-board.md"
STORY_DASHBOARD = STORIES_DIR / "STORY_STATUS_DASHBOARD.md"


def iso_now_z() -> str:
    return dt.datetime.utcnow().replace(microsecond=0).isoformat() + "Z"


def read_text(p: pathlib.Path) -> str:
    return p.read_text(encoding="utf-8")


def write_text(p: pathlib.Path, content: str) -> None:
    p.write_text(content, encoding="utf-8")


def parse_story_meta(path: pathlib.Path) -> Dict[str, str]:
    text = read_text(path)
    meta: Dict[str, str] = {}

    # Story id from filename, e.g., 3.1.frontend-... -> 3.1
    m = re.match(r"^(\d+\.\d+)\.", path.name)
    if m:
        meta["id"] = m.group(1)
    # Frontmatter YAML
    if text.startswith("---\n"):
        end = text.find("\n---", 4)
        if end != -1:
            fm = text[4:end]
            try:
                data = yaml.safe_load(fm) or {}
                if isinstance(data, dict):
                    for k in ("id", "title", "status"):
                        if k in data:
                            meta[k] = str(data[k])
            except Exception:
                pass
    # Fallback: parse Status section
    m2 = re.search(r"^##\s+Status\s*\n+([A-Za-z \-]+)", text, re.M)
    if m2 and "status" not in meta:
        meta["status"] = m2.group(1).strip()
    # Title fallback from H1
    if "title" not in meta:
        m3 = re.search(r"^#\s+Story\s+\d+\.\d+\s*:\s*(.+)$", text, re.M)
        if m3:
            meta["title"] = m3.group(1).strip()
    return meta


def find_gate(story_id: str) -> Optional[pathlib.Path]:
    if not GATES_DIR.exists():
        return None
    candidates = sorted(GATES_DIR.glob(f"{story_id}*.yml"))
    return candidates[0] if candidates else None


def parse_gate(path: pathlib.Path) -> Dict[str, str]:
    try:
        data = yaml.safe_load(read_text(path)) or {}
        if not isinstance(data, dict):
            return {}
        return {
            "gate": str(data.get("gate", "")).upper() or "",
            "updated": str(data.get("updated", "")),
        }
    except Exception:
        return {}


STATUS_MAP = {
    "DRAFT": "📋 **DRAFT**",
    "APPROVED": "🟢 **APPROVED**",
    "INPROGRESS": "🟡 **IN PROGRESS**",
    "IN PROGRESS": "🟡 **IN PROGRESS**",
    "REVIEW": "🔵 **REVIEW**",
    "DONE": "✅ **COMPLETED**",
}


def fmt_status(raw: str) -> Optional[str]:
    if not raw:
        return None
    key = raw.strip().upper()
    return STATUS_MAP.get(key)


def fmt_gate(g: str) -> str:
    g = (g or "").upper()
    if g == "PASS":
        return "✅ PASS"
    if g == "CONCERNS":
        return "⚠️ CONCERNS"
    if g == "FAIL":
        return "❌ FAIL"
    if g == "WAIVED":
        return "⚖️ WAIVED"
    if g == "BLOCKED":
        return "🔒 BLOCKED"
    return "-"


def update_last_updated_line(md: str, utc_timestamp: str) -> str:
    return re.sub(r"(^\*\*Last Updated\*\*\s*:\s*).*$",
                  rf"\1{utc_timestamp}", md, flags=re.M)


def update_last_updated_date(md: str, date_str: str) -> str:
    # Matches e.g., **Last Updated**: 2025-09-14
    return re.sub(r"(^\*\*Last Updated\*\*\s*:\s*).*$",
                  rf"\1{date_str}", md, flags=re.M)


def update_table_rows(md: str, story_info: Dict[str, Dict[str, str]], columns: Dict[str, int]) -> str:
    """Update markdown table rows for stories based on provided column indices.

    columns: mapping like {"Status": 3, "QA Gate": 5, "QA Gate Timestamp": 6}
    Indexing counts pipes; index 0 is text before first pipe, so typical usable cells start at 1.
    We will split, trim, and rebuild with original spacing normalized.
    """
    lines = md.splitlines()
    out = []
    row_pat = re.compile(r"^\|\s*\*\*(\d+\.\d+)\*\*\s*\|")
    for line in lines:
        m = row_pat.match(line)
        if not m:
            out.append(line)
            continue
        sid = m.group(1)
        info = story_info.get(sid)
        if not info:
            out.append(line)
            continue
        parts = [p.strip() for p in line.split("|")]
        # Ensure we have enough columns
        def set_col(col_name: str, value: Optional[str]):
            idx = columns.get(col_name)
            if idx is None or value is None:
                return
            if idx < len(parts):
                parts[idx] = f" {value} "

        set_col("Status", info.get("status_cell"))
        set_col("QA Gate", info.get("gate_cell"))
        set_col("QA Gate Timestamp", info.get("gate_ts"))
        # Rebuild line
        out.append("|".join(parts))
    return "\n".join(out) + ("\n" if md.endswith("\n") else "")


def collect_story_info() -> Dict[str, Dict[str, str]]:
    result: Dict[str, Dict[str, str]] = {}
    for path in sorted(STORIES_DIR.glob("*.md")):
        if path.name.upper() == "STORY_STATUS_DASHBOARD.md".upper():
            continue
        meta = parse_story_meta(path)
        sid = meta.get("id")
        if not sid:
            continue
        gate = find_gate(sid)
        gate_info = parse_gate(gate) if gate else {}
        status_cell = fmt_status(meta.get("status", ""))
        gate_cell = fmt_gate(gate_info.get("gate", "")) if gate_info else "-"
        info = {
            "status_raw": meta.get("status", ""),
            "status_cell": status_cell or "",
            "gate_cell": gate_cell,
            "gate_ts": gate_info.get("updated", "") if gate_info else "",
        }
        result[sid] = info
    return result


def main() -> int:
    story_info = collect_story_info()
    now_iso = iso_now_z()
    today = dt.date.today().isoformat()

    # Update project board
    if PROJECT_BOARD.exists():
        md = read_text(PROJECT_BOARD)
        md = update_last_updated_line(md, now_iso)
        # Columns by header for project board Epic tables
        # Header: | Story | Title | Status | Progress | QA Gate | QA Gate Timestamp | ...
        md = update_table_rows(md, story_info, {"Status": 3, "QA Gate": 5, "QA Gate Timestamp": 6})
        write_text(PROJECT_BOARD, md)

    # Update story dashboard
    if STORY_DASHBOARD.exists():
        md2 = read_text(STORY_DASHBOARD)
        md2 = update_last_updated_date(md2, today)
        # Header: | Story | Title | Status | Progress | QA Gate | ...
        md2 = update_table_rows(md2, story_info, {"Status": 3, "QA Gate": 5})
        write_text(STORY_DASHBOARD, md2)

    print("✅ Project board and story dashboard updated.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

