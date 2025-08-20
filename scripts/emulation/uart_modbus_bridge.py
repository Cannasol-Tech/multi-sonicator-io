#!/usr/bin/env python3
"""
UART↔Modbus RTU bridge for pysimulavr PTY.
- Attaches to /tmp/tty-msio and serves a Modbus RTU slave (unit=2) using pymodbus.
- Reuses SonicatorDataBlock from rtu_server for register semantics.
- Intended as a temporary bridge until firmware implements Modbus RTU.
"""
from __future__ import annotations

import os
import threading
from typing import Optional

from pymodbus.datastore import ModbusDeviceContext, ModbusServerContext
from pymodbus.server import StartSerialServer
from pymodbus.transaction import ModbusRtuFramer

# Reuse the same DataBlock semantics as RTU fallback
from rtu_server import SonicatorDataBlock  # type: ignore

DEFAULT_PORT = "/tmp/tty-msio"


def make_context() -> ModbusServerContext:
    block = SonicatorDataBlock()
    device = ModbusDeviceContext(di=None, co=None, hr=block, ir=None)
    return ModbusServerContext(devices={2: device}, single=False)


def start_bridge(port: str = DEFAULT_PORT, baudrate: int = 115200) -> threading.Thread:
    ctx = make_context()

    def _run():
        print(f"[UART-RTU-BRIDGE] Starting on {port} @ {baudrate} 8N1 (unit=2)", flush=True)
        StartSerialServer(
            context=ctx,
            framer=ModbusRtuFramer,
            port=port,
            baudrate=baudrate,
            bytesize=8,
            parity="N",
            stopbits=1,
            timeout=1,
        )

    t = threading.Thread(target=_run, name="uart-rtu-bridge", daemon=True)
    t.start()
    return t


if __name__ == "__main__":
    # Allow manual start for debugging
    port = os.environ.get("BRIDGE_PORT", DEFAULT_PORT)
    baud = int(os.environ.get("BRIDGE_BAUD", "115200"))
    start_bridge(port, baud).join()
