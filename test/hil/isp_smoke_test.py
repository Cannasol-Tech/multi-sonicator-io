#!/usr/bin/env python3
"""
ISP Programming Smoke Test

Validates end-to-end ISP programming using Arduino as ISP for ATmega32A.
- Detects Arduino port
- Ensures ArduinoISP sketch is loaded (uploads if needed)
- Warns about required 10µF capacitor on Arduino RESET→GND
- Verifies ISP communication (device signature)
- Uploads firmware via PlatformIO (env: atmega32a)
- Verifies again post-upload

Usage:
  python3 test/hil/isp_smoke_test.py [--port /dev/cu.usbmodemX] [--env atmega32a] [--non-interactive] [--json]

Notes:
- Requires `arduino-cli`, `avrdude`, and `pio` on PATH
- Non-interactive mode prints warnings instead of prompting
"""

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path


def run(cmd: list[str], check: bool = True, cwd: str | None = None) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, check=False)


def list_serial_ports() -> list[str]:
    ports: list[str] = []
    try:
        import serial.tools.list_ports  # type: ignore
        ports = [p.device for p in serial.tools.list_ports.comports()]
    except Exception:
        pass
    # Fallback globbing for macOS
    if sys.platform == "darwin":
        for base in ("/dev/cu.", "/dev/tty."):
            try:
                for p in Path("/dev").glob(base.split("/dev/")[1] + "*"):
                    ports.append(str(p))
            except Exception:
                pass
    return sorted(set(ports))


def detect_arduino_port(preferred: str | None = None) -> str | None:
    if preferred:
        return preferred
    candidates = list_serial_ports()
    # Prefer cu.* first on macOS
    if sys.platform == "darwin":
        cu = [p for p in candidates if "/dev/cu." in p]
        tty = [p for p in candidates if "/dev/tty." in p]
        candidates = cu + tty
    return candidates[0] if candidates else None


def detect_current_sketch(port: str) -> str:
    """Return 'arduino_isp', 'hil_wrapper', or 'unknown'."""
    # Try HIL wrapper PING at 115200
    try:
        import serial  # type: ignore
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(1.0)
        ser.write(b"PING\n")
        time.sleep(0.2)
        response = b""
        if ser.in_waiting:
            response = ser.readline()
        ser.close()
        if b"PONG" in (response or b"").upper() or b"OK" in (response or b"").upper():
            return "hil_wrapper"
    except Exception:
        pass
    # Try STK500v1 sync at 19200 (ArduinoISP)
    try:
        import serial  # type: ignore
        ser = serial.Serial(port, 19200, timeout=1)
        time.sleep(1.0)
        ser.write(bytes([0x30, 0x20]))  # get sync
        time.sleep(0.2)
        resp = ser.read(4)
        ser.close()
        if len(resp) >= 2 and resp[0] == 0x14 and resp[1] == 0x10:
            return "arduino_isp"
    except Exception:
        pass
    return "unknown"


def ensure_arduino_isp(port: str, non_interactive: bool) -> bool:
    if detect_current_sketch(port) == "arduino_isp":
        return True
    # Compile and upload local ArduinoISP sketch
    sketch_dir = Path("test/acceptance/sketches/ArduinoISP.ino").parent
    if not sketch_dir.exists():
        print("ERROR: ArduinoISP sketch directory not found.")
        return False
    fqbn = os.environ.get("ARDUINO_FQBN", "arduino:renesas_uno:unor4wifi")
    res = run(["arduino-cli", "compile", "--fqbn", fqbn, str(sketch_dir)], check=False)
    if res.returncode != 0:
        print("ERROR: Failed to compile ArduinoISP:")
        print(res.stderr.strip())
        return False
    res = run(["arduino-cli", "upload", "--fqbn", fqbn, "--port", port, str(sketch_dir)], check=False)
    if res.returncode != 0:
        print("ERROR: Failed to upload ArduinoISP:")
        print(res.stderr.strip())
        return False
    if not non_interactive:
        print("IMPORTANT: Connect a 10uF capacitor between RESET (+) and GND (-) on the Arduino before ISP.")
    time.sleep(2.0)
    return detect_current_sketch(port) == "arduino_isp"


def avrdude_probe(port: str) -> tuple[bool, str]:
    cmd = [
        "avrdude", "-c", "stk500v1", "-P", port, "-b", "19200",
        "-p", "m32", "-v",
    ]
    res = run(cmd, check=False)
    out = (res.stdout + res.stderr).lower()
    ok = (res.returncode == 0) or ("device signature" in out)
    return ok, out


def pio_upload(env: str, port: str) -> bool:
    res = run(["pio", "run", "-e", env, "--target", "upload", "--upload-port", port], check=False)
    if res.returncode != 0:
        # Print a brief snippet to help diagnosis
        print("ERROR: PlatformIO upload failed:")
        print((res.stdout or "")[-400:])
        print((res.stderr or "")[-400:])
        return False
    return True


def main() -> int:
    ap = argparse.ArgumentParser(description="ATmega32A ISP smoke test via Arduino as ISP")
    ap.add_argument("--port", help="Arduino serial port (auto-detect if omitted)")
    ap.add_argument("--env", default="atmega32a", help="PlatformIO env name (default: atmega32a)")
    ap.add_argument("--non-interactive", action="store_true", help="Do not prompt; print warnings only")
    ap.add_argument("--json", action="store_true", help="Emit JSON summary to stdout")
    args = ap.parse_args()

    summary = {
        "port": None,
        "sketch": None,
        "isp_ready": False,
        "pre_probe_ok": False,
        "upload_ok": False,
        "post_probe_ok": False,
    }

    port = detect_arduino_port(args.port)
    summary["port"] = port
    if not port:
        msg = "No Arduino port detected"
        if args.json:
            print(json.dumps({"passed": False, "error": msg}))
        else:
            print(f"ERROR: {msg}")
        return 2

    sketch = detect_current_sketch(port)
    summary["sketch"] = sketch

    if sketch != "arduino_isp":
        if not ensure_arduino_isp(port, args.non_interactive):
            err = "Failed to load ArduinoISP"
            if args.json:
                print(json.dumps({"passed": False, "error": err}))
            else:
                print(f"ERROR: {err}")
            return 3

    summary["isp_ready"] = True

    ok, _out = avrdude_probe(port)
    summary["pre_probe_ok"] = ok
    if not ok:
        err = "ArduinoISP not communicating with ATmega32A"
        if args.json:
            print(json.dumps({"passed": False, "error": err}))
        else:
            print(f"ERROR: {err}")
        return 4

    if not args.non_interactive:
        print("Proceeding to upload firmware via PlatformIO (ensure capacitor remains connected)...")
    if not pio_upload(args.env, port):
        if args.json:
            print(json.dumps({"passed": False, "error": "Firmware upload failed"}))
        else:
            print("ERROR: Firmware upload failed")
        return 5
    summary["upload_ok"] = True

    time.sleep(1.0)
    ok2, _out2 = avrdude_probe(port)
    summary["post_probe_ok"] = ok2

    passed = summary["isp_ready"] and summary["pre_probe_ok"] and summary["upload_ok"] and summary["post_probe_ok"]
    if args.json:
        print(json.dumps({"passed": passed, **summary}))
    else:
        print(f"ISP Smoke Test: {'PASS' if passed else 'FAIL'} on {port}")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())

