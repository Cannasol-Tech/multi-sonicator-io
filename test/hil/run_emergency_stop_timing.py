#!/usr/bin/env python3
"""
Run an on-hardware emergency-stop timing validation.

Uses the Arduino Test Wrapper via HardwareInterface and SafetyInterlocks.
PASS if response time <= 100ms; SKIP (exit 0) if hardware is not available.
"""

import sys
import json
import argparse
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(PROJECT_ROOT))

try:
    from test.hil.safety_interlocks import SafetyInterlocks
    from test.acceptance.hil_framework.hardware_interface import HardwareInterface
except Exception as e:
    print(f"ERROR: Failed to import HIL modules: {e}")
    sys.exit(2)


def main() -> int:
    parser = argparse.ArgumentParser(description="Emergency-stop timing validation")
    parser.add_argument("--port", help="Serial port for Arduino (auto-detect if omitted)", default=None)
    parser.add_argument("--json", action="store_true", help="Emit JSON result to stdout")
    args = parser.parse_args()

    hw = HardwareInterface(serial_port=args.port)
    if not hw.verify_connection():
        msg = "SKIP: No Arduino Test Wrapper detected; skipping timing test"
        if args.json:
            print(json.dumps({"skipped": True, "reason": msg}))
        else:
            print(msg)
        return 0

    safety = SafetyInterlocks(hardware_interface=hw)
    result = safety.test_emergency_stop_response()

    # Normalize fields
    passed = bool(result.get("passed", False))
    resp_ms = float(result.get("response_time_ms", 0.0))
    spec_ms = float(result.get("specification_ms", 100.0))

    if args.json:
        print(json.dumps({
            "passed": passed,
            "response_time_ms": round(resp_ms, 2),
            "specification_ms": round(spec_ms, 2)
        }))
    else:
        status = "PASS" if passed else "FAIL"
        print(f"Emergency stop timing: {status} — {resp_ms:.1f}ms (spec <= {spec_ms:.0f}ms)")

    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())

