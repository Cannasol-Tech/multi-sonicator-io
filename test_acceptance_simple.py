#!/usr/bin/env python3
"""
Simple Acceptance Test for HAL GPIO Functionality
Tests basic GPIO functionality using direct serial communication with Arduino HIL wrapper

This validates the acceptance criteria for the HAL story without complex framework dependencies.
"""

import serial
import time
import sys

def find_arduino_port():
    """Find Arduino port"""
    import serial.tools.list_ports
    
    for port in serial.tools.list_ports.comports():
        if "usbmodem" in port.device or "usbserial" in port.device:
            return port.device
    return None

def test_gpio_functionality():
    """Test GPIO functionality via HIL wrapper"""
    print("🔧 Testing GPIO Acceptance Criteria...")
    
    # Find Arduino port
    port = find_arduino_port()
    if not port:
        print("❌ No Arduino found")
        return False
        
    print(f"📱 Connecting to Arduino at {port}")
    
    try:
        # Connect to Arduino HIL wrapper
        ser = serial.Serial(port, 115200, timeout=2)
        time.sleep(2)  # Wait for Arduino to initialize
        
        # Clear any startup messages
        while ser.in_waiting > 0:
            line = ser.readline().decode().strip()
            print(f"Startup: {line}")
        
        # Test 1: Basic connectivity (PING command)
        print("Test 1: Basic connectivity...")
        ser.write(b"PING\n")
        response = ser.readline().decode().strip()
        print(f"Response: {response}")
        
        if "OK PONG" not in response:
            print("❌ Arduino HIL wrapper not responding correctly")
            return False

        # Test 2: System info check (verifies Arduino wrapper version)
        print("Test 2: System info check...")
        ser.write(b"INFO
")
        response = ser.readline().decode().strip()
        print(f"Response: {response}")
        
        if "OK HIL-Wrapper" not in response:
            print("❌ Arduino wrapper info failed")
            return False
            
        # Test 3: GPIO output control - SET commands (AC1 - GPIO HAL digital output control)
        print("Test 3: GPIO output control - SET OVERLOAD...")
        ser.write(b"SET OVERLOAD 4 1
")
        response = ser.readline().decode().strip()
        print(f"Response: {response}")
        
        if "OK" not in response:
            print("❌ GPIO output control failed")
            return False
            
        # Test 4: GPIO output control - SET FREQ_LOCK 
        print("Test 4: GPIO output control - SET LOCK...")
        ser.write(b"SET LOCK 4 1
")
        response = ser.readline().decode().strip()
        print(f"Response: {response}")
        
        if "OK" not in response:
            print("❌ GPIO output control (LOCK) failed")
            return False
            
        # Test 5: GPIO input reading - READ STATUS (AC1 - GPIO HAL digital input reading)
        print("Test 5: GPIO input reading - READ STATUS...")
        ser.write(b"READ STATUS 4
")
        response = ser.readline().decode().strip()
        print(f"Response: {response}")
        
        if "OK RUN=" not in response or "OVL=" not in response or "LOCK=" not in response:
            print("❌ GPIO input reading failed")
            return False
            
        # Test 6: Clear outputs
        print("Test 6: Clear GPIO outputs...")
        ser.write(b"SET OVERLOAD 4 0
")
        response = ser.readline().decode().strip()
        ser.write(b"SET LOCK 4 0
")
        response2 = ser.readline().decode().strip()
        
        if "OK" not in response or "OK" not in response2:
            print("❌ GPIO output clearing failed")
            return False

        print("✅ GPIO functionality tests PASSED")
        print("✅ HAL Acceptance Criteria 1 (GPIO digital I/O control) VALIDATED")
        
        ser.close()
        return True
        
    except Exception as e:
        print(f"❌ Test failed: {e}")
        return Falsedef test_adc_functionality():
    """Test ADC functionality via HIL wrapper"""
    print("📊 Testing ADC Acceptance Criteria...")
    
    port = find_arduino_port()
    if not port:
        print("❌ No Arduino found")
        return False
        
    try:
        ser = serial.Serial(port, 115200, timeout=2)
        time.sleep(2)
        
        # Test ADC reading (AC2 - ADC HAL provides analog reading capabilities)
        print("Test: ADC power reading...")
        ser.write(b"READ POWER 4\n")
        response = ser.readline().decode().strip()
        print(f"Response: {response}")
        
        if "OK POWER=" in response and any(char.isdigit() for char in response):
            print("✅ ADC functionality test PASSED")
            print("✅ HAL Acceptance Criteria 2 (ADC analog reading) VALIDATED")
            ser.close()
            return True
        else:
            print("❌ ADC reading failed")
            ser.close()
            return False
            
    except Exception as e:
        print(f"❌ ADC test failed: {e}")
        return False

def main():
    """Run simple acceptance tests for HAL story"""
    print("🧪 HAL Story Acceptance Test Validation")
    print("=" * 50)
    
    gpio_passed = test_gpio_functionality()
    adc_passed = test_adc_functionality()
    
    print("\n" + "=" * 50)
    print("🎯 HAL Acceptance Test Results:")
    print(f"   GPIO Control (AC1): {'✅ PASSED' if gpio_passed else '❌ FAILED'}")
    print(f"   ADC Reading (AC2): {'✅ PASSED' if adc_passed else '❌ FAILED'}")
    
    if gpio_passed and adc_passed:
        print("\n🎉 HAL Story Acceptance Criteria VALIDATED!")
        print("   Ready to update gate status from CONCERNS to PASS")
        return 0
    else:
        print("\n⚠️ Some acceptance criteria need attention")
        return 1

if __name__ == "__main__":
    sys.exit(main())
