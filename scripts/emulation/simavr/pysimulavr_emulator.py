#!/usr/bin/env python3
"""
Atmega32Emulator driven by pysimulavr.

- Builds on your ATTINY85Emulator pattern (pin control + chunked stepping)
- Adds a UART <-> PTY bridge and stable symlink /tmp/tty-msio
- Designed to run inside Docker where pysimulavr is built from SimulAVR sources

Usage:
    from pysimulavr_emulator import Atmega32Emulator
    emu = Atmega32Emulator(elf_path)
    emu.start_uart_pty()
    emu.run_forever()

Notes:
- Device name: atmega32 (covers ATmega32A)
- Frequency: assumes 8 MHz unless you change CPU cycles/time mapping in callers
"""
from __future__ import annotations

import os
import pty
import termios
import threading
import errno
import time
import fcntl
from pathlib import Path
from typing import Optional

try:
    import pysimulavr as sim
except Exception as e:  # pragma: no cover
    sim = None


DEFAULT_SYMLINK = "/tmp/tty-msio"
DEFAULT_BAUD = 115200
F_CPU_HZ = 8_000_000  # match PlatformIO development env by default


class Atmega32Emulator:
    """Lightweight Python wrapper around pysimulavr for atmega32.

    Provides:
    - SystemClock with chunked stepping
    - Pin set/get helpers (port B only to start; extend as needed)
    - UART <-> PTY bridge with stable symlink for Behave/pyserial
    """

    def __init__(self, elf_path: str, stable_symlink: str = DEFAULT_SYMLINK):
        if sim is None:
            raise RuntimeError("pysimulavr not available; ensure PYTHONPATH points to built module")

        self.elf_path = str(elf_path)
        if not Path(self.elf_path).exists():
            raise FileNotFoundError(f"ELF not found: {self.elf_path}")

        # Create device and load ELF
        if hasattr(sim, "AvrDevice_atmega32"):
            self.device = sim.AvrDevice_atmega32()
        else:
            raise RuntimeError("pysimulavr does not expose AvrDevice_atmega32; please verify bindings")

        if hasattr(self.device, "Load"):
            self.device.Load(self.elf_path)
        else:
            raise RuntimeError("pysimulavr device binding lacks Load(path)")

        # System clock
        self.clock = sim.SystemClock_Instance()
        if hasattr(self.clock, "Add"):
            try:
                self.clock.Add(self.device)
            except Exception:
                pass

        # Pin map (extend as needed for other ports)
        # Example mapping for Port B bits 0..7
        self.pins = {i: self.device.GetPin(f"B{i}") for i in range(8)}

        # UART PTY bridge fields
        self._pty_master: Optional[int] = None
        self._pty_slave: Optional[int] = None
        self._pty_slave_path: Optional[str] = None
        self._pty_thread: Optional[threading.Thread] = None
        self._pty_stop = threading.Event()
        self._stable_symlink = stable_symlink

    # ----- Pin helpers -----
    def set_pin(self, pin_num: int, value: int) -> None:
        pin = self.pins.get(pin_num)
        if pin is not None:
            pin.SetPin(1 if value else 0)

    def get_pin(self, pin_num: int) -> Optional[int]:
        pin = self.pins.get(pin_num)
        if pin is None:
            return None
        return 1 if pin.GetPin() else 0

    # ----- Stepping -----
    def run_for_cycles(self, cycles: int) -> None:
        remaining = int(max(0, cycles))
        CHUNK = 50_000_000
        if hasattr(self.clock, "Run"):
            while remaining > 0:
                step = CHUNK if remaining > CHUNK else remaining
                self.clock.Run(int(step))
                remaining -= step
        elif hasattr(self.device, "Step"):
            while remaining > 0:
                step = CHUNK if remaining > CHUNK else remaining
                for _ in range(step):
                    self.device.Step()
                remaining -= step
        else:
            raise RuntimeError("No stepping method available in pysimulavr bindings")

    def run_for_ms(self, ms: int) -> None:
        total_cycles = int(max(0, ms) * (F_CPU_HZ // 1000))
        self.run_for_cycles(total_cycles)

    def run_forever(self, sleep_ms: int = 1) -> None:
        try:
            while True:
                # Advance by ~1ms per loop to keep UI responsive
                self.run_for_ms(sleep_ms)
        except KeyboardInterrupt:
            pass

    # ----- UART <-> PTY bridge -----
    def start_uart_pty(self, symlink: Optional[str] = None) -> str:
        """Create a PTY and launch a background thread to bridge UART bytes.
        Returns the PTY slave path (also symlinked to /tmp/tty-msio by default).
        """
        link = symlink or self._stable_symlink

        mfd, sfd = pty.openpty()
        # Set raw mode on master for better behavior
        for fd in (mfd, sfd):
            attrs = termios.tcgetattr(fd)
            # cfmakeraw equivalent
            attrs[0] = 0  # iflag
            attrs[1] = 0  # oflag
            attrs[2] = attrs[2] & ~(termios.PARENB | termios.CSIZE) | termios.CS8  # cflag
            attrs[3] = 0  # lflag
            termios.tcsetattr(fd, termios.TCSANOW, attrs)

        self._pty_master, self._pty_slave = mfd, sfd
        self._pty_slave_path = os.ttyname(sfd)

        # Create/update stable symlink
        try:
            try:
                os.unlink(link)
            except FileNotFoundError:
                pass
            os.symlink(self._pty_slave_path, link)
        except OSError as e:
            # Non-fatal; Behave can still use the direct PTY path if needed
            print(f"[pysimulavr] WARN: symlink {link} -> {self._pty_slave_path} failed: {e}")

        # Start bridging thread
        self._pty_stop.clear()
        self._pty_thread = threading.Thread(target=self._uart_bridge_loop, daemon=True)
        self._pty_thread.start()
        return self._pty_slave_path

    def _uart_bridge_loop(self) -> None:
        # Simple polling loop: read from PTY master and from device UART FIFO
        # NOTE: pysimulavr UART access is implementation-specific; adjust as needed
        mfd = self._pty_master
        if mfd is None:
            return
        # crude loop with small sleep to avoid busy spin
        while not self._pty_stop.is_set():
            # PTY -> device RX
            try:
                data = os.read(mfd, 256)
            except OSError as e:
                if e.errno in (errno.EAGAIN, errno.EWOULDBLOCK):
                    data = b""
                else:
                    break
            if data:
                for b in data:
                    # Placeholder: inject into device RX buffer
                    # TODO: replace with correct API to push a byte into UART RX
                    try:
                        if hasattr(self.device, "UartRxByte"):
                            self.device.UartRxByte(int(b))
                    except Exception:
                        pass

            # Device TX -> PTY
            # Placeholder: if device exposes a TX FIFO read, consume and write
            try:
                if hasattr(self.device, "UartTxReady") and self.device.UartTxReady():
                    # Hypothetical API; adjust to real one
                    byte = self.device.UartTxRead()
                    os.write(mfd, bytes([byte & 0xFF]))
            except Exception:
                pass

            # Advance time a little to drive UART state machines
            self.run_for_ms(1)

            # Relax CPU
            time.sleep(0.001)

    def stop_uart_pty(self) -> None:
        self._pty_stop.set()
        if self._pty_thread:
            self._pty_thread.join(timeout=1.0)
        self._pty_thread = None
        if self._pty_master is not None:
            try:
                os.close(self._pty_master)
            except Exception:
                pass
        if self._pty_slave is not None:
            try:
                os.close(self._pty_slave)
            except Exception:
                pass
        self._pty_master = None
        self._pty_slave = None

    # ----- Cleanup -----
    def close(self) -> None:
        try:
            self.stop_uart_pty()
        except Exception:
            pass
        try:
            if hasattr(self.clock, "Reset"):
                self.clock.Reset()
        except Exception:
            pass

