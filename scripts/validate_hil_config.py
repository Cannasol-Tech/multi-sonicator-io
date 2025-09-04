#!/usr/bin/env python3
"""
HIL Configuration Validator
Validates hil_config.yaml for completeness, correctness, and safety
"""

import os
import sys
import yaml
import argparse
from pathlib import Path
from typing import Dict, List, Any, Optional

class HILConfigValidator:
    def __init__(self, config_path: str):
        self.config_path = Path(config_path)
        self.config = None
        self.errors = []
        self.warnings = []
        
    def load_config(self) -> bool:
        """Load and parse the configuration file"""
        try:
            with open(self.config_path, 'r') as f:
                self.config = yaml.safe_load(f)
            return True
        except FileNotFoundError:
            self.errors.append(f"Configuration file not found: {self.config_path}")
            return False
        except yaml.YAMLError as e:
            self.errors.append(f"YAML parsing error: {e}")
            return False
    
    def validate_structure(self) -> bool:
        """Validate the overall configuration structure"""
        if not self.config:
            self.errors.append("Configuration is empty or invalid")
            return False
        
        # Check for required sections
        validation_config = self.config.get('validation', {})
        required_sections = validation_config.get('required_sections', [])
        
        for section in required_sections:
            if section not in self.config:
                self.errors.append(f"Required section missing: {section}")
        
        # Check configuration version
        config_version = validation_config.get('config_version')
        if not config_version:
            self.warnings.append("Configuration version not specified")
        
        return len(self.errors) == 0
    
    def validate_hardware_config(self) -> bool:
        """Validate hardware configuration"""
        hardware = self.config.get('hardware', {})
        
        # Required hardware settings
        required_fields = ['target_mcu', 'programmer', 'programmer_port', 'target_serial_port']
        for field in required_fields:
            if field not in hardware:
                self.errors.append(f"Hardware configuration missing: {field}")
        
        # Validate MCU type
        target_mcu = hardware.get('target_mcu')
        if target_mcu and target_mcu not in ['ATmega32A', 'ATmega328P']:
            self.warnings.append(f"Unsupported MCU type: {target_mcu}")
        
        # Validate programmer type
        programmer = hardware.get('programmer')
        if programmer and programmer not in ['arduino_as_isp', 'usbasp', 'avrisp']:
            self.warnings.append(f"Unsupported programmer: {programmer}")
        
        return True
    
    def validate_power_supply(self) -> bool:
        """Validate power supply configuration"""
        power = self.config.get('power_supply', {})
        
        # Check voltage ranges
        input_voltage = power.get('input_voltage', 0)
        if input_voltage < 12 or input_voltage > 30:
            self.warnings.append(f"Input voltage {input_voltage}V outside typical range (12-30V)")
        
        regulated_5v = power.get('regulated_5v', 0)
        if abs(regulated_5v - 5.0) > 0.5:
            self.errors.append(f"5V rail voltage {regulated_5v}V outside acceptable range (4.5-5.5V)")
        
        return True
    
    def validate_modbus_config(self) -> bool:
        """Validate MODBUS configuration"""
        modbus = self.config.get('modbus', {})
        
        # Validate slave ID
        slave_id = modbus.get('slave_id', 0)
        if slave_id < 1 or slave_id > 247:
            self.errors.append(f"MODBUS slave ID {slave_id} outside valid range (1-247)")
        
        # Validate baud rate
        baud_rate = modbus.get('baud_rate', 0)
        valid_baud_rates = [9600, 19200, 38400, 57600, 115200]
        if baud_rate not in valid_baud_rates:
            self.warnings.append(f"MODBUS baud rate {baud_rate} not in common rates: {valid_baud_rates}")
        
        # Validate register ranges
        register_ranges = modbus.get('register_ranges', {})
        for name, range_def in register_ranges.items():
            if len(range_def) != 2:
                self.errors.append(f"Invalid register range for {name}: {range_def}")
            elif range_def[0] >= range_def[1]:
                self.errors.append(f"Invalid register range for {name}: start >= end")
        
        return True
    
    def validate_pin_mapping(self) -> bool:
        """Validate pin mapping configuration"""
        pin_mapping = self.config.get('pin_mapping', {})
        
        # Check for required pins
        required_pins = ['FREQ_DIV10_4', 'FREQ_LOCK_4', 'OVERLOAD_4', 'START_4', 'RESET_4', 'POWER_SENSE_4']
        for pin in required_pins:
            if pin not in pin_mapping:
                self.warnings.append(f"Pin mapping missing for: {pin}")
        
        # Validate Arduino pin assignments
        used_pins = set()
        for signal, pin in pin_mapping.items():
            if pin in used_pins:
                self.errors.append(f"Pin {pin} assigned to multiple signals")
            used_pins.add(pin)
            
            # Basic pin format validation
            if not (pin.startswith('D') or pin.startswith('A')):
                self.warnings.append(f"Unusual pin format for {signal}: {pin}")
        
        return True
    
    def validate_safety_limits(self) -> bool:
        """Validate safety limits"""
        testing = self.config.get('testing', {})
        safety_limits = testing.get('safety_limits', {})
        
        # Check safety limits are reasonable
        max_voltage = safety_limits.get('max_voltage', 0)
        if max_voltage > 50:
            self.warnings.append(f"High maximum voltage limit: {max_voltage}V")
        
        max_current = safety_limits.get('max_current', 0)
        if max_current > 10:
            self.warnings.append(f"High maximum current limit: {max_current}A")
        
        max_power = safety_limits.get('max_power', 0)
        if max_power > 200:
            self.warnings.append(f"High maximum power limit: {max_power}W")
        
        return True
    
    def validate_all(self) -> bool:
        """Run all validation checks"""
        if not self.load_config():
            return False
        
        validation_methods = [
            self.validate_structure,
            self.validate_hardware_config,
            self.validate_power_supply,
            self.validate_modbus_config,
            self.validate_pin_mapping,
            self.validate_safety_limits
        ]
        
        success = True
        for method in validation_methods:
            try:
                method()
            except Exception as e:
                self.errors.append(f"Validation error in {method.__name__}: {e}")
                success = False
        
        return success and len(self.errors) == 0
    
    def print_results(self):
        """Print validation results"""
        print(f"HIL Configuration Validation: {self.config_path}")
        print("=" * 60)
        
        if self.errors:
            print(f"❌ ERRORS ({len(self.errors)}):")
            for error in self.errors:
                print(f"   • {error}")
            print()
        
        if self.warnings:
            print(f"⚠️  WARNINGS ({len(self.warnings)}):")
            for warning in self.warnings:
                print(f"   • {warning}")
            print()
        
        if not self.errors and not self.warnings:
            print("✅ Configuration is valid with no issues")
        elif not self.errors:
            print("✅ Configuration is valid with warnings")
        else:
            print("❌ Configuration has errors that must be fixed")
        
        return len(self.errors) == 0

def main():
    parser = argparse.ArgumentParser(description="Validate HIL configuration file")
    parser.add_argument("config_file", nargs="?", 
                       default="test/acceptance/hil_framework/hil_config.yaml",
                       help="Path to HIL configuration file")
    
    args = parser.parse_args()
    
    validator = HILConfigValidator(args.config_file)
    success = validator.validate_all()
    valid = validator.print_results()
    
    sys.exit(0 if valid else 1)

if __name__ == "__main__":
    main()
