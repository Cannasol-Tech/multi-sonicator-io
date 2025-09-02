#!/usr/bin/env python3
"""
Robust ArduinoISP uploader for Arduino UNO R4 WiFi (or other boards via FQBN).
- Detects port via `arduino-cli board list --format json`
- Ensures core is installed
- Ensures ArduinoISP sketch is present (downloads if needed)
- Compiles and uploads with retries, optional 1200-baud touch to enter bootloader

Env overrides:
- ARDUINO_FQBN (default: arduino:renesas_uno:unor4wifi)
- ARDUINO_PORT (optional, overrides auto-detection)
- SKETCH_DIR (default: test/hardware/sketches/ArduinoISP)
"""
import json
import os
import subprocess
import sys
import time
from pathlib import Path

FQBN = os.environ.get("ARDUINO_FQBN", "arduino:renesas_uno:unor4wifi")
PORT_OVERRIDE = os.environ.get("ARDUINO_PORT", "").strip()
SKETCH_DIR = Path(os.environ.get("SKETCH_DIR", "test/hardware/sketches/ArduinoISP")).resolve()

ARDUINO_CLI = os.environ.get("ARDUINO_CLI", "arduino-cli")


def run(cmd: list[str], check: bool = True, capture: bool = True) -> subprocess.CompletedProcess:
    print("$", " ".join(cmd))
    return subprocess.run(cmd, text=True, capture_output=capture, check=check)


def ensure_sketch_exists() -> None:
    SKETCH_DIR.mkdir(parents=True, exist_ok=True)
    sketch_file = SKETCH_DIR / "ArduinoISP.ino"
    if sketch_file.exists():
        return
    url = "https://raw.githubusercontent.com/arduino/arduino-examples/main/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino"
    print(f"Downloading ArduinoISP sketch to {sketch_file} ...")
    cp = run(["curl", "-fsSL", url, "-o", str(sketch_file)], check=True)


def _prefer_tty(port: str) -> str:
    if port.startswith("/dev/cu."):
        tty = port.replace("/dev/cu.", "/dev/tty.")
        if os.path.exists(tty):
            return tty
    return port


def list_board_port() -> str:
    if PORT_OVERRIDE:
        return _prefer_tty(PORT_OVERRIDE)
    try:
        cp = run([ARDUINO_CLI, "board", "list", "--format", "json"], check=True)
        data = json.loads(cp.stdout or "{}")
        for d in data.get("detected_ports", []):
            boards = d.get("matching_boards") or []
            if any("unor4wifi" in (b.get("fqbn") or "") for b in boards):
                addr = d.get("port", {}).get("address", "")
                if addr:
                    return _prefer_tty(addr)
    except Exception as e:
        print(f"WARN: board list failed: {e}")
    return ""


def touch_1200(port: str) -> None:
    try:
        import serial  # type: ignore
        s = serial.Serial(port, 1200)
        s.close()
        time.sleep(1.0)
    except Exception as e:
        print(f"WARN: 1200-baud touch failed: {e}")
        time.sleep(1.0)


def ensure_core_installed() -> None:
    try:
        run([ARDUINO_CLI, "core", "update-index"], check=False)
        run([ARDUINO_CLI, "core", "install", "arduino:renesas_uno"], check=False)
    except Exception:
        pass


def compile_sketch() -> None:
    run([ARDUINO_CLI, "compile", "--fqbn", FQBN, str(SKETCH_DIR)], check=True)


def _alt_port(port: str) -> str:
    if port.startswith("/dev/tty."):
        return port.replace("/dev/tty.", "/dev/cu.")
    if port.startswith("/dev/cu."):
        return port.replace("/dev/cu.", "/dev/tty.")
    return port

def upload_sketch(port: str) -> bool:
    # Try primary port
    cp = run([ARDUINO_CLI, "upload", "--fqbn", FQBN, "--port", port, str(SKETCH_DIR)], check=False)
    if cp.returncode == 0:
        return True
    # Try alternate device node (tty<->cu)
    alt = _alt_port(port)
    if alt != port:
        print(f"Retry on alternate device node: {alt}")
        cp2 = run([ARDUINO_CLI, "upload", "--fqbn", FQBN, "--port", alt, str(SKETCH_DIR)], check=False)
        return cp2.returncode == 0
    return False


def main() -> int:
    ensure_core_installed()
    ensure_sketch_exists()
    compile_sketch()

    port = list_board_port()
    if not port:
        print("ERROR: No Arduino port detected. Set ARDUINO_PORT=/dev/tty... and retry.")
        return 2

    print(f"Using port: {port}")

    # Try upload up to 3 times, with a 1200-baud touch between attempts
    for attempt in range(1, 4):
        if upload_sketch(port):
            print("OK: ArduinoISP uploaded successfully")
            return 0
        print(f"WARN: Upload attempt {attempt} failed; retrying...")
        touch_1200(port)
        # Re-detect port in case it changed
        port2 = list_board_port()
        if port2 and port2 != port:
            print(f"INFO: Port changed to {port2}")
            port = port2
    print("ERROR: Failed to upload ArduinoISP after retries")
    return 1


if __name__ == "__main__":
    sys.exit(main())

