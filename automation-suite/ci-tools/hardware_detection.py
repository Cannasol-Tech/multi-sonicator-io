#!/usr/bin/env python3
"""
Generic Hardware Detection and Simulation Fallback Tool
Automatically detects available hardware and falls back to simulation when needed
"""

import os
import sys
import json
import yaml
import subprocess
import argparse
from pathlib import Path
import serial.tools.list_ports
import platform
import time

class HardwareDetector:
    def __init__(self, config_path=None):
        """
        Initialize hardware detection tool
        
        Args:
            config_path: Path to configuration file
        """
        self.config = self.load_config(config_path)
        self.detected_hardware = {}
        self.simulation_enabled = False
        
    def load_config(self, config_path):
        """Load configuration from file or use defaults"""
        default_config = {
            'detection': {
                'serial_ports': True,
                'usb_devices': True,
                'network_devices': True,
                'custom_commands': []
            },
            'serial': {
                'baudrates': [9600, 19200, 38400, 57600, 115200],
                'timeout': 2.0,
                'expected_responses': []
            },
            'simulation': {
                'auto_fallback': True,
                'simulation_mode': 'virtual',
                'log_simulation': True
            },
            'hardware_profiles': {
                'sonicator': {
                    'serial_vid_pid': ['0x0403:0x6001'],  # FTDI
                    'description_patterns': ['CT2000', 'Sonicator'],
                    'baudrate': 38400,
                    'test_command': 'READ_STATUS'
                },
                'arduino': {
                    'serial_vid_pid': ['0x2341:0x0043', '0x1a86:0x7523'],
                    'description_patterns': ['Arduino', 'CH340'],
                    'baudrate': 115200,
                    'test_command': 'AT'
                }
            }
        }
        
        if config_path and Path(config_path).exists():
            with open(config_path, 'r') as f:
                if config_path.endswith('.yaml') or config_path.endswith('.yml'):
                    user_config = yaml.safe_load(f)
                else:
                    user_config = json.load(f)
            # Merge with defaults
            default_config.update(user_config)
            
        return default_config
    
    def detect_serial_devices(self):
        """Detect available serial devices"""
        devices = []
        
        try:
            ports = serial.tools.list_ports.comports()
            
            for port in ports:
                device_info = {
                    'port': port.device,
                    'description': port.description,
                    'hwid': port.hwid,
                    'vid_pid': f"{port.vid:04x}:{port.pid:04x}" if port.vid and port.pid else None,
                    'manufacturer': port.manufacturer,
                    'product': port.product,
                    'serial_number': port.serial_number,
                    'type': 'serial'
                }
                
                # Try to match against hardware profiles
                device_info['profiles'] = self.match_hardware_profiles(device_info)
                devices.append(device_info)
                
        except Exception as e:
            print(f"Error detecting serial devices: {e}")
            
        return devices
    
    def match_hardware_profiles(self, device_info):
        """Match device against known hardware profiles"""
        matches = []
        
        for profile_name, profile in self.config['hardware_profiles'].items():
            # Check VID:PID
            if device_info.get('vid_pid') in profile.get('serial_vid_pid', []):
                matches.append(profile_name)
                continue
            
            # Check description patterns
            description = device_info.get('description', '').lower()
            for pattern in profile.get('description_patterns', []):
                if pattern.lower() in description:
                    matches.append(profile_name)
                    break
        
        return matches
    
    def test_device_communication(self, device_info):
        """Test communication with a detected device"""
        if 'serial' not in device_info['type']:
            return False
        
        # Find matching profile for test parameters
        profile = None
        for profile_name in device_info.get('profiles', []):
            if profile_name in self.config['hardware_profiles']:
                profile = self.config['hardware_profiles'][profile_name]
                break
        
        if not profile:
            return False
        
        try:
            import serial
            
            with serial.Serial(
                device_info['port'],
                baudrate=profile.get('baudrate', 9600),
                timeout=self.config['serial']['timeout']
            ) as ser:
                
                # Send test command if specified
                test_command = profile.get('test_command')
                if test_command:
                    ser.write(test_command.encode() + b'\r\n')
                    response = ser.readline().decode().strip()
                    
                    # Check for expected responses
                    expected = profile.get('expected_responses', [])
                    if expected and not any(exp in response for exp in expected):
                        return False
                
                return True
                
        except Exception as e:
            print(f"Communication test failed for {device_info['port']}: {e}")
            return False
    
    def detect_usb_devices(self):
        """Detect USB devices (platform-specific)"""
        devices = []
        
        try:
            if platform.system() == "Linux":
                result = subprocess.run(['lsusb'], capture_output=True, text=True)
                if result.returncode == 0:
                    for line in result.stdout.strip().split('\n'):
                        if line.strip():
                            parts = line.split(': ', 1)
                            if len(parts) == 2:
                                devices.append({
                                    'bus_device': parts[0],
                                    'description': parts[1],
                                    'type': 'usb'
                                })
                                
            elif platform.system() == "Windows":
                # Could use wmic or PowerShell commands
                pass
            elif platform.system() == "Darwin":  # macOS
                result = subprocess.run(['system_profiler', 'SPUSBDataType'], 
                                      capture_output=True, text=True)
                # Parse macOS USB output
                pass
                
        except Exception as e:
            print(f"Error detecting USB devices: {e}")
            
        return devices
    
    def run_custom_detection_commands(self):
        """Run custom detection commands from configuration"""
        results = []
        
        for cmd_config in self.config['detection'].get('custom_commands', []):
            try:
                command = cmd_config['command']
                expected_output = cmd_config.get('expected_output', '')
                
                result = subprocess.run(
                    command.split(), 
                    capture_output=True, 
                    text=True, 
                    timeout=10
                )
                
                success = (result.returncode == 0 and 
                          (not expected_output or expected_output in result.stdout))
                
                results.append({
                    'command': command,
                    'success': success,
                    'output': result.stdout.strip(),
                    'error': result.stderr.strip()
                })
                
            except Exception as e:
                results.append({
                    'command': cmd_config['command'],
                    'success': False,
                    'error': str(e)
                })
        
        return results
    
    def detect_all_hardware(self):
        """Run complete hardware detection"""
        print("🔍 Starting hardware detection...")
        
        self.detected_hardware = {
            'timestamp': time.time(),
            'environment': self.get_environment_info(),
            'devices': {
                'serial': [],
                'usb': [],
                'custom': []
            },
            'summary': {
                'total_devices': 0,
                'communication_tested': 0,
                'communication_successful': 0
            }
        }
        
        # Detect serial devices
        if self.config['detection']['serial_ports']:
            print("  Detecting serial devices...")
            serial_devices = self.detect_serial_devices()
            
            # Test communication for devices with profiles
            for device in serial_devices:
                if device.get('profiles'):
                    device['communication_test'] = self.test_device_communication(device)
                    self.detected_hardware['summary']['communication_tested'] += 1
                    if device.get('communication_test'):
                        self.detected_hardware['summary']['communication_successful'] += 1
            
            self.detected_hardware['devices']['serial'] = serial_devices
            print(f"    Found {len(serial_devices)} serial devices")
        
        # Detect USB devices
        if self.config['detection']['usb_devices']:
            print("  Detecting USB devices...")
            usb_devices = self.detect_usb_devices()
            self.detected_hardware['devices']['usb'] = usb_devices
            print(f"    Found {len(usb_devices)} USB devices")
        
        # Run custom detection commands
        if self.config['detection']['custom_commands']:
            print("  Running custom detection commands...")
            custom_results = self.run_custom_detection_commands()
            self.detected_hardware['devices']['custom'] = custom_results
            print(f"    Executed {len(custom_results)} custom commands")
        
        # Calculate totals
        total_devices = (len(self.detected_hardware['devices']['serial']) +
                        len(self.detected_hardware['devices']['usb']) +
                        len(self.detected_hardware['devices']['custom']))
        self.detected_hardware['summary']['total_devices'] = total_devices
        
        # Determine if simulation should be enabled
        self.simulation_enabled = self.should_enable_simulation()
        self.detected_hardware['simulation_enabled'] = self.simulation_enabled
        
        return self.detected_hardware
    
    def should_enable_simulation(self):
        """Determine if simulation should be enabled"""
        if not self.config['simulation']['auto_fallback']:
            return False
        
        # Check if we're in a CI environment
        ci_indicators = ['CI', 'GITHUB_ACTIONS', 'TRAVIS', 'JENKINS', 'GITLAB_CI']
        if any(os.getenv(indicator) for indicator in ci_indicators):
            return True
        
        # Check if any hardware with successful communication was found
        successful_devices = 0
        for device in self.detected_hardware['devices']['serial']:
            if device.get('communication_test'):
                successful_devices += 1
        
        # Enable simulation if no working hardware found
        return successful_devices == 0
    
    def get_environment_info(self):
        """Get environment information"""
        return {
            'platform': platform.platform(),
            'python_version': platform.python_version(),
            'hostname': platform.node(),
            'ci_environment': self.detect_ci_environment()
        }
    
    def detect_ci_environment(self):
        """Detect if running in CI environment"""
        ci_envs = {
            'GITHUB_ACTIONS': 'GitHub Actions',
            'TRAVIS': 'Travis CI',
            'JENKINS_URL': 'Jenkins',
            'GITLAB_CI': 'GitLab CI',
            'CIRCLECI': 'CircleCI',
            'AZURE_HTTP_USER_AGENT': 'Azure DevOps'
        }
        
        for env_var, name in ci_envs.items():
            if os.getenv(env_var):
                return name
        
        return None
    
    def generate_report(self, output_path='hardware_detection.json'):
        """Generate hardware detection report"""
        with open(output_path, 'w') as f:
            json.dump(self.detected_hardware, f, indent=2)
        
        # Print summary
        print("\n" + "="*50)
        print("HARDWARE DETECTION REPORT")
        print("="*50)
        
        summary = self.detected_hardware['summary']
        print(f"Total devices found: {summary['total_devices']}")
        print(f"Communication tests: {summary['communication_tested']}")
        print(f"Successful tests: {summary['communication_successful']}")
        print(f"Simulation enabled: {self.simulation_enabled}")
        
        # Print device details
        for device_type, devices in self.detected_hardware['devices'].items():
            if devices and device_type == 'serial':
                print(f"\nSerial Devices:")
                for device in devices:
                    status = "✅" if device.get('communication_test') else "❌" if 'communication_test' in device else "⚪"
                    profiles = ", ".join(device.get('profiles', ['Unknown']))
                    print(f"  {status} {device['port']} - {profiles}")
                    print(f"      {device['description']}")
        
        print(f"\nFull report saved to: {output_path}")
        
        # Set environment variables for CI/CD
        if self.simulation_enabled:
            os.environ['HARDWARE_SIMULATION'] = 'true'
            print("\n🎭 Simulation mode enabled - HARDWARE_SIMULATION=true")
        else:
            os.environ['HARDWARE_SIMULATION'] = 'false'
            print("\n🔧 Hardware mode enabled - HARDWARE_SIMULATION=false")
        
        return self.detected_hardware

def main():
    parser = argparse.ArgumentParser(description='Generic Hardware Detection Tool')
    parser.add_argument('--config', '-c', help='Configuration file path')
    parser.add_argument('--output', '-o', help='Output report file', default='hardware_detection.json')
    parser.add_argument('--force-simulation', action='store_true', help='Force simulation mode')
    parser.add_argument('--test-communication', action='store_true', help='Test device communication')
    
    args = parser.parse_args()
    
    # Initialize detection tool
    detector = HardwareDetector(config_path=args.config)
    
    # Force simulation if requested
    if args.force_simulation:
        detector.config['simulation']['auto_fallback'] = True
        detector.simulation_enabled = True
    
    # Run detection
    results = detector.detect_all_hardware()
    
    # Generate report
    detector.generate_report(args.output)
    
    # Exit with appropriate code
    if detector.simulation_enabled:
        print("\nExiting with simulation mode enabled")
        sys.exit(0)
    else:
        successful = results['summary']['communication_successful']
        if successful > 0:
            print(f"\nExiting with {successful} working hardware devices")
            sys.exit(0)
        else:
            print("\nExiting - no working hardware found")
            sys.exit(1)

if __name__ == '__main__':
    main()