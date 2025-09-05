



from dataclasses import dataclass
import logging
from typing import Optional

import glob

import serial
import serial.tools.list_ports
import time
import platform

@dataclass
class WRAPPER_PINS:
    UART_RXD = "D2"
    UART_TXD = "D3"
    STATUS_LED = "D4"
    class SONICATOR_4:
        FREQ_DIV10_4 = "D7"
        FREQ_LOCK_4 = "D8"
        OVERLOAD_4 = "A2"
        START_4 = "A3"
        RESET_4 = "A4"
        POWER_SENSE_4 = "A1"
        AMPLITUDE_ALL = "D9"
    


class HardwareInterface:
    def __init__(self, serial_port=None, baud_rate=115200):
        self.baud_rate = baud_rate
        self.serial_connection = None
        self.logger = logging.getLogger(__name__)
        self.connected = False
        self.serial_port = serial_port
        # Pin mapping verified against docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)
        self.pin_mapping = {
            'FREQ_DIV10_4': WRAPPER_PINS.SONICATOR_4.FREQ_DIV10_4,
            'FREQ_LOCK_4': WRAPPER_PINS.SONICATOR_4.FREQ_LOCK_4,
            'OVERLOAD_4': WRAPPER_PINS.SONICATOR_4.OVERLOAD_4,
            'START_4': WRAPPER_PINS.SONICATOR_4.START_4,
            'RESET_4': WRAPPER_PINS.SONICATOR_4.RESET_4,
            'POWER_SENSE_4': WRAPPER_PINS.SONICATOR_4.POWER_SENSE_4,
            'AMPLITUDE_ALL': WRAPPER_PINS.SONICATOR_4.AMPLITUDE_ALL,  # PWM
            'UART_RXD': WRAPPER_PINS.UART_RXD,        # MODBUS RTU RX
            'UART_TXD': WRAPPER_PINS.UART_TXD,        # MODBUS RTU TX
            'STATUS_LED': WRAPPER_PINS.STATUS_LED       # Status LED
        }

    def _find_macos_usb_serial_ports(self) -> list:
        patterns = [
            "/dev/tty.usbserial-*",
            "/dev/tty.usbmodem*",
            "/dev/tty.wchusbserial*",
            "/dev/cu.usbserial-*",
            "/dev/cu.usbmodem*",
            "/dev/cu.wchusbserial*",
        ]
        ports = []
        for pattern in patterns:
            ports.extend(glob.glob(pattern))
        return sorted(ports)
    
    def verify_connection(self) -> bool:

        if self.serial_connection and getattr(self.serial_connection, 'is_open', False):
            try:
                self.send_command("PING")
                self.connected = True
                return True
            except Exception:
                self.connected = False
        ports_to_try = []
        if self.serial_port:
            ports_to_try.append(self.serial_port)
        if platform.system() == "Darwin" and self.serial_port and "*" in self.serial_port:
            macos_ports = self._find_macos_usb_serial_ports()
            cu_first = sorted([p for p in macos_ports if "/dev/cu." in p]) + sorted([p for p in macos_ports if "/dev/tty." in p])
            ports_to_try.extend(cu_first)
        try:
            all_ports = serial.tools.list_ports.comports()
            for port in all_ports:
                if port.device not in ports_to_try:
                    ports_to_try.append(port.device)
        except Exception:
            pass
        if platform.system() == "Darwin" and self.serial_port and "*" not in self.serial_port:
            if self.serial_port.startswith("/dev/tty."):
                twin = self.serial_port.replace("/dev/tty.", "/dev/cu.")
                if twin not in ports_to_try:
                    ports_to_try.append(twin)
            elif self.serial_port.startswith("/dev/cu."):
                twin = self.serial_port.replace("/dev/cu.", "/dev/tty.")
                if twin not in ports_to_try:
                    ports_to_try.append(twin)
        self.logger.info(f"Candidate Arduino ports to try: {ports_to_try}")
        for port in ports_to_try:
            try:
                self.logger.info(f"Attempting connection to {port}")
                if self.serial_connection and getattr(self.serial_connection, 'is_open', False):
                    self.serial_connection.close()
                self.serial_connection = serial.Serial(
                    port=port,
                    baudrate=self.baud_rate,
                    timeout=2.0,
                    write_timeout=2.0
                )

                # TIMING FIX: Extended wait for Arduino initialization
                self.logger.debug(f"Waiting for Arduino initialization on {port}...")
                time.sleep(3.0)

                # BUFFER CLEARING FIX: More thorough startup message clearing
                start = time.time()
                startup_messages = []
                while time.time() - start < 2.0:  # Extended clearing time
                    try:
                        if self.serial_connection.in_waiting:
                            data = self.serial_connection.read(self.serial_connection.in_waiting)
                            if data:
                                startup_messages.append(data.decode('ascii', errors='ignore'))
                        else:
                            break
                    except Exception:
                        break
                    time.sleep(0.05)  # Slightly longer sleep for stability

                if startup_messages:
                    self.logger.debug(f"Cleared startup messages: {startup_messages}")

                # Reset buffers to ensure clean state
                self.serial_connection.reset_input_buffer()
                self.serial_connection.reset_output_buffer()
                self.serial_connection.write(("PING\n").encode("utf-8"))
                self.serial_connection.flush()
                line = None
                start = time.time()
                while time.time() - start < 0.6:
                    try:
                        if self.serial_connection.in_waiting:
                            line = self.serial_connection.readline()
                            break
                    except Exception:
                        break
                    time.sleep(0.01)
                response = line or b""
                if b"PONG" in response or b"OK" in response or b"READY" in response.upper():
                    self.connected = True
                    self.serial_port = port
                    self.logger.info(f"✅ Arduino Test Harness connection verified on {port}")
                    return True
                else:
                    self.serial_connection.write(("PING\n").encode("utf-8"))
                    self.serial_connection.flush()
                    start = time.time()
                    line = None
                    while time.time() - start < 0.6:
                        try:
                            if self.serial_connection.in_waiting:
                                line = self.serial_connection.readline()
                                break
                        except Exception:
                            break
                        time.sleep(0.01)
                    response = line or b""
                    if b"PONG" in response or b"OK" in response or b"READY" in response.upper():
                        self.connected = True
                        self.serial_port = port
                        self.logger.info(f"✅ Arduino Test Harness connection verified on {port}")
                        return True
            except serial.SerialException as e:
                self.logger.debug(f"Serial connection failed on {port}: {e}")
                continue
            except Exception as e:
                self.logger.debug(f"Connection error on {port}: {e}")
                continue
        self.logger.error("Failed to connect to any Arduino port")
        self.logger.error("Available ports:")
        try:
            for port in serial.tools.list_ports.comports():
                self.logger.error(f"  {port.device} - {port.description}")
        except Exception:
            pass
        return False
    # --- HIL Harness command helpers and device operations ---
    def _resolve_pin(self, pin: str) -> str:
        """Map logical pin name to harness pin name if available."""
        if not isinstance(pin, str):
            pin = str(pin)
        return self.pin_mapping.get(pin, pin)

    def _ensure_serial(self):
        """Ensure the serial port is open and ready with proper Arduino initialization."""
        if self.serial_connection and getattr(self.serial_connection, 'is_open', False):
            return self.serial_connection
        # Attempt reconnect using last known serial_port
        if self.serial_port:
            try:
                import serial, time
                self.serial_connection = serial.Serial(
                    port=self.serial_port,
                    baudrate=self.baud_rate,
                    timeout=2.0,  # Increased timeout for Arduino communication
                    write_timeout=2.0,
                )

                # TIMING FIX: Allow Arduino to initialize after reconnection
                time.sleep(1.0)  # Increased from 0.2 to 1.0 seconds

                # BUFFER CLEARING FIX: Clear any startup messages
                try:
                    if self.serial_connection.in_waiting:
                        startup_data = self.serial_connection.read(self.serial_connection.in_waiting)
                        self.logger.debug(f"Cleared startup data on reconnect: {startup_data}")
                    self.serial_connection.reset_input_buffer()
                    self.serial_connection.reset_output_buffer()
                except Exception:
                    pass

                return self.serial_connection
            except Exception as e:
                self.logger.error(f"Failed to reopen serial on {self.serial_port}: {e}")
        raise RuntimeError("Serial connection is not open")

    def cleanup(self) -> None:
        """Close the serial connection cleanly."""
        try:
            if self.serial_connection and getattr(self.serial_connection, 'is_open', False):
                self.serial_connection.close()
        except Exception:
            pass
        finally:
            self.serial_connection = None
            self.connected = False

    def send_command(self, command: str, read_timeout: float = 1.0) -> str:
        """Send a single-line ASCII command to the Arduino Test Harness and return one line of response.
        Returns empty string on timeout.
        """
        try:
            import time
            ser = self._ensure_serial()
            # Drain any residual input quickly
            try:
                if ser.in_waiting:
                    _ = ser.read(ser.in_waiting)
            except Exception:
                pass
            # Send command
            line = (command.strip() + "\n").encode("ascii", errors="ignore")
            ser.write(line)
            ser.flush()
            # Read one line with a short timeout loop
            start = time.time()
            response = b""
            while time.time() - start < max(0.05, read_timeout):
                try:
                    if ser.in_waiting:
                        response = ser.readline()
                        break
                except Exception:
                    break
                time.sleep(0.01)
            try:
                return response.decode("ascii", errors="ignore").strip()
            except Exception:
                return ""
        except Exception as e:
            self.logger.debug(f"send_command error: {e}")
            return ""

    # --- GPIO helpers ---
    def write_gpio_pin(self, pin: str, state: bool) -> bool:
        pin_name = self._resolve_pin(pin)
        cmd = f"WRITE_PIN {pin_name} {'HIGH' if state else 'LOW'}"
        resp = self.send_command(cmd)
        return bool(resp and "OK" in resp.upper())

    def read_gpio_pin(self, pin: str) -> bool:
        pin_name = self._resolve_pin(pin)
        resp = self.send_command(f"READ_PIN {pin_name}")
        # Expected format: "PIN <pin> HIGH" or "PIN <pin> LOW"
        if not resp:
            return False
        up = resp.upper()
        if "HIGH" in up:
            return True
        if "LOW" in up:
            return False
        # Fallback: treat non-empty as True
        return False

    # --- ADC helpers ---
    def read_adc_channel(self, channel: str) -> Optional[int]:
        """Read ADC channel using Arduino Test Wrapper commands"""
        # Map channel names to Arduino wrapper commands
        if channel in ["POWER_SENSE_4", "POWER_4"]:
            resp = self.send_command("READ POWER 4")
            # Expected: "OK POWER=428" format
            if resp and "POWER=" in resp:
                try:
                    power_value = resp.split("POWER=")[1].strip()
                    return int(power_value)
                except (IndexError, ValueError):
                    self.logger.debug(f"Failed to parse power ADC response: {resp}")
                    return None
        else:
            # For other channels, try generic ADC read
            ch = self._resolve_pin(channel)
            resp = self.send_command(f"READ ADC {ch}")
            # Expected: "OK ADC=<value>" format
            if resp and "ADC=" in resp:
                try:
                    adc_value = resp.split("ADC=")[1].strip()
                    return int(adc_value)
                except (IndexError, ValueError):
                    self.logger.debug(f"Failed to parse ADC response: {resp}")
                    return None
            else:
                # If generic ADC command failed, the channel may not be supported
                self.logger.debug(f"ADC channel {channel} not supported or command failed: {resp}")
                return None

        self.logger.debug(f"ADC read failed for channel {channel}")
        return None

    def adc_to_voltage(self, adc_value: int, reference_voltage: float = 5.0, resolution_bits: int = 10) -> float:
        """Convert ADC counts to voltage"""
        try:
            max_counts = (1 << resolution_bits) - 1
            return (float(adc_value) / float(max_counts)) * float(reference_voltage)
        except Exception:
            return 0.0

    def voltage_to_adc(self, voltage: float, reference_voltage: float = 5.0, resolution_bits: int = 10) -> int:
        """Convert voltage to expected ADC counts"""
        try:
            max_counts = (1 << resolution_bits) - 1
            return int((voltage / reference_voltage) * max_counts)
        except Exception:
            return 0

    def apply_voltage_to_adc_channel(self, channel: str, voltage: float) -> bool:
        """Apply voltage to ADC channel (simulation for testing)"""
        # In a real HIL setup, this would control external voltage sources
        # For now, we simulate by storing the expected voltage
        if not hasattr(self, '_simulated_voltages'):
            self._simulated_voltages = {}

        self._simulated_voltages[channel] = voltage
        self.logger.debug(f"Simulated voltage {voltage}V applied to channel {channel}")
        return True

    def read_adc_voltage(self, channel: str, reference_voltage: float = 5.0) -> Optional[float]:
        """Read ADC channel and convert to voltage"""
        adc_value = self.read_adc_channel(channel)
        if adc_value is not None:
            return self.adc_to_voltage(adc_value, reference_voltage)
        return None

    def verify_adc_voltage(self, channel: str, expected_voltage: float, tolerance: float = 0.1) -> bool:
        """Verify ADC channel reads expected voltage within tolerance"""
        actual_voltage = self.read_adc_voltage(channel)
        if actual_voltage is None:
            return False

        return abs(actual_voltage - expected_voltage) <= tolerance

    # --- MODBUS helpers via harness passthrough ---
    def modbus_read_register(self, address: int) -> Optional[int]:
        try:
            addr_hex = f"{int(address) & 0xFFFF:04X}"
            resp = self.send_command(f"MODBUS_READ {addr_hex}")
            # Harness prints: "MODBUS <addr> <value>" with hex value
            if not resp:
                return None
            parts = resp.strip().split()
            if len(parts) >= 3 and parts[0].upper() == "MODBUS":
                try:
                    return int(parts[2], 16)
                except Exception:
                    return None
            # Some harnesses may just echo the value
            try:
                return int(resp.strip(), 16)
            except Exception:
                return None
        except Exception as e:
            self.logger.debug(f"modbus_read_register error: {e}")
            return None

    def modbus_write_register(self, address: int, value: int) -> bool:
        try:
            addr_hex = f"{int(address) & 0xFFFF:04X}"
            val_hex = f"{int(value) & 0xFFFF:04X}"
            resp = self.send_command(f"MODBUS_WRITE {addr_hex} {val_hex}")
            return bool(resp and "OK" in resp.upper())
        except Exception as e:
            self.logger.debug(f"modbus_write_register error: {e}")
            return False
