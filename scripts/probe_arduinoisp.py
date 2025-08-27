#!/usr/bin/env python3
"""
Probe whether an Arduino is running the ArduinoISP sketch without requiring a target MCU.

Method: STK500v1 sign-on. Send '1' + 0x20 and expect STK_INSYNC (0x14) + 'AVR ISP' + STK_OK (0x10).
Baud rate: 19200 (ArduinoISP default).

Usage:
  python scripts/probe_arduinoisp.py /dev/cu.usbmodemXXXX
Exit codes:
  0 = ArduinoISP detected
  1 = Not detected / error
"""
import sys
import time

try:
    import serial  # type: ignore
except Exception:
    print("pyserial not installed. Install with: pip install pyserial", file=sys.stderr)
    sys.exit(1)

if len(sys.argv) < 2:
    print("Usage: probe_arduinoisp.py <serial_port>", file=sys.stderr)
    sys.exit(1)

port = sys.argv[1]

try:
    ser = serial.Serial(port, 19200, timeout=1)
    # Give time for boards that auto-reset on open
    time.sleep(1.0)

    # Send STK500v1 sign-on: '1' + CRC_EOP (0x20)
    ser.write(b'1\x20')
    ser.flush()
    time.sleep(0.2)

    resp = ser.read(64)
    ser.close()

    # Expect STK_INSYNC (0x14) ... 'AVR ISP' ... STK_OK (0x10)
    if b"AVR ISP" in resp or (len(resp) >= 2 and resp[0] == 0x14 and resp[-1] == 0x10):
        print("OK: ArduinoISP sign-on detected")
        sys.exit(0)
    else:
        print(f"NO: Unexpected response: {resp!r}")
        sys.exit(1)
except Exception as e:
    print(f"Error: {e}", file=sys.stderr)
    sys.exit(1)

