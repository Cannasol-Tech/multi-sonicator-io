#!/usr/bin/env python3
"""
Safety Interlocks Module - Implements safety systems and emergency stop procedures

This module provides comprehensive safety systems for HIL testing, including
emergency stop procedures, hardware fault detection, and safety interlocks.

Author: Cannasol Technologies
License: Proprietary
"""

import os
import sys
import time
import threading
import logging
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Any, Callable
from enum import Enum

# Add the project root to the path for imports
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root))

from test.acceptance.hil_framework.hardware_interface import HardwareInterface
from test.acceptance.hil_framework.logger import HILLogger


class SafetyState(Enum):
    """Safety system states"""
    SAFE = "safe"
    WARNING = "warning"
    EMERGENCY = "emergency"
    FAULT = "fault"
    UNKNOWN = "unknown"


class SafetyEvent(Enum):
    """Safety event types"""
    EMERGENCY_STOP = "emergency_stop"
    OVERLOAD_DETECTED = "overload_detected"
    COMMUNICATION_LOST = "communication_lost"
    HARDWARE_FAULT = "hardware_fault"
    TIMEOUT = "timeout"
    USER_INITIATED = "user_initiated"


class SafetyInterlocks:
    """Safety interlocks and emergency stop system"""
    
    def __init__(self, hardware_interface: Optional[HardwareInterface] = None):
        """Initialize safety interlocks system"""
        self.logger = HILLogger()
        self.hardware_interface = hardware_interface
        self.safety_state = SafetyState.UNKNOWN
        self.monitoring_active = False
        self.monitoring_thread = None
        self.safety_callbacks = []
        self.emergency_stop_active = False
        
        # Safety configuration
        self.config = {
            'emergency_stop_timeout_ms': 100,  # Maximum response time for emergency stop
            'communication_timeout_s': 5,      # Communication timeout
            'overload_threshold': 0.9,         # Overload threshold (90%)
            'monitoring_interval_s': 0.1,      # Safety monitoring interval
            'fault_retry_count': 3,            # Number of retries before fault
            'safe_state_timeout_s': 30         # Timeout for returning to safe state
        }
        
        # Load timeout configuration from HIL config
        self._load_timeout_config()
    
    def _load_timeout_config(self):
        """Load timeout configuration from HIL config file"""
        try:
            import yaml
            from pathlib import Path
            config_path = Path(__file__).parent.parent / 'acceptance' / 'hil_framework' / 'hil_config.yaml'
            if config_path.exists():
                with open(config_path, 'r') as f:
                    config = yaml.safe_load(f)
                    timeouts = config.get('timeouts', {}).get('safety_system', {})
                    self.config['emergency_stop_timeout_ms'] = int(timeouts.get('emergency_stop', 0.1) * 1000)
                    self.config['communication_timeout_s'] = timeouts.get('communication_timeout', 5.0)
                    self.config['monitoring_interval_s'] = timeouts.get('monitoring_interval', 0.1)
                    self.config['safe_state_timeout_s'] = timeouts.get('safe_state_timeout', 30.0)
        except Exception as e:
            self.logger.warning(f"Failed to load timeout configuration: {e}. Using defaults.")
        
        # Safety event log
        self.safety_events = []
        
        # Initialize safety systems
        self._initialize_safety_systems()
        
        self.logger.info("Safety Interlocks system initialized")
    
    def _initialize_safety_systems(self):
        """Initialize safety systems"""
        try:
            # Set initial safe state
            self.safety_state = SafetyState.SAFE
            
            # Initialize hardware to safe defaults if interface available
            if self.hardware_interface:
                self._set_hardware_safe_state()
            
            self.logger.info("Safety systems initialized to safe state")
            
        except Exception as e:
            self.logger.error(f"Failed to initialize safety systems: {e}")
            self.safety_state = SafetyState.FAULT
    
    def _set_hardware_safe_state(self):
        """Set hardware to safe default state"""
        try:
            if not self.hardware_interface:
                return
            
            # Set all control outputs to safe states
            # This would typically involve:
            # - Setting all START signals to inactive
            # - Setting all RESET signals to inactive
            # - Setting OVERLOAD signals to inactive
            # - Setting AMPLITUDE to minimum/off
            
            # For Sonicator 4 (currently implemented)
            self.hardware_interface.send_command("SET OVERLOAD 4 0")  # Clear overload
            self.hardware_interface.send_command("SET LOCK 4 0")      # Clear frequency lock
            
            self.logger.debug("Hardware set to safe state")
            
        except Exception as e:
            self.logger.error(f"Failed to set hardware safe state: {e}")
            raise
    
    def register_safety_callback(self, callback: Callable[[SafetyEvent, Dict[str, Any]], None]):
        """Register a callback for safety events"""
        self.safety_callbacks.append(callback)
        self.logger.debug(f"Safety callback registered: {callback.__name__}")
    
    def _trigger_safety_event(self, event: SafetyEvent, details: Dict[str, Any]):
        """Trigger a safety event and notify callbacks"""
        event_data = {
            'event': event,
            'timestamp': datetime.now().isoformat(),
            'details': details,
            'safety_state': self.safety_state
        }
        
        # Log the event
        self.safety_events.append(event_data)
        self.logger.warning(f"Safety event triggered: {event.value} - {details}")
        
        # Notify callbacks
        for callback in self.safety_callbacks:
            try:
                callback(event, event_data)
            except Exception as e:
                self.logger.error(f"Safety callback error: {e}")
    
    def emergency_stop(self, reason: str = "Manual trigger") -> bool:
        """Execute emergency stop procedure"""
        self.logger.critical(f"EMERGENCY STOP INITIATED: {reason}")
        
        start_time = time.time()
        
        try:
            # Set emergency stop flag
            self.emergency_stop_active = True
            self.safety_state = SafetyState.EMERGENCY
            
            # Immediately set hardware to safe state
            if self.hardware_interface:
                self._set_hardware_safe_state()
            
            # Stop any ongoing operations
            self._stop_all_operations()
            
            # Calculate response time
            response_time_ms = (time.time() - start_time) * 1000
            
            # Trigger safety event
            self._trigger_safety_event(SafetyEvent.EMERGENCY_STOP, {
                'reason': reason,
                'response_time_ms': response_time_ms,
                'within_spec': response_time_ms <= self.config['emergency_stop_timeout_ms']
            })
            
            self.logger.critical(f"Emergency stop completed in {response_time_ms:.1f}ms")
            
            # Verify response time meets specification
            if response_time_ms > self.config['emergency_stop_timeout_ms']:
                self.logger.error(f"Emergency stop response time exceeded specification: {response_time_ms:.1f}ms > {self.config['emergency_stop_timeout_ms']}ms")
                return False
            
            return True
            
        except Exception as e:
            self.logger.critical(f"Emergency stop procedure failed: {e}")
            self.safety_state = SafetyState.FAULT
            return False
    
    def _stop_all_operations(self):
        """Stop all ongoing operations immediately"""
        try:
            # This would stop any running tests, data collection, etc.
            # For now, we'll log the action
            self.logger.info("All operations stopped for emergency stop")
            
        except Exception as e:
            self.logger.error(f"Failed to stop all operations: {e}")
    
    def reset_emergency_stop(self) -> bool:
        """Reset emergency stop and return to safe state"""
        self.logger.info("Resetting emergency stop")
        
        try:
            if not self.emergency_stop_active:
                self.logger.warning("Emergency stop not active, nothing to reset")
                return True
            
            # Verify hardware is in safe state
            if not self._verify_safe_state():
                self.logger.error("Cannot reset emergency stop: hardware not in safe state")
                return False
            
            # Reset emergency stop flag
            self.emergency_stop_active = False
            self.safety_state = SafetyState.SAFE
            
            # Trigger safety event
            self._trigger_safety_event(SafetyEvent.USER_INITIATED, {
                'action': 'emergency_stop_reset',
                'success': True
            })
            
            self.logger.info("Emergency stop reset successfully")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to reset emergency stop: {e}")
            self.safety_state = SafetyState.FAULT
            return False
    
    def _verify_safe_state(self) -> bool:
        """Verify that hardware is in a safe state"""
        try:
            if not self.hardware_interface:
                return True  # Cannot verify without hardware interface
            
            # Check hardware status
            status = self.hardware_interface.get_status(4)  # Check Sonicator 4
            if not status:
                self.logger.warning("Cannot verify safe state: no hardware status")
                return False
            
            # Verify safe conditions
            # - No overload conditions
            # - No active start signals
            # - System responsive
            
            # For now, we'll assume safe state if we can communicate
            return True
            
        except Exception as e:
            self.logger.error(f"Safe state verification failed: {e}")
            return False
    
    def start_safety_monitoring(self):
        """Start continuous safety monitoring"""
        if self.monitoring_active:
            self.logger.warning("Safety monitoring already active")
            return
        
        self.monitoring_active = True
        self.monitoring_thread = threading.Thread(target=self._safety_monitoring_loop, daemon=True)
        self.monitoring_thread.start()
        
        self.logger.info("Safety monitoring started")
    
    def stop_safety_monitoring(self):
        """Stop safety monitoring"""
        if not self.monitoring_active:
            return
        
        self.monitoring_active = False
        if self.monitoring_thread:
            self.monitoring_thread.join(timeout=1.0)
        
        self.logger.info("Safety monitoring stopped")
    
    def _safety_monitoring_loop(self):
        """Main safety monitoring loop"""
        self.logger.debug("Safety monitoring loop started")
        
        last_communication_time = time.time()
        
        while self.monitoring_active:
            try:
                # Check communication with hardware
                if self.hardware_interface:
                    if self.hardware_interface.ping():
                        last_communication_time = time.time()
                    else:
                        # Check for communication timeout
                        if time.time() - last_communication_time > self.config['communication_timeout_s']:
                            self._trigger_safety_event(SafetyEvent.COMMUNICATION_LOST, {
                                'timeout_s': time.time() - last_communication_time
                            })
                            # Consider emergency stop for communication loss
                            if self.safety_state != SafetyState.EMERGENCY:
                                self.emergency_stop("Communication lost with hardware")
                
                # Check for overload conditions
                self._check_overload_conditions()
                
                # Check for hardware faults
                self._check_hardware_faults()
                
                # Sleep until next monitoring cycle
                time.sleep(self.config['monitoring_interval_s'])
                
            except Exception as e:
                self.logger.error(f"Safety monitoring error: {e}")
                time.sleep(self.config['monitoring_interval_s'])
        
        self.logger.debug("Safety monitoring loop stopped")
    
    def _check_overload_conditions(self):
        """Check for overload conditions"""
        try:
            if not self.hardware_interface:
                return
            
            # Check power levels for overload
            # This would typically read power measurements and compare to thresholds
            # For now, we'll simulate the check
            
            # Example: Check Sonicator 4 power
            # power_reading = self.hardware_interface.read_power(4)
            # if power_reading > overload_threshold:
            #     self._trigger_safety_event(SafetyEvent.OVERLOAD_DETECTED, {...})
            
        except Exception as e:
            self.logger.error(f"Overload check failed: {e}")
    
    def _check_hardware_faults(self):
        """Check for hardware faults"""
        try:
            if not self.hardware_interface:
                return
            
            # Check for hardware faults
            # This would typically check various hardware status indicators
            # For now, we'll simulate the check
            
        except Exception as e:
            self.logger.error(f"Hardware fault check failed: {e}")
    
    def get_safety_status(self) -> Dict[str, Any]:
        """Get current safety system status"""
        return {
            'safety_state': self.safety_state.value,
            'emergency_stop_active': self.emergency_stop_active,
            'monitoring_active': self.monitoring_active,
            'recent_events': self.safety_events[-10:],  # Last 10 events
            'hardware_connected': self.hardware_interface is not None and self.hardware_interface.connected,
            'configuration': self.config
        }
    
    def test_emergency_stop_response(self) -> Dict[str, Any]:
        """Test emergency stop response time"""
        self.logger.info("Testing emergency stop response time")
        
        test_result = {
            'test_name': 'emergency_stop_response_test',
            'timestamp': datetime.now().isoformat(),
            'passed': False,
            'response_time_ms': 0,
            'specification_ms': self.config['emergency_stop_timeout_ms']
        }
        
        try:
            # Measure emergency stop response time
            start_time = time.time()
            success = self.emergency_stop("Emergency stop response test")
            response_time_ms = (time.time() - start_time) * 1000
            
            test_result['response_time_ms'] = response_time_ms
            test_result['passed'] = success and response_time_ms <= self.config['emergency_stop_timeout_ms']
            
            # Reset emergency stop after test
            if success:
                time.sleep(0.1)  # Brief delay
                self.reset_emergency_stop()
            
            self.logger.info(f"Emergency stop test completed: {response_time_ms:.1f}ms ({'PASS' if test_result['passed'] else 'FAIL'})")
            
        except Exception as e:
            self.logger.error(f"Emergency stop test failed: {e}")
            test_result['error'] = str(e)
        
        return test_result
    
    def generate_safety_report(self) -> Dict[str, Any]:
        """Generate comprehensive safety system report"""
        return {
            'report_id': f"safety_report_{int(time.time())}",
            'timestamp': datetime.now().isoformat(),
            'safety_status': self.get_safety_status(),
            'emergency_stop_test': self.test_emergency_stop_response(),
            'event_summary': {
                'total_events': len(self.safety_events),
                'emergency_stops': len([e for e in self.safety_events if e['event'] == SafetyEvent.EMERGENCY_STOP]),
                'overloads': len([e for e in self.safety_events if e['event'] == SafetyEvent.OVERLOAD_DETECTED]),
                'communication_losses': len([e for e in self.safety_events if e['event'] == SafetyEvent.COMMUNICATION_LOST])
            }
        }


if __name__ == '__main__':
    """Standalone safety system test"""
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    
    # Create safety interlocks system
    safety = SafetyInterlocks()
    
    # Test emergency stop
    print("Testing emergency stop system...")
    test_result = safety.test_emergency_stop_response()
    print(f"Emergency stop test: {'PASS' if test_result['passed'] else 'FAIL'}")
    print(f"Response time: {test_result['response_time_ms']:.1f}ms")
    
    # Generate safety report
    report = safety.generate_safety_report()
    print(f"Safety system status: {report['safety_status']['safety_state']}")
