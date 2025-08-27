#!/usr/bin/env python3
"""
HIL Workflow Automation Script

Orchestrates the complete HIL setup workflow:
1. Build ATmega32A firmware
2. Check if Arduino ISP is loaded, upload if needed
3. Program ATmega32A with latest build
4. Program HIL wrapper sketch on Arduino
5. Launch serial sandbox mode

Usage:
    python scripts/hil_workflow.py [--env development|production] [--port /dev/cu.usbmodem...]
"""

import argparse
import os
import subprocess
import sys
import time
from pathlib import Path
from typing import Optional

import serial.tools.list_ports


def run_command(cmd: list[str], cwd: Optional[str] = None, check: bool = True) -> subprocess.CompletedProcess:
    """Run a command and return the result."""
    print(f"Running: {' '.join(cmd)}")
    if cwd:
        print(f"  in: {cwd}")
    result = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if check and result.returncode != 0:
        print(f"Command failed with exit code {result.returncode}")
        print(f"STDOUT: {result.stdout}")
        print(f"STDERR: {result.stderr}")
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
        return candidates[0][1]
    return None


def check_arduino_isp_loaded(port: str) -> bool:
    """Check if Arduino ISP sketch is loaded by attempting ISP communication."""
    try:
        # Try to communicate with ATmega32A via Arduino ISP
        result = run_command([
            "avrdude", "-c", "stk500v1", "-P", port, "-b", "19200",
            "-p", "m32", "-v"
        ], check=False)
        return result.returncode == 0
    except Exception:
        return False


def upload_arduino_isp(port: str) -> None:
    """Upload Arduino ISP sketch to the Arduino."""
    print("Arduino ISP not detected. Uploading Arduino ISP sketch...")
    
    # Check if arduino-cli is available
    arduino_cli = None
    for cmd in ["arduino-cli", "arduino"]:
        try:
            run_command([cmd, "--version"], check=False)
            arduino_cli = cmd
            break
        except FileNotFoundError:
            continue
    
    if not arduino_cli:
        print("ERROR: arduino-cli not found. Please install Arduino CLI or use Arduino IDE to upload ArduinoISP sketch.")
        print("Install: brew install arduino-cli")
        sys.exit(1)
    
    # Upload ArduinoISP example
    run_command([
        arduino_cli, "compile", "--fqbn", "arduino:avr:uno", 
        "--library", "ArduinoISP", "ArduinoISP"
    ])
    
    run_command([
        arduino_cli, "upload", "--fqbn", "arduino:avr:uno", 
        "--port", port, "ArduinoISP"
    ])
    
    print("Arduino ISP uploaded successfully. Waiting for reset...")
    time.sleep(3)


def build_atmega_firmware(env: str = "development") -> None:
    """Build ATmega32A firmware using PlatformIO."""
    print(f"Building ATmega32A firmware ({env})...")
    run_command(["pio", "run", "-e", env])


def program_atmega(env: str = "development") -> None:
    """Program ATmega32A with built firmware."""
    print(f"Programming ATmega32A with {env} firmware...")
    run_command(["pio", "run", "-e", env, "--target", "upload"])


def upload_hil_wrapper(arduino_port: str) -> None:
    """Upload HIL wrapper sketch to Arduino."""
    print("Uploading HIL wrapper sketch to Arduino...")
    
    wrapper_dir = Path("test/hardware/arduino_test_wrapper")
    if not wrapper_dir.exists():
        print(f"ERROR: HIL wrapper directory not found: {wrapper_dir}")
        sys.exit(1)
    
    # Check if arduino-cli is available
    arduino_cli = None
    for cmd in ["arduino-cli", "arduino"]:
        try:
            run_command([cmd, "--version"], check=False)
            arduino_cli = cmd
            break
        except FileNotFoundError:
            continue
    
    if not arduino_cli:
        print("ERROR: arduino-cli not found. Please install Arduino CLI.")
        print("Install: brew install arduino-cli")
        sys.exit(1)
    
    # Compile and upload wrapper
    run_command([
        arduino_cli, "compile", "--fqbn", "arduino:avr:uno", 
        str(wrapper_dir / "arduino_test_wrapper.ino")
    ])
    
    run_command([
        arduino_cli, "upload", "--fqbn", "arduino:avr:uno", 
        "--port", arduino_port, str(wrapper_dir / "arduino_test_wrapper.ino")
    ])
    
    print("HIL wrapper uploaded successfully. Waiting for reset...")
    time.sleep(2)


def launch_serial_sandbox(port: str) -> None:
    """Launch interactive serial connection to HIL wrapper."""
    print(f"Launching serial sandbox on {port}")
    print("Commands: PING, INFO, SET OVERLOAD <unit> <0|1>, etc.")
    print("Press Ctrl+C to exit")
    
    try:
        import serial
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(0.5)  # Let Arduino reset
        
        print("Connected! Type commands (or 'quit' to exit):")
        
        while True:
            try:
                cmd = input("> ").strip()
                if cmd.lower() in ("quit", "exit"):
                    break
                if cmd:
                    ser.write((cmd + "\n").encode())
                    time.sleep(0.1)
                    while ser.in_waiting:
                        response = ser.readline().decode().strip()
                        if response:
                            print(f"< {response}")
            except KeyboardInterrupt:
                break
        
        ser.close()
        print("Serial sandbox closed.")
        
    except ImportError:
        print("pyserial not installed. Install with: pip install pyserial")
        print(f"Manually connect to {port} at 115200 baud")
    except Exception as e:
        print(f"Serial connection failed: {e}")


def main():
    parser = argparse.ArgumentParser(description="HIL Workflow Automation")
    parser.add_argument("--env", choices=["development", "production"], 
                       default="development", help="Build environment")
    parser.add_argument("--port", help="Arduino port (auto-detect if not specified)")
    parser.add_argument("--skip-build", action="store_true", help="Skip firmware build")
    parser.add_argument("--skip-isp-check", action="store_true", help="Skip Arduino ISP check")
    parser.add_argument("--skip-program", action="store_true", help="Skip ATmega programming")
    parser.add_argument("--skip-wrapper", action="store_true", help="Skip HIL wrapper upload")
    parser.add_argument("--skip-sandbox", action="store_true", help="Skip serial sandbox")
    
    args = parser.parse_args()
    
    # Find Arduino port
    arduino_port = args.port or find_arduino_port()
    if not arduino_port:
        print("ERROR: Could not find Arduino port. Specify with --port")
        sys.exit(1)
    
    print(f"Using Arduino port: {arduino_port}")
    
    try:
        # Step 1: Build ATmega firmware
        if not args.skip_build:
            build_atmega_firmware(args.env)
        
        # Step 2: Check/upload Arduino ISP
        if not args.skip_isp_check:
            if not check_arduino_isp_loaded(arduino_port):
                upload_arduino_isp(arduino_port)
            else:
                print("Arduino ISP already loaded.")
        
        # Step 3: Program ATmega
        if not args.skip_program:
            program_atmega(args.env)
        
        # Step 4: Upload HIL wrapper
        if not args.skip_wrapper:
            upload_hil_wrapper(arduino_port)
        
        # Step 5: Launch serial sandbox
        if not args.skip_sandbox:
            launch_serial_sandbox(arduino_port)
            
    except KeyboardInterrupt:
        print("\nWorkflow interrupted by user.")
    except Exception as e:
        print(f"Workflow failed: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
