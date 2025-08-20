# Minimal MODBUS RTU client helpers for simulation and HIL
# Uses pymodbus for RTU over a serial PTY. Defaults match PRD.

from __future__ import annotations

import time
from dataclasses import dataclass
from typing import Optional

try:
    from pymodbus.client import ModbusSerialClient
    try:
        # pymodbus>=3
        from pymodbus.framer.rtu_framer import ModbusRtuFramer  # type: ignore
    except Exception:  # pragma: no cover
        ModbusRtuFramer = None  # type: ignore
except Exception:  # pragma: no cover - runtime env may install later
    ModbusSerialClient = None  # type: ignore
    ModbusRtuFramer = None  # type: ignore


@dataclass
class ModbusConfig:
    port: str = "/tmp/tty-msio"  # Created by scripts/emulation/cli.py
    baudrate: int = 115200
    bytesize: int = 8
    parity: str = "N"
    stopbits: int = 1
    slave_id: int = 2  # PRD §7
    timeout: float = 0.5


class ModbusRTU:
    def __init__(self, cfg: Optional[ModbusConfig] = None) -> None:
        self.cfg = cfg or ModbusConfig()
        if ModbusSerialClient is None:
            raise RuntimeError("pymodbus not available in environment")
        # Build common kwargs that work across pymodbus versions.
        kwargs = dict(
            port=self.cfg.port,
            baudrate=self.cfg.baudrate,
            bytesize=self.cfg.bytesize,
            parity=self.cfg.parity,
            stopbits=self.cfg.stopbits,
            timeout=self.cfg.timeout,
        )
        # pymodbus v3 requires explicit framer; older versions ignore it
        if ModbusRtuFramer is not None:
            kwargs["framer"] = ModbusRtuFramer
        # Prefer non-exclusive open to allow multiple scenarios to connect
        # to the same PTY without OS-level TIOCEXCL conflicts. If the
        # constructor doesn't accept 'exclusive', fall back without it.
        try:
            self.client = ModbusSerialClient(**{**kwargs, "exclusive": False})
        except TypeError:
            self.client = ModbusSerialClient(**kwargs)
        # Best-effort default unit/slave on client object for widest compatibility
        for attr in ("device_id", "unit_id", "unit", "slave"):
            try:
                setattr(self.client, attr, self.cfg.slave_id)
            except Exception:
                pass
        if not self.client.connect():
            raise RuntimeError(f"Failed to open serial port {self.cfg.port}")

    def close(self) -> None:
        try:
            self.client.close()
        except Exception:
            pass

    @staticmethod
    def logical_to_zero_based(addr_4xxxx: int) -> int:
        # Convert logical 4xxxx to zero-based holding register index
        if addr_4xxxx < 40001:
            raise ValueError("Expected logical holding register >= 40001")
        return addr_4xxxx - 40001

    def write_holding(self, addr_4xxxx: int, value: int) -> None:
        reg = self.logical_to_zero_based(addr_4xxxx)
        # Explicitly pass unit/slave each call for widest compatibility
        try:
            # pymodbus>=3.11 uses device_id
            rr = self.client.write_register(address=reg, value=value, device_id=self.cfg.slave_id)
        except TypeError:
            try:
                # pymodbus>=3 prefers 'slave'
                rr = self.client.write_register(address=reg, value=value, slave=self.cfg.slave_id)
            except TypeError:
                try:
                    # Older API uses 'unit'
                    rr = self.client.write_register(address=reg, value=value, unit=self.cfg.slave_id)
                except TypeError:
                    # Last resort: bare call
                    rr = self.client.write_register(address=reg, value=value)
        if rr.isError():  # type: ignore
            raise RuntimeError(f"MODBUS write error at {addr_4xxxx}: {rr}")

    def read_holding(self, addr_4xxxx: int, count: int = 1) -> list[int]:
        reg = self.logical_to_zero_based(addr_4xxxx)
        try:
            rr = self.client.read_holding_registers(address=reg, count=count, device_id=self.cfg.slave_id)
        except TypeError:
            try:
                rr = self.client.read_holding_registers(address=reg, count=count, slave=self.cfg.slave_id)
            except TypeError:
                try:
                    rr = self.client.read_holding_registers(address=reg, count=count, unit=self.cfg.slave_id)
                except TypeError:
                    rr = self.client.read_holding_registers(address=reg, count=count)
        if rr.isError():  # type: ignore
            raise RuntimeError(f"MODBUS read error at {addr_4xxxx}: {rr}")
        return list(rr.registers)  # type: ignore

    def poll_until(self, fn, timeout_s: float = 0.1, interval_s: float = 0.01) -> bool:
        deadline = time.monotonic() + timeout_s
        while time.monotonic() < deadline:
            if fn():
                return True
            time.sleep(interval_s)
        return False
