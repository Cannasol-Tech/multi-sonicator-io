"""
HIL Serial Protocol Adapter

Purpose
- Provide a single source of truth for the ASCII command protocol used by
  the Arduino wrapper (arduino_test_wrapper.ino) and optional extender.
- Offer a small, dependency-light API for both automated tests and an interactive CLI.

Dependencies
- pyserial (install via: pip install pyserial)

This module intentionally avoids any project-specific test frameworks.
"""
from __future__ import annotations

import time
from dataclasses import dataclass
from typing import Optional, Tuple

import serial  # type: ignore


DEFAULT_BAUD = 115200
DEFAULT_TIMEOUT = 0.2  # seconds
LINE_END = "\n"


@dataclass
class HilInfo:
    version: str
    mapping_checksum: str


class HilSerial:
    """Lightweight line-based protocol client.

    Protocol (line ASCII):
      - Commands:  e.g., "SET OVERLOAD 1 1\n"
      - Responses: e.g., "RESP START 1 0\n" or "OK\n" or "ERR <msg>\n"
    """

    def __init__(self, port: str, baud: int = DEFAULT_BAUD, timeout: float = DEFAULT_TIMEOUT):
        self.port = port
        self.baud = baud
        self.timeout = timeout
        self._ser: Optional[serial.Serial] = None

    # Context manager support
    def __enter__(self) -> "HilSerial":
        self.open()
        return self

    def __exit__(self, exc_type, exc, tb) -> None:
        self.close()

    def open(self) -> None:
        if self._ser and self._ser.is_open:
            return
        self._ser = serial.Serial(self.port, self.baud, timeout=self.timeout)
        # Give the wrapper a moment after port open/reset
        time.sleep(0.15)
        self._drain()

    def close(self) -> None:
        if self._ser:
            try:
                self._ser.close()
            finally:
                self._ser = None

    def _ensure(self) -> serial.Serial:
        if not self._ser or not self._ser.is_open:
            raise RuntimeError("Serial port is not open")
        return self._ser

    def _drain(self) -> None:
        ser = self._ensure()
        try:
            ser.reset_input_buffer()
            ser.reset_output_buffer()
        except Exception:
            pass

    def _write_line(self, line: str) -> None:
        ser = self._ensure()
        ser.write((line + LINE_END).encode("ascii", errors="ignore"))

    def _read_line(self) -> Optional[str]:
        ser = self._ensure()
        raw = ser.readline()
        if not raw:
            return None
        try:
            return raw.decode("ascii", errors="ignore").strip()
        except Exception:
            return None

    # Generic command helpers
    def command(self, *parts: str, expect: Optional[str] = None) -> Optional[str]:
        self._write_line(" ".join(parts))
        if expect is None:
            return self._read_line()
        # Read until a line with the expected prefix or timeout
        t0 = time.time()
        while (time.time() - t0) < (self.timeout * 5):  # allow multiple lines
            line = self._read_line()
            if line is None:
                continue
            if line.startswith("ERR"):
                raise RuntimeError(line)
            if line.startswith(expect):
                return line
            if line in ("OK", "READY", "WRAPPER READY"):
                # Not the expected response; keep listening
                continue
        return None

    # High-level operations (mirror PRD Section 13.1)
    def set_overload(self, unit: int, value: int) -> None:
        self.command("SET", "OVERLOAD", str(unit), str(int(bool(value))))

    def set_freq_lock(self, unit: int, value: int) -> None:
        self.command("SET", "FLOCK", str(unit), str(int(bool(value))))

    def set_frequency(self, unit: int, hz: int) -> None:
        self.command("SET", "FREQ", str(unit), str(int(hz)))

    def set_power(self, unit: int, watts: float) -> None:
        self.command("SET", "POWER", str(unit), f"{watts:.2f}")

    def read_start(self, unit: int) -> Optional[int]:
        line = self.command("READ", "START", str(unit), expect="RESP START")
        if not line:
            return None
        parts = line.split()
        return int(parts[-1]) if parts and parts[-1].isdigit() else None

    def read_reset_pulse(self, unit: int) -> Optional[int]:
        line = self.command("READ", "RESET_PULSE", str(unit), expect="RESP RESET_PULSE")
        if not line:
            return None
        parts = line.split()
        return int(parts[-1]) if parts and parts[-1].isdigit() else None

    def read_amplitude_adc(self) -> Optional[int]:
        line = self.command("READ", "AMP", expect="RESP AMP")
        if not line:
            return None
        parts = line.split()
        try:
            return int(parts[-1])
        except Exception:
            return None

    def info(self) -> Optional[HilInfo]:
        line = self.command("INFO", expect="RESP INFO")
        if not line:
            return None
        parts = line.split()
        if len(parts) >= 4:
            return HilInfo(version=parts[2], mapping_checksum=parts[3])
        return None
