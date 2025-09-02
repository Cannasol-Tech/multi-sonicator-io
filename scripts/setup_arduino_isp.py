#!/usr/bin/env python3
"""
Automated Arduino ISP Setup Script
Detects Arduino, checks if ArduinoISP is loaded, and uploads it if needed.
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path

def run_command(cmd, check=True, capture_output=True):
    """Run a command and return the result"""
    print(f"$ {' '.join(cmd)}")
    return subprocess.run(cmd, check=check, capture_output=capture_output, text=True)

def detect_arduino_port():
    """Detect Arduino port using detect_hardware.py"""
    script_dir = Path(__file__).parent
    detect_script = script_dir / "detect_hardware.py"
    
    try:
        result = run_command([sys.executable, str(detect_script), "--check-arduino"], check=False)
        if result.returncode == 0:
            # Parse output to get port (this is a simplified approach)
            # In a real implementation, you might want to modify detect_hardware.py to return JSON
            import serial.tools.list_ports
            ports = serial.tools.list_ports.comports()
            
            # Look for Arduino-like devices
            for port in ports:
                description = (port.description or "").lower()
                manufacturer = (port.manufacturer or "").lower()
                device = port.device.lower()
                
                arduino_indicators = [
                    'arduino', 'uno', 'nano', 'mega',
                    'usbmodem', 'usbserial',
                    'ch340', 'ftdi', 'cp210x'
                ]
                
                if any(indicator in description for indicator in arduino_indicators) or \
                   any(indicator in manufacturer for indicator in arduino_indicators) or \
                   any(indicator in device for indicator in arduino_indicators):
                    return port.device
            
        return None
    except Exception as e:
        print(f"Error detecting Arduino: {e}")
        return None

def check_arduino_isp_loaded(port):
    """Check if ArduinoISP sketch is loaded using probe script"""
    script_dir = Path(__file__).parent
    probe_script = script_dir / "probe_arduinoisp.py"
    
    if not probe_script.exists():
        print("Warning: probe_arduinoisp.py not found, assuming ArduinoISP not loaded")
        return False
    
    try:
        result = run_command([sys.executable, str(probe_script), port], check=False)
        return result.returncode == 0
    except Exception as e:
        print(f"Error probing ArduinoISP: {e}")
        return False

def detect_board_fqbn(port):
    """Detect the FQBN of the Arduino board"""
    try:
        result = run_command(["arduino-cli", "board", "list", "--format", "json"], check=False)
        if result.returncode == 0:
            import json
            data = json.loads(result.stdout or "{}")
            for detected_port in data.get("detected_ports", []):
                if detected_port.get("port", {}).get("address") == port:
                    boards = detected_port.get("matching_boards", [])
                    if boards:
                        return boards[0].get("fqbn")
    except Exception as e:
        print(f"Error detecting board FQBN: {e}")

    # Default fallback
    return "arduino:avr:uno"

def upload_arduino_isp(port):
    """Upload ArduinoISP sketch using upload_arduinoisp.py or direct arduino-cli"""
    script_dir = Path(__file__).parent
    upload_script = script_dir / "upload_arduinoisp.py"

    # Try using the existing upload script first
    if upload_script.exists():
        try:
            # Detect board type and set appropriate environment variables
            fqbn = detect_board_fqbn(port)
            print(f"Detected board FQBN: {fqbn}")

            env = os.environ.copy()
            env["ARDUINO_PORT"] = port
            env["ARDUINO_FQBN"] = fqbn

            result = subprocess.run([sys.executable, str(upload_script)],
                                  env=env, check=False)
            return result.returncode == 0
        except Exception as e:
            print(f"Error using upload script: {e}")

    # Fallback to direct arduino-cli approach
    print("Fallback: Using direct arduino-cli approach...")
    return upload_arduino_isp_direct(port)

def upload_arduino_isp_direct(port):
    """Direct arduino-cli upload approach"""
    try:
        fqbn = detect_board_fqbn(port)
        print(f"Using FQBN: {fqbn}")

        # Ensure sketch directory exists
        sketch_dir = Path("test/hardware/sketches/ArduinoISP")
        sketch_dir.mkdir(parents=True, exist_ok=True)
        sketch_file = sketch_dir / "ArduinoISP.ino"

        # Download sketch if not present
        if not sketch_file.exists():
            print("Downloading ArduinoISP sketch...")
            url = "https://raw.githubusercontent.com/arduino/arduino-examples/main/examples/11.ArduinoISP/ArduinoISP/ArduinoISP.ino"
            run_command(["curl", "-fsSL", url, "-o", str(sketch_file)])

        # Install core if needed
        if "avr" in fqbn:
            run_command(["arduino-cli", "core", "install", "arduino:avr"], check=False)
        elif "renesas_uno" in fqbn:
            run_command(["arduino-cli", "core", "install", "arduino:renesas_uno"], check=False)

        # Compile
        print("Compiling ArduinoISP sketch...")
        run_command(["arduino-cli", "compile", "--fqbn", fqbn, str(sketch_dir)])

        # Upload
        print("Uploading ArduinoISP sketch...")
        run_command(["arduino-cli", "upload", "--fqbn", fqbn, "--port", port, str(sketch_dir)])

        return True

    except Exception as e:
        print(f"Error in direct upload: {e}")
        return False

def main():
    """Main setup logic"""
    parser = argparse.ArgumentParser(description='Setup Arduino as ISP automatically')
    parser.add_argument('--port', help='Arduino port (auto-detect if not specified)')
    parser.add_argument('--force-upload', action='store_true', 
                       help='Force upload ArduinoISP even if already detected')
    parser.add_argument('--check-only', action='store_true',
                       help='Only check if ArduinoISP is ready, do not upload')
    
    args = parser.parse_args()
    
    print("🔧 Arduino ISP Setup")
    print("=" * 40)
    
    # Step 1: Detect Arduino port
    port = args.port
    if not port:
        print("Step 1: Detecting Arduino...")
        port = detect_arduino_port()
        if not port:
            print("❌ No Arduino detected. Please connect Arduino and try again.")
            return 1
    
    print(f"✅ Arduino detected on port: {port}")
    
    # Step 2: Check if ArduinoISP is already loaded
    print("Step 2: Checking if ArduinoISP sketch is loaded...")
    isp_loaded = check_arduino_isp_loaded(port)
    
    if isp_loaded and not args.force_upload:
        print("✅ ArduinoISP sketch is already loaded and responding")
        if args.check_only:
            return 0
        print("🎯 Arduino as ISP is ready for use!")
        return 0
    
    if args.check_only:
        print("❌ ArduinoISP sketch is not loaded")
        return 1
    
    # Step 3: Upload ArduinoISP sketch
    print("Step 3: Uploading ArduinoISP sketch...")
    if upload_arduino_isp(port):
        print("✅ ArduinoISP sketch uploaded successfully")
        
        # Step 4: Verify upload (with delay for Arduino reset)
        print("Step 4: Waiting for Arduino to reset and start ArduinoISP...")
        import time
        time.sleep(3)  # Give Arduino time to reset and start new sketch

        print("Step 5: Verifying ArduinoISP is responding...")
        # Try verification a few times as the Arduino might still be initializing
        for attempt in range(3):
            if check_arduino_isp_loaded(port):
                print("✅ ArduinoISP verified and ready")
                print("🎯 Arduino as ISP setup complete!")
                return 0
            if attempt < 2:
                print(f"Verification attempt {attempt + 1} failed, retrying...")
                time.sleep(2)

        print("⚠️  ArduinoISP uploaded but not responding properly")
        print("This might be normal - try running the hardware-sandbox target to test ISP functionality")
        return 1
    else:
        print("❌ Failed to upload ArduinoISP sketch")
        return 1

if __name__ == "__main__":
    sys.exit(main())
