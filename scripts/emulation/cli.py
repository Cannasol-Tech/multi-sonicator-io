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
import os

import sys as _sys
from pathlib import Path as _Path

# Ensure we can import local modules when running from project root or container
_SCRIPT_DIR = _Path(__file__).resolve().parent
_SIMAVR_DIR = _SCRIPT_DIR / "simavr"
for _p in (str(_SCRIPT_DIR), str(_SIMAVR_DIR)):
    if _p not in _sys.path:
        _sys.path.insert(0, _p)

from pysimulavr_emulator import Atmega32Emulator, DEFAULT_SYMLINK
# Allow RTU server fallback
try:
    from rtu_server import run_server as run_rtu_server
    _rtu_import_error = None
except Exception as _e:
    run_rtu_server = None  # type: ignore
    _rtu_import_error = _e

PROJECT_ROOT = Path(__file__).resolve().parents[2]
PIO_ENV = "development"
ELF_RELATIVE = Path(".pio") / "build" / PIO_ENV / "firmware.elf"


def resolve_pio_cmd() -> list[str]:
    """Resolve PlatformIO invocation.

    Order: 'pio' -> 'platformio' -> 'python3 -m platformio'.
    Returns the command list suitable for subprocess.run.
    """
    if shutil.which("pio"):
        return ["pio"]
    if shutil.which("platformio"):
        return ["platformio"]
    # Fallback to module execution
    return [sys.executable or "python3", "-m", "platformio"]


def build_elf() -> Path:
    elf_path = PROJECT_ROOT / ELF_RELATIVE
    # Fast path: use existing ELF if present
    if elf_path.exists():
        print(f"[pysimulavr-runner] ELF exists, skipping build: {elf_path}")
        return elf_path

    print("[pysimulavr-runner] Building ELF via PlatformIO...")
    cmd = resolve_pio_cmd() + ["run", "-e", PIO_ENV]
    proc = subprocess.run(cmd, cwd=PROJECT_ROOT)
    if proc.returncode != 0:
        print("ERROR: PlatformIO build failed.")
        sys.exit(proc.returncode)
    if not elf_path.exists():
        print(f"ERROR: ELF not found at {elf_path}")
        sys.exit(1)
    print(f"[pysimulavr-runner] Built ELF: {elf_path}")
    return elf_path


def main() -> int:
    mode = os.environ.get("EMU_MODE", "pysimulavr").strip().lower()
    if mode == "rtu-server":
        if run_rtu_server is None:
            print("ERROR: RTU server fallback not available:", _rtu_import_error)
            return 1
        print("[pysimulavr-runner] EMU_MODE=rtu-server; starting fallback RTU server")
        return int(run_rtu_server() or 0)

    # Bridge mode: provide a Modbus server on a null-modem peer of /tmp/tty-msio
    if os.environ.get("EMU_BRIDGE", "0").strip() == "1":
        try:
            from rtu_server import start_null_modem, SERVER_LINK  # type: ignore
            from uart_modbus_bridge import start_bridge  # type: ignore
        except Exception as e:
            print("[pysimulavr-runner] Bridge prerequisites missing:", e)
            return 1
        print("[pysimulavr-runner] EMU_BRIDGE=1; starting null-modem and UART↔Modbus RTU bridge")
        proc = start_null_modem(client_link=str(DEFAULT_SYMLINK))
        try:
            start_bridge(port=SERVER_LINK)
            # Wait forever while socat + bridge serve
            proc.wait()
        finally:
            proc.terminate()
        return 0

    # Default: run pysimulavr emulator
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

