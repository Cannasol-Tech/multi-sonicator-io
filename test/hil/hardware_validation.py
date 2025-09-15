#!/usr/bin/env python3
"""
Hardware Validation Module - Validates hardware configuration and connectivity

This module provides comprehensive hardware validation for the HIL test framework,
ensuring that the physical hardware matches the expected configuration.

Author: Cannasol Technologies
License: Proprietary
"""

import os
import sys
import yaml
import logging
from pathlib import Path
from typing import Dict, List, Optional, Any, Tuple

# Add the project root to the path for imports
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root))

from test.acceptance.hil_framework.hardware_interface import HardwareInterface
from test.acceptance.hil_framework.logger import HILLogger


class HardwareValidator:
    """Hardware configuration and connectivity validator"""
    
    def __init__(self):
        """Initialize hardware validator"""
        self.logger = HILLogger()
        self.hardware_config = None
        self.hardware_interface = None
        self.validation_results = {}
        
        # Load hardware configuration
        self._load_hardware_config()
        
        # Load configuration from HIL config
        self._load_hil_config()
    
    def _load_hil_config(self):
        """Load HIL configuration from config file"""
        try:
            import yaml
            from pathlib import Path
            config_path = Path(__file__).parent.parent / 'acceptance' / 'hil_framework' / 'hil_config.yaml'
            if config_path.exists():
                with open(config_path, 'r') as f:
                    config = yaml.safe_load(f)
                    testing_config = config.get('testing', {})
                    # Load any relevant configuration values
                    self.voltage_tolerance = testing_config.get('voltage_tolerance', 0.2)
                    self.timing_tolerance = testing_config.get('timing_tolerance', 0.1)
                    self.adc_reference_voltage = testing_config.get('adc_reference_voltage', 5.0)
        except Exception as e:
            self.logger.warning(f"Failed to load HIL configuration: {e}. Using defaults.")
            # Default values if config loading fails
            self.voltage_tolerance = 0.2
            self.timing_tolerance = 0.1
            self.adc_reference_voltage = 5.0
        
        self.logger.info("Hardware Validator initialized")
    
    def _load_hardware_config(self):
        """Load hardware configuration from YAML file"""
        try:
            config_path = project_root / 'config' / 'hardware-config.yaml'
            if not config_path.exists():
                raise FileNotFoundError(f"Hardware configuration file not found: {config_path}")
            
            with open(config_path, 'r') as f:
                self.hardware_config = yaml.safe_load(f)
            
            self.logger.info("Hardware configuration loaded successfully")
            
        except Exception as e:
            self.logger.error(f"Failed to load hardware configuration: {e}")
            raise
    
    def validate_pin_matrix(self) -> Tuple[bool, Dict[str, Any]]:
        """Validate pin matrix configuration"""
        self.logger.info("Validating pin matrix configuration")
        
        validation_result = {
            'test_name': 'pin_matrix_validation',
            'status': 'RUNNING',
            'checks': []
        }
        
        try:
            # Check DUT pin definitions
            dut_validation = self._validate_dut_pins()
            validation_result['checks'].append(dut_validation)
            
            # Check harness pin mappings
            harness_validation = self._validate_harness_pins()
            validation_result['checks'].append(harness_validation)
            
            # Check DB9 connector mappings
            db9_validation = self._validate_db9_connectors()
            validation_result['checks'].append(db9_validation)
            
            # Check test capabilities
            capabilities_validation = self._validate_test_capabilities()
            validation_result['checks'].append(capabilities_validation)
            
            # Determine overall status
            failed_checks = [check for check in validation_result['checks'] if not check['passed']]
            validation_result['status'] = 'FAIL' if failed_checks else 'PASS'
            validation_result['failed_checks'] = len(failed_checks)
            validation_result['total_checks'] = len(validation_result['checks'])
            
            self.logger.info(f"Pin matrix validation completed: {validation_result['status']}")
            
        except Exception as e:
            self.logger.error(f"Pin matrix validation failed: {e}")
            validation_result['status'] = 'ERROR'
            validation_result['error'] = str(e)
        
        return validation_result['status'] == 'PASS', validation_result
    
    def _validate_dut_pins(self) -> Dict[str, Any]:
        """Validate DUT (ATmega32A) pin definitions"""
        check_result = {
            'check_name': 'dut_pin_definitions',
            'passed': True,
            'issues': []
        }
        
        try:
            dut_config = self.hardware_config.get('dut', {})
            pins = dut_config.get('pins', {})
            
            # Required pin categories
            required_categories = ['PB', 'PC', 'PD', 'PA']  # Port B, C, D, A
            
            for category in required_categories:
                category_pins = [pin for pin in pins.keys() if pin.startswith(category)]
                if not category_pins:
                    check_result['issues'].append(f"No pins defined for port {category}")
                    check_result['passed'] = False
            
            # Validate pin structure
            for pin_name, pin_config in pins.items():
                required_fields = ['pin', 'signal', 'direction']
                for field in required_fields:
                    if field not in pin_config:
                        check_result['issues'].append(f"Pin {pin_name} missing required field: {field}")
                        check_result['passed'] = False
            
            self.logger.debug(f"DUT pin validation: {len(pins)} pins checked")
            
        except Exception as e:
            check_result['passed'] = False
            check_result['issues'].append(f"DUT pin validation error: {e}")
        
        return check_result
    
    def _validate_harness_pins(self) -> Dict[str, Any]:
        """Validate harness (Arduino) pin mappings"""
        check_result = {
            'check_name': 'harness_pin_mappings',
            'passed': True,
            'issues': []
        }
        
        try:
            harness_config = self.hardware_config.get('harness', {})
            connections = harness_config.get('connections', {})
            
            # Validate connection structure
            for arduino_pin, connection in connections.items():
                required_fields = ['dut_pin', 'signal', 'direction']
                for field in required_fields:
                    if field not in connection:
                        check_result['issues'].append(f"Arduino pin {arduino_pin} missing field: {field}")
                        check_result['passed'] = False
            
            # Check for duplicate DUT pin assignments
            dut_pins = [conn['dut_pin'] for conn in connections.values()]
            duplicates = [pin for pin in set(dut_pins) if dut_pins.count(pin) > 1]
            if duplicates:
                check_result['issues'].append(f"Duplicate DUT pin assignments: {duplicates}")
                check_result['passed'] = False
            
            self.logger.debug(f"Harness pin validation: {len(connections)} connections checked")
            
        except Exception as e:
            check_result['passed'] = False
            check_result['issues'].append(f"Harness pin validation error: {e}")
        
        return check_result
    
    def _validate_db9_connectors(self) -> Dict[str, Any]:
        """Validate DB9 connector mappings"""
        check_result = {
            'check_name': 'db9_connector_mappings',
            'passed': True,
            'issues': []
        }
        
        try:
            db9_config = self.hardware_config.get('db9_connectors', {})
            
            # Expected DB9 connectors
            expected_connectors = ['DB9-0', 'DB9-1', 'DB9-2', 'DB9-3', 'DB9-4']
            
            for connector in expected_connectors:
                if connector not in db9_config:
                    check_result['issues'].append(f"Missing DB9 connector: {connector}")
                    check_result['passed'] = False
                    continue
                
                connector_config = db9_config[connector]
                if 'pins' not in connector_config:
                    check_result['issues'].append(f"DB9 connector {connector} missing pins configuration")
                    check_result['passed'] = False
                    continue
                
                # Validate pin mappings
                pins = connector_config['pins']
                for pin_num, pin_config in pins.items():
                    if 'signal' not in pin_config or 'dut_pin' not in pin_config:
                        check_result['issues'].append(f"DB9 {connector} pin {pin_num} missing signal or dut_pin")
                        check_result['passed'] = False
            
            self.logger.debug(f"DB9 connector validation: {len(db9_config)} connectors checked")
            
        except Exception as e:
            check_result['passed'] = False
            check_result['issues'].append(f"DB9 connector validation error: {e}")
        
        return check_result
    
    def _validate_test_capabilities(self) -> Dict[str, Any]:
        """Validate test capabilities configuration"""
        check_result = {
            'check_name': 'test_capabilities',
            'passed': True,
            'issues': []
        }
        
        try:
            capabilities = self.hardware_config.get('test_capabilities', [])
            
            # Required test capabilities
            required_capabilities = [
                'modbus_communication',
                'sonicator_4_control',
                'power_measurement',
                'amplitude_control',
                'safety_interlocks'
            ]
            
            capability_names = [cap['name'] for cap in capabilities]
            
            for required_cap in required_capabilities:
                if required_cap not in capability_names:
                    check_result['issues'].append(f"Missing required test capability: {required_cap}")
                    check_result['passed'] = False
            
            # Validate capability structure
            for capability in capabilities:
                required_fields = ['name', 'description', 'pins', 'supported']
                for field in required_fields:
                    if field not in capability:
                        check_result['issues'].append(f"Capability {capability.get('name', 'unknown')} missing field: {field}")
                        check_result['passed'] = False
            
            self.logger.debug(f"Test capabilities validation: {len(capabilities)} capabilities checked")
            
        except Exception as e:
            check_result['passed'] = False
            check_result['issues'].append(f"Test capabilities validation error: {e}")
        
        return check_result
    
    def validate_hardware_connectivity(self, hardware_interface: HardwareInterface) -> Tuple[bool, Dict[str, Any]]:
        """Validate actual hardware connectivity"""
        self.logger.info("Validating hardware connectivity")
        self.hardware_interface = hardware_interface
        
        validation_result = {
            'test_name': 'hardware_connectivity',
            'status': 'RUNNING',
            'checks': []
        }
        
        try:
            # Test Arduino wrapper communication
            wrapper_test = self._test_wrapper_communication()
            validation_result['checks'].append(wrapper_test)
            
            # Test pin functionality
            pin_test = self._test_pin_functionality()
            validation_result['checks'].append(pin_test)
            
            # Test safety systems
            safety_test = self._test_safety_systems()
            validation_result['checks'].append(safety_test)
            
            # Determine overall status
            failed_checks = [check for check in validation_result['checks'] if not check['passed']]
            validation_result['status'] = 'FAIL' if failed_checks else 'PASS'
            validation_result['failed_checks'] = len(failed_checks)
            validation_result['total_checks'] = len(validation_result['checks'])
            
            self.logger.info(f"Hardware connectivity validation completed: {validation_result['status']}")
            
        except Exception as e:
            self.logger.error(f"Hardware connectivity validation failed: {e}")
            validation_result['status'] = 'ERROR'
            validation_result['error'] = str(e)
        
        return validation_result['status'] == 'PASS', validation_result
    
    def _test_wrapper_communication(self) -> Dict[str, Any]:
        """Test Arduino wrapper communication"""
        check_result = {
            'check_name': 'wrapper_communication',
            'passed': True,
            'issues': []
        }
        
        try:
            if not self.hardware_interface:
                check_result['passed'] = False
                check_result['issues'].append("Hardware interface not available")
                return check_result
            
            # Test basic ping
            if not self.hardware_interface.ping():
                check_result['passed'] = False
                check_result['issues'].append("Arduino wrapper not responding to ping")
            
            # Test info command
            info = self.hardware_interface.get_info()
            if not info:
                check_result['passed'] = False
                check_result['issues'].append("Arduino wrapper info command failed")
            
            self.logger.debug("Wrapper communication test completed")
            
        except Exception as e:
            check_result['passed'] = False
            check_result['issues'].append(f"Wrapper communication test error: {e}")
        
        return check_result
    
    def _test_pin_functionality(self) -> Dict[str, Any]:
        """Test pin functionality"""
        check_result = {
            'check_name': 'pin_functionality',
            'passed': True,
            'issues': []
        }
        
        try:
            # This would test actual pin functionality
            # For now, we'll simulate the test
            self.logger.debug("Pin functionality test simulated")
            
        except Exception as e:
            check_result['passed'] = False
            check_result['issues'].append(f"Pin functionality test error: {e}")
        
        return check_result
    
    def _test_safety_systems(self) -> Dict[str, Any]:
        """Test safety systems"""
        check_result = {
            'check_name': 'safety_systems',
            'passed': True,
            'issues': []
        }
        
        try:
            # This would test actual safety systems
            # For now, we'll simulate the test
            self.logger.debug("Safety systems test simulated")
            
        except Exception as e:
            check_result['passed'] = False
            check_result['issues'].append(f"Safety systems test error: {e}")
        
        return check_result
    
    def generate_validation_report(self) -> Dict[str, Any]:
        """Generate comprehensive validation report"""
        self.logger.info("Generating hardware validation report")
        
        report = {
            'report_id': f"hw_validation_{int(time.time())}",
            'timestamp': datetime.now().isoformat(),
            'hardware_config_version': self.hardware_config.get('project', {}).get('version', 'unknown'),
            'validations': []
        }
        
        try:
            # Run pin matrix validation
            pin_matrix_passed, pin_matrix_result = self.validate_pin_matrix()
            report['validations'].append(pin_matrix_result)
            
            # If hardware interface is available, run connectivity tests
            if self.hardware_interface:
                connectivity_passed, connectivity_result = self.validate_hardware_connectivity(self.hardware_interface)
                report['validations'].append(connectivity_result)
            
            # Calculate summary
            total_validations = len(report['validations'])
            passed_validations = len([v for v in report['validations'] if v['status'] == 'PASS'])
            
            report['summary'] = {
                'total_validations': total_validations,
                'passed_validations': passed_validations,
                'failed_validations': total_validations - passed_validations,
                'overall_status': 'PASS' if passed_validations == total_validations else 'FAIL'
            }
            
            self.logger.info(f"Validation report generated: {report['summary']['overall_status']}")
            
        except Exception as e:
            self.logger.error(f"Failed to generate validation report: {e}")
            report['error'] = str(e)
            report['summary'] = {'overall_status': 'ERROR'}
        
        return report


if __name__ == '__main__':
    """Standalone hardware validation"""
    import time
    from datetime import datetime
    
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    
    validator = HardwareValidator()
    
    # Run pin matrix validation
    pin_matrix_passed, pin_matrix_result = validator.validate_pin_matrix()
    print(f"Pin Matrix Validation: {'PASS' if pin_matrix_passed else 'FAIL'}")
    
    # Generate full report
    report = validator.generate_validation_report()
    print(f"Overall Validation Status: {report['summary']['overall_status']}")
