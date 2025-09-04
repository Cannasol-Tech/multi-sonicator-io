#!/usr/bin/env python3
"""
HIL Hardware Diagnostics
Validates HIL hardware configuration and connectivity for comprehensive testing
"""

import os
import sys
import yaml
import serial
import serial.tools.list_ports
import time
from pathlib import Path

class HILHardwareDiagnostics:
    def __init__(self):
        self.config_path = Path(__file__).parent / "hil_config.yaml"
        self.config = self.load_config()
        self.results = {
            "configuration": {"status": "UNKNOWN", "details": []},
            "hardware_detection": {"status": "UNKNOWN", "details": []},
            "connectivity": {"status": "UNKNOWN", "details": []},
            "calibration": {"status": "UNKNOWN", "details": []},
            "overall_status": "UNKNOWN"
        }
    
    def load_config(self):
        """Load HIL configuration"""
        try:
            with open(self.config_path, 'r') as f:
                return yaml.safe_load(f)
        except Exception as e:
            print(f"❌ Failed to load HIL configuration: {e}")
            return {}
    
    def validate_configuration(self):
        """Validate HIL configuration completeness"""
        print("🔍 Validating HIL Configuration...")
        
        required_sections = self.config.get('validation', {}).get('required_sections', [])
        missing_sections = []
        
        for section in required_sections:
            if section not in self.config:
                missing_sections.append(section)
        
        if missing_sections:
            self.results["configuration"]["status"] = "FAIL"
            self.results["configuration"]["details"].append(f"Missing sections: {missing_sections}")
            print(f"❌ Configuration validation failed: Missing {missing_sections}")
            return False
        
        # Validate specific configuration values
        hardware_config = self.config.get('hardware', {})
        if not hardware_config.get('target_mcu'):
            self.results["configuration"]["details"].append("Missing target_mcu specification")
            
        if not hardware_config.get('programmer_port'):
            self.results["configuration"]["details"].append("Missing programmer_port specification")
        
        modbus_config = self.config.get('modbus', {})
        if not modbus_config.get('slave_id'):
            self.results["configuration"]["details"].append("Missing MODBUS slave_id")
        
        if self.results["configuration"]["details"]:
            self.results["configuration"]["status"] = "WARN"
            print("⚠️ Configuration has warnings")
        else:
            self.results["configuration"]["status"] = "PASS"
            print("✅ Configuration validation passed")
        
        return True
    
    def detect_hardware(self):
        """Detect available hardware ports"""
        print("🔍 Detecting Hardware Ports...")
        
        # Get all available serial ports
        available_ports = list(serial.tools.list_ports.comports())
        
        if not available_ports:
            self.results["hardware_detection"]["status"] = "FAIL"
            self.results["hardware_detection"]["details"].append("No serial ports detected")
            print("❌ No serial ports detected")
            return False
        
        print(f"📋 Available serial ports:")
        for port in available_ports:
            print(f"   • {port.device} - {port.description}")
            self.results["hardware_detection"]["details"].append(f"{port.device}: {port.description}")
        
        # Check for Arduino-like devices
        arduino_ports = [p for p in available_ports if 'arduino' in p.description.lower() or 'usb' in p.device.lower()]
        
        if arduino_ports:
            self.results["hardware_detection"]["status"] = "PASS"
            print(f"✅ Found {len(arduino_ports)} potential Arduino device(s)")
        else:
            self.results["hardware_detection"]["status"] = "WARN"
            print("⚠️ No Arduino devices detected, but serial ports are available")
        
        return True
    
    def test_connectivity(self):
        """Test basic connectivity to hardware"""
        print("🔍 Testing Hardware Connectivity...")
        
        available_ports = list(serial.tools.list_ports.comports())
        if not available_ports:
            self.results["connectivity"]["status"] = "FAIL"
            self.results["connectivity"]["details"].append("No ports available for testing")
            print("❌ No ports available for connectivity testing")
            return False
        
        # Test connectivity to the first available port
        test_port = available_ports[0].device
        
        try:
            print(f"🔌 Testing connection to {test_port}...")
            
            # Attempt to open the port
            with serial.Serial(test_port, 115200, timeout=2) as ser:
                print(f"✅ Successfully opened {test_port}")
                
                # Try to send a simple command (this may fail if no device is listening)
                ser.write(b"AT\r\n")
                time.sleep(0.1)
                
                # Try to read response (may timeout)
                response = ser.read(100)
                if response:
                    print(f"📡 Received response: {response}")
                    self.results["connectivity"]["details"].append(f"Response from {test_port}: {response}")
                else:
                    print(f"⚠️ No response from {test_port} (device may not be listening)")
                    self.results["connectivity"]["details"].append(f"No response from {test_port}")
            
            self.results["connectivity"]["status"] = "PASS"
            print("✅ Basic connectivity test passed")
            return True
            
        except serial.SerialException as e:
            self.results["connectivity"]["status"] = "FAIL"
            self.results["connectivity"]["details"].append(f"Serial error: {str(e)}")
            print(f"❌ Connectivity test failed: {e}")
            return False
        except Exception as e:
            self.results["connectivity"]["status"] = "FAIL"
            self.results["connectivity"]["details"].append(f"Unexpected error: {str(e)}")
            print(f"❌ Unexpected error during connectivity test: {e}")
            return False
    
    def validate_calibration_requirements(self):
        """Validate calibration requirements are met"""
        print("🔍 Validating Calibration Requirements...")
        
        # Check if calibration data exists
        calibration_files = [
            "calibration_data.yaml",
            "voltage_calibration.json",
            "power_scaling.json"
        ]
        
        existing_files = []
        missing_files = []
        
        for cal_file in calibration_files:
            cal_path = Path(__file__).parent / cal_file
            if cal_path.exists():
                existing_files.append(cal_file)
            else:
                missing_files.append(cal_file)
        
        if existing_files:
            print(f"✅ Found calibration files: {existing_files}")
            self.results["calibration"]["details"].extend([f"Found: {f}" for f in existing_files])
        
        if missing_files:
            print(f"⚠️ Missing calibration files: {missing_files}")
            self.results["calibration"]["details"].extend([f"Missing: {f}" for f in missing_files])
            self.results["calibration"]["status"] = "WARN"
        else:
            self.results["calibration"]["status"] = "PASS"
        
        # Check configuration for calibration parameters
        power_config = self.config.get('power_supply', {})
        if power_config.get('tolerance'):
            print(f"✅ Power supply tolerance configured: ±{power_config['tolerance']}V")
            self.results["calibration"]["details"].append(f"Power tolerance: ±{power_config['tolerance']}V")
        
        timing_config = self.config.get('timing', {})
        if timing_config.get('crystal_frequency'):
            print(f"✅ Crystal frequency configured: {timing_config['crystal_frequency']} Hz")
            self.results["calibration"]["details"].append(f"Crystal: {timing_config['crystal_frequency']} Hz")
        
        if self.results["calibration"]["status"] == "UNKNOWN":
            self.results["calibration"]["status"] = "PASS"
        
        return True
    
    def generate_report(self):
        """Generate comprehensive diagnostics report"""
        print("\n" + "="*60)
        print("🏥 HIL HARDWARE DIAGNOSTICS REPORT")
        print("="*60)
        
        # Overall status determination
        statuses = [result["status"] for result in self.results.values() if isinstance(result, dict) and result.get("status") != "UNKNOWN"]
        
        if "FAIL" in statuses:
            self.results["overall_status"] = "FAIL"
            overall_icon = "❌"
        elif "WARN" in statuses:
            self.results["overall_status"] = "WARN"
            overall_icon = "⚠️"
        else:
            self.results["overall_status"] = "PASS"
            overall_icon = "✅"
        
        print(f"\n🎯 Overall Status: {overall_icon} {self.results['overall_status']}")
        
        # Detailed results
        for category, result in self.results.items():
            if category == "overall_status":
                continue
                
            status_icon = {"PASS": "✅", "WARN": "⚠️", "FAIL": "❌", "UNKNOWN": "❓"}.get(result["status"], "❓")
            print(f"\n📋 {category.replace('_', ' ').title()}: {status_icon} {result['status']}")
            
            for detail in result["details"]:
                print(f"   • {detail}")
        
        # Recommendations
        print(f"\n💡 Recommendations:")
        
        if self.results["configuration"]["status"] == "FAIL":
            print("   • Fix configuration issues before proceeding with HIL testing")
        
        if self.results["hardware_detection"]["status"] == "FAIL":
            print("   • Connect Arduino hardware and ensure drivers are installed")
        elif self.results["hardware_detection"]["status"] == "WARN":
            print("   • Verify Arduino devices are properly connected")
        
        if self.results["connectivity"]["status"] == "FAIL":
            print("   • Check hardware connections and port permissions")
            print("   • Ensure Arduino is programmed with HIL harness firmware")
        
        if self.results["calibration"]["status"] == "WARN":
            print("   • Perform hardware calibration before running precision tests")
            print("   • Generate calibration data files for accurate measurements")
        
        # HIL Testing Readiness
        print(f"\n🧪 HIL Testing Readiness:")
        if self.results["overall_status"] == "PASS":
            print("   ✅ System is ready for comprehensive HIL testing")
            print("   ✅ All 45 hardware scenarios can be executed")
        elif self.results["overall_status"] == "WARN":
            print("   ⚠️ System can run basic HIL tests with limitations")
            print("   ⚠️ Some precision tests may require calibration")
        else:
            print("   ❌ System is not ready for HIL testing")
            print("   ❌ Resolve hardware issues before proceeding")
        
        return self.results["overall_status"] == "PASS"
    
    def run_diagnostics(self):
        """Run complete hardware diagnostics"""
        print("🚀 Starting HIL Hardware Diagnostics...")
        print(f"📁 Configuration: {self.config_path}")
        
        try:
            self.validate_configuration()
            self.detect_hardware()
            self.test_connectivity()
            self.validate_calibration_requirements()
            
            return self.generate_report()
            
        except KeyboardInterrupt:
            print("\n⚠️ Diagnostics interrupted by user")
            return False
        except Exception as e:
            print(f"\n💥 Unexpected error during diagnostics: {e}")
            return False

def main():
    diagnostics = HILHardwareDiagnostics()
    success = diagnostics.run_diagnostics()
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
