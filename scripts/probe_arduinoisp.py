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
    # Try multiple times with different approaches
    for attempt in range(3):
        try:
            ser = serial.Serial(port, 19200, timeout=2)
            # Give time for boards that auto-reset on open
            time.sleep(2.0 if attempt == 0 else 1.0)

            # Clear any existing data
            ser.reset_input_buffer()
            ser.reset_output_buffer()

            # Send STK500v1 sign-on: '1' + CRC_EOP (0x20)
            ser.write(b'1\x20')
            ser.flush()
            time.sleep(0.5)

            resp = ser.read(64)
            ser.close()

            # Expect STK_INSYNC (0x14) ... 'AVR ISP' ... STK_OK (0x10)
            if b"AVR ISP" in resp or (len(resp) >= 2 and resp[0] == 0x14 and resp[-1] == 0x10):
                print("OK: ArduinoISP sign-on detected")
                sys.exit(0)
            elif len(resp) > 0:
                print(f"Attempt {attempt + 1}: Unexpected response: {resp!r}")
            else:
                print(f"Attempt {attempt + 1}: No response")

            if attempt < 2:
                time.sleep(1.0)

        except serial.SerialException as e:
            print(f"Attempt {attempt + 1}: Serial error: {e}")
            if attempt < 2:
                time.sleep(1.0)
            else:
                raise

    print("NO: ArduinoISP not detected after multiple attempts")
    sys.exit(1)

except Exception as e:
    print(f"Error: {e}", file=sys.stderr)
    sys.exit(1)

