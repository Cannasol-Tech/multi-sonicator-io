#!/usr/bin/env python3
"""
update_traceability.py

Scans Behave .feature files for @TODO:<key> and @trace:<key> tags and produces
- docs/traceability/bdd_traceability_matrix.csv (machine-readable)
- summary to stdout

Exit with non-zero status if any scenario contains a @TODO:<key> without a matching @trace:<key>.

This complements existing traceability scripts by enforcing a lightweight
"TODO must be traceable" rule to support pure BDD development workflows.
"""
from __future__ import annotations
import csv
import os
import re
import sys
from typing import List, Dict, Tuple

RE_TAG_LINE = re.compile(r"^\s*@.+$")
RE_SCENARIO = re.compile(r"^\s*Scenario:\s*(.+?)\s*$")
RE_TODO = re.compile(r"@TODO:([A-Za-z0-9_\-\.]+)")
RE_TRACE = re.compile(r"@trace:([A-Za-z0-9_\-\.]+)")

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
FEATURE_DIR = os.path.join(ROOT, "test", "acceptance", "features")
OUT_DIR = os.path.join(ROOT, "docs", "traceability")
OUT_CSV = os.path.join(OUT_DIR, "bdd_traceability_matrix.csv")


def scan_feature_file(path: str) -> List[Dict[str, str]]:
    rows: List[Dict[str, str]] = []
    pending_tags: List[str] = []
    scenario_name: str | None = None
    tags_for_current: List[str] = []

    def flush():
        nonlocal scenario_name, tags_for_current
        if scenario_name is None:
            return
        tags_str = " ".join(tags_for_current)
        todo_match = RE_TODO.search(tags_str)
        trace_match = RE_TRACE.search(tags_str)
        rows.append({
            "feature_file": os.path.relpath(path, ROOT),
            "scenario": scenario_name.strip(),
            "todo_key": todo_match.group(1) if todo_match else "",
            "trace_key": trace_match.group(1) if trace_match else "",
            "tags": tags_str,
            "status": "pending" if "@pending" in tags_for_current or (todo_match and not trace_match) else "ready"
        })
        scenario_name = None
        tags_for_current = []

    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            if RE_TAG_LINE.match(line):
                # Accumulate tags until a scenario line
                tags_for_current.extend(line.strip().split())
                continue
            m = RE_SCENARIO.match(line)
            if m:
                # Start of a new scenario; flush previous
                flush()
                scenario_name = m.group(1)
                # keep existing tags_for_current for this scenario
                continue
        # end for
    # Flush last scenario in file
    flush()
    return rows


def main() -> int:
    if not os.path.isdir(FEATURE_DIR):
        print(f"❌ Feature directory not found: {FEATURE_DIR}", file=sys.stderr)
        return 2
    os.makedirs(OUT_DIR, exist_ok=True)

    all_rows: List[Dict[str, str]] = []
    todo_without_trace: List[Tuple[str, str, str]] = []  # (file, scenario, todo_key)

    for root, _dirs, files in os.walk(FEATURE_DIR):
        for fn in files:
            if not fn.endswith(".feature"):
                continue
            path = os.path.join(root, fn)
            rows = scan_feature_file(path)
            all_rows.extend(rows)
            for r in rows:
                if r["todo_key"] and not r["trace_key"]:
                    todo_without_trace.append((r["feature_file"], r["scenario"], r["todo_key"]))

    # Write CSV
    with open(OUT_CSV, "w", encoding="utf-8", newline="") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["feature_file", "scenario", "todo_key", "trace_key", "status", "tags"])
        for r in all_rows:
            writer.writerow([r["feature_file"], r["scenario"], r["todo_key"], r["trace_key"], r["status"], r["tags"]])

    # Summary
    total = len(all_rows)
    todos = sum(1 for r in all_rows if r["todo_key"]) 
    missing_trace = len(todo_without_trace)
    print(f"📊 BDD Traceability Scan: {total} scenarios, {todos} with @TODO, {missing_trace} TODO without @trace")
    if missing_trace:
        print("❌ The following scenarios have @TODO without a matching @trace tag:")
        for file, scenario, todo in todo_without_trace:
            print(f" - {file} :: {scenario} (@TODO:{todo})")
        return 1
    print("✅ All @TODO scenarios have matching @trace tags")
    print(f"📝 Matrix written to {os.path.relpath(OUT_CSV, ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
