#!/usr/bin/env python3
"""
CI/CD HIL Integration - Integration script for HIL testing in CI/CD pipelines

This module provides integration between HIL testing and CI/CD pipelines,
including automated test execution, result reporting, and build gate integration.

Author: Cannasol Technologies
License: Proprietary
"""

import os
import sys
import json
import time
import logging
import subprocess
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Optional, Any

# Add the project root to the path for imports
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root))


class CIHILIntegration:
    """CI/CD HIL integration controller"""
    
    def __init__(self):
        """Initialize CI/CD HIL integration"""
        self.logger = logging.getLogger(__name__)
        
        # CI/CD configuration
        self.config = {
            'hil_test_timeout': 300,        # 5 minutes timeout for HIL tests
            'hardware_check_timeout': 60,   # 1 minute timeout for hardware check
            'retry_attempts': 2,            # Number of retry attempts
            'retry_delay': 30,              # Delay between retries (seconds)
            'required_success_rate': 90.0,  # Minimum success rate for passing
            'artifact_retention_days': 30   # Artifact retention period
        }
        
        # CI environment detection
        self.ci_environment = self._detect_ci_environment()
        
        # Paths
        self.artifacts_dir = project_root / 'ci_artifacts'
        self.reports_dir = project_root / 'test' / 'data' / 'reports'
        
        # Ensure directories exist
        self.artifacts_dir.mkdir(exist_ok=True)
        self.reports_dir.mkdir(parents=True, exist_ok=True)
        
        self.logger.info(f"CI/CD HIL Integration initialized (Environment: {self.ci_environment})")
    
    def _detect_ci_environment(self) -> str:
        """Detect CI/CD environment"""
        if os.getenv('GITHUB_ACTIONS'):
            return 'github_actions'
        elif os.getenv('JENKINS_URL'):
            return 'jenkins'
        elif os.getenv('GITLAB_CI'):
            return 'gitlab_ci'
        elif os.getenv('TRAVIS'):
            return 'travis_ci'
        elif os.getenv('CIRCLECI'):
            return 'circle_ci'
        else:
            return 'local'
    
    def check_hardware_availability(self) -> Dict[str, Any]:
        """Check if HIL hardware is available for testing"""
        self.logger.info("Checking HIL hardware availability")
        
        check_result = {
            'timestamp': datetime.now().isoformat(),
            'hardware_available': False,
            'checks': []
        }
        
        try:
            # Check for Arduino hardware
            arduino_check = self._check_arduino_availability()
            check_result['checks'].append(arduino_check)
            
            # Check for serial ports
            port_check = self._check_serial_ports()
            check_result['checks'].append(port_check)
            
            # Check for HIL configuration
            config_check = self._check_hil_configuration()
            check_result['checks'].append(config_check)
            
            # Determine overall availability
            failed_checks = [c for c in check_result['checks'] if not c['passed']]
            check_result['hardware_available'] = len(failed_checks) == 0
            
            if check_result['hardware_available']:
                self.logger.info("HIL hardware is available for testing")
            else:
                self.logger.warning(f"HIL hardware not available: {len(failed_checks)} checks failed")
            
        except Exception as e:
            self.logger.error(f"Hardware availability check failed: {e}")
            check_result['error'] = str(e)
        
        return check_result
    
    def _check_arduino_availability(self) -> Dict[str, Any]:
        """Check Arduino availability"""
        check = {
            'name': 'arduino_availability',
            'passed': False,
            'message': ''
        }
        
        try:
            # Try to detect Arduino using the existing script
            detect_script = project_root / 'scripts' / 'detect_hardware.py'
            if detect_script.exists():
                result = subprocess.run([sys.executable, str(detect_script)], 
                                      capture_output=True, text=True, timeout=30)
                if result.returncode == 0:
                    check['passed'] = True
                    check['message'] = 'Arduino hardware detected'
                else:
                    check['message'] = f'Arduino detection failed: {result.stderr}'
            else:
                check['message'] = 'Hardware detection script not found'
                
        except subprocess.TimeoutExpired:
            check['message'] = 'Arduino detection timed out'
        except Exception as e:
            check['message'] = f'Arduino detection error: {e}'
        
        return check
    
    def _check_serial_ports(self) -> Dict[str, Any]:
        """Check serial port availability"""
        check = {
            'name': 'serial_ports',
            'passed': False,
            'message': ''
        }
        
        try:
            # Basic check for common serial port patterns
            import glob
            
            # Common serial port patterns
            port_patterns = [
                '/dev/ttyUSB*',
                '/dev/ttyACM*',
                '/dev/cu.usbmodem*',
                '/dev/cu.usbserial*'
            ]
            
            available_ports = []
            for pattern in port_patterns:
                available_ports.extend(glob.glob(pattern))
            
            if available_ports:
                check['passed'] = True
                check['message'] = f'Serial ports available: {available_ports}'
            else:
                check['message'] = 'No serial ports detected'
                
        except Exception as e:
            check['message'] = f'Serial port check error: {e}'
        
        return check
    
    def _check_hil_configuration(self) -> Dict[str, Any]:
        """Check HIL configuration availability"""
        check = {
            'name': 'hil_configuration',
            'passed': False,
            'message': ''
        }
        
        try:
            # Check for hardware configuration file
            config_file = project_root / 'config' / 'hardware-config.yaml'
            if config_file.exists():
                check['passed'] = True
                check['message'] = 'HIL configuration file found'
            else:
                check['message'] = 'HIL configuration file not found'
                
        except Exception as e:
            check['message'] = f'Configuration check error: {e}'
        
        return check
    
    def run_hil_tests_for_ci(self) -> Dict[str, Any]:
        """Run HIL tests for CI/CD pipeline"""
        self.logger.info("Running HIL tests for CI/CD pipeline")
        
        ci_result = {
            'ci_run_id': f"ci_{datetime.now().strftime('%Y%m%d_%H%M%S')}",
            'environment': self.ci_environment,
            'start_time': datetime.now().isoformat(),
            'status': 'RUNNING',
            'hardware_check': None,
            'test_results': None,
            'artifacts': []
        }
        
        try:
            # Check hardware availability
            hardware_check = self.check_hardware_availability()
            ci_result['hardware_check'] = hardware_check
            
            if not hardware_check['hardware_available']:
                ci_result['status'] = 'SKIPPED'
                ci_result['message'] = 'HIL hardware not available - tests skipped'
                self.logger.warning("HIL tests skipped - hardware not available")
                return ci_result
            
            # Run HIL tests with retry logic
            test_results = None
            for attempt in range(self.config['retry_attempts'] + 1):
                try:
                    self.logger.info(f"Running HIL tests (attempt {attempt + 1})")
                    
                    # Run the HIL automation script
                    automation_script = project_root / 'test' / 'scripts' / 'hil_automation.py'
                    if automation_script.exists():
                        result = subprocess.run([
                            sys.executable, str(automation_script),
                            '--mode', 'ci',
                            '--suite', 'basic'
                        ], capture_output=True, text=True, timeout=self.config['hil_test_timeout'])
                        
                        if result.returncode == 0:
                            # Load test results
                            results_file = self.artifacts_dir / 'hil_test_results.json'
                            if results_file.exists():
                                with open(results_file, 'r') as f:
                                    test_results = json.load(f)
                            break
                        else:
                            self.logger.warning(f"HIL test attempt {attempt + 1} failed: {result.stderr}")
                    else:
                        self.logger.error("HIL automation script not found")
                        break
                        
                except subprocess.TimeoutExpired:
                    self.logger.warning(f"HIL test attempt {attempt + 1} timed out")
                
                # Wait before retry
                if attempt < self.config['retry_attempts']:
                    time.sleep(self.config['retry_delay'])
            
            ci_result['test_results'] = test_results
            
            # Determine CI status
            if test_results:
                if test_results.get('status') == 'SUCCESS':
                    # Check success rate
                    summary = test_results.get('test_results', {}).get('summary', {})
                    success_rate = summary.get('success_rate', 0)
                    
                    if success_rate >= self.config['required_success_rate']:
                        ci_result['status'] = 'PASS'
                    else:
                        ci_result['status'] = 'FAIL'
                        ci_result['message'] = f'Success rate {success_rate:.1f}% below required {self.config["required_success_rate"]}%'
                else:
                    ci_result['status'] = 'FAIL'
                    ci_result['message'] = f'HIL tests failed: {test_results.get("status")}'
            else:
                ci_result['status'] = 'ERROR'
                ci_result['message'] = 'No test results available'
            
            # Collect artifacts
            ci_result['artifacts'] = self._collect_ci_artifacts()
            
            ci_result['end_time'] = datetime.now().isoformat()
            
            self.logger.info(f"CI HIL tests completed: {ci_result['status']}")
            
        except Exception as e:
            self.logger.error(f"CI HIL tests failed: {e}")
            ci_result['status'] = 'ERROR'
            ci_result['error'] = str(e)
            ci_result['end_time'] = datetime.now().isoformat()
        
        # Save CI results
        self._save_ci_results(ci_result)
        
        return ci_result
    
    def _collect_ci_artifacts(self) -> List[str]:
        """Collect CI artifacts"""
        artifacts = []
        
        try:
            # Collect test results
            if (self.artifacts_dir / 'hil_test_results.json').exists():
                artifacts.append('hil_test_results.json')
            
            if (self.artifacts_dir / 'hil_test_summary.txt').exists():
                artifacts.append('hil_test_summary.txt')
            
            # Collect test data
            test_data_dir = project_root / 'test' / 'data' / 'results' / 'current'
            if test_data_dir.exists():
                for result_file in test_data_dir.glob('*.json'):
                    # Copy to artifacts directory
                    artifact_file = self.artifacts_dir / result_file.name
                    artifact_file.write_text(result_file.read_text())
                    artifacts.append(result_file.name)
            
            self.logger.info(f"Collected {len(artifacts)} CI artifacts")
            
        except Exception as e:
            self.logger.error(f"Failed to collect CI artifacts: {e}")
        
        return artifacts
    
    def _save_ci_results(self, ci_result: Dict[str, Any]):
        """Save CI results"""
        try:
            # Save to artifacts directory
            results_file = self.artifacts_dir / 'ci_hil_results.json'
            with open(results_file, 'w') as f:
                json.dump(ci_result, f, indent=2)
            
            # Generate CI summary
            summary_file = self.artifacts_dir / 'ci_hil_summary.txt'
            with open(summary_file, 'w') as f:
                f.write("HIL CI/CD Integration Results\n")
                f.write("============================\n")
                f.write(f"Status: {ci_result['status']}\n")
                f.write(f"Environment: {ci_result['environment']}\n")
                f.write(f"Start Time: {ci_result['start_time']}\n")
                f.write(f"End Time: {ci_result.get('end_time', 'N/A')}\n")
                
                if ci_result.get('hardware_check'):
                    hw_check = ci_result['hardware_check']
                    f.write(f"Hardware Available: {hw_check['hardware_available']}\n")
                
                if ci_result.get('test_results'):
                    test_results = ci_result['test_results']
                    if 'test_results' in test_results and 'summary' in test_results['test_results']:
                        summary = test_results['test_results']['summary']
                        f.write(f"Tests Passed: {summary['passed_tests']}/{summary['total_tests']}\n")
                        f.write(f"Success Rate: {summary['success_rate']:.1f}%\n")
                
                if ci_result.get('message'):
                    f.write(f"Message: {ci_result['message']}\n")
            
            self.logger.info(f"CI results saved to {results_file}")
            
        except Exception as e:
            self.logger.error(f"Failed to save CI results: {e}")
    
    def generate_build_gate_report(self, ci_result: Dict[str, Any]) -> bool:
        """Generate build gate report and return pass/fail status"""
        try:
            status = ci_result.get('status', 'ERROR')
            
            # Build gate logic
            if status == 'PASS':
                self.logger.info("BUILD GATE: PASS - HIL tests successful")
                return True
            elif status == 'SKIPPED':
                self.logger.info("BUILD GATE: PASS - HIL tests skipped (hardware not available)")
                return True  # Don't fail build if hardware not available
            else:
                self.logger.error(f"BUILD GATE: FAIL - HIL tests {status}")
                return False
                
        except Exception as e:
            self.logger.error(f"Build gate report generation failed: {e}")
            return False


def main():
    """Main entry point for CI/CD HIL integration"""
    import argparse
    
    parser = argparse.ArgumentParser(description='CI/CD HIL Integration')
    parser.add_argument('--action', choices=['check', 'test', 'gate'], default='test',
                       help='Action to perform')
    parser.add_argument('--verbose', '-v', action='store_true', help='Verbose logging')
    
    args = parser.parse_args()
    
    # Configure logging
    log_level = logging.DEBUG if args.verbose else logging.INFO
    logging.basicConfig(level=log_level, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    
    # Create CI integration
    ci_integration = CIHILIntegration()
    
    try:
        if args.action == 'check':
            # Check hardware availability
            result = ci_integration.check_hardware_availability()
            print(f"Hardware Available: {result['hardware_available']}")
            sys.exit(0 if result['hardware_available'] else 1)
            
        elif args.action == 'test':
            # Run HIL tests for CI
            result = ci_integration.run_hil_tests_for_ci()
            print(f"CI HIL Tests: {result['status']}")
            
            # Generate build gate report
            gate_pass = ci_integration.generate_build_gate_report(result)
            sys.exit(0 if gate_pass else 1)
            
        elif args.action == 'gate':
            # Load existing results and check build gate
            results_file = ci_integration.artifacts_dir / 'ci_hil_results.json'
            if results_file.exists():
                with open(results_file, 'r') as f:
                    result = json.load(f)
                gate_pass = ci_integration.generate_build_gate_report(result)
                sys.exit(0 if gate_pass else 1)
            else:
                print("No CI results found")
                sys.exit(1)
                
    except Exception as e:
        print(f"CI HIL integration failed: {e}")
        sys.exit(1)


if __name__ == '__main__':
    main()
