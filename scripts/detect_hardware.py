#!/usr/bin/env python3
"""
Hardware Detection Script for Multi-Sonicator CI/CD Pipeline
Detects if physical hardware is available for testing
"""

import os
import sys
import subprocess
import platform
import argparse
import serial.tools.list_ports

def detect_arduino_programmer():
    """
    Detect if Arduino programmer is available
    """
    try:
        # Check for Arduino ISP programmer
        ports = serial.tools.list_ports.comports()
        arduino_ports = []
        
        for port in ports:
            port_name = port.device.lower()
            description = (port.description or "").lower()
            manufacturer = (port.manufacturer or "").lower()
            
            # Check for Arduino-specific identifiers
            arduino_indicators = [
                'arduino', 'uno', 'nano', 'mega',
                'usbmodem', 'usbserial',
                'ch340', 'ftdi', 'cp210x'
            ]
            
            if any(indicator in description for indicator in arduino_indicators) or \
               any(indicator in manufacturer for indicator in arduino_indicators) or \
               any(indicator in port_name for indicator in arduino_indicators):
                arduino_ports.append(port)
        
        if arduino_ports:
            print(f"Found Arduino programmer: {arduino_ports[0].device}")
            return True
            
    except Exception as e:
        print(f"Error detecting Arduino programmer: {e}")
        
    return False

def detect_arduino_isp():
    """
    Detect if Arduino as ISP is specifically available and responding
    Uses probe_arduinoisp.py for accurate detection
    """
    try:
        # First detect any Arduino
        if not detect_arduino_programmer():
            return False

        # Get Arduino port
        ports = serial.tools.list_ports.comports()
        arduino_port = None

        # Look for usbmodem devices (common for Arduino on macOS)
        import glob
        if platform.system() == "Darwin":
            usbmodem_ports = glob.glob("/dev/tty.usbmodem*")
            if usbmodem_ports:
                arduino_port = usbmodem_ports[0]

        # Fallback to general Arduino detection
        if not arduino_port:
            arduino_ports = [
                port for port in ports
                if 'arduino' in (port.description or "").lower() or
                   'uno' in (port.description or "").lower() or
                   'usbmodem' in port.device.lower()
            ]

            if arduino_ports:
                arduino_port = arduino_ports[0].device

        if not arduino_port:
            print("No Arduino port found")
            return False

        # Use probe script to verify ArduinoISP is loaded
        script_dir = os.path.dirname(os.path.abspath(__file__))
        probe_script = os.path.join(script_dir, "probe_arduinoisp.py")

        if os.path.exists(probe_script):
            result = subprocess.run([sys.executable, probe_script, arduino_port],
                                  capture_output=True, text=True)
            if result.returncode == 0:
                print(f"✅ Arduino as ISP detected and responding on: {arduino_port}")
                return True
            else:
                print(f"Arduino found on {arduino_port} but ArduinoISP sketch not loaded")
                return False
        else:
            # Fallback to basic port detection if probe script not found
            print(f"Arduino as ISP ready: {arduino_port}")
            return True

    except Exception as e:
        print(f"Error detecting Arduino as ISP: {e}")

    return False

def detect_target_hardware():
    """
    Detect if target ATmega32A hardware is available
    """
    try:
        # Check if avrdude is available
        result = subprocess.run(['which', 'avrdude'], 
                              capture_output=True, 
                              text=True)
        
        if result.returncode != 0:
            print("avrdude not found in PATH")
            return False
            
        # Try to detect target hardware via ISP
        # This would require specific hardware setup
        print("Target hardware detection not implemented yet")
        return False
        
    except Exception as e:
        print(f"Error detecting target hardware: {e}")
        return False

def detect_test_hardware():
    """
    Detect if test hardware (CT2000 sonicators) is available
    """
    try:
        # Check for specific test hardware ports/interfaces
        # This would be implemented based on actual test setup
        print("Test hardware detection not implemented yet")
        return False
        
    except Exception as e:
        print(f"Error detecting test hardware: {e}")
        return False

def main():
    """
    Main hardware detection logic
    Returns 0 if hardware is available, 1 if not
    """
    parser = argparse.ArgumentParser(description='Multi-Sonicator Hardware Detection')
    parser.add_argument('--check-arduino', action='store_true', 
                       help='Check for Arduino programmer')
    parser.add_argument('--check-arduino-isp', action='store_true',
                       help='Check for Arduino as ISP specifically')
    parser.add_argument('--check-target', action='store_true',
                       help='Check for target hardware')
    parser.add_argument('--check-test', action='store_true',
                       help='Check for test hardware')
    
    args = parser.parse_args()
    
    # If specific checks requested, only do those
    if args.check_arduino_isp:
        if detect_arduino_isp():
            print("✓ Arduino as ISP ready")
            return 0
        else:
            print("✗ Arduino as ISP not detected")
            return 1
    
    if args.check_arduino:
        if detect_arduino_programmer():
            print("✓ Arduino programmer detected")
            return 0
        else:
            print("✗ Arduino programmer not found")
            return 1
    
    if args.check_target:
        if detect_target_hardware():
            print("✓ Target hardware detected")
            return 0
        else:
            print("✗ Target hardware not found")
            return 1
    
    if args.check_test:
        if detect_test_hardware():
            print("✓ Test hardware detected")
            return 0
        else:
            print("✗ Test hardware not found")
            return 1
    
    # Default: comprehensive check
    print("Multi-Sonicator Hardware Detection")
    print("=" * 40)
    
    # Check environment
    print(f"Platform: {platform.platform()}")
    print(f"Python: {sys.version}")
    
    # Check for CI environment
    if os.getenv('CI') == 'true':
        print("Running in CI environment - no hardware available")
        return 1
    
    # Check for hardware components
    hardware_available = False
    
    print("\nChecking for hardware components...")
    
    # Check Arduino programmer
    if detect_arduino_programmer():
        print("✓ Arduino programmer detected")
        hardware_available = True
    else:
        print("✗ Arduino programmer not found")
    
    # Check target hardware
    if detect_target_hardware():
        print("✓ Target hardware detected")
        hardware_available = True
    else:
        print("✗ Target hardware not found")
    
    # Check test hardware
    if detect_test_hardware():
        print("✓ Test hardware detected")
        hardware_available = True
    else:
        print("✗ Test hardware not found")
    
    # Summary
    print("\n" + "=" * 40)
    if hardware_available:
        print("✓ Hardware available for testing")
        return 0
    else:
        print("✗ No hardware available - using simulation")
        return 1

if __name__ == "__main__":
    sys.exit(main())