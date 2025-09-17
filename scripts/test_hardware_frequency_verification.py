#!/usr/bin/env python3
"""
Comprehensive Hardware Frequency Verification Test

This script provides a complete demonstration and verification of the frequency
generation and MODBUS reading functionality with actual ATmega32A hardware.

It combines:
1. HIL frequency generation via Arduino Test Wrapper
2. ATmega32A frequency measurement via frequency counter
3. MODBUS register reading and verification
4. Comprehensive test reporting

Usage:
    python3 test_hardware_frequency_verification.py [options]
"""

import sys
import time
import json
import argparse
from pathlib import Path
from datetime import datetime

# Add test framework paths
sys.path.append(str(Path(__file__).parent.parent / 'test' / 'acceptance'))

try:
    from hil_framework.hardware_interface import HardwareInterface
    from pymodbus.client import ModbusSerialClient
    from pymodbus.framer.rtu_framer import ModbusRtuFramer
except ImportError as e:
    print(f"❌ Import error: {e}")
    print("Please ensure test dependencies are installed and HIL framework is available")
    sys.exit(1)

class HardwareFrequencyVerificationTest:
    def __init__(self):
        self.hil_interface = None
        self.modbus_client = None
        self.test_results = []
        self.start_time = datetime.now()
        
    def setup_hil_interface(self, port=None):
        """Set up HIL hardware interface"""
        print("🔧 Setting up HIL hardware interface...")
        
        try:
            self.hil_interface = HardwareInterface(serial_port=port)
            if not self.hil_interface.connect():
                raise Exception("Failed to connect to HIL wrapper")
            print("✅ HIL hardware interface connected")
            return True
        except Exception as e:
            print(f"❌ HIL setup failed: {e}")
            return False
            
    def setup_modbus_client(self, port=None, baud_rate=115200):
        """Set up MODBUS RTU client for ATmega32A"""
        print("🔧 Setting up MODBUS RTU client...")
        
        try:
            # Auto-detect port if not specified
            if not port:
                port = self._find_modbus_port()
                
            self.modbus_client = ModbusSerialClient(
                port=port,
                baudrate=baud_rate,
                parity='N',
                stopbits=1, 
                bytesize=8,
                timeout=2.0,
                framer=ModbusRtuFramer
            )
            
            self.modbus_client.unit_id = 2  # ATmega32A slave ID
            
            if not self.modbus_client.connect():
                raise Exception(f"Failed to connect to MODBUS device on {port}")
                
            print(f"✅ MODBUS client connected to {port} at {baud_rate} baud")
            return True
        except Exception as e:
            print(f"❌ MODBUS setup failed: {e}")
            return False
            
    def _find_modbus_port(self):
        """Find suitable port for MODBUS communication"""
        try:
            import serial.tools.list_ports
            ports = list(serial.tools.list_ports.comports())
            
            # Look for USB serial devices
            for port in ports:
                if 'USB' in port.description.upper():
                    return port.device
                    
            # Fallback to first available port
            if ports:
                return ports[0].device
                
        except Exception:
            pass
            
        return '/dev/ttyUSB0'  # Default fallback
        
    def verify_frequency_pipeline(self, input_frequency_hz, tolerance_percent=5.0):
        """Verify complete frequency pipeline from HIL to MODBUS"""
        test_name = f"Frequency Pipeline {input_frequency_hz}Hz"
        print(f"\n🧪 Testing: {test_name}")
        print("-" * 60)
        
        result = {
            'test_name': test_name,
            'input_frequency_hz': input_frequency_hz,
            'expected_modbus_hz': input_frequency_hz * 10,  # ÷10 scaling compensation
            'timestamp': datetime.now().isoformat(),
            'success': False,
            'error': None,
            'measurements': []
        }
        
        try:
            # Step 1: Generate frequency via HIL
            print(f"1️⃣ Generating {input_frequency_hz}Hz via HIL wrapper...")
            if not self.hil_interface.set_frequency(4, input_frequency_hz):
                raise Exception(f"Failed to set HIL frequency to {input_frequency_hz}Hz")
            print(f"   ✅ HIL frequency set to {input_frequency_hz}Hz")
            
            # Step 2: Wait for frequency measurement stabilization
            stabilization_time = 2.0  # seconds
            print(f"2️⃣ Waiting {stabilization_time}s for ATmega32A frequency measurement...")
            time.sleep(stabilization_time)
            print("   ✅ Frequency measurement stabilization complete")
            
            # Step 3: Read MODBUS register multiple times for stability
            register = 40370  # REG_SON4_FREQ_DIV10_HZ
            address = register - 40001
            readings = []
            
            print(f"3️⃣ Reading MODBUS register {register} (0x{register-40001:04X})...")
            
            for i in range(5):  # Take 5 readings
                response = self.modbus_client.read_holding_registers(address=address, count=1)
                
                if response.isError():
                    raise Exception(f"MODBUS read failed: {response}")
                    
                measured_freq = response.registers[0]
                readings.append(measured_freq)
                
                print(f"   📊 Reading {i+1}: {measured_freq}Hz")
                time.sleep(0.2)  # 200ms between readings
                
            # Step 4: Analyze readings
            avg_freq = sum(readings) / len(readings)
            min_freq = min(readings)
            max_freq = max(readings)
            freq_range = max_freq - min_freq
            
            result['measurements'] = readings
            result['average_frequency_hz'] = avg_freq
            result['frequency_range_hz'] = freq_range
            
            # Step 5: Verify frequency accuracy
            expected_freq = input_frequency_hz * 10
            tolerance_hz = max(expected_freq * tolerance_percent / 100.0, 10)
            frequency_error = abs(avg_freq - expected_freq)
            
            print(f"4️⃣ Verification:")
            print(f"   📈 Average measured: {avg_freq:.1f}Hz")
            print(f"   🎯 Expected: {expected_freq}Hz ± {tolerance_hz:.1f}Hz")
            print(f"   📏 Frequency range: {freq_range}Hz")
            print(f"   ❓ Error: {frequency_error:.1f}Hz")
            
            # Check accuracy
            if frequency_error <= tolerance_hz:
                print(f"   ✅ PASS: Frequency within tolerance")
                result['success'] = True
            else:
                print(f"   ❌ FAIL: Frequency error {frequency_error:.1f}Hz > tolerance {tolerance_hz:.1f}Hz")
                result['error'] = f"Frequency accuracy failure: {frequency_error:.1f}Hz error"
                
            # Check stability  
            stability_threshold = max(expected_freq * 0.02, 50)  # 2% or 50Hz
            if freq_range <= stability_threshold:
                print(f"   ✅ PASS: Frequency stable (range {freq_range}Hz ≤ {stability_threshold:.1f}Hz)")
            else:
                print(f"   ⚠️  WARNING: Frequency unstable (range {freq_range}Hz > {stability_threshold:.1f}Hz)")
                if result['success']:  # Don't override accuracy failure
                    result['error'] = f"Frequency stability warning: {freq_range}Hz range"
                    
        except Exception as e:
            result['error'] = str(e)
            print(f"   ❌ FAIL: {e}")
            
        self.test_results.append(result)
        return result
        
    def run_comprehensive_test_suite(self):
        """Run comprehensive test suite with multiple frequencies"""
        print("🎯 COMPREHENSIVE HARDWARE FREQUENCY VERIFICATION TEST SUITE")
        print("=" * 80)
        print("Testing complete frequency pipeline:")
        print("HIL generates → Arduino D7 → ATmega32A PB0 → frequency_calculate() → MODBUS register 40370")
        print()
        
        # Test frequencies covering different ranges
        test_frequencies = [
            0,      # Zero/off state
            1800,   # Low CT2000 range
            2000,   # Nominal frequency
            2200,   # High CT2000 range
        ]
        
        all_passed = True
        
        for freq in test_frequencies:
            result = self.verify_frequency_pipeline(freq)
            if not result['success']:
                all_passed = False
                
        return all_passed
        
    def generate_test_report(self, output_file=None):
        """Generate comprehensive test report"""
        print("\n" + "=" * 80)
        print("📊 HARDWARE FREQUENCY VERIFICATION TEST REPORT")
        print("=" * 80)
        
        total_duration = (datetime.now() - self.start_time).total_seconds()
        passed_tests = len([r for r in self.test_results if r['success']])
        total_tests = len(self.test_results)
        
        print(f"⏱️  Total Duration: {total_duration:.2f} seconds")
        print(f"📈 Test Results: {passed_tests}/{total_tests} PASSED")
        print()
        
        # Summary table
        print("📋 Test Summary:")
        print("   Input (Hz) | Expected MODBUS (Hz) | Measured (Hz) | Status")
        print("   -----------|---------------------|---------------|--------")
        
        for result in self.test_results:
            input_freq = result['input_frequency_hz']
            expected = result['expected_modbus_hz']
            
            if result['success'] and 'average_frequency_hz' in result:
                measured = f"{result['average_frequency_hz']:.0f}"
                status = "✅ PASS"
            else:
                measured = "N/A"
                status = "❌ FAIL"
                
            print(f"   {input_freq:10d} | {expected:19d} | {measured:13s} | {status}")
            
        print()
        
        # Detailed results
        for result in self.test_results:
            print(f"🔍 {result['test_name']}:")
            if result['success']:
                if 'measurements' in result and result['measurements']:
                    readings = result['measurements']
                    print(f"   📊 Readings: {readings}")
                    print(f"   📈 Statistics: avg={result.get('average_frequency_hz', 0):.1f}Hz, range={result.get('frequency_range_hz', 0)}Hz")
            else:
                print(f"   ❌ Error: {result.get('error', 'Unknown error')}")
            print()
            
        # Save to file if requested
        if output_file:
            report_data = {
                'timestamp': datetime.now().isoformat(),
                'duration_seconds': total_duration,
                'total_tests': total_tests,
                'passed_tests': passed_tests,
                'success_rate': passed_tests / total_tests if total_tests > 0 else 0,
                'test_results': self.test_results
            }
            
            with open(output_file, 'w') as f:
                json.dump(report_data, f, indent=2)
            print(f"📄 Detailed report saved: {output_file}")
            
        return passed_tests == total_tests
        
    def cleanup(self):
        """Clean up connections"""
        if self.hil_interface:
            try:
                self.hil_interface.disconnect()
            except:
                pass
                
        if self.modbus_client and self.modbus_client.connected:
            try:
                self.modbus_client.close()
            except:
                pass

def main():
    """Main function"""
    parser = argparse.ArgumentParser(
        description="Comprehensive hardware frequency verification test",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument('--hil-port', type=str, help='HIL wrapper serial port')
    parser.add_argument('--modbus-port', type=str, help='MODBUS RTU serial port') 
    parser.add_argument('--modbus-baud', type=int, default=115200, help='MODBUS baud rate')
    parser.add_argument('--output', type=str, help='Output file for test report (JSON)')
    parser.add_argument('--tolerance', type=float, default=5.0, help='Frequency tolerance percentage')
    
    args = parser.parse_args()
    
    test = HardwareFrequencyVerificationTest()
    
    try:
        # Set up hardware interfaces
        if not test.setup_hil_interface(args.hil_port):
            print("❌ Failed to set up HIL interface")
            return 1
            
        if not test.setup_modbus_client(args.modbus_port, args.modbus_baud):
            print("❌ Failed to set up MODBUS client")
            return 1
            
        # Run comprehensive test suite
        all_passed = test.run_comprehensive_test_suite()
        
        # Generate report
        report_success = test.generate_test_report(args.output)
        
        if all_passed and report_success:
            print("🎉 All tests PASSED - Hardware frequency verification successful!")
            return 0
        else:
            print("❌ Some tests FAILED - See report for details")
            return 1
            
    except Exception as e:
        print(f"❌ Test suite failed: {e}")
        return 1
    finally:
        test.cleanup()

if __name__ == "__main__":
    sys.exit(main())