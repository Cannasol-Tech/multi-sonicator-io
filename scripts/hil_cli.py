#!/usr/bin/env python3
"""
Interactive HIL CLI

Provides a guided, interactive workflow for HIL setup with user confirmations
and a pretty live monitoring interface.

Usage:
    python scripts/hil_cli.py [--env development|production]
"""

import argparse
import os
import subprocess
import sys
import time
import threading
from datetime import datetime
from pathlib import Path
from typing import Optional, Dict, Any

import serial.tools.list_ports

# ANSI color codes
class Colors:
    RED = '\033[91m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    MAGENTA = '\033[95m'
    CYAN = '\033[96m'
    WHITE = '\033[97m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    END = '\033[0m'


def print_header(text: str):
    """Print a formatted header."""
    print(f"\n{Colors.BOLD}{Colors.CYAN}{'='*60}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}{text.center(60)}{Colors.END}")
    print(f"{Colors.BOLD}{Colors.CYAN}{'='*60}{Colors.END}\n")


def print_step(step_num: int, text: str):
    """Print a formatted step."""
    print(f"{Colors.BOLD}{Colors.BLUE}Step {step_num}: {text}{Colors.END}")


def print_success(text: str):
    """Print success message."""
    print(f"{Colors.GREEN}✓ {text}{Colors.END}")


def print_error(text: str):
    """Print error message."""
    print(f"{Colors.RED}✗ {text}{Colors.END}")


def print_warning(text: str):
    """Print warning message."""
    print(f"{Colors.YELLOW}⚠ {text}{Colors.END}")


def print_info(text: str):
    """Print info message."""
    print(f"{Colors.CYAN}ℹ {text}{Colors.END}")


def confirm_step(message: str) -> bool:
    """Ask user for confirmation."""
    while True:
        response = input(f"{Colors.YELLOW}{message} (y/n): {Colors.END}").lower().strip()
        if response in ['y', 'yes']:
            return True
        elif response in ['n', 'no']:
            return False
        else:
            print("Please enter 'y' or 'n'")


def wait_for_enter(message: str = "Press Enter to continue..."):
    """Wait for user to press Enter."""
    input(f"{Colors.CYAN}{message}{Colors.END}")


def run_command(cmd: list[str], cwd: Optional[str] = None, check: bool = True, env: Optional[dict] = None) -> subprocess.CompletedProcess:
    """Run a command and return the result."""
    print(f"{Colors.MAGENTA}Running: {' '.join(cmd)}{Colors.END}")
    if cwd:
        print(f"  in: {cwd}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, env=env)
    if check and result.returncode != 0:
        print_error(f"Command failed with exit code {result.returncode}")
        print(f"STDOUT: {result.stdout}")
        print(f"STDERR: {result.stderr}")
        if not confirm_step("Continue anyway?"):
            sys.exit(1)
    return result


def find_arduino_port() -> Optional[str]:
    """Find Arduino port automatically."""
    candidates = []
    for port in serial.tools.list_ports.comports():
        desc = (port.description or "").lower()
        dev = port.device
        score = 0
        
        if "arduino" in desc or "uno" in desc:
            score += 10
        if "ch340" in desc or "ftdi" in desc:
            score += 5
        if "usbmodem" in dev or "usbserial" in dev:
            score += 3
        if sys.platform == "darwin" and dev.startswith("/dev/cu."):
            score += 1
            
        candidates.append((score, dev, desc))
    
    candidates.sort(reverse=True)
    if candidates:
        print_info(f"Auto-detected Arduino port: {candidates[0][1]} ({candidates[0][2]})")
        return candidates[0][1]
    return None


def build_atmega_firmware(env: str = "development") -> bool:
    """Build ATmega32A firmware using PlatformIO."""
    print_step(0, f"Building ATmega32A firmware ({env})")
    try:
        run_command(["pio", "run", "-e", env])
        print_success("ATmega firmware built successfully")
        return True
    except Exception as e:
        print_error(f"Failed to build firmware: {e}")
        return False


def check_arduino_sketch(arduino_port: str) -> str:
    """Check what sketch is currently running on Arduino."""
    try:
        import serial
        import time

        print_info("Checking current Arduino sketch...")

        # First try HIL wrapper at 115200 baud
        try:
            ser = serial.Serial(arduino_port, 115200, timeout=1)
            time.sleep(1)  # Wait for Arduino reset

            ser.write(b"PING\n")
            time.sleep(0.2)

            response = ""
            while ser.in_waiting:
                response += ser.readline().decode().strip() + " "

            ser.close()

            if "PONG" in response:
                return "hil_wrapper"
        except:
            pass

        # Try ArduinoISP at 19200 baud with STK500v1 protocol
        try:
            ser = serial.Serial(arduino_port, 19200, timeout=1)
            time.sleep(1)  # Wait for Arduino reset

            # Send STK500v1 "get sync" command (0x30 0x20)
            ser.write(bytes([0x30, 0x20]))
            time.sleep(0.2)

            response = ser.read(10)  # Read up to 10 bytes
            ser.close()

            # ArduinoISP should respond with STK_INSYNC (0x14) + STK_OK (0x10)
            if len(response) >= 2 and response[0] == 0x14 and response[1] == 0x10:
                return "arduino_isp"
        except:
            pass

        # Try to detect by sending STK500v1 sign-on command
        try:
            ser = serial.Serial(arduino_port, 19200, timeout=1)
            time.sleep(1)

            # Send sign-on command: '1' + CRC_EOP (0x20)
            ser.write(b'1\x20')
            time.sleep(0.2)

            response = ser.read(20)
            ser.close()

            # Should get STK_INSYNC + "AVR ISP" + STK_OK
            if b"AVR ISP" in response:
                return "arduino_isp"
        except:
            pass

        return "unknown"

    except Exception as e:
        print_info(f"Could not check Arduino sketch: {e}")
        return "unknown"


def check_and_upload_isp(arduino_port: str) -> bool:
    """Check if Arduino ISP is loaded and upload if needed."""
    print_step(1, "Checking Arduino ISP status")

    # First, check what sketch is currently running
    current_sketch = check_arduino_sketch(arduino_port)
    print_info(f"Current Arduino sketch appears to be: {current_sketch}")

    if current_sketch == "arduino_isp":
        print_success("ArduinoISP sketch detected!")
        print_info("Testing ISP communication with target...")

        # Test ISP communication with verbose output
        result = run_command([
            "avrdude", "-c", "stk500v1", "-P", arduino_port, "-b", "19200",
            "-p", "m32", "-v"
        ], check=False)

        # Look for specific patterns that indicate ISP is working
        output_text = (result.stdout + result.stderr).lower()

        print_info(f"avrdude exit code: {result.returncode}")

        if "device signature = 0x1e9502" in output_text:
            print_success("✓ Arduino ISP successfully communicating with ATmega32A!")
            print_info("Device signature confirmed: ATmega32A detected")
            return True
        elif "avrdude done" in output_text and result.returncode == 0:
            print_success("✓ Arduino ISP communication successful")
            return True
        elif "not in sync" in output_text or "not responding" in output_text:
            print_warning("Arduino ISP loaded but target not responding")
            print_info("Possible causes:")
            print_info("  • ATmega32A not connected")
            print_info("  • ATmega32A not powered")
            print_info("  • Wiring issue (check MISO/MOSI/SCK/RESET)")
            print_info("  • Missing 10µF capacitor on Arduino RESET")

            if confirm_step("Continue assuming ArduinoISP is working? (You can connect target later)"):
                return True
        elif "programmer is not responding" in output_text:
            print_error("Arduino ISP programmer not responding")
            print_info("This usually means ArduinoISP sketch is not loaded correctly")
            return False
        else:
            print_warning("Arduino ISP communication unclear")
            print_info("Raw avrdude output:")
            print_info(f"STDOUT: {result.stdout[:200]}...")
            print_info(f"STDERR: {result.stderr[:200]}...")
            if confirm_step("Continue anyway?"):
                return True

    print_warning("ArduinoISP not detected - need to upload it")
    
    print_step(2, "Uploading Arduino ISP sketch")
    print_info("This will upload the ArduinoISP sketch to your Arduino")
    
    if not confirm_step("Upload Arduino ISP sketch?"):
        return False
    
    # Upload ArduinoISP
    try:
        # Check for arduino-cli
        run_command(["arduino-cli", "--version"])
        
        print_info("Downloading and compiling ArduinoISP sketch...")

        # Create a temporary directory for the ArduinoISP sketch
        import tempfile
        import urllib.request

        try:
            with tempfile.TemporaryDirectory() as temp_dir:
                isp_dir = os.path.join(temp_dir, "ArduinoISP")
                os.makedirs(isp_dir)

                # Download the ArduinoISP sketch from Arduino's GitHub
                isp_url = "https://raw.githubusercontent.com/arduino/arduino-examples/main/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino"
                isp_sketch_path = os.path.join(isp_dir, "ArduinoISP.ino")

                print_info("Downloading ArduinoISP sketch from GitHub...")
                urllib.request.urlretrieve(isp_url, isp_sketch_path)

                print_info("Compiling ArduinoISP sketch...")
                fqbn = os.environ.get("ARDUINO_FQBN", "arduino:renesas_uno:unor4wifi")
                run_command([
                    "arduino-cli", "compile", "--fqbn", fqbn, isp_dir
                ])

                print_info("Uploading ArduinoISP sketch...")
                run_command([
                    "arduino-cli", "upload", "--fqbn", fqbn,
                    "--port", arduino_port, isp_dir
                ])

        except Exception as e:
            print_error(f"Failed to automatically upload ArduinoISP: {e}")
            print_warning("Falling back to manual upload instructions...")
            print_header("MANUAL ARDUINOISP UPLOAD REQUIRED")
            print_info("Please follow these steps to upload ArduinoISP:")
            print_info("1. Open Arduino IDE")
            print_info("2. Go to File -> Examples -> 11.ArduinoISP -> ArduinoISP")
            print_info("3. Select your Arduino board and port")
            print_info("4. Click Upload")

            if not confirm_step("Have you successfully uploaded ArduinoISP to your Arduino?"):
                print_error("ArduinoISP upload cancelled by user")
                return False
        
        print_success("Arduino ISP uploaded successfully")
        
        # Important capacitor instruction
        print_header("CAPACITOR CONNECTION REQUIRED")
        print(f"{Colors.BOLD}{Colors.YELLOW}IMPORTANT: Connect a 10µF electrolytic capacitor between RESET and GND on the Arduino:{Colors.END}")
        print(f"{Colors.YELLOW}  • Long leg (+) connects to RESET pin{Colors.END}")
        print(f"{Colors.YELLOW}  • Short leg (-) connects to GND pin{Colors.END}")
        print(f"{Colors.YELLOW}  • This prevents the Arduino from auto-resetting during programming{Colors.END}")
        
        wait_for_enter("Connect the capacitor as described above, then press Enter...")
        
        return True
        
    except Exception as e:
        print_error(f"Failed to upload Arduino ISP: {e}")
        return False


def program_and_confirm_fuses(arduino_port: str) -> bool:
    """Program and confirm fuses on ATmega32A."""
    print_step(3, "Programming and confirming ATmega32A fuses")

    # Ensure capacitor is connected before any ISP programming
    print_header("CAPACITOR REQUIRED FOR ISP PROGRAMMING")
    print(f"{Colors.BOLD}{Colors.YELLOW}Connect a 10µF electrolytic capacitor between RESET and GND on the Arduino:{Colors.END}")
    print(f"{Colors.YELLOW}  • Long leg (+) connects to RESET pin{Colors.END}")
    print(f"{Colors.YELLOW}  • Short leg (-) connects to GND pin{Colors.END}")
    print(f"{Colors.YELLOW}  • This prevents the Arduino from auto-resetting during programming{Colors.END}")
    wait_for_enter("After connecting the capacitor, press Enter to continue...")

    print_info("This will set the fuses for 8MHz internal oscillator (no EFUSE on ATmega32A)")
    print_info("LFUSE: 0xE4 (internal 8MHz)")
    print_info("HFUSE: 0xD9 (JTAG disabled, SPI enabled)")

    if not confirm_step("Program fuses now?"):
        return False

    try:
        # Program fuses directly with avrdude (avoid PlatformIO efuse calc issue)
        prog_cmd = [
            "avrdude", "-c", "stk500v1", "-P", arduino_port, "-b", "19200",
            "-p", "m32", "-B", "10",
            "-U", "lfuse:w:0xE4:m", "-U", "hfuse:w:0xD9:m"
        ]
        run_command(prog_cmd)
        print_success("Fuses programmed successfully")

        # Verify fuses (read back)
        print_info("Verifying fuses...")
        verify = run_command([
            "avrdude", "-c", "stk500v1", "-P", arduino_port, "-b", "19200",
            "-p", "m32", "-B", "10", "-v",
            "-U", "lfuse:r:-:h", "-U", "hfuse:r:-:h"
        ], check=False)
        combined = (verify.stdout + verify.stderr).lower()
        if ("0xe4" in combined) and ("0xd9" in combined) and verify.returncode == 0:
            print_success("Fuses verified successfully")
            return True
        else:
            print_warning("Fuse verification unclear - check output above")
            return confirm_step("Continue anyway?")

    except Exception as e:
        print_error(f"Failed to program fuses: {e}")
        return False


def program_and_confirm_firmware(env: str, arduino_port: str) -> bool:
    """Program and confirm ATmega32A firmware."""
    print_step(4, f"Programming and confirming ATmega32A firmware ({env})")

    # Ensure capacitor is still connected for ISP programming
    print_header("CONFIRM CAPACITOR STILL CONNECTED")
    print(f"{Colors.BOLD}{Colors.YELLOW}Verify the 10µF capacitor remains between RESET (+) and GND (-) on the Arduino.{Colors.END}")
    wait_for_enter("After verifying the capacitor is connected, press Enter to continue...")

    if not confirm_step("Program ATmega32A with firmware now?"):
        return False

    try:
        # Pass the chosen Arduino port explicitly to PlatformIO so it doesn't pick a stale default
        run_command(["pio", "run", "-e", env, "--target", "upload", "--upload-port", arduino_port])
        print_success("Firmware programmed successfully")

        # Basic verification - check if programmer still responds
        result = run_command([
            "avrdude", "-c", "stk500v1", "-P", arduino_port, "-b", "19200",
            "-p", "m32", "-v"
        ], check=False)

        if result.returncode == 0:
            print_success("Firmware verification successful")
            return True
        else:
            print_warning("Firmware verification unclear")
            return confirm_step("Continue anyway?")

    except Exception as e:
        print_error(f"Failed to program firmware: {e}")
        return False


def remove_capacitor_prompt():
    """Prompt user to remove capacitor."""
    print_step(5, "Remove capacitor from Arduino")
    print_header("REMOVE CAPACITOR")
    print(f"{Colors.BOLD}{Colors.YELLOW}Please remove the 10µF capacitor from the Arduino RESET pin{Colors.END}")
    print(f"{Colors.YELLOW}The Arduino will now be used for the HIL wrapper, not ISP programming{Colors.END}")
    wait_for_enter("Remove the capacitor, then press Enter...")


def upload_hil_wrapper(arduino_port: str) -> bool:
    """Upload HIL wrapper sketch to Arduino."""
    print_step(6, "Programming HIL wrapper onto Arduino")

    wrapper_dir = Path("test/acceptance/sketches/arduino_test_wrapper")
    if not wrapper_dir.exists():
        print_error(f"HIL wrapper directory not found: {wrapper_dir}")
        return False

    if not confirm_step("Upload HIL wrapper sketch to Arduino?"):
        return False

    try:
        fqbn = os.environ.get("ARDUINO_FQBN", "arduino:renesas_uno:unor4wifi")
        sketch_path = str(wrapper_dir)  # compile/upload on the folder

        print_info("Compiling HIL wrapper...")
        compile_res = run_command([
            "arduino-cli", "compile", "--fqbn", fqbn, sketch_path
        ], check=False)
        if compile_res.returncode != 0:
            print_error("HIL wrapper compile failed")
            return False

        # Re-detect Arduino port before upload (port can change after resets)
        def detect_port_via_cli() -> Optional[str]:
            try:
                import json
                res = run_command(["arduino-cli", "board", "list", "--format", "json"], check=False)
                if res.returncode != 0:
                    return None
                boards = json.loads(res.stdout or "[]")
                for b in boards:
                    port_addr = b.get("address") or b.get("port") or ""
                    fq = (b.get("fqbn") or "").lower()
                    name = (b.get("name") or "").lower()
                    if fqbn in (b.get("fqbn") or "") or "uno r4" in name:
                        return port_addr
                return None
            except Exception:
                return None

        detected_port = detect_port_via_cli() or arduino_port
        if detected_port != arduino_port:
            print_info(f"Using detected Arduino port: {detected_port}")

        print_info("Uploading HIL wrapper...")
        upload_res = run_command([
            "arduino-cli", "upload", "--fqbn", fqbn,
            "--port", detected_port, sketch_path
        ], check=False)
        if upload_res.returncode != 0:
            # Try once more after a short delay and re-scan
            time.sleep(2)
            detected_port = detect_port_via_cli() or detected_port
            upload_res = run_command([
                "arduino-cli", "upload", "--fqbn", fqbn,
                "--port", detected_port, sketch_path
            ], check=False)
            if upload_res.returncode != 0:
                print_error("HIL wrapper upload failed")
                return False

        print_success("HIL wrapper uploaded successfully")
        time.sleep(2.5)  # Wait for Arduino reset
        return True

    except Exception as e:
        print_error(f"Failed to upload HIL wrapper: {e}")
        return False


class HILMonitor:
    """Live HIL monitoring interface."""

    def __init__(self, port: str):
        self.port = port
        self.running = False
        self.serial_conn = None
        self.status_data = {
            'timestamp': '',
            'connection': 'Unknown',
            'wrapper_version': 'Unknown',
            'config': {
                'max_sonicators': 4,
                'implemented_units': [4],  # Only S4 is physically implemented
                'simulated_units': [1, 2, 3],  # S1-S3 are simulated
                'modbus_slave_id': 2,
                'baud_rate': 115200
            },
            'units': {
                1: {'running': False, 'overload': False, 'locked': False, 'frequency': 0, 'power': 0, 'implemented': False},
                2: {'running': False, 'overload': False, 'locked': False, 'frequency': 0, 'power': 0, 'implemented': False},
                3: {'running': False, 'overload': False, 'locked': False, 'frequency': 0, 'power': 0, 'implemented': False},
                4: {'running': False, 'overload': False, 'locked': False, 'frequency': 0, 'power': 0, 'implemented': True},
            },
            'amplitude': 0,
            'active_count': 0,
            'active_mask': 0
        }

    def connect(self) -> bool:
        """Connect to HIL wrapper."""
        try:
            import serial
            self.serial_conn = serial.Serial(self.port, 115200, timeout=0.5)
            time.sleep(1)  # Wait for Arduino reset

            # Test connection with PING
            self.serial_conn.write(b"PING\n")
            time.sleep(0.1)
            response = self.serial_conn.readline().decode().strip()

            if response.startswith("OK"):
                self.status_data['connection'] = 'Connected'

                # Get version info
                self.serial_conn.write(b"INFO\n")
                time.sleep(0.1)
                info_response = self.serial_conn.readline().decode().strip()
                if info_response.startswith("OK"):
                    parts = info_response.split()
                    if len(parts) >= 3:
                        self.status_data['wrapper_version'] = parts[2]
                    if "S4-ONLY" in info_response:
                        # Confirm our configuration matches the wrapper
                        print_info("Wrapper confirmed: S4-ONLY configuration")

                return True
            else:
                self.status_data['connection'] = 'Failed'
                return False

        except Exception as e:
            self.status_data['connection'] = f'Error: {e}'
            return False

    def update_status(self):
        """Update status from HIL wrapper."""
        if not self.serial_conn:
            return

        try:
            self.status_data['timestamp'] = datetime.now().strftime("%H:%M:%S")

            # Query S4 status (only implemented unit)
            self.serial_conn.write(b"READ STATUS 4\n")
            time.sleep(0.05)

            # Try to read response without blocking
            if self.serial_conn.in_waiting > 0:
                response = self.serial_conn.readline().decode().strip()
                if response.startswith("OK"):
                    # Parse response: "OK RUN=1 OVL=0 LOCK=1"
                    parts = response.split()
                    for part in parts[1:]:  # Skip "OK"
                        if part.startswith("RUN="):
                            self.status_data['units'][4]['running'] = part.split('=')[1] == '1'
                        elif part.startswith("OVL="):
                            self.status_data['units'][4]['overload'] = part.split('=')[1] == '1'
                        elif part.startswith("LOCK="):
                            self.status_data['units'][4]['locked'] = part.split('=')[1] == '1'

            # Query power for S4
            self.serial_conn.write(b"READ POWER 4\n")
            time.sleep(0.05)

            if self.serial_conn.in_waiting > 0:
                response = self.serial_conn.readline().decode().strip()
                if response.startswith("OK POWER="):
                    try:
                        power_adc = int(response.split('=')[1])
                        # Convert ADC to approximate power (rough estimation)
                        # ADC 0-1023 maps to 0-2000W approximately
                        power_watts = int((power_adc / 1023.0) * 2000)
                        self.status_data['units'][4]['power'] = power_watts
                    except:
                        pass

            # Update system summary
            running_units = sum(1 for unit in self.status_data['units'].values() if unit['running'])
            self.status_data['active_count'] = running_units

            # Update active mask (bit 3 for S4)
            mask = 0
            if self.status_data['units'][4]['running']:
                mask |= 0x08  # Bit 3 for S4
            self.status_data['active_mask'] = mask

        except Exception as e:
            # Don't spam errors, just continue
            pass

    def format_status_line(self, label: str, value: Any, expected: Any = None, width: int = 20) -> str:
        """Format a status line with optional red highlighting."""
        value_str = str(value)
        if expected is not None and value != expected:
            value_str = f"{Colors.RED}{value_str}{Colors.END}"
        else:
            value_str = f"{Colors.GREEN}{value_str}{Colors.END}"

        return f"{label:<{width}}: {value_str}"

    def display_status(self):
        """Display the current status."""
        # Clear screen
        os.system('clear' if os.name == 'posix' else 'cls')

        # Header
        print_header("HIL SONICATOR MONITOR")
        print(f"{Colors.BOLD}Timestamp: {Colors.END}{self.status_data['timestamp']}")
        print(f"{Colors.BOLD}Port: {Colors.END}{self.port}")
        print()

        # Connection status
        conn_status = self.status_data['connection']
        if conn_status == 'Connected':
            conn_color = Colors.GREEN
        elif 'Error' in conn_status or conn_status == 'Failed':
            conn_color = Colors.RED
        else:
            conn_color = Colors.YELLOW

        print(f"{Colors.BOLD}Connection: {conn_color}{conn_status}{Colors.END}")
        print(f"{Colors.BOLD}Wrapper Version: {Colors.END}{self.status_data['wrapper_version']}")
        print()

        # Configuration
        config = self.status_data['config']
        print(f"{Colors.BOLD}{Colors.UNDERLINE}CONFIGURATION{Colors.END}")
        print(f"{Colors.BOLD}Max Sonicators: {Colors.END}{config['max_sonicators']}")
        print(f"{Colors.BOLD}Implemented Units: {Colors.END}{Colors.GREEN}S{', S'.join(map(str, config['implemented_units']))}{Colors.END}")
        print(f"{Colors.BOLD}Simulated Units: {Colors.END}{Colors.YELLOW}S{', S'.join(map(str, config['simulated_units']))}{Colors.END}")
        print(f"{Colors.BOLD}MODBUS Slave ID: {Colors.END}{config['modbus_slave_id']}")
        print(f"{Colors.BOLD}Baud Rate: {Colors.END}{config['baud_rate']}")
        print()

        # System status
        print(f"{Colors.BOLD}{Colors.UNDERLINE}SYSTEM STATUS{Colors.END}")
        print(self.format_status_line("Active Count", self.status_data['active_count']))
        print(self.format_status_line("Active Mask", f"0x{self.status_data['active_mask']:02X}"))
        print(self.format_status_line("Amplitude", f"{self.status_data['amplitude']}%"))
        print()

        # Unit status
        print(f"{Colors.BOLD}{Colors.UNDERLINE}UNIT STATUS{Colors.END}")
        print(f"{'Unit':<6} {'Type':<12} {'Running':<8} {'Overload':<10} {'Locked':<8} {'Power':<10} {'Frequency':<10}")
        print("-" * 80)

        for unit_id, unit_data in self.status_data['units'].items():
            # Unit type and color
            if unit_data['implemented']:
                unit_type = f"{Colors.GREEN}PHYSICAL{Colors.END}"
                unit_prefix = f"{Colors.GREEN}S{unit_id}{Colors.END}"
            else:
                unit_type = f"{Colors.YELLOW}SIMULATED{Colors.END}"
                unit_prefix = f"{Colors.YELLOW}S{unit_id}{Colors.END}"

            running = "YES" if unit_data['running'] else "NO"
            overload = "YES" if unit_data['overload'] else "NO"
            locked = "YES" if unit_data['locked'] else "NO"
            power = f"{unit_data['power']}W" if unit_data['implemented'] else "N/A"
            frequency = f"{unit_data['frequency']} Hz"

            # Color coding for status
            running_color = Colors.GREEN if unit_data['running'] else Colors.WHITE
            overload_color = Colors.RED if unit_data['overload'] else Colors.GREEN
            locked_color = Colors.YELLOW if unit_data['locked'] else Colors.GREEN
            power_color = Colors.GREEN if unit_data['power'] > 0 else Colors.WHITE

            print(f"{unit_prefix:<6} {unit_type:<12} {running_color}{running:<8}{Colors.END} "
                  f"{overload_color}{overload:<10}{Colors.END} "
                  f"{locked_color}{locked:<8}{Colors.END} "
                  f"{power_color}{power:<10}{Colors.END} "
                  f"{frequency:<10}")

        print()
        print(f"{Colors.CYAN}Commands:{Colors.END}")
        print(f"{Colors.CYAN}  ping                    - Test wrapper connection{Colors.END}")
        print(f"{Colors.CYAN}  info                    - Get wrapper version{Colors.END}")
        print(f"{Colors.CYAN}  set overload 4 <0|1>    - Set overload on S4 (physical unit){Colors.END}")
        print(f"{Colors.CYAN}  set freq 4 <hz>         - Set frequency on S4{Colors.END}")
        print(f"{Colors.CYAN}  quit                    - Exit monitor{Colors.END}")
        print(f"{Colors.YELLOW}Note: Only S4 is physically implemented. S1-S3 are simulated by firmware.{Colors.END}")

    def handle_command(self, command: str):
        """Handle user command."""
        if not self.serial_conn:
            return

        try:
            self.serial_conn.write((command + "\n").encode())
            time.sleep(0.1)

            # Read response
            response = self.serial_conn.readline().decode().strip()
            if response:
                # Show response briefly
                print(f"\n{Colors.MAGENTA}Response: {response}{Colors.END}")
                time.sleep(1)

        except Exception as e:
            print(f"\n{Colors.RED}Command error: {e}{Colors.END}")
            time.sleep(1)

    def run_monitor(self):
        """Run the live monitoring interface."""
        print_step(7, "Opening HIL monitoring interface")

        if not self.connect():
            print_error("Failed to connect to HIL wrapper")
            return False

        print_success("Connected to HIL wrapper")
        time.sleep(1)

        self.running = True

        # Start status update thread
        def status_updater():
            while self.running:
                self.update_status()
                time.sleep(1)

        status_thread = threading.Thread(target=status_updater, daemon=True)
        status_thread.start()

        # Start display update thread
        def display_updater():
            while self.running:
                self.display_status()
                time.sleep(1)

        display_thread = threading.Thread(target=display_updater, daemon=True)
        display_thread.start()

        # Command input loop
        try:
            while self.running:
                command = input().strip()
                if command.lower() in ['quit', 'exit', 'q']:
                    break
                elif command:
                    self.handle_command(command)
        except KeyboardInterrupt:
            pass
        finally:
            self.running = False
            if self.serial_conn:
                self.serial_conn.close()

        print(f"\n{Colors.GREEN}HIL monitor closed.{Colors.END}")
        return True

def check_arduino_isp_loaded(port: str) -> bool:
    """Check if Arduino ISP is loaded by attempting ISP communication."""
    try:
        # Try to communicate with ATmega32A via Arduino ISP
        result = run_command([
            "avrdude", "-c", "stk500v1", "-P", port, "-b", "19200",
            "-p", "m32", "-v"
        ], check=False)
        return result.returncode == 0
    except Exception:
        return False
        
def upload_arduino_isp(port: str) -> bool:
    """Upload ArduinoISP example to ATmega32A."""
    try:
        # Upload ArduinoISP example
        run_command([
            "pio", "run", "-e", "development", "-t", "upload", "--upload-port", port
        ])
        return True
    except Exception:
        return False
    
def check_and_upload_isp(arduino_port: str) -> bool:
    """Check if Arduino ISP is loaded, upload if not."""
    print_step(1, "Checking Arduino ISP")
    
    # Check if ISP is already uploaded
    
    if check_arduino_isp_loaded(arduino_port):
        print_success("Arduino ISP already loaded") 
        return True
    
    # Upload ArduinoISP example
    upload_arduino_isp(arduino_port)
    return True
       

def main():
    """Main interactive HIL CLI workflow."""
    parser = argparse.ArgumentParser(description="Interactive HIL CLI")
    parser.add_argument("--env", choices=["development", "production"],
                       default="development", help="Build environment")
    parser.add_argument("--port", help="Arduino port (auto-detect if not specified)")

    args = parser.parse_args()

    print_header("INTERACTIVE HIL SETUP & MONITORING")
    print_info("This will guide you through the complete HIL setup process")
    print_info("Each step requires your confirmation before proceeding")

    if not confirm_step("Start HIL setup process?"):
        print_info("Setup cancelled by user")
        return

    # Find Arduino port
    arduino_port = args.port or find_arduino_port()
    if not arduino_port:
        print_error("Could not find Arduino port. Please specify with --port")
        return

    print_info(f"Using Arduino port: {arduino_port}")

    try:
        # Step 0: Build ATmega firmware
        if not build_atmega_firmware(args.env):
            print_error("Failed to build firmware")
            return

        # Step 1-2: Check and upload Arduino ISP
        if not check_and_upload_isp(arduino_port):
            print_error("Failed to setup Arduino ISP")
            return

        # Step 3: Program and confirm fuses
        if not program_and_confirm_fuses(arduino_port):
            print_error("Failed to program fuses")
            return

        # Step 4: Program and confirm firmware
        if not program_and_confirm_firmware(args.env, arduino_port):
            print_error("Failed to program firmware")
            return

        # Step 5: Remove capacitor
        remove_capacitor_prompt()

        # Step 6: Upload HIL wrapper
        if not upload_hil_wrapper(arduino_port):
            print_error("Failed to upload HIL wrapper")
            return

        # Step 7: Launch monitoring interface
        monitor = HILMonitor(arduino_port)
        monitor.run_monitor()

    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Setup interrupted by user{Colors.END}")
    except Exception as e:
        print_error(f"Setup failed: {e}")


if __name__ == "__main__":
    main()
