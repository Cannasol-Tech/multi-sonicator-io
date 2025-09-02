#!/usr/bin/env python3
"""
HIL Controller - Main controller for Hardware-in-the-Loop testing integration with Behave

This module provides the core HIL controller that integrates with the Behave BDD framework
for acceptance testing with real ATmega32A hardware.

Author: Cannasol Technologies  
License: Proprietary
"""

import os
import sys
import yaml
import time
import logging
from pathlib import Path
from typing import Dict, Optional, Any

from .hardware_interface import HardwareInterface
from .programmer import ArduinoISPProgrammer
from .logger import HILLogger


class HILController:
    """HIL Controller integrated with Behave acceptance testing framework"""
    
    def __init__(self, config_file='hil_config.yaml'):
        """Initialize HIL test framework with hardware configuration for Behave"""
        self.config_file = config_file
        self.config = self._load_config()
        self.logger = HILLogger()
        self.hardware_interface = None
        self.programmer = None
        self.hardware_ready = False
        
        self.logger.info("HIL Controller initialized")
        
    def _load_config(self) -> Dict[str, Any]:
        """Load HIL configuration from YAML file"""
        config_path = Path(__file__).parent / self.config_file
        
        # Default configuration if file doesn't exist
        default_config = {
            'hardware': {
                'target_mcu': 'ATmega32A',
                'programmer': 'arduino_as_isp',
                'programmer_port': '/dev/ttyUSB0',
                'target_serial_port': '/dev/ttyUSB1'
            },
            'power_supply': {
                'input_voltage': 24.0,
                'regulated_5v': 5.0,
                'tolerance': 0.1
            },
            'timing': {
                'crystal_frequency': 16000000,
                'serial_baud_rate': 115200
            },
            'modbus': {
                'slave_id': 2,
                'baud_rate': 115200,
                'timeout': 1.0
            },
            'behave': {
                'profile': 'hil',
                'junit_output': True,
                'junit_directory': 'acceptance-junit',
                'hardware_required': True
            }
        }
        
        if config_path.exists():
            try:
                with open(config_path, 'r') as f:
                    loaded_config = yaml.safe_load(f)
                    # Merge with defaults
                    default_config.update(loaded_config)
            except Exception as e:
                self.logger.warning(f"Failed to load config file {config_path}: {e}")
        
        return default_config
        
    def setup_hardware(self) -> bool:
        """Initialize hardware connections and verify basic connectivity"""
        try:
            self.logger.info("Setting up HIL hardware connections...")
            
            # Initialize hardware interface
            serial_port = self.config['hardware']['target_serial_port']
            baud_rate = self.config['timing']['serial_baud_rate']
            
            self.hardware_interface = HardwareInterface(serial_port, baud_rate)
            
            # Initialize programmer
            programmer_port = self.config['hardware']['programmer_port']
            self.programmer = ArduinoISPProgrammer(programmer_port)
            
            # Verify basic connectivity
            if not self.hardware_interface.verify_connection():
                self.logger.error("Failed to establish hardware interface connection")
                return False
                
            if not self.programmer.verify_connection():
                self.logger.error("Failed to establish programmer connection")
                return False
                
            self.hardware_ready = True
            self.logger.info("HIL hardware setup completed successfully")
            return True
            
        except Exception as e:
            self.logger.error(f"Hardware setup failed: {e}")
            return False
        
    def program_firmware(self, firmware_path: str) -> bool:
        """Upload firmware to ATmega32A via Arduino as ISP"""
        try:
            if not self.hardware_ready:
                self.logger.error("Hardware not ready for firmware programming")
                return False
                
            self.logger.info(f"Programming firmware: {firmware_path}")
            
            if not Path(firmware_path).exists():
                self.logger.error(f"Firmware file not found: {firmware_path}")
                return False
                
            # Program using Arduino as ISP
            success = self.programmer.program_firmware(firmware_path)
            
            if success:
                self.logger.info("Firmware programming completed successfully")
                # Wait for target to boot
                time.sleep(2)
                return True
            else:
                self.logger.error("Firmware programming failed")
                return False
                
        except Exception as e:
            self.logger.error(f"Firmware programming error: {e}")
            return False
        
    def verify_firmware_version(self) -> Optional[str]:
        """Verify firmware version and build information"""
        try:
            if not self.hardware_interface:
                return None
                
            response = self.hardware_interface.send_command("VERSION")
            if response:
                self.logger.info(f"Firmware version: {response}")
                return response
            else:
                self.logger.warning("Could not retrieve firmware version")
                return None
                
        except Exception as e:
            self.logger.error(f"Version verification error: {e}")
            return None
        
    def run_acceptance_scenario(self, scenario_context: Any) -> bool:
        """Execute Behave scenario with HIL hardware validation"""
        try:
            self.logger.info(f"Running HIL scenario: {scenario_context.scenario.name}")
            
            if not self.hardware_ready:
                self.logger.error("Hardware not ready for scenario execution")
                return False
                
            # Scenario execution is handled by step definitions
            # This method provides framework support
            return True
            
        except Exception as e:
            self.logger.error(f"Scenario execution error: {e}")
            return False
        
    def cleanup_hardware(self) -> None:
        """Safe hardware cleanup and disconnect"""
        try:
            self.logger.info("Cleaning up HIL hardware connections...")
            
            if self.hardware_interface:
                self.hardware_interface.cleanup()
                
            if self.programmer:
                self.programmer.cleanup()
                
            self.hardware_ready = False
            self.logger.info("HIL hardware cleanup completed")
            
        except Exception as e:
            self.logger.error(f"Hardware cleanup error: {e}")
            
    def get_config(self) -> Dict[str, Any]:
        """Get HIL configuration for test scenarios"""
        return self.config.copy()
        

def main():
    """CLI entry point for HIL controller setup and testing"""
    import argparse
    
    parser = argparse.ArgumentParser(description="HIL Controller CLI")
    parser.add_argument('--setup', action='store_true', help='Setup HIL hardware')
    parser.add_argument('--cleanup', action='store_true', help='Cleanup HIL hardware')
    parser.add_argument('--config', default='hil_config.yaml', help='Config file path')
    
    args = parser.parse_args()
    
    controller = HILController(args.config)
    
    if args.setup:
        success = controller.setup_hardware()
        sys.exit(0 if success else 1)
    elif args.cleanup:
        controller.cleanup_hardware()
        sys.exit(0)
    else:
        print("HIL Controller - Use --setup or --cleanup")
        sys.exit(1)


if __name__ == '__main__':
    main()
