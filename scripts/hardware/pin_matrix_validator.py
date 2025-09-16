#!/usr/bin/env python3
"""
Pin Matrix Validator

Reads config/hardware-config.yaml (single source of truth) and validates:
- Every DB9 connector pin maps to an existing DUT pin in dut.pins
- The signal name on the DB9 entry matches the DUT pin's signal
- All referenced pins in test_capabilities exist in dut.pins (or 'all')

Outputs a markdown report and exits non‑zero on validation failures.
"""
from __future__ import annotations
import argparse
import sys
from pathlib import Path
from typing import Dict, Any, List, Tuple

import yaml


ROOT = Path(__file__).resolve().parents[2]
CFG_PATH = ROOT / "config" / "hardware-config.yaml"
REPORT_DIR = ROOT / "test" / "reports" / "hardware"
REPORT_FILE = REPORT_DIR / "pin_matrix_report.md"


def load_yaml(path: Path) -> Dict[str, Any]:
    with open(path, "r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def validate_db9_mappings(cfg: Dict[str, Any]) -> Tuple[List[str], List[str]]:
    errors: List[str] = []
    warnings: List[str] = []
    dut_pins: Dict[str, Dict[str, Any]] = (cfg.get("dut", {}).get("pins", {}) or {})
    db9: Dict[str, Any] = cfg.get("db9_connectors", {}) or {}

    # Build reverse map: signal -> dut_pin key(s)
    signal_to_pins: Dict[str, List[str]] = {}
    for dut_key, meta in dut_pins.items():
        sig = str(meta.get("signal", "")).strip()
        if sig:
            signal_to_pins.setdefault(sig, []).append(dut_key)

    # Validate each DB9 mapping
    for conn_name, conn in db9.items():
        pins = (conn or {}).get("pins", {}) or {}
        for pin_num, entry in pins.items():
            sig = str(entry.get("signal", "")).strip()
            dut_pin = str(entry.get("dut_pin", "")).strip()
            if not dut_pin:
                errors.append(f"{conn_name} pin {pin_num}: missing dut_pin")
                continue
            if dut_pin not in dut_pins:
                errors.append(f"{conn_name} pin {pin_num}: dut_pin '{dut_pin}' not defined in dut.pins")
                continue
            dut_sig = str(dut_pins[dut_pin].get("signal", "")).strip()
            if sig and dut_sig and sig != dut_sig:
                errors.append(
                    f"{conn_name} pin {pin_num}: signal mismatch: DB9='{sig}' vs DUT '{dut_pin}' signal='{dut_sig}'"
                )
            # Basic direction sanity: DB9 side implies direction via context; skip strict check

    return errors, warnings


def validate_test_capabilities(cfg: Dict[str, Any]) -> Tuple[List[str], List[str]]:
    errors: List[str] = []
    warnings: List[str] = []
    dut_pins: Dict[str, Dict[str, Any]] = (cfg.get("dut", {}).get("pins", {}) or {})
    caps = cfg.get("test_capabilities", []) or []
    for cap in caps:
        name = cap.get("name", "<unnamed>")
        pins = cap.get("pins", []) or []
        if pins == ["all"]:
            continue
        for p in pins:
            if p not in dut_pins:
                errors.append(f"test_capabilities '{name}': pin '{p}' not found in dut.pins")
    return errors, warnings


def write_report(ok: bool, errors: List[str], warnings: List[str]) -> None:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    lines: List[str] = []
    lines.append("# Pin Matrix Validation Report")
    lines.append("")
    lines.append(f"Result: {'PASS' if ok else 'FAIL'}")
    lines.append("")
    if warnings:
        lines.append("## Warnings")
        lines.extend([f"- {w}" for w in warnings])
        lines.append("")
    if errors:
        lines.append("## Errors")
        lines.extend([f"- {e}" for e in errors])
        lines.append("")
    with open(REPORT_FILE, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))


def main(argv: List[str]) -> int:
    parser = argparse.ArgumentParser(description="Validate hardware pin matrix against DB9 mappings and capabilities")
    parser.add_argument("--config", default=str(CFG_PATH), help="Path to hardware-config.yaml")
    args = parser.parse_args(argv)

    cfg = load_yaml(Path(args.config))
    errors: List[str] = []
    warnings: List[str] = []

    e1, w1 = validate_db9_mappings(cfg)
    e2, w2 = validate_test_capabilities(cfg)
    errors.extend(e1)
    errors.extend(e2)
    warnings.extend(w1)
    warnings.extend(w2)

    ok = len(errors) == 0
    write_report(ok, errors, warnings)
    print(f"Pin matrix validation {'PASS' if ok else 'FAIL'} — report: {REPORT_FILE}")
    if not ok:
        for e in errors:
            print(f"ERROR: {e}", file=sys.stderr)
    return 0 if ok else 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))

