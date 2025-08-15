#!/usr/bin/env python3
"""
Simulavr runner for Multi-Sonicator firmware

- Builds the ELF with PlatformIO (development env)
- Launches simulavr with the correct device and CPU frequency
- Prints helpful tips and keeps the emulator running in the foreground

Prerequisites:
- simulavr installed and available in PATH
- Python 3.8+

Notes:
- UART PTY hookup is pending. This runner currently focuses on standing up the CPU+ELF.
- Device: simulavr supports atmega32 (not explicitly atmega32a). We use "atmega32" with f_cpu=8MHz per platformio.ini.
"""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parents[2]
PIO_ENV = "development"
ELF_RELATIVE = Path(".pio") / "build" / PIO_ENV / "firmware.elf"


def check_tool(name: str) -> None:
    if shutil.which(name) is None:
        print(f"ERROR: Required tool '{name}' not found in PATH.")
        sys.exit(1)


def build_elf() -> Path:
    print("[simulavr-runner] Building ELF via PlatformIO...")
    cmd = ["platformio", "run", "-e", PIO_ENV]
    proc = subprocess.run(cmd, cwd=PROJECT_ROOT)
    if proc.returncode != 0:
        print("ERROR: PlatformIO build failed.")
        sys.exit(proc.returncode)
    elf_path = PROJECT_ROOT / ELF_RELATIVE
    if not elf_path.exists():
        print(f"ERROR: ELF not found at {elf_path}")
        sys.exit(1)
    print(f"[simulavr-runner] Built ELF: {elf_path}")
    return elf_path


def launch_simulavr(elf_path: Path, f_cpu_hz: int = 8_000_000, device: str = "atmega32", gdb_port: int | None = None) -> int:
    cmd = [
        "simulavr",
        "-d", device,
        "-f", str(elf_path),
        "-F", str(f_cpu_hz),
        "-v",
    ]
    if gdb_port is not None:
        cmd += ["-g", "-p", str(gdb_port)]
    print("[simulavr-runner] Launching:", " ".join(cmd))
    # Run in foreground so logs are visible; user can Ctrl-C to stop
    return subprocess.call(cmd, cwd=PROJECT_ROOT)


def main() -> int:
    parser = argparse.ArgumentParser(description="Run simulavr for Multi-Sonicator firmware")
    parser.add_argument("--gdb", dest="gdb_port", type=int, default=None, help="Expose GDB server on this TCP port")
    parser.add_argument("--f-cpu", dest="f_cpu", type=int, default=8_000_000, help="CPU frequency in Hz (default 8MHz)")
    parser.add_argument("--device", dest="device", type=str, default="atmega32", help="Simulated device name (default atmega32)")
    args = parser.parse_args()

    check_tool("platformio")
    check_tool("simulavr")

    elf = build_elf()

    print("""
[simulavr-runner] NOTE
- UART PTY hookup not configured yet. We'll add this after validating CPU execution.
- If you need tracing, use -t <file> or -c vcd:... options (to be exposed via flags later).
- To quit, press Ctrl-C.
"""
    )

    rc = launch_simulavr(elf, f_cpu_hz=args.f_cpu, device=args.device, gdb_port=args.gdb_port)
    if rc != 0:
        print(f"simulavr exited with code {rc}")
    return rc


if __name__ == "__main__":
    raise SystemExit(main())
