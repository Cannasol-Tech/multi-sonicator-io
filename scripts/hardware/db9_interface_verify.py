#!/usr/bin/env python3
"""
DB9 Interface Verification

Verifies per-connector requirements for DB9-1..4 (CT2000 sonicators) and DB9-0/DB9-5
(Main Automation MODBUS) based on config/hardware-config.yaml.

Checks performed (static, config-level):
- Required pins exist per connector with expected signal names
- Sonicator channels map to channel-suffixed signals (e.g., OVERLOAD_1 on DB9-1)
- DUT pin references exist for each mapping

Outputs a markdown report and returns non-zero on validation failures.

Signal continuity/signal integrity and mechanical checks require HIL/physical
hardware and are stubbed with clear TODOs pointing to HIL scripts.
"""
from __future__ import annotations
import sys
from pathlib import Path
from typing import Dict, Any, List, Tuple
import yaml

ROOT = Path(__file__).resolve().parents[2]
CFG_PATH = ROOT / "config" / "hardware-config.yaml"
REPORT_DIR = ROOT / "test" / "reports" / "hardware"
REPORT_FILE = REPORT_DIR / "db9_interface_report.md"


def load_yaml(path: Path) -> Dict[str, Any]:
    with open(path, "r", encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def ensure_pins_exists(conn_name: str, pins: Dict[str, Any], required: Dict[int, str], errors: List[str]):
    for pin_num, expected_sig in required.items():
        entry = pins.get(pin_num)
        if entry is None:
            errors.append(f"{conn_name}: missing pin {pin_num} (expected {expected_sig})")
            continue
        sig = str(entry.get("signal", "")).strip()
        if sig != expected_sig:
            errors.append(
                f"{conn_name}: pin {pin_num} signal mismatch: expected '{expected_sig}', found '{sig or '<empty>'}'"
            )
        dut_pin = str(entry.get("dut_pin", "")).strip()
        if not dut_pin:
            errors.append(f"{conn_name}: pin {pin_num} has no dut_pin reference (signal {sig or expected_sig})")


def build_required_for_channel(ch: int) -> Dict[int, str]:
    return {
        1: f"OVERLOAD_{ch}",
        2: f"RESET_{ch}",
        3: f"FREQ_LOCK_{ch}",
        4: f"FREQ_DIV10_{ch}",
        5: f"POWER_SENSE_{ch}",
        7: f"START_{ch}",
        8: "AMPLITUDE_ALL",
    }


def verify_db9_interfaces(cfg: Dict[str, Any]) -> Tuple[List[str], List[str], List[str]]:
    errors: List[str] = []
    warnings: List[str] = []
    notes: List[str] = []

    db9 = cfg.get("db9_connectors", {}) or {}
    if not db9:
        errors.append("No db9_connectors defined in hardware-config.yaml")
        return errors, warnings, notes

    # Sonicator connectors DB9-1..DB9-4
    for ch in (1, 2, 3, 4):
        name = f"DB9-{ch}"
        conn = db9.get(name)
        if not conn:
            errors.append(f"Missing connector {name}")
            continue
        pins = (conn.get("pins") or {})
        # normalize key types (YAML may parse numeric keys as ints)
        pins_norm: Dict[int, Any] = {int(k): v for k, v in pins.items()}
        ensure_pins_exists(name, pins_norm, build_required_for_channel(ch), errors)

    # MODBUS connectors DB9-0 and DB9-5 (alias)
    for name in ("DB9-0", "DB9-5"):
        conn = db9.get(name)
        if not conn:
            warnings.append(f"{name} not present (optional; at least one MODBUS DB9 required)")
            continue
        pins = (conn.get("pins") or {})
        pins_norm: Dict[int, Any] = {int(k): v for k, v in pins.items()}
        ensure_pins_exists(name, pins_norm, {8: "UART_RXD", 9: "UART_TXD"}, errors)

    # Notes for physical tests
    notes.append("Signal integrity/continuity and mechanical checks require HIL.")
    notes.append("Use acceptance steps to loop-back/oscilloscope checks for MODBUS and per-channel pins.")

    return errors, warnings, notes


def write_report(errors: List[str], warnings: List[str], notes: List[str]) -> None:
    REPORT_DIR.mkdir(parents=True, exist_ok=True)
    lines: List[str] = []
    lines.append("# DB9 Interface Verification Report")
    lines.append("")
    ok = (len(errors) == 0)
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
    if notes:
        lines.append("## Notes")
        lines.extend([f"- {n}" for n in notes])
        lines.append("")
    REPORT_FILE.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    cfg = load_yaml(CFG_PATH)
    errors, warnings, notes = verify_db9_interfaces(cfg)
    write_report(errors, warnings, notes)
    print(f"DB9 interface verification {'PASS' if not errors else 'FAIL'} — report: {REPORT_FILE}")
    return 0 if not errors else 2


if __name__ == "__main__":
    sys.exit(main())

