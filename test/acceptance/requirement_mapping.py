"""
Requirement mapping and coverage reporting for PRD Functional Requirements (FR1–FR11).

- Adds a simple scanner that looks for @requirement_FRx tags in Behave feature files.
- Writes a Markdown and JSON summary for traceability.
"""
from __future__ import annotations

import json
import os
import re
from dataclasses import dataclass, field
from typing import Dict, List, Set


PRD_FUNCTIONAL_REQUIREMENTS: Dict[str, str] = {
    "FR1": "Control up to four CT2000 sonicator units concurrently via a single interface.",
    "FR2": "Provide individual start and stop control for each connected sonicator unit.",
    "FR3": "Provide individual amplitude control per unit with clamping to 20–100%.",
    "FR4": "Provide individual overload reset per unit.",
    "FR5": "Monitor and report real-time power (Watts) per unit.",
    "FR6": "Monitor and report real-time operating frequency (Hz) per unit.",
    "FR7": "Monitor and report status per unit (Overload, Frequency Lock, COMM_FAULT).",
    "FR8": "Expose all control/monitoring over a MODBUS RTU slave interface.",
    "FR9": "Report total number of active sonicators and a bitmask of running states.",
    "FR10": "On power-up, all sonicator units default to 'off' and require explicit start.",
    "FR11": "On PLC/HMI comms loss >1s, all sonicators enter a safe, non-operational state.",
}


RE_TAG = re.compile(r"@requirement_FR(\d+)\b")
RE_AC = re.compile(r"@story_0_3_AC(\d+)\b")


@dataclass
class Coverage:
    found: Dict[str, List[str]] = field(default_factory=dict)
    pending: Set[str] = field(default_factory=set)
    ac_found: Dict[str, List[str]] = field(default_factory=dict)
    ac_pending: Set[str] = field(default_factory=set)

    def add(self, fr: str, path: str, is_pending: bool):
        self.found.setdefault(fr, [])
        if path not in self.found[fr]:
            self.found[fr].append(path)
        if is_pending:
            self.pending.add(fr)

    def add_ac(self, ac: str, path: str, is_pending: bool):
        self.ac_found.setdefault(ac, [])
        if path not in self.ac_found[ac]:
            self.ac_found[ac].append(path)
        if is_pending:
            self.ac_pending.add(ac)


def scan_feature_tags(features_dir: str) -> Coverage:
    cov = Coverage()
    for root, _dirs, files in os.walk(features_dir):
        for name in files:
            if not name.endswith(".feature"):
                continue
            path = os.path.join(root, name)
            try:
                with open(path, "r", encoding="utf-8") as f:
                    content = f.read()
            except Exception:
                continue
            is_pending = "@pending" in content
            for m in RE_TAG.finditer(content):
                fr_num = m.group(1)
                fr = f"FR{fr_num}"
                cov.add(fr, os.path.relpath(path), is_pending)
            for m in RE_AC.finditer(content):
                ac_num = m.group(1)
                ac = f"AC{ac_num}"
                cov.add_ac(ac, os.path.relpath(path), is_pending)
    return cov


def to_markdown(cov: Coverage) -> str:
    all_frs = list(PRD_FUNCTIONAL_REQUIREMENTS.keys())
    lines = [
        "# PRD Functional Requirements Coverage (BDD Tags)",
        "",
        "This report summarizes which PRD requirements (FR1–FR11) are tagged in Behave feature files.",
        "Tags used: `@requirement_FRx`.",
        "",
        "| FR | Description | Tagged In | Pending? |",
        "|----|-------------|-----------|----------|",
    ]
    for fr in all_frs:
        desc = PRD_FUNCTIONAL_REQUIREMENTS[fr]
        files = cov.found.get(fr, [])
        pending = (fr in cov.pending)
        files_str = "<none>" if not files else "<br/>".join(files)
        lines.append(f"| {fr} | {desc} | {files_str} | {'yes' if pending else 'no'} |")
    return "\n".join(lines) + "\n"


def write_reports(cov: Coverage, out_dir_md: str = "final", junit_dir_json: str = "acceptance-junit") -> None:
    os.makedirs(out_dir_md, exist_ok=True)
    os.makedirs(junit_dir_json, exist_ok=True)
    md = to_markdown(cov)
    with open(os.path.join(out_dir_md, "requirements-coverage.md"), "w", encoding="utf-8") as f:
        f.write(md)
    payload = {
        "requirements": PRD_FUNCTIONAL_REQUIREMENTS,
        "coverage": cov.found,
        "pending": sorted(list(cov.pending)),
    }
    with open(os.path.join(junit_dir_json, "requirements_coverage.json"), "w", encoding="utf-8") as f:
        json.dump(payload, f, indent=2)

    # Story S-0.3 acceptance coverage (AC1–AC12)
    ac_lines = [
        "# Story S-0.3 Acceptance Criteria Coverage",
        "",
        "Tags used: `@story_0_3_AC{n}`.",
        "",
        "| AC | Tagged In | Pending? |",
        "|----|-----------|----------|",
    ]
    for i in range(1, 13):
        ac = f"AC{i}"
        files = cov.ac_found.get(ac, [])
        pending = (ac in cov.ac_pending)
        files_str = "<none>" if not files else "<br/>".join(files)
        ac_lines.append(f"| {ac} | {files_str} | {'yes' if pending else 'no'} |")
    ac_payload = {
        "story": "S-0.3",
        "coverage": cov.ac_found,
        "pending": sorted(list(cov.ac_pending)),
    }
    with open(os.path.join(junit_dir_json, "story_S-0.3_coverage.json"), "w", encoding="utf-8") as f:
        json.dump(ac_payload, f, indent=2)


def summarize(features_dir: str = "test/acceptance/features") -> Coverage:
    cov = scan_feature_tags(features_dir)
    write_reports(cov)
    return cov


if __name__ == "__main__":
    cov = summarize()
    print(to_markdown(cov))
