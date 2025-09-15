#!/usr/bin/env python3
"""
Documentation Coverage Checker

Computes documentation coverage for:
- Firmware (C/C++) via Doxygen XML in docs/site/firmware/xml
- Web UI backend (TypeScript) via TypeDoc JSON in docs/site/web/backend/typedoc.json
- Web UI frontend (TypeScript) via TypeDoc JSON in docs/site/web/frontend/typedoc.json

Compares against a baseline JSON and fails nonzero if coverage drops.

Usage:
  python3 scripts/doc_coverage_check.py \
    --baseline docs/coverage/doc_coverage_baseline.json \
    [--min-firmware 0.0] [--min-backend 0.0] [--min-frontend 0.0] \
    [--write-baseline-if-missing]

Exits with 0 on success, non-zero on failure.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Dict, Any, Tuple

try:
    import xml.etree.ElementTree as ET
except Exception as e:
    ET = None  # type: ignore


REPO_ROOT = Path(__file__).resolve().parents[1]
DOXYGEN_XML_DIR = REPO_ROOT / "docs/site/firmware/xml"
TYPEDOC_BACKEND_JSON = REPO_ROOT / "docs/site/web/backend/typedoc.json"
TYPEDOC_FRONTEND_JSON = REPO_ROOT / "docs/site/web/frontend/typedoc.json"


def compute_doxygen_coverage(xml_dir: Path) -> Tuple[int, int, float]:
    if not xml_dir.exists():
        return (0, 0, 0.0)
    index_file = xml_dir / "index.xml"
    if not index_file.exists():
        return (0, 0, 0.0)
    try:
        tree = ET.parse(str(index_file))
        root = tree.getroot()
    except Exception:
        return (0, 0, 0.0)

    # Count members documented: heuristic — members whose referenced compound xml has non-empty brief/detailed
    total = 0
    documented = 0
    # Collect all member refs from index
    members = root.findall(".//member")
    for m in members:
        total += 1
        refid = m.get("refid")
        kind = m.get("kind")
        if not refid:
            continue
        compound_file = xml_dir / f"{refid}.xml"
        if not compound_file.exists():
            continue
        try:
            ctree = ET.parse(str(compound_file))
            croot = ctree.getroot()
        except Exception:
            continue
        # Look for either briefdescription or detaileddescription with non-empty text
        # memberdef nodes contain documentation
        doc_nodes = croot.findall(".//memberdef")
        documented_here = False
        for dn in doc_nodes:
            brief = "".join(dn.findtext("briefdescription", default="").split())
            detail = "".join(dn.findtext("detaileddescription", default="").split())
            if brief or detail:
                documented_here = True
                break
        if documented_here:
            documented += 1
    coverage = (documented / total) if total else 0.0
    return (documented, total, coverage)


def compute_typedoc_coverage(json_path: Path) -> Tuple[int, int, float]:
    if not json_path.exists():
        return (0, 0, 0.0)
    try:
        data = json.loads(json_path.read_text(encoding="utf-8"))
    except Exception:
        return (0, 0, 0.0)

    total = 0
    documented = 0

    def walk(ref: Dict[str, Any]):
        nonlocal total, documented
        # Consider reflections of kinds: Class, Interface, Function, Method, Property, Enum, TypeAlias
        kind = ref.get("kindString", "")
        # We will count all reflections that represent API surfaces
        if kind in {"Class", "Interface", "Function", "Method", "Property", "Enum", "Type alias", "Namespace", "Module"}:
            total += 1
            comment = ref.get("comment") or {}
            # TypeDoc v0.26 comment model might use summary
            has_summary = bool(comment.get("summary"))
            has_block_tags = bool(comment.get("blockTags"))
            has_old_text = bool(comment.get("shortText") or comment.get("text"))
            if has_summary or has_block_tags or has_old_text:
                documented += 1
        for child in ref.get("children", []) or []:
            walk(child)
        # signatures for functions/methods
        for sig in ref.get("signatures", []) or []:
            walk(sig)

    walk(data)
    coverage = (documented / total) if total else 0.0
    return (documented, total, coverage)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--baseline", required=True, help="Path to baseline JSON file")
    ap.add_argument("--min-firmware", type=float, default=0.0)
    ap.add_argument("--min-backend", type=float, default=0.0)
    ap.add_argument("--min-frontend", type=float, default=0.0)
    ap.add_argument("--write-baseline-if-missing", action="store_true")
    args = ap.parse_args()

    fw_doc, fw_total, fw_cov = compute_doxygen_coverage(DOXYGEN_XML_DIR)
    be_doc, be_total, be_cov = compute_typedoc_coverage(TYPEDOC_BACKEND_JSON)
    fe_doc, fe_total, fe_cov = compute_typedoc_coverage(TYPEDOC_FRONTEND_JSON)

    current = {
        "firmware": {"documented": fw_doc, "total": fw_total, "coverage": round(fw_cov, 4)},
        "backend":  {"documented": be_doc, "total": be_total, "coverage": round(be_cov, 4)},
        "frontend": {"documented": fe_doc, "total": fe_total, "coverage": round(fe_cov, 4)},
    }

    baseline_path = Path(args.baseline)
    if not baseline_path.exists():
        if args.write_baseline_if_missing:
            baseline_path.parent.mkdir(parents=True, exist_ok=True)
            baseline_path.write_text(json.dumps(current, indent=2), encoding="utf-8")
            print(f"Baseline created at {baseline_path}")
            print(json.dumps(current, indent=2))
            return 0
        else:
            print(f"Baseline missing: {baseline_path}")
            print(json.dumps(current, indent=2))
            return 1

    try:
        baseline = json.loads(baseline_path.read_text(encoding="utf-8"))
    except Exception:
        print("Failed to read baseline JSON")
        print(json.dumps(current, indent=2))
        return 1

    failures = []

    # Check against minimum thresholds first (absolute gates)
    if fw_cov < args.min_firmware:
        failures.append(f"Firmware coverage {fw_cov:.4f} below minimum {args.min_firmware:.4f}")
    if be_cov < args.min_backend:
        failures.append(f"Backend coverage {be_cov:.4f} below minimum {args.min_backend:.4f}")
    if fe_cov < args.min_frontend:
        failures.append(f"Frontend coverage {fe_cov:.4f} below minimum {args.min_frontend:.4f}")

    # No-drop gates (relative to baseline)
    def compare(name: str, now: Dict[str, Any], base: Dict[str, Any]):
        if not base:
            return
        now_cov = now.get("coverage", 0.0)
        base_cov = base.get("coverage", 0.0)
        if now_cov + 1e-9 < base_cov:  # allow tiny float noise
            failures.append(f"{name} coverage dropped: {now_cov:.4f} < baseline {base_cov:.4f}")

    compare("Firmware", current.get("firmware", {}), baseline.get("firmware", {}))
    compare("Backend", current.get("backend", {}), baseline.get("backend", {}))
    compare("Frontend", current.get("frontend", {}), baseline.get("frontend", {}))

    print("Current documentation coverage")
    print(json.dumps(current, indent=2))

    if failures:
        print("\nFAILURES:")
        for f in failures:
            print(f"- {f}")
        return 2

    print("\nDocumentation coverage checks passed (no drops detected).")
    return 0


if __name__ == "__main__":
    sys.exit(main())
