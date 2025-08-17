#!/usr/bin/env python3
"""
Run SimulAVR via pysimulavr for Multi-Sonicator firmware.
- Builds the ELF with PlatformIO (development env)
- Instantiates atmega32 device via pysimulavr
- Creates a UART PTY and symlink /tmp/tty-msio
- Runs emulator in foreground until terminated
"""
from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path

from pysimulavr_emulator import Atmega32Emulator, DEFAULT_SYMLINK

PROJECT_ROOT = Path(__file__).resolve().parents[2]
PIO_ENV = "development"
ELF_RELATIVE = Path(".pio") / "build" / PIO_ENV / "firmware.elf"


def check_tool(name: str) -> None:
    if shutil.which(name) is None:
        print(f"ERROR: Required tool '{name}' not found in PATH.")
        sys.exit(1)


def build_elf() -> Path:
    print("[pysimulavr-runner] Building ELF via PlatformIO...")
    cmd = ["platformio", "run", "-e", PIO_ENV]
    proc = subprocess.run(cmd, cwd=PROJECT_ROOT)
    if proc.returncode != 0:
        print("ERROR: PlatformIO build failed.")
        sys.exit(proc.returncode)
    elf_path = PROJECT_ROOT / ELF_RELATIVE
    if not elf_path.exists():
        print(f"ERROR: ELF not found at {elf_path}")
        sys.exit(1)
    print(f"[pysimulavr-runner] Built ELF: {elf_path}")
    return elf_path


def main() -> int:
    check_tool("platformio")

    elf = build_elf()

    print("[pysimulavr-runner] Launching pysimulavr with UART PTY at", DEFAULT_SYMLINK)
    emu = Atmega32Emulator(str(elf), stable_symlink=str(DEFAULT_SYMLINK))
    emu.start_uart_pty()

    # Run until interrupted
    try:
        emu.run_forever()
    finally:
        emu.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

