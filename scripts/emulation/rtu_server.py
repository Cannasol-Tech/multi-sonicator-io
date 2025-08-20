#!/usr/bin/env python3
"""
Minimal RTU server fallback to unblock Behave tests when pysimulavr is unavailable.
- Creates a null-modem PTY pair via socat
- Symlinks client side to /tmp/tty-msio
- Runs a pymodbus RTU server bound to the server side
- Implements only the registers needed by smoke/start_stop scenarios

Registers (logical 4xxxx):
- 40005..40008: control start/stop per unit (1..4). Write-only in tests.
- 40021..40024: status bitfield per unit; bit0 reflects running state
- 40035: Active Sonicator Count (number of running units)
- 40036: Active Sonicator Mask (bit i set when unit i running)
"""
from __future__ import annotations

import os
import signal
import subprocess
import sys
import threading
import time
from pathlib import Path
from typing import Dict
import threading

from pymodbus.server import StartSerialServer
from pymodbus.datastore import ModbusDeviceContext, ModbusServerContext, ModbusSparseDataBlock

DEFAULT_CLIENT_SYMLINK = "/tmp/tty-msio"
SERVER_LINK = "/tmp/tty-msio-srv"


class SonicatorDataBlock(ModbusSparseDataBlock):
    def __init__(self) -> None:
        # Internal state
        self.running: Dict[int, int] = {1: 0, 2: 0, 3: 0, 4: 0}
        self._lock = threading.Lock()
        # Pre-populate required addresses (0-based indices)
        store = {}
        # control start/stop 40005..40008 -> indices 5..8 (accept writes)
        for idx in range(5, 9):
            store[idx] = 0
        # status bitfields 40021..40024 -> indices 21..24
        for unit in range(1, 5):
            store[21 + (unit - 1)] = 0
        # count 40035 -> idx 35; mask 40036 -> idx 36
        store[35] = 0
        store[36] = 0
        super().__init__(store)

    def setValues(self, address: int, values) -> None:  # noqa: N802 pymodbus API
        # Handle writes to control registers 40005..40008 (idx 4..7)
        with self._lock:
            if 5 <= address <= 8:
                # Expect single value writes from tests
                try:
                    val = int(values if isinstance(values, int) else list(values)[0])
                except Exception:
                    val = int(values[0]) if hasattr(values, "__getitem__") else int(values)
                unit = (address - 5) + 1
                self.running[unit] = 1 if val else 0
                print(f"[RTU-SRV] WRITE addr={address} (reg={40001+address}) unit={unit} val={val} running={self.running}", flush=True)
                self._recompute()
            else:
                super().setValues(address, values)

    def _recompute(self) -> None:
        # Update status bitfields 40021..40024 (idx 20..23), bit0 is running
        for unit in range(1, 5):
            bf = 1 if self.running[unit] else 0
            super().setValues(21 + (unit - 1), [bf])
        # Update count/mask
        count = sum(self.running.values())
        mask = 0
        for unit in range(1, 5):
            if self.running[unit]:
                mask |= (1 << (unit - 1))
        super().setValues(35, [count])
        super().setValues(36, [mask])

    # Make reads permissive: if a requested address is not preset, treat as 0
    def getValues(self, address: int, count: int = 1):  # noqa: N802 pymodbus API
        with self._lock:
            values = []
            for off in range(count):
                idx = address + off
                # Derived registers
                if 21 <= idx <= 24:
                    unit = (idx - 21) + 1
                    v = 1 if self.running.get(unit, 0) else 0
                    print(f"[RTU-SRV] READ status idx={idx} reg={40001+idx} unit={unit} -> {v}", flush=True)
                    values.append(v)
                    continue
                if idx == 35:
                    v = sum(self.running.values())
                    print(f"[RTU-SRV] READ count idx=35 reg=40036 -> {v} running={self.running}", flush=True)
                    values.append(v)
                    continue
                if idx == 36:
                    mask = 0
                    for u in range(1, 5):
                        if self.running.get(u, 0):
                            mask |= (1 << (u - 1))
                    print(f"[RTU-SRV] READ mask idx=36 reg=40037 -> {mask} running={self.running}", flush=True)
                    values.append(mask)
                    continue
                # Stored registers
                try:
                    values.append(super().getValues(idx, 1)[0])
                except Exception:
                    values.append(0)
            return values

    # Validate all requested ranges within a broad window we care about
    def validate(self, address: int, count: int = 1) -> bool:  # noqa: N802
        # Accept typical ranges used by tests
        end = address + max(1, count) - 1
        # Allow 0..80 to be safe for smoke tests after +1 shift
        return 0 <= address <= 80 and 0 <= end <= 80


def make_context() -> ModbusServerContext:
    block = SonicatorDataBlock()
    # ModbusDeviceContext replaces ModbusSlaveContext in pymodbus>=3.11
    device = ModbusDeviceContext(di=None, co=None, hr=block, ir=None)
    # Bind explicitly to unit/slave 2 to match client default
    try:
        # pymodbus>=3 uses 'devices'
        return ModbusServerContext(devices={2: device}, single=False)
    except TypeError:  # pragma: no cover
        # older pymodbus uses 'slaves'
        return ModbusServerContext(slaves={2: device}, single=False)


def start_null_modem(client_link: str = DEFAULT_CLIENT_SYMLINK, server_link: str = SERVER_LINK) -> subprocess.Popen:
    # Remove stale links
    for p in (client_link, server_link):
        try:
            os.unlink(p)
        except FileNotFoundError:
            pass
    cmd = [
        "socat",
        "-d", "-d",
        f"pty,raw,echo=0,link={client_link}",
        f"pty,raw,echo=0,link={server_link}",
    ]
    return subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)


def run_server() -> int:
    print("[rtu-server] starting null-modem PTYs and RTU server...")
    soc = start_null_modem()
    # Wait a bit for links to be created
    deadline = time.time() + 5.0
    while time.time() < deadline and not (os.path.exists(DEFAULT_CLIENT_SYMLINK) and os.path.exists(SERVER_LINK)):
        time.sleep(0.05)
    if not os.path.exists(DEFAULT_CLIENT_SYMLINK) or not os.path.exists(SERVER_LINK):
        try:
            out = soc.stdout.read().decode("utf-8", errors="ignore") if soc.stdout else ""
        except Exception:
            out = ""
        print("[rtu-server] ERROR: PTY links not created\n", out)
        return 1

    ctx = make_context()

    # Ensure clean shutdown on SIGTERM
    stop_evt = threading.Event()

    def _handle(sig, frm):  # noqa: ARG001
        stop_evt.set()

    signal.signal(signal.SIGTERM, _handle)

    # Start server in background thread so we can watch for stop
    t = threading.Thread(
        target=StartSerialServer,
        kwargs=dict(context=ctx, port=SERVER_LINK, baudrate=115200, bytesize=8, parity="N", stopbits=1, defer_start=False),
        daemon=True,
    )
    t.start()
    print("[rtu-server] ready on", DEFAULT_CLIENT_SYMLINK)
    try:
        while not stop_evt.is_set():
            time.sleep(0.1)
    finally:
        try:
            soc.terminate()
        except Exception:
            pass
    return 0


if __name__ == "__main__":
    raise SystemExit(run_server())
