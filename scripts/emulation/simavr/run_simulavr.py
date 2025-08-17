#!/usr/bin/env python3
"""
Minimal simulavr runner for CI/local BDD emulation.

- Target: ATmega32A @ 16 MHz
- Loads ELF from .pio/build/development/firmware.elf
- No PTY wiring or simmsio. Pure simulavr process.

This script keeps the simulavr process in the foreground until it receives SIGTERM/SIGINT,
so Makefile targets can background it and kill it reliably.
"""
import os
import signal
import subprocess
import sys
from pathlib import Path

ELF_PATH = Path(".pio/build/development/firmware.elf")
SIMULAVR_BIN = os.environ.get("SIMULAVR_BIN", "simulavr")
MCU = os.environ.get("SIMULAVR_MCU", "atmega32")

proc = None

def ensure_elf_exists() -> None:
    if not ELF_PATH.exists():
        print(f"[run_simulavr] ELF not found at {ELF_PATH}. Building development ELF...", flush=True)
        # Try to build using PlatformIO
        try:
            subprocess.check_call(["pio", "run", "-e", "development"])
        except (subprocess.CalledProcessError, FileNotFoundError) as e:
            print(f"[run_simulavr] Failed to build ELF: {e}", file=sys.stderr)
            sys.exit(1)
        if not ELF_PATH.exists():
            print(f"[run_simulavr] ELF still not found after build: {ELF_PATH}", file=sys.stderr)
            sys.exit(1)


def handle_signal(signum, frame):
    global proc
    print(f"[run_simulavr] Received signal {signum}, terminating simulavr...", flush=True)
    if proc and proc.poll() is None:
        try:
            proc.terminate()
        except Exception:
            pass
        try:
            proc.wait(timeout=5)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass
    sys.exit(0)


def main():
    global proc
    ensure_elf_exists()

    cmd = [
        SIMULAVR_BIN,
        "-d", MCU,
        "-f", str(ELF_PATH),
        # Add options here if needed (e.g., clock, tracing). Keeping minimal for stability.
    ]

    print(f"[run_simulavr] Launching: {' '.join(cmd)}", flush=True)
    try:
        proc = subprocess.Popen(cmd)
    except FileNotFoundError:
        print("[run_simulavr] 'simulavr' not found in PATH. Please install simulavr.", file=sys.stderr)
        sys.exit(1)

    # Register clean shutdown handlers
    signal.signal(signal.SIGTERM, handle_signal)
    signal.signal(signal.SIGINT, handle_signal)

    # Wait for child
    return_code = proc.wait()
    print(f"[run_simulavr] simulavr exited with code {return_code}")
    sys.exit(return_code)


if __name__ == "__main__":
    main()
