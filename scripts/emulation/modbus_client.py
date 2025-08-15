#!/usr/bin/env python3
"""
Simple Modbus RTU client wrapper for emulator/HIL tests using pymodbus.

- Default serial: /tmp/tty-msio (symlink to emulator PTY; adjust as needed)
- Default baud: 115200 8N1, timeout 0.2s

Usage example:
    from modbus_client import RtuClient
    with RtuClient(port="/tmp/tty-msio", unit=2) as mb:
        mb.write_reg(0, 75)  # 40001 = 0-based address 0
        val = mb.read_reg(0)
"""
from __future__ import annotations

from dataclasses import dataclass
from typing import Optional, Sequence
import time

from pymodbus.client import ModbusSerialClient  # type: ignore


DEFAULT_PORT = "/tmp/tty-msio"
DEFAULT_BAUD = 115200
DEFAULT_UNIT = 2
DEFAULT_TIMEOUT = 0.2


@dataclass
class RtuClient:
    port: str = DEFAULT_PORT
    unit: int = DEFAULT_UNIT
    baudrate: int = DEFAULT_BAUD
    stopbits: int = 1
    bytesize: int = 8
    parity: str = "N"
    timeout: float = DEFAULT_TIMEOUT

    def __post_init__(self):
        self._client: Optional[ModbusSerialClient] = None

    def __enter__(self) -> "RtuClient":
        self.open()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def open(self) -> None:
        if self._client is not None:
            return
        self._client = ModbusSerialClient(
            method="rtu",
            port=self.port,
            baudrate=self.baudrate,
            stopbits=self.stopbits,
            bytesize=self.bytesize,
            parity=self.parity,
            timeout=self.timeout,
            retry_on_empty=True,
        )
        if not self._client.connect():
            raise RuntimeError(f"Cannot open Modbus serial port: {self.port}")

    def close(self) -> None:
        if self._client is not None:
            try:
                self._client.close()
            finally:
                self._client = None

    def _ensure(self) -> ModbusSerialClient:
        if self._client is None:
            raise RuntimeError("Client not open")
        return self._client

    # Basic helpers
    def read_reg(self, address: int) -> int:
        cli = self._ensure()
        rr = cli.read_holding_registers(address=address, count=1, unit=self.unit)
        if rr.isError():  # type: ignore[attr-defined]
            raise RuntimeError(f"Modbus error: {rr}")
        return rr.registers[0]

    def read_regs(self, address: int, count: int) -> Sequence[int]:
        cli = self._ensure()
        rr = cli.read_holding_registers(address=address, count=count, unit=self.unit)
        if rr.isError():  # type: ignore[attr-defined]
            raise RuntimeError(f"Modbus error: {rr}")
        return rr.registers

    def write_reg(self, address: int, value: int) -> None:
        cli = self._ensure()
        wr = cli.write_register(address=address, value=value, unit=self.unit)
        if wr.isError():  # type: ignore[attr-defined]
            raise RuntimeError(f"Modbus error: {wr}")

    def write_regs(self, address: int, values: Sequence[int]) -> None:
        cli = self._ensure()
        wr = cli.write_registers(address=address, values=list(values), unit=self.unit)
        if wr.isError():  # type: ignore[attr-defined]
            raise RuntimeError(f"Modbus error: {wr}")

    # Time-bounded helper (<= deadline seconds)
    def wait_reg_equals(self, address: int, expected: int, deadline_s: float = 0.1) -> bool:
        t0 = time.time()
        while time.time() - t0 < deadline_s:
            try:
                val = self.read_reg(address)
                if val == expected:
                    return True
            except Exception:
                pass
            time.sleep(0.01)
        return False
