



import logging

class HardwareInterface:
    def __init__(self, serial_port=None, baud_rate=115200):
        self.baud_rate = baud_rate
        self.serial_connection = None
        self.logger = logging.getLogger(__name__)
        self.connected = False
        self.serial_port = serial_port
        # Pin mapping verified against docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)
        self.pin_mapping = {
            'FREQ_DIV10_4': 'D7',
            'FREQ_LOCK_4': 'D8',
            'OVERLOAD_4': 'A2',
            'START_4': 'A3',
            'RESET_4': 'A4',
            'POWER_SENSE_4': 'A1',
            'AMPLITUDE_ALL': 'D9',  # PWM
            'UART_RXD': 'D2',        # MODBUS RTU RX
            'UART_TXD': 'D3',        # MODBUS RTU TX
            'STATUS_LED': 'D4'       # Status LED
        }

    def _find_macos_usb_serial_ports(self) -> list:
        import glob
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
        import serial
        import serial.tools.list_ports
        import time
        import platform
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
                time.sleep(2)
                start = time.time()
                while time.time() - start < 1.0:
                    try:
                        if self.serial_connection.in_waiting:
                            _ = self.serial_connection.read(self.serial_connection.in_waiting)
                        else:
                            break
                    except Exception:
                        break
                    time.sleep(0.02)
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
    # ...existing code...
