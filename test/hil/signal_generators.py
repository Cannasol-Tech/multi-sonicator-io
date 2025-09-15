#!/usr/bin/env python3
"""
Signal Generators Module - Test signal generation utilities for HIL testing

This module provides comprehensive signal generation capabilities for HIL testing,
including GPIO signals, PWM signals, frequency generation, and analog simulation.

Author: Cannasol Technologies
License: Proprietary
"""

import os
import sys
import time
import math
import threading
import logging
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Any, Tuple
from enum import Enum

# Add the project root to the path for imports
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root))

from test.acceptance.hil_framework.hardware_interface import HardwareInterface
from test.acceptance.hil_framework.logger import HILLogger


class SignalType(Enum):
    """Signal types for generation"""
    DIGITAL = "digital"
    PWM = "pwm"
    FREQUENCY = "frequency"
    ANALOG = "analog"


class WaveformType(Enum):
    """Waveform types for signal generation"""
    SQUARE = "square"
    SINE = "sine"
    TRIANGLE = "triangle"
    SAWTOOTH = "sawtooth"
    CUSTOM = "custom"


class SignalGenerators:
    """Signal generation utilities for HIL testing"""
    
    def __init__(self, hardware_interface: HardwareInterface):
        """Initialize signal generators"""
        self.logger = HILLogger()
        self.hardware_interface = hardware_interface
        self.active_signals = {}
        self.generation_threads = {}
        
        # Signal generation configuration
        self.config = {
            'max_frequency_hz': 50000,      # Maximum frequency generation
            'min_frequency_hz': 1,          # Minimum frequency generation
            'pwm_resolution': 8,            # PWM resolution (8-bit = 0-255)
            'analog_resolution': 10,        # ADC resolution (10-bit = 0-1023)
            'max_voltage': 5.0,             # Maximum voltage (5V system)
            'update_interval_ms': 10        # Signal update interval
        }
        
        # Load configuration from HIL config
        self._load_config()
    
    def _load_config(self):
        """Load configuration from HIL config file"""
        try:
            import yaml
            from pathlib import Path
            config_path = Path(__file__).parent.parent / 'acceptance' / 'hil_framework' / 'hil_config.yaml'
            if config_path.exists():
                with open(config_path, 'r') as f:
                    config = yaml.safe_load(f)
                    testing_config = config.get('testing', {})
                    self.config['max_frequency_hz'] = testing_config.get('max_frequency_hz', 50000)
                    self.config['min_frequency_hz'] = testing_config.get('min_frequency_hz', 1)
                    self.config['pwm_resolution'] = testing_config.get('pwm_resolution', 8)
                    self.config['analog_resolution'] = testing_config.get('analog_resolution', 10)
                    self.config['max_voltage'] = testing_config.get('max_voltage', 5.0)
                    self.config['update_interval_ms'] = int(testing_config.get('update_interval_ms', 10))
        except Exception as e:
            self.logger.warning(f"Failed to load configuration: {e}. Using defaults.")
        
        self.logger.info("Signal Generators initialized")
    
    def generate_digital_signal(self, channel: str, state: bool) -> bool:
        """Generate a digital signal (HIGH/LOW)"""
        try:
            self.logger.debug(f"Generating digital signal on {channel}: {'HIGH' if state else 'LOW'}")
            
            # Map channel to appropriate command
            if channel == "OVERLOAD_4":
                command = f"SET OVERLOAD 4 {1 if state else 0}"
            elif channel == "FREQ_LOCK_4":
                command = f"SET LOCK 4 {1 if state else 0}"
            else:
                self.logger.error(f"Unsupported digital signal channel: {channel}")
                return False
            
            # Send command to hardware
            response = self.hardware_interface.send_command(command)
            success = response and "OK" in response
            
            if success:
                self.active_signals[channel] = {
                    'type': SignalType.DIGITAL,
                    'state': state,
                    'timestamp': datetime.now().isoformat()
                }
            
            return success
            
        except Exception as e:
            self.logger.error(f"Failed to generate digital signal on {channel}: {e}")
            return False
    
    def generate_pwm_signal(self, channel: str, duty_cycle: float, frequency: Optional[float] = None) -> bool:
        """Generate a PWM signal with specified duty cycle"""
        try:
            if not 0 <= duty_cycle <= 100:
                self.logger.error(f"Invalid duty cycle: {duty_cycle}% (must be 0-100%)")
                return False
            
            self.logger.debug(f"Generating PWM signal on {channel}: {duty_cycle}% duty cycle")
            
            # For now, we can't directly control PWM generation from the test wrapper
            # This would typically be implemented by the DUT firmware
            # We can simulate by setting voltage levels
            
            if channel == "AMPLITUDE_ALL":
                # Simulate PWM by setting equivalent voltage
                voltage = (duty_cycle / 100.0) * self.config['max_voltage']
                return self.generate_analog_signal(channel, voltage)
            else:
                self.logger.error(f"Unsupported PWM channel: {channel}")
                return False
            
        except Exception as e:
            self.logger.error(f"Failed to generate PWM signal on {channel}: {e}")
            return False
    
    def generate_frequency_signal(self, channel: str, frequency: float, duration: Optional[float] = None) -> bool:
        """Generate a frequency signal"""
        try:
            if not self.config['min_frequency_hz'] <= frequency <= self.config['max_frequency_hz']:
                self.logger.error(f"Frequency {frequency}Hz out of range ({self.config['min_frequency_hz']}-{self.config['max_frequency_hz']}Hz)")
                return False
            
            self.logger.debug(f"Generating frequency signal on {channel}: {frequency}Hz")
            
            # Map channel to appropriate command
            if channel.startswith("FREQ_DIV10_"):
                sonicator = channel.split("_")[-1]
                command = f"SET FREQ {sonicator} {frequency}"
            else:
                self.logger.error(f"Unsupported frequency signal channel: {channel}")
                return False
            
            # Send command to hardware
            response = self.hardware_interface.send_command(command)
            success = response and "OK" in response
            
            if success:
                self.active_signals[channel] = {
                    'type': SignalType.FREQUENCY,
                    'frequency': frequency,
                    'duration': duration,
                    'timestamp': datetime.now().isoformat()
                }
                
                # If duration specified, schedule signal stop
                if duration:
                    self._schedule_signal_stop(channel, duration)
            
            return success
            
        except Exception as e:
            self.logger.error(f"Failed to generate frequency signal on {channel}: {e}")
            return False
    
    def generate_analog_signal(self, channel: str, voltage: float) -> bool:
        """Generate an analog signal (voltage simulation)"""
        try:
            if not 0 <= voltage <= self.config['max_voltage']:
                self.logger.error(f"Voltage {voltage}V out of range (0-{self.config['max_voltage']}V)")
                return False
            
            self.logger.debug(f"Generating analog signal on {channel}: {voltage}V")
            
            # Map channel to appropriate command
            if channel == "POWER_SENSE_4" or channel == "AMPLITUDE_ALL":
                command = f"SET VOLTAGE {channel} {voltage}"
            else:
                self.logger.error(f"Unsupported analog signal channel: {channel}")
                return False
            
            # Send command to hardware
            response = self.hardware_interface.send_command(command)
            success = response and "OK" in response
            
            if success:
                self.active_signals[channel] = {
                    'type': SignalType.ANALOG,
                    'voltage': voltage,
                    'timestamp': datetime.now().isoformat()
                }
            
            return success
            
        except Exception as e:
            self.logger.error(f"Failed to generate analog signal on {channel}: {e}")
            return False
    
    def generate_waveform(self, channel: str, waveform: WaveformType, frequency: float, 
                         amplitude: float, duration: float) -> bool:
        """Generate a complex waveform"""
        try:
            self.logger.info(f"Generating {waveform.value} waveform on {channel}: {frequency}Hz, {amplitude}V, {duration}s")
            
            # Stop any existing waveform on this channel
            self.stop_signal(channel)
            
            # Create waveform generation thread
            thread = threading.Thread(
                target=self._waveform_generation_loop,
                args=(channel, waveform, frequency, amplitude, duration),
                daemon=True
            )
            
            self.generation_threads[channel] = thread
            thread.start()
            
            self.active_signals[channel] = {
                'type': SignalType.ANALOG,
                'waveform': waveform.value,
                'frequency': frequency,
                'amplitude': amplitude,
                'duration': duration,
                'timestamp': datetime.now().isoformat()
            }
            
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to generate waveform on {channel}: {e}")
            return False
    
    def _waveform_generation_loop(self, channel: str, waveform: WaveformType, 
                                 frequency: float, amplitude: float, duration: float):
        """Waveform generation loop"""
        try:
            start_time = time.time()
            sample_rate = 1000  # 1kHz sample rate
            sample_interval = 1.0 / sample_rate
            
            while time.time() - start_time < duration:
                current_time = time.time() - start_time
                
                # Calculate waveform value
                if waveform == WaveformType.SINE:
                    value = amplitude * math.sin(2 * math.pi * frequency * current_time)
                elif waveform == WaveformType.SQUARE:
                    value = amplitude if math.sin(2 * math.pi * frequency * current_time) >= 0 else -amplitude
                elif waveform == WaveformType.TRIANGLE:
                    phase = (frequency * current_time) % 1.0
                    value = amplitude * (4 * abs(phase - 0.5) - 1)
                elif waveform == WaveformType.SAWTOOTH:
                    phase = (frequency * current_time) % 1.0
                    value = amplitude * (2 * phase - 1)
                else:
                    value = 0
                
                # Convert to positive voltage (0 to max_voltage)
                voltage = (value + amplitude) / (2 * amplitude) * self.config['max_voltage']
                voltage = max(0, min(voltage, self.config['max_voltage']))
                
                # Generate the signal
                self.generate_analog_signal(channel, voltage)
                
                # Wait for next sample
                time.sleep(sample_interval)
            
            # Clean up after waveform completion
            self.stop_signal(channel)
            
        except Exception as e:
            self.logger.error(f"Waveform generation loop error on {channel}: {e}")
    
    def _schedule_signal_stop(self, channel: str, delay: float):
        """Schedule a signal to stop after a delay"""
        def stop_after_delay():
            time.sleep(delay)
            self.stop_signal(channel)
        
        thread = threading.Thread(target=stop_after_delay, daemon=True)
        thread.start()
    
    def stop_signal(self, channel: str) -> bool:
        """Stop signal generation on a channel"""
        try:
            if channel not in self.active_signals:
                self.logger.debug(f"No active signal on channel {channel}")
                return True
            
            self.logger.debug(f"Stopping signal on {channel}")
            
            # Stop generation thread if exists
            if channel in self.generation_threads:
                # Thread will stop naturally when the loop exits
                del self.generation_threads[channel]
            
            # Set channel to safe state
            signal_info = self.active_signals[channel]
            if signal_info['type'] == SignalType.DIGITAL:
                self.generate_digital_signal(channel, False)
            elif signal_info['type'] == SignalType.ANALOG:
                self.generate_analog_signal(channel, 0.0)
            elif signal_info['type'] == SignalType.PWM:
                self.generate_pwm_signal(channel, 0.0)
            
            # Remove from active signals
            del self.active_signals[channel]
            
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to stop signal on {channel}: {e}")
            return False
    
    def stop_all_signals(self) -> bool:
        """Stop all active signal generation"""
        try:
            self.logger.info("Stopping all signal generation")
            
            channels = list(self.active_signals.keys())
            success = True
            
            for channel in channels:
                if not self.stop_signal(channel):
                    success = False
            
            return success
            
        except Exception as e:
            self.logger.error(f"Failed to stop all signals: {e}")
            return False
    
    def get_active_signals(self) -> Dict[str, Any]:
        """Get information about active signals"""
        return dict(self.active_signals)
    
    def test_signal_generation(self) -> Dict[str, Any]:
        """Test signal generation capabilities"""
        self.logger.info("Testing signal generation capabilities")
        
        test_results = {
            'test_name': 'signal_generation_test',
            'timestamp': datetime.now().isoformat(),
            'tests': []
        }
        
        try:
            # Test digital signal generation
            digital_test = {
                'name': 'digital_signal_test',
                'passed': True,
                'details': []
            }
            
            # Test OVERLOAD_4 signal
            if self.generate_digital_signal("OVERLOAD_4", True):
                digital_test['details'].append("OVERLOAD_4 HIGH: PASS")
                time.sleep(0.1)
                if self.generate_digital_signal("OVERLOAD_4", False):
                    digital_test['details'].append("OVERLOAD_4 LOW: PASS")
                else:
                    digital_test['passed'] = False
                    digital_test['details'].append("OVERLOAD_4 LOW: FAIL")
            else:
                digital_test['passed'] = False
                digital_test['details'].append("OVERLOAD_4 HIGH: FAIL")
            
            test_results['tests'].append(digital_test)
            
            # Test analog signal generation
            analog_test = {
                'name': 'analog_signal_test',
                'passed': True,
                'details': []
            }
            
            # Test voltage simulation
            test_voltages = [0.0, 2.5, 5.0]
            for voltage in test_voltages:
                if self.generate_analog_signal("POWER_SENSE_4", voltage):
                    analog_test['details'].append(f"POWER_SENSE_4 {voltage}V: PASS")
                else:
                    analog_test['passed'] = False
                    analog_test['details'].append(f"POWER_SENSE_4 {voltage}V: FAIL")
                time.sleep(0.1)
            
            test_results['tests'].append(analog_test)
            
            # Calculate overall result
            failed_tests = [t for t in test_results['tests'] if not t['passed']]
            test_results['overall_result'] = 'PASS' if not failed_tests else 'FAIL'
            test_results['passed_tests'] = len(test_results['tests']) - len(failed_tests)
            test_results['total_tests'] = len(test_results['tests'])
            
            self.logger.info(f"Signal generation test completed: {test_results['overall_result']}")
            
        except Exception as e:
            self.logger.error(f"Signal generation test failed: {e}")
            test_results['error'] = str(e)
            test_results['overall_result'] = 'ERROR'
        
        finally:
            # Clean up - stop all test signals
            self.stop_all_signals()
        
        return test_results


if __name__ == '__main__':
    """Standalone signal generator test"""
    import sys
    sys.path.append(str(Path(__file__).parent.parent))
    
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    
    # This would require actual hardware interface
    print("Signal generator test requires hardware interface")
    print("Run as part of HIL test framework for full functionality")
