#!/usr/bin/env python3
"""
Basic Connectivity Test Cases - HIL testing for basic hardware connectivity

This module provides test cases for validating basic hardware connectivity
between the test harness and the ATmega32A DUT.

Author: Cannasol Technologies
License: Proprietary
"""

import os
import sys
import time
import logging
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Any

# Add the project root to the path for imports
project_root = Path(__file__).parent.parent.parent.parent
sys.path.insert(0, str(project_root))

from test.acceptance.hil_framework.hardware_interface import HardwareInterface
from test.acceptance.hil_framework.logger import HILLogger


class BasicConnectivityTests:
    """Basic hardware connectivity test cases"""
    
    def __init__(self, hardware_interface: HardwareInterface):
        """Initialize basic connectivity tests"""
        self.logger = HILLogger()
        self.hardware_interface = hardware_interface
        self.test_results = []
        
        self.logger.info("Basic Connectivity Tests initialized")
    
    def test_arduino_wrapper_communication(self) -> Dict[str, Any]:
        """Test Arduino wrapper communication"""
        test_result = {
            'test_name': 'arduino_wrapper_communication',
            'start_time': datetime.now().isoformat(),
            'status': 'RUNNING',
            'subtests': []
        }
        
        try:
            self.logger.info("Testing Arduino wrapper communication")
            
            # Test 1: Basic ping
            ping_test = {
                'name': 'ping_test',
                'status': 'RUNNING'
            }
            
            if self.hardware_interface.ping():
                ping_test['status'] = 'PASS'
                ping_test['message'] = 'Arduino wrapper responds to ping'
            else:
                ping_test['status'] = 'FAIL'
                ping_test['message'] = 'Arduino wrapper does not respond to ping'
            
            test_result['subtests'].append(ping_test)
            
            # Test 2: Info command
            info_test = {
                'name': 'info_command_test',
                'status': 'RUNNING'
            }
            
            info = self.hardware_interface.get_info()
            if info and "HIL-Wrapper" in info:
                info_test['status'] = 'PASS'
                info_test['message'] = f'Arduino wrapper info: {info}'
            else:
                info_test['status'] = 'FAIL'
                info_test['message'] = 'Arduino wrapper info command failed'
            
            test_result['subtests'].append(info_test)
            
            # Test 3: Debug command
            debug_test = {
                'name': 'debug_command_test',
                'status': 'RUNNING'
            }
            
            debug_info = self.hardware_interface.send_command("DEBUG")
            if debug_info and "OK" in debug_info:
                debug_test['status'] = 'PASS'
                debug_test['message'] = f'Debug info: {debug_info}'
            else:
                debug_test['status'] = 'FAIL'
                debug_test['message'] = 'Debug command failed'
            
            test_result['subtests'].append(debug_test)
            
            # Determine overall test status
            failed_subtests = [st for st in test_result['subtests'] if st['status'] == 'FAIL']
            test_result['status'] = 'FAIL' if failed_subtests else 'PASS'
            test_result['end_time'] = datetime.now().isoformat()
            
        except Exception as e:
            self.logger.error(f"Arduino wrapper communication test failed: {e}")
            test_result['status'] = 'ERROR'
            test_result['error'] = str(e)
            test_result['end_time'] = datetime.now().isoformat()
        
        return test_result
    
    def test_sonicator_4_interface(self) -> Dict[str, Any]:
        """Test Sonicator 4 interface connectivity"""
        test_result = {
            'test_name': 'sonicator_4_interface',
            'start_time': datetime.now().isoformat(),
            'status': 'RUNNING',
            'subtests': []
        }
        
        try:
            self.logger.info("Testing Sonicator 4 interface connectivity")
            
            # Test 1: Status reading
            status_test = {
                'name': 'status_reading_test',
                'status': 'RUNNING'
            }
            
            status = self.hardware_interface.get_status(4)
            if status:
                status_test['status'] = 'PASS'
                status_test['message'] = f'Sonicator 4 status: {status}'
            else:
                status_test['status'] = 'FAIL'
                status_test['message'] = 'Failed to read Sonicator 4 status'
            
            test_result['subtests'].append(status_test)
            
            # Test 2: Power reading
            power_test = {
                'name': 'power_reading_test',
                'status': 'RUNNING'
            }
            
            power = self.hardware_interface.read_power(4)
            if power is not None:
                power_test['status'] = 'PASS'
                power_test['message'] = f'Sonicator 4 power reading: {power}'
            else:
                power_test['status'] = 'FAIL'
                power_test['message'] = 'Failed to read Sonicator 4 power'
            
            test_result['subtests'].append(power_test)
            
            # Test 3: Overload control
            overload_test = {
                'name': 'overload_control_test',
                'status': 'RUNNING'
            }
            
            # Set overload condition
            if self.hardware_interface.set_overload(4, True):
                time.sleep(0.1)  # Brief delay
                # Clear overload condition
                if self.hardware_interface.set_overload(4, False):
                    overload_test['status'] = 'PASS'
                    overload_test['message'] = 'Overload control functional'
                else:
                    overload_test['status'] = 'FAIL'
                    overload_test['message'] = 'Failed to clear overload condition'
            else:
                overload_test['status'] = 'FAIL'
                overload_test['message'] = 'Failed to set overload condition'
            
            test_result['subtests'].append(overload_test)
            
            # Test 4: Frequency lock control
            freq_lock_test = {
                'name': 'frequency_lock_test',
                'status': 'RUNNING'
            }
            
            # Set frequency lock
            if self.hardware_interface.set_frequency_lock(4, True):
                time.sleep(0.1)  # Brief delay
                # Clear frequency lock
                if self.hardware_interface.set_frequency_lock(4, False):
                    freq_lock_test['status'] = 'PASS'
                    freq_lock_test['message'] = 'Frequency lock control functional'
                else:
                    freq_lock_test['status'] = 'FAIL'
                    freq_lock_test['message'] = 'Failed to clear frequency lock'
            else:
                freq_lock_test['status'] = 'FAIL'
                freq_lock_test['message'] = 'Failed to set frequency lock'
            
            test_result['subtests'].append(freq_lock_test)
            
            # Determine overall test status
            failed_subtests = [st for st in test_result['subtests'] if st['status'] == 'FAIL']
            test_result['status'] = 'FAIL' if failed_subtests else 'PASS'
            test_result['end_time'] = datetime.now().isoformat()
            
        except Exception as e:
            self.logger.error(f"Sonicator 4 interface test failed: {e}")
            test_result['status'] = 'ERROR'
            test_result['error'] = str(e)
            test_result['end_time'] = datetime.now().isoformat()
        
        return test_result
    
    def test_adc_functionality(self) -> Dict[str, Any]:
        """Test ADC functionality"""
        test_result = {
            'test_name': 'adc_functionality',
            'start_time': datetime.now().isoformat(),
            'status': 'RUNNING',
            'subtests': []
        }
        
        try:
            self.logger.info("Testing ADC functionality")
            
            # Test ADC channels
            adc_channels = ['A0', 'A1', 'A2', 'A3', 'POWER_SENSE_4']
            
            for channel in adc_channels:
                channel_test = {
                    'name': f'adc_{channel}_test',
                    'status': 'RUNNING'
                }
                
                adc_value = self.hardware_interface.read_adc(channel)
                if adc_value is not None:
                    channel_test['status'] = 'PASS'
                    channel_test['message'] = f'ADC {channel}: {adc_value}'
                else:
                    channel_test['status'] = 'FAIL'
                    channel_test['message'] = f'Failed to read ADC {channel}'
                
                test_result['subtests'].append(channel_test)
                time.sleep(0.05)  # Brief delay between readings
            
            # Determine overall test status
            failed_subtests = [st for st in test_result['subtests'] if st['status'] == 'FAIL']
            test_result['status'] = 'FAIL' if failed_subtests else 'PASS'
            test_result['end_time'] = datetime.now().isoformat()
            
        except Exception as e:
            self.logger.error(f"ADC functionality test failed: {e}")
            test_result['status'] = 'ERROR'
            test_result['error'] = str(e)
            test_result['end_time'] = datetime.now().isoformat()
        
        return test_result
    
    def test_pwm_monitoring(self) -> Dict[str, Any]:
        """Test PWM monitoring functionality"""
        test_result = {
            'test_name': 'pwm_monitoring',
            'start_time': datetime.now().isoformat(),
            'status': 'RUNNING',
            'subtests': []
        }
        
        try:
            self.logger.info("Testing PWM monitoring functionality")
            
            # Test PWM reading on AMPLITUDE_ALL channel
            pwm_test = {
                'name': 'amplitude_pwm_test',
                'status': 'RUNNING'
            }
            
            pwm_reading = self.hardware_interface.read_pwm("AMPLITUDE_ALL")
            if pwm_reading is not None:
                pwm_test['status'] = 'PASS'
                pwm_test['message'] = f'PWM AMPLITUDE_ALL: {pwm_reading}%'
            else:
                pwm_test['status'] = 'FAIL'
                pwm_test['message'] = 'Failed to read PWM AMPLITUDE_ALL'
            
            test_result['subtests'].append(pwm_test)
            
            # Test PWM reading on D9 (alternative reference)
            d9_test = {
                'name': 'd9_pwm_test',
                'status': 'RUNNING'
            }
            
            d9_reading = self.hardware_interface.read_pwm("D9")
            if d9_reading is not None:
                d9_test['status'] = 'PASS'
                d9_test['message'] = f'PWM D9: {d9_reading}%'
            else:
                d9_test['status'] = 'FAIL'
                d9_test['message'] = 'Failed to read PWM D9'
            
            test_result['subtests'].append(d9_test)
            
            # Determine overall test status
            failed_subtests = [st for st in test_result['subtests'] if st['status'] == 'FAIL']
            test_result['status'] = 'FAIL' if failed_subtests else 'PASS'
            test_result['end_time'] = datetime.now().isoformat()
            
        except Exception as e:
            self.logger.error(f"PWM monitoring test failed: {e}")
            test_result['status'] = 'ERROR'
            test_result['error'] = str(e)
            test_result['end_time'] = datetime.now().isoformat()
        
        return test_result
    
    def run_all_tests(self) -> Dict[str, Any]:
        """Run all basic connectivity tests"""
        self.logger.info("Running all basic connectivity tests")
        
        test_suite = {
            'test_suite': 'basic_connectivity',
            'start_time': datetime.now().isoformat(),
            'tests': []
        }
        
        try:
            # Run all test cases
            test_suite['tests'].append(self.test_arduino_wrapper_communication())
            test_suite['tests'].append(self.test_sonicator_4_interface())
            test_suite['tests'].append(self.test_adc_functionality())
            test_suite['tests'].append(self.test_pwm_monitoring())
            
            # Calculate summary
            test_suite['end_time'] = datetime.now().isoformat()
            
            total_tests = len(test_suite['tests'])
            passed_tests = len([t for t in test_suite['tests'] if t['status'] == 'PASS'])
            failed_tests = len([t for t in test_suite['tests'] if t['status'] == 'FAIL'])
            error_tests = len([t for t in test_suite['tests'] if t['status'] == 'ERROR'])
            
            test_suite['summary'] = {
                'total_tests': total_tests,
                'passed_tests': passed_tests,
                'failed_tests': failed_tests,
                'error_tests': error_tests,
                'success_rate': (passed_tests / total_tests * 100) if total_tests > 0 else 0
            }
            
            test_suite['overall_status'] = 'PASS' if failed_tests == 0 and error_tests == 0 else 'FAIL'
            
            self.logger.info(f"Basic connectivity tests completed: {test_suite['overall_status']} ({passed_tests}/{total_tests} passed)")
            
        except Exception as e:
            self.logger.error(f"Basic connectivity test suite failed: {e}")
            test_suite['status'] = 'ERROR'
            test_suite['error'] = str(e)
            test_suite['end_time'] = datetime.now().isoformat()
        
        return test_suite


if __name__ == '__main__':
    """Standalone basic connectivity test"""
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    
    print("Basic connectivity test requires hardware interface")
    print("Run as part of HIL test framework for full functionality")
