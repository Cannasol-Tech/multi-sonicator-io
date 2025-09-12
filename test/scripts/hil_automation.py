#!/usr/bin/env python3
"""
HIL Automation Controller - Main automation controller for HIL testing

This module provides the main automation controller that orchestrates HIL test
execution, hardware management, and integration with CI/CD pipelines.

Author: Cannasol Technologies
License: Proprietary
"""

import os
import sys
import json
import time
import logging
import argparse
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Any

# Add the project root to the path for imports
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root))

from test.acceptance.hil_framework.hil_controller import HILController
from test.acceptance.hil_framework.hardware_interface import HardwareInterface
from test.acceptance.hil_framework.logger import HILLogger
from test.hil.hil_test_runner import HILTestRunner
from test.hil.safety_interlocks import SafetyInterlocks
from test.hil.hardware_validation import HardwareValidator


class HILAutomationController:
    """Main HIL automation controller"""
    
    def __init__(self, config_file: Optional[str] = None):
        """Initialize HIL automation controller"""
        self.logger = HILLogger()
        self.config_file = config_file
        self.hil_controller = None
        self.test_runner = None
        self.safety_system = None
        self.hardware_validator = None
        
        # Automation configuration
        self.config = {
            'max_retry_attempts': 3,
            'retry_delay_seconds': 5,
            'test_timeout_seconds': 300,
            'hardware_setup_timeout': 60,
            'safety_check_interval': 10,
            'auto_recovery_enabled': True,
            'ci_mode': False
        }
        
        # Test execution state
        self.execution_state = {
            'current_test': None,
            'start_time': None,
            'hardware_ready': False,
            'safety_armed': False,
            'tests_completed': 0,
            'tests_failed': 0
        }
        
        self.logger.info("HIL Automation Controller initialized")
    
    def initialize_systems(self) -> bool:
        """Initialize all HIL systems"""
        self.logger.info("Initializing HIL systems")
        
        try:
            # Initialize HIL controller
            self.hil_controller = HILController(self.config_file or 'hil_config.yaml')

            # Initialize test runner
            self.test_runner = HILTestRunner(self.config_file or 'hil_config.yaml')
            
            # Initialize hardware validator
            self.hardware_validator = HardwareValidator()
            
            # Initialize safety system
            if self.hil_controller.hardware_interface:
                self.safety_system = SafetyInterlocks(self.hil_controller.hardware_interface)
                self.safety_system.start_safety_monitoring()
                self.execution_state['safety_armed'] = True
            
            self.logger.info("HIL systems initialized successfully")
            return True
            
        except Exception as e:
            self.logger.error(f"Failed to initialize HIL systems: {e}")
            return False
    
    def setup_hardware(self) -> bool:
        """Setup and validate hardware"""
        self.logger.info("Setting up hardware")
        
        try:
            # Initialize hardware
            if not self.hil_controller.setup_hardware():
                self.logger.error("Failed to initialize hardware")
                return False
            
            # Validate hardware configuration
            pin_matrix_valid, pin_matrix_result = self.hardware_validator.validate_pin_matrix()
            if not pin_matrix_valid:
                self.logger.error("Pin matrix validation failed")
                return False
            
            # Validate hardware connectivity
            if self.hil_controller.hardware_interface:
                connectivity_valid, connectivity_result = self.hardware_validator.validate_hardware_connectivity(
                    self.hil_controller.hardware_interface
                )
                if not connectivity_valid:
                    self.logger.error("Hardware connectivity validation failed")
                    return False
            
            self.execution_state['hardware_ready'] = True
            self.logger.info("Hardware setup completed successfully")
            return True
            
        except Exception as e:
            self.logger.error(f"Hardware setup failed: {e}")
            return False
    
    def run_test_suite(self, suite_name: str = 'basic') -> Dict[str, Any]:
        """Run a complete test suite with automation"""
        self.logger.info(f"Running automated test suite: {suite_name}")
        self.execution_state['start_time'] = datetime.now()
        
        test_results = {
            'automation_run_id': f"auto_{self.execution_state['start_time'].strftime('%Y%m%d_%H%M%S')}",
            'suite_name': suite_name,
            'start_time': self.execution_state['start_time'].isoformat(),
            'automation_config': self.config.copy(),
            'test_results': None,
            'hardware_validation': None,
            'safety_report': None
        }
        
        try:
            # Pre-test safety check
            if not self._perform_safety_check():
                test_results['status'] = 'ABORTED'
                test_results['error'] = 'Safety check failed'
                return test_results
            
            # Run hardware validation
            validation_report = self.hardware_validator.generate_validation_report()
            test_results['hardware_validation'] = validation_report
            
            if validation_report['summary']['overall_status'] != 'PASS':
                self.logger.warning("Hardware validation issues detected, proceeding with caution")
            
            # Run the test suite
            suite_results = self.test_runner.run_test_suite(suite_name)
            test_results['test_results'] = suite_results
            
            # Update execution state
            if 'summary' in suite_results:
                self.execution_state['tests_completed'] = suite_results['summary']['total_tests']
                self.execution_state['tests_failed'] = suite_results['summary']['failed_tests']
            
            # Generate safety report
            if self.safety_system:
                safety_report = self.safety_system.generate_safety_report()
                test_results['safety_report'] = safety_report
            
            # Determine overall status
            test_success = suite_results.get('summary', {}).get('failed_tests', 1) == 0
            hardware_success = validation_report['summary']['overall_status'] == 'PASS'
            safety_success = test_results.get('safety_report', {}).get('safety_status', {}).get('safety_state') == 'safe'
            
            if test_success and hardware_success and safety_success:
                test_results['status'] = 'SUCCESS'
            else:
                test_results['status'] = 'FAILURE'
            
            test_results['end_time'] = datetime.now().isoformat()
            
            self.logger.info(f"Automated test suite completed: {test_results['status']}")
            
        except Exception as e:
            self.logger.error(f"Automated test suite failed: {e}")
            test_results['status'] = 'ERROR'
            test_results['error'] = str(e)
            test_results['end_time'] = datetime.now().isoformat()
        
        finally:
            # Post-test cleanup
            self._cleanup_after_tests()
        
        return test_results
    
    def _perform_safety_check(self) -> bool:
        """Perform pre-test safety check"""
        self.logger.info("Performing safety check")
        
        try:
            if not self.safety_system:
                self.logger.warning("Safety system not available")
                return True  # Continue without safety system in development
            
            # Check safety system status
            safety_status = self.safety_system.get_safety_status()
            
            if safety_status['emergency_stop_active']:
                self.logger.error("Emergency stop is active - cannot proceed with tests")
                return False
            
            if safety_status['safety_state'] not in ['safe', 'unknown']:
                self.logger.error(f"Safety system not in safe state: {safety_status['safety_state']}")
                return False
            
            # Test emergency stop functionality
            emergency_test = self.safety_system.test_emergency_stop_response()
            if not emergency_test['passed']:
                self.logger.error("Emergency stop test failed")
                return False
            
            self.logger.info("Safety check passed")
            return True
            
        except Exception as e:
            self.logger.error(f"Safety check failed: {e}")
            return False
    
    def _cleanup_after_tests(self):
        """Cleanup after test execution"""
        try:
            self.logger.info("Performing post-test cleanup")
            
            # Stop safety monitoring
            if self.safety_system:
                self.safety_system.stop_safety_monitoring()
            
            # Disconnect hardware
            if self.hil_controller and self.hil_controller.hardware_interface:
                self.hil_controller.cleanup()
            
            # Reset execution state
            self.execution_state['current_test'] = None
            self.execution_state['hardware_ready'] = False
            
            self.logger.info("Post-test cleanup completed")
            
        except Exception as e:
            self.logger.error(f"Post-test cleanup failed: {e}")
    
    def run_continuous_testing(self, interval_minutes: int = 60) -> None:
        """Run continuous testing at specified intervals"""
        self.logger.info(f"Starting continuous testing (interval: {interval_minutes} minutes)")
        
        try:
            while True:
                self.logger.info("Starting continuous test cycle")
                
                # Initialize systems
                if not self.initialize_systems():
                    self.logger.error("System initialization failed, retrying in next cycle")
                    time.sleep(interval_minutes * 60)
                    continue
                
                # Setup hardware
                if not self.setup_hardware():
                    self.logger.error("Hardware setup failed, retrying in next cycle")
                    time.sleep(interval_minutes * 60)
                    continue
                
                # Run test suite
                results = self.run_test_suite('basic')
                
                # Log results
                if results['status'] == 'SUCCESS':
                    self.logger.info("Continuous test cycle completed successfully")
                else:
                    self.logger.warning(f"Continuous test cycle failed: {results['status']}")
                
                # Wait for next cycle
                self.logger.info(f"Waiting {interval_minutes} minutes for next test cycle")
                time.sleep(interval_minutes * 60)
                
        except KeyboardInterrupt:
            self.logger.info("Continuous testing stopped by user")
        except Exception as e:
            self.logger.error(f"Continuous testing failed: {e}")
    
    def run_ci_pipeline(self) -> int:
        """Run HIL tests as part of CI/CD pipeline"""
        self.logger.info("Running HIL tests for CI/CD pipeline")
        self.config['ci_mode'] = True
        
        try:
            # Initialize systems
            if not self.initialize_systems():
                self.logger.error("CI: System initialization failed")
                return 1
            
            # Setup hardware
            if not self.setup_hardware():
                self.logger.error("CI: Hardware setup failed")
                return 1
            
            # Run test suite
            results = self.run_test_suite('basic')
            
            # Generate CI reports
            self._generate_ci_reports(results)
            
            # Return appropriate exit code
            if results['status'] == 'SUCCESS':
                self.logger.info("CI: HIL tests passed")
                return 0
            else:
                self.logger.error(f"CI: HIL tests failed - {results['status']}")
                return 1
                
        except Exception as e:
            self.logger.error(f"CI pipeline failed: {e}")
            return 1
    
    def _generate_ci_reports(self, results: Dict[str, Any]):
        """Generate reports for CI/CD pipeline"""
        try:
            # Save results to CI artifacts directory
            ci_dir = project_root / 'ci_artifacts'
            ci_dir.mkdir(exist_ok=True)
            
            # Save test results
            results_file = ci_dir / 'hil_test_results.json'
            with open(results_file, 'w') as f:
                json.dump(results, f, indent=2)
            
            # Generate summary report
            summary_file = ci_dir / 'hil_test_summary.txt'
            with open(summary_file, 'w') as f:
                f.write(f"HIL Test Results Summary\n")
                f.write(f"========================\n")
                f.write(f"Status: {results['status']}\n")
                f.write(f"Suite: {results['suite_name']}\n")
                f.write(f"Start Time: {results['start_time']}\n")
                f.write(f"End Time: {results.get('end_time', 'N/A')}\n")
                
                if 'test_results' in results and 'summary' in results['test_results']:
                    summary = results['test_results']['summary']
                    f.write(f"Total Tests: {summary['total_tests']}\n")
                    f.write(f"Passed: {summary['passed_tests']}\n")
                    f.write(f"Failed: {summary['failed_tests']}\n")
                    f.write(f"Success Rate: {summary['success_rate']:.1f}%\n")
            
            self.logger.info(f"CI reports generated in {ci_dir}")
            
        except Exception as e:
            self.logger.error(f"Failed to generate CI reports: {e}")


def main():
    """Main entry point for HIL automation"""
    parser = argparse.ArgumentParser(description='HIL Automation Controller')
    parser.add_argument('--mode', choices=['single', 'continuous', 'ci'], default='single',
                       help='Execution mode')
    parser.add_argument('--suite', default='basic', help='Test suite to run')
    parser.add_argument('--config', help='HIL configuration file')
    parser.add_argument('--interval', type=int, default=60, 
                       help='Interval for continuous testing (minutes)')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose logging')
    
    args = parser.parse_args()
    
    # Configure logging
    log_level = logging.DEBUG if args.verbose else logging.INFO
    logging.basicConfig(level=log_level, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    
    # Create automation controller
    controller = HILAutomationController(args.config)
    
    try:
        if args.mode == 'single':
            # Initialize systems
            if not controller.initialize_systems():
                sys.exit(1)
            
            # Setup hardware
            if not controller.setup_hardware():
                sys.exit(1)
            
            # Run test suite
            results = controller.run_test_suite(args.suite)
            
            # Print summary
            print(f"\nHIL Automation Results:")
            print(f"  Status: {results['status']}")
            print(f"  Suite: {results['suite_name']}")
            
            if 'test_results' in results and 'summary' in results['test_results']:
                summary = results['test_results']['summary']
                print(f"  Tests: {summary['passed_tests']}/{summary['total_tests']} passed")
                print(f"  Success Rate: {summary['success_rate']:.1f}%")
            
            # Exit with appropriate code
            sys.exit(0 if results['status'] == 'SUCCESS' else 1)
            
        elif args.mode == 'continuous':
            controller.run_continuous_testing(args.interval)
            
        elif args.mode == 'ci':
            exit_code = controller.run_ci_pipeline()
            sys.exit(exit_code)
            
    except KeyboardInterrupt:
        print("\nHIL automation stopped by user")
        sys.exit(0)
    except Exception as e:
        print(f"HIL automation failed: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
