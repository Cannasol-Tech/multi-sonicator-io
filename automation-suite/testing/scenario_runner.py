#!/usr/bin/env python3
"""
Generic Test Scenario Runner
Executes automated test scenarios with hardware simulation support
"""

import os
import sys
import json
import yaml
import time
import random
import argparse
import subprocess
from pathlib import Path
from datetime import datetime
from enum import Enum
import threading
import queue

class TestResult(Enum):
    PASS = "PASS"
    FAIL = "FAIL"
    SKIP = "SKIP"
    ERROR = "ERROR"

class TestScenario:
    def __init__(self, name, description, steps, config=None):
        self.name = name
        self.description = description
        self.steps = steps
        self.config = config or {}
        self.result = None
        self.start_time = None
        self.end_time = None
        self.duration = None
        self.output = []
        self.error_details = None
    
    def log(self, message):
        """Log a message with timestamp"""
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        log_entry = f"[{timestamp}] {message}"
        self.output.append(log_entry)
        print(f"  {log_entry}")
    
    def execute(self, runner_context):
        """Execute the test scenario"""
        self.start_time = time.time()
        self.log(f"Starting scenario: {self.name}")
        
        try:
            for i, step in enumerate(self.steps, 1):
                self.log(f"Step {i}: {step['description']}")
                
                if not self._execute_step(step, runner_context):
                    self.result = TestResult.FAIL
                    self.log(f"Step {i} failed")
                    break
                    
                self.log(f"Step {i} completed successfully")
                
                # Add delay if specified
                if 'delay' in step:
                    time.sleep(step['delay'])
            
            if self.result is None:
                self.result = TestResult.PASS
                self.log("Scenario completed successfully")
                
        except Exception as e:
            self.result = TestResult.ERROR
            self.error_details = str(e)
            self.log(f"Scenario error: {e}")
        
        self.end_time = time.time()
        self.duration = self.end_time - self.start_time
        self.log(f"Scenario finished: {self.result.value} ({self.duration:.2f}s)")
        
        return self.result == TestResult.PASS
    
    def _execute_step(self, step, runner_context):
        """Execute a single test step"""
        step_type = step.get('type', 'command')
        
        if step_type == 'command':
            return self._execute_command_step(step, runner_context)
        elif step_type == 'api_call':
            return self._execute_api_step(step, runner_context)
        elif step_type == 'validation':
            return self._execute_validation_step(step, runner_context)
        elif step_type == 'simulation':
            return self._execute_simulation_step(step, runner_context)
        elif step_type == 'custom':
            return self._execute_custom_step(step, runner_context)
        else:
            raise ValueError(f"Unknown step type: {step_type}")
    
    def _execute_command_step(self, step, runner_context):
        """Execute a command step"""
        command = step['command']
        timeout = step.get('timeout', 30)
        expected_return_code = step.get('expected_return_code', 0)
        
        try:
            result = subprocess.run(
                command.split() if isinstance(command, str) else command,
                capture_output=True,
                text=True,
                timeout=timeout,
                cwd=runner_context.get('working_directory', '.')
            )
            
            if result.returncode != expected_return_code:
                self.log(f"Command failed with return code {result.returncode}")
                self.log(f"stdout: {result.stdout}")
                self.log(f"stderr: {result.stderr}")
                return False
            
            # Check expected output if specified
            if 'expected_output' in step:
                if step['expected_output'] not in result.stdout:
                    self.log(f"Expected output not found: {step['expected_output']}")
                    return False
            
            return True
            
        except subprocess.TimeoutExpired:
            self.log(f"Command timed out after {timeout}s")
            return False
        except Exception as e:
            self.log(f"Command execution error: {e}")
            return False
    
    def _execute_api_step(self, step, runner_context):
        """Execute an API call step"""
        # Placeholder for API call implementation
        # This would integrate with project-specific APIs
        self.log("API call step - implementation needed")
        return True
    
    def _execute_validation_step(self, step, runner_context):
        """Execute a validation step"""
        validation_type = step.get('validation_type', 'file_exists')
        
        if validation_type == 'file_exists':
            file_path = step['file_path']
            exists = Path(file_path).exists()
            if not exists:
                self.log(f"File does not exist: {file_path}")
            return exists
        
        elif validation_type == 'environment_variable':
            var_name = step['variable_name']
            expected_value = step.get('expected_value')
            actual_value = os.getenv(var_name)
            
            if expected_value is not None:
                match = actual_value == expected_value
                if not match:
                    self.log(f"Environment variable mismatch: {var_name} = {actual_value}, expected {expected_value}")
                return match
            else:
                exists = actual_value is not None
                if not exists:
                    self.log(f"Environment variable not set: {var_name}")
                return exists
        
        else:
            self.log(f"Unknown validation type: {validation_type}")
            return False
    
    def _execute_simulation_step(self, step, runner_context):
        """Execute a simulation step"""
        sim_type = step.get('simulation_type', 'delay')
        
        if sim_type == 'delay':
            delay = step.get('duration', 1.0)
            self.log(f"Simulating delay of {delay}s")
            time.sleep(delay)
            return True
        
        elif sim_type == 'random_failure':
            failure_rate = step.get('failure_rate', 0.1)
            if random.random() < failure_rate:
                self.log(f"Simulated random failure (rate: {failure_rate})")
                return False
            return True
        
        elif sim_type == 'resource_usage':
            # Simulate resource usage
            self.log("Simulating resource usage")
            return True
        
        else:
            self.log(f"Unknown simulation type: {sim_type}")
            return False
    
    def _execute_custom_step(self, step, runner_context):
        """Execute a custom step (extensibility point)"""
        custom_function = step.get('function')
        if custom_function and custom_function in runner_context.get('custom_functions', {}):
            func = runner_context['custom_functions'][custom_function]
            return func(step, self, runner_context)
        else:
            self.log(f"Custom function not found: {custom_function}")
            return False

class TestScenarioRunner:
    def __init__(self, config_path=None):
        """Initialize test scenario runner"""
        self.config = self.load_config(config_path)
        self.scenarios = []
        self.results = []
        self.context = {
            'working_directory': Path.cwd(),
            'environment': self.get_environment_info(),
            'custom_functions': {}
        }
    
    def load_config(self, config_path):
        """Load configuration from file or use defaults"""
        default_config = {
            'execution': {
                'parallel': False,
                'max_workers': 4,
                'continue_on_failure': True,
                'timeout_per_scenario': 300
            },
            'reporting': {
                'detailed_output': True,
                'save_logs': True,
                'output_format': 'json'
            },
            'simulation': {
                'enabled': False,
                'fault_injection': False,
                'random_delays': False
            }
        }
        
        if config_path and Path(config_path).exists():
            with open(config_path, 'r') as f:
                if config_path.endswith('.yaml') or config_path.endswith('.yml'):
                    user_config = yaml.safe_load(f)
                else:
                    user_config = json.load(f)
            default_config.update(user_config)
            
        return default_config
    
    def get_environment_info(self):
        """Get environment information"""
        import platform
        return {
            'platform': platform.platform(),
            'python_version': platform.python_version(),
            'timestamp': datetime.now().isoformat(),
            'simulation_enabled': os.getenv('HARDWARE_SIMULATION', 'false').lower() == 'true'
        }
    
    def load_scenarios_from_file(self, scenarios_file):
        """Load test scenarios from a file"""
        with open(scenarios_file, 'r') as f:
            if scenarios_file.endswith('.yaml') or scenarios_file.endswith('.yml'):
                scenarios_data = yaml.safe_load(f)
            else:
                scenarios_data = json.load(f)
        
        for scenario_data in scenarios_data.get('scenarios', []):
            scenario = TestScenario(
                name=scenario_data['name'],
                description=scenario_data['description'],
                steps=scenario_data['steps'],
                config=scenario_data.get('config', {})
            )
            self.scenarios.append(scenario)
    
    def add_scenario(self, scenario):
        """Add a test scenario"""
        self.scenarios.append(scenario)
    
    def register_custom_function(self, name, function):
        """Register a custom function for use in scenarios"""
        self.context['custom_functions'][name] = function
    
    def run_scenario(self, scenario):
        """Run a single scenario"""
        print(f"\n🧪 Running scenario: {scenario.name}")
        print(f"   Description: {scenario.description}")
        
        success = scenario.execute(self.context)
        self.results.append(scenario)
        
        # Print result
        status_icon = "✅" if success else "❌"
        print(f"{status_icon} {scenario.name}: {scenario.result.value} ({scenario.duration:.2f}s)")
        
        return success
    
    def run_all_scenarios(self):
        """Run all loaded scenarios"""
        if not self.scenarios:
            print("No scenarios to run")
            return True
        
        print(f"\n🚀 Starting test execution ({len(self.scenarios)} scenarios)")
        print("="*60)
        
        start_time = time.time()
        total_scenarios = len(self.scenarios)
        passed_scenarios = 0
        
        if self.config['execution']['parallel']:
            passed_scenarios = self._run_scenarios_parallel()
        else:
            for scenario in self.scenarios:
                if self.run_scenario(scenario):
                    passed_scenarios += 1
                elif not self.config['execution']['continue_on_failure']:
                    break
        
        end_time = time.time()
        total_duration = end_time - start_time
        
        # Print summary
        print("\n" + "="*60)
        print("TEST EXECUTION SUMMARY")
        print("="*60)
        print(f"Total scenarios: {total_scenarios}")
        print(f"Passed: {passed_scenarios}")
        print(f"Failed: {total_scenarios - passed_scenarios}")
        print(f"Success rate: {(passed_scenarios/total_scenarios)*100:.1f}%")
        print(f"Total duration: {total_duration:.2f}s")
        
        return passed_scenarios == total_scenarios
    
    def _run_scenarios_parallel(self):
        """Run scenarios in parallel"""
        import concurrent.futures
        
        max_workers = self.config['execution']['max_workers']
        passed_scenarios = 0
        
        with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
            # Submit all scenarios
            future_to_scenario = {
                executor.submit(self.run_scenario, scenario): scenario 
                for scenario in self.scenarios
            }
            
            # Collect results
            for future in concurrent.futures.as_completed(future_to_scenario):
                scenario = future_to_scenario[future]
                try:
                    if future.result():
                        passed_scenarios += 1
                except Exception as e:
                    print(f"Scenario {scenario.name} generated an exception: {e}")
        
        return passed_scenarios
    
    def generate_report(self, output_path='test_results.json'):
        """Generate test execution report"""
        report = {
            'metadata': {
                'generated_at': datetime.now().isoformat(),
                'config': self.config,
                'environment': self.context['environment']
            },
            'summary': {
                'total_scenarios': len(self.results),
                'passed': len([r for r in self.results if r.result == TestResult.PASS]),
                'failed': len([r for r in self.results if r.result == TestResult.FAIL]),
                'errors': len([r for r in self.results if r.result == TestResult.ERROR]),
                'skipped': len([r for r in self.results if r.result == TestResult.SKIP]),
                'total_duration': sum([r.duration or 0 for r in self.results])
            },
            'scenarios': []
        }
        
        # Add scenario details
        for scenario in self.results:
            scenario_data = {
                'name': scenario.name,
                'description': scenario.description,
                'result': scenario.result.value,
                'duration': scenario.duration,
                'start_time': scenario.start_time,
                'end_time': scenario.end_time,
                'output': scenario.output if self.config['reporting']['detailed_output'] else [],
                'error_details': scenario.error_details
            }
            report['scenarios'].append(scenario_data)
        
        # Save report
        with open(output_path, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"\nDetailed report saved to: {output_path}")
        return report

def create_example_scenarios():
    """Create example test scenarios"""
    scenarios = [
        TestScenario(
            name="BASIC_SYSTEM_CHECK",
            description="Verify basic system functionality",
            steps=[
                {
                    'type': 'validation',
                    'description': 'Check if required files exist',
                    'validation_type': 'file_exists',
                    'file_path': '.'
                },
                {
                    'type': 'command',
                    'description': 'Run basic system command',
                    'command': 'echo "System check successful"',
                    'expected_output': 'System check successful'
                }
            ]
        ),
        TestScenario(
            name="BUILD_VERIFICATION",
            description="Verify project builds successfully",
            steps=[
                {
                    'type': 'command',
                    'description': 'Clean previous build',
                    'command': 'echo "Cleaning build"',
                    'timeout': 30
                },
                {
                    'type': 'command',
                    'description': 'Build project',
                    'command': 'echo "Building project"',
                    'timeout': 300
                },
                {
                    'type': 'validation',
                    'description': 'Verify build output',
                    'validation_type': 'file_exists',
                    'file_path': '.'
                }
            ]
        ),
        TestScenario(
            name="SIMULATION_TEST",
            description="Test simulation functionality",
            steps=[
                {
                    'type': 'simulation',
                    'description': 'Simulate device initialization',
                    'simulation_type': 'delay',
                    'duration': 1.0
                },
                {
                    'type': 'simulation',
                    'description': 'Random failure test',
                    'simulation_type': 'random_failure',
                    'failure_rate': 0.05
                }
            ]
        )
    ]
    
    return scenarios

def main():
    parser = argparse.ArgumentParser(description='Generic Test Scenario Runner')
    parser.add_argument('--config', '-c', help='Configuration file path')
    parser.add_argument('--scenarios', '-s', help='Scenarios file path')
    parser.add_argument('--output', '-o', help='Output report file', default='test_results.json')
    parser.add_argument('--parallel', action='store_true', help='Run scenarios in parallel')
    parser.add_argument('--create-example', action='store_true', help='Create example scenarios file')
    
    args = parser.parse_args()
    
    # Create example scenarios file if requested
    if args.create_example:
        scenarios_data = {
            'scenarios': [
                {
                    'name': scenario.name,
                    'description': scenario.description,
                    'steps': scenario.steps
                }
                for scenario in create_example_scenarios()
            ]
        }
        
        with open('example_scenarios.yaml', 'w') as f:
            yaml.dump(scenarios_data, f, default_flow_style=False, indent=2)
        
        print("Example scenarios created in 'example_scenarios.yaml'")
        return
    
    # Initialize runner
    runner = TestScenarioRunner(config_path=args.config)
    
    # Override parallel setting if specified
    if args.parallel:
        runner.config['execution']['parallel'] = True
    
    # Load scenarios
    if args.scenarios:
        runner.load_scenarios_from_file(args.scenarios)
    else:
        # Use example scenarios if no file specified
        for scenario in create_example_scenarios():
            runner.add_scenario(scenario)
    
    # Run scenarios
    success = runner.run_all_scenarios()
    
    # Generate report
    runner.generate_report(args.output)
    
    # Exit with appropriate code
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()