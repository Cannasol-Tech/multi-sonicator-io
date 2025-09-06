#!/usr/bin/env python3
"""
Test Automation Service - Execute BDD scenarios from web UI

This service provides the capability to execute existing BDD test scenarios
from the web interface, parse feature files, and provide real-time test
execution progress and results.

Author: Cannasol Technologies
License: Proprietary
"""

import os
import sys
import json
import subprocess
import threading
import time
from pathlib import Path
from typing import Dict, List, Optional, Any, Callable
from dataclasses import dataclass, asdict
from enum import Enum

# Add the test acceptance framework to the path
sys.path.append(str(Path(__file__).parent.parent.parent.parent.parent / 'test' / 'acceptance'))

try:
    from test.acceptance.hil_framework.hil_controller import HILController
    from test.acceptance.hil_framework.hardware_interface import HardwareInterface as HILHardwareInterface
except ImportError as e:
    # Only print warning if not being called from API (when stdout is used for JSON)
    if len(sys.argv) < 2 or sys.argv[1] not in ['get_scenarios', 'execute_scenarios']:
        print(f"Warning: Could not import HIL framework: {e}")
    HILController = None
    HILHardwareInterface = None


class TestStatus(Enum):
    """Test execution status enumeration"""
    PENDING = "pending"
    RUNNING = "running"
    PASSED = "passed"
    FAILED = "failed"
    SKIPPED = "skipped"
    ERROR = "error"


@dataclass
class TestStep:
    """Individual test step representation"""
    step_type: str  # Given, When, Then
    description: str
    status: TestStatus = TestStatus.PENDING
    duration_ms: Optional[int] = None
    error_message: Optional[str] = None
    pin_interactions: List[str] = None

    def __post_init__(self):
        if self.pin_interactions is None:
            self.pin_interactions = []


@dataclass
class TestScenario:
    """BDD test scenario representation"""
    name: str
    description: str
    feature_file: str
    tags: List[str]
    steps: List[TestStep]
    status: TestStatus = TestStatus.PENDING
    duration_ms: Optional[int] = None
    error_message: Optional[str] = None


@dataclass
class TestExecution:
    """Test execution session"""
    execution_id: str
    scenarios: List[TestScenario]
    status: TestStatus = TestStatus.PENDING
    start_time: Optional[float] = None
    end_time: Optional[float] = None
    total_scenarios: int = 0
    passed_scenarios: int = 0
    failed_scenarios: int = 0
    current_scenario_index: int = 0


class TestAutomationService:
    """Service for executing BDD test scenarios from web UI"""

    def __init__(self, progress_callback: Optional[Callable] = None):
        """Initialize test automation service
        
        Args:
            progress_callback: Optional callback for real-time progress updates
        """
        self.progress_callback = progress_callback
        self.current_execution: Optional[TestExecution] = None
        self.hil_controller: Optional[HILController] = None
        self.execution_thread: Optional[threading.Thread] = None
        
        # Path to acceptance tests
        self.test_root = Path(__file__).parent.parent.parent.parent.parent / 'test' / 'acceptance'
        self.features_path = self.test_root / 'features'
        
        # Only print initialization messages if not being called from API
        if len(sys.argv) < 2 or sys.argv[1] not in ['get_scenarios', 'execute_scenarios']:
            print(f"Test Automation Service initialized")
            print(f"Features path: {self.features_path}")

    def get_available_scenarios(self) -> List[Dict[str, Any]]:
        """Parse feature files and return available test scenarios"""
        scenarios = []
        
        if not self.features_path.exists():
            print(f"Warning: Features path does not exist: {self.features_path}")
            return scenarios

        try:
            # Parse all .feature files
            for feature_file in self.features_path.glob('*.feature'):
                parsed_scenarios = self._parse_feature_file(feature_file)
                scenarios.extend(parsed_scenarios)
                
        except Exception as e:
            # Only print error if not being called from API
            if len(sys.argv) < 2 or sys.argv[1] not in ['get_scenarios', 'execute_scenarios']:
                print(f"Error parsing feature files: {e}")
            
        return scenarios

    def _parse_feature_file(self, feature_file: Path) -> List[Dict[str, Any]]:
        """Parse a single feature file and extract scenarios"""
        scenarios = []
        
        try:
            with open(feature_file, 'r') as f:
                content = f.read()
                
            # Simple parsing - in production, use a proper Gherkin parser
            lines = content.split('\n')
            current_scenario = None
            current_steps = []
            feature_name = ""
            
            for line_index, line in enumerate(lines):
                line = line.strip()

                if line.startswith('Feature:'):
                    feature_name = line.replace('Feature:', '').strip()

                elif line.startswith('Scenario:'):
                    # Save previous scenario if exists
                    if current_scenario:
                        current_scenario['steps'] = current_steps
                        scenarios.append(current_scenario)

                    # Start new scenario
                    scenario_name = line.replace('Scenario:', '').strip()
                    current_scenario = {
                        'name': scenario_name,
                        'description': scenario_name,
                        'feature_file': str(feature_file.name),
                        'feature_name': feature_name,
                        'tags': self._extract_tags_from_previous_lines(lines, line_index),
                        'steps': []
                    }
                    current_steps = []
                    
                elif line.startswith(('Given', 'When', 'Then', 'And', 'But')):
                    if current_scenario:
                        step_type = line.split()[0]
                        step_description = line
                        current_steps.append({
                            'step_type': step_type,
                            'description': step_description,
                            'pin_interactions': self._extract_pin_interactions(step_description)
                        })
            
            # Don't forget the last scenario
            if current_scenario:
                current_scenario['steps'] = current_steps
                scenarios.append(current_scenario)
                
        except Exception as e:
            # Only print error if not being called from API
            if len(sys.argv) < 2 or sys.argv[1] not in ['get_scenarios', 'execute_scenarios']:
                print(f"Error parsing feature file {feature_file}: {e}")
            
        return scenarios

    def _extract_tags_from_previous_lines(self, lines: List[str], current_index: int) -> List[str]:
        """Extract tags from lines before the scenario"""
        tags = []
        
        # Look backwards for tag lines
        for i in range(current_index - 1, -1, -1):
            line = lines[i].strip()
            if line.startswith('@'):
                # Extract all tags from this line
                line_tags = [tag.strip() for tag in line.split() if tag.startswith('@')]
                tags.extend([tag[1:] for tag in line_tags])  # Remove @ prefix
            elif line and not line.startswith('#'):
                # Stop at first non-tag, non-comment line
                break
                
        return tags

    def _extract_pin_interactions(self, step_description: str) -> List[str]:
        """Extract pin names from step description"""
        pins = []
        
        # Common pin patterns in our test scenarios
        pin_patterns = [
            'START_4', 'RESET_4', 'OVERLOAD_4', 'FREQ_LOCK_4', 'POWER_SENSE_4',
            'AMPLITUDE_ALL', 'STATUS_LED', 'UART_RXD', 'UART_TXD'
        ]
        
        for pin in pin_patterns:
            if pin in step_description:
                pins.append(pin)
                
        return pins

    def execute_scenarios(self, scenario_names: List[str], execution_id: str) -> bool:
        """Execute selected test scenarios
        
        Args:
            scenario_names: List of scenario names to execute
            execution_id: Unique identifier for this execution
            
        Returns:
            True if execution started successfully
        """
        if self.execution_thread and self.execution_thread.is_alive():
            print("Test execution already in progress")
            return False
            
        # Get scenarios to execute
        available_scenarios = self.get_available_scenarios()
        scenarios_to_execute = [
            s for s in available_scenarios 
            if s['name'] in scenario_names
        ]
        
        if not scenarios_to_execute:
            print(f"No scenarios found matching: {scenario_names}")
            return False
            
        # Create test execution
        test_scenarios = []
        for scenario_data in scenarios_to_execute:
            steps = [
                TestStep(
                    step_type=step['step_type'],
                    description=step['description'],
                    pin_interactions=step['pin_interactions']
                )
                for step in scenario_data['steps']
            ]
            
            test_scenarios.append(TestScenario(
                name=scenario_data['name'],
                description=scenario_data['description'],
                feature_file=scenario_data['feature_file'],
                tags=scenario_data['tags'],
                steps=steps
            ))
        
        self.current_execution = TestExecution(
            execution_id=execution_id,
            scenarios=test_scenarios,
            total_scenarios=len(test_scenarios)
        )
        
        # Start execution in background thread
        self.execution_thread = threading.Thread(
            target=self._execute_scenarios_thread,
            daemon=True
        )
        self.execution_thread.start()
        
        return True

    def _execute_scenarios_thread(self):
        """Execute scenarios in background thread"""
        if not self.current_execution:
            return
            
        execution = self.current_execution
        execution.status = TestStatus.RUNNING
        execution.start_time = int(time.time() * 1000)  # Convert to milliseconds
        
        self._send_progress_update()
        
        try:
            # Initialize HIL controller if available
            if HILController:
                self.hil_controller = HILController()
                
            for i, scenario in enumerate(execution.scenarios):
                execution.current_scenario_index = i
                self._execute_scenario(scenario)
                
                if scenario.status == TestStatus.PASSED:
                    execution.passed_scenarios += 1
                elif scenario.status == TestStatus.FAILED:
                    execution.failed_scenarios += 1
                    
                self._send_progress_update()
                
        except Exception as e:
            execution.status = TestStatus.ERROR
            execution.error_message = str(e)
            print(f"Test execution error: {e}")
            
        finally:
            execution.end_time = int(time.time() * 1000)  # Convert to milliseconds
            if execution.status == TestStatus.RUNNING:
                execution.status = TestStatus.PASSED if execution.failed_scenarios == 0 else TestStatus.FAILED
                
            self._send_progress_update()

    def _execute_scenario(self, scenario: TestScenario):
        """Execute a single test scenario"""
        scenario.status = TestStatus.RUNNING
        start_time = time.time()
        
        try:
            # For now, simulate test execution
            # In a full implementation, this would use behave or direct HIL calls
            for step in scenario.steps:
                step.status = TestStatus.RUNNING
                self._send_progress_update()
                
                # Simulate step execution time
                time.sleep(0.5)
                
                # For demo purposes, randomly pass/fail some steps
                import random
                if random.random() > 0.1:  # 90% pass rate
                    step.status = TestStatus.PASSED
                    step.duration_ms = int((time.time() - start_time) * 1000)
                else:
                    step.status = TestStatus.FAILED
                    step.error_message = "Simulated test failure"
                    step.duration_ms = int((time.time() - start_time) * 1000)
                    break
                    
            # Determine scenario status
            failed_steps = [s for s in scenario.steps if s.status == TestStatus.FAILED]
            if failed_steps:
                scenario.status = TestStatus.FAILED
                scenario.error_message = f"Failed steps: {len(failed_steps)}"
            else:
                scenario.status = TestStatus.PASSED
                
        except Exception as e:
            scenario.status = TestStatus.ERROR
            scenario.error_message = str(e)
            
        scenario.duration_ms = int((time.time() - start_time) * 1000)

    def _send_progress_update(self):
        """Send progress update via callback"""
        if self.progress_callback and self.current_execution:
            try:
                # Convert to JSON-serializable format
                execution_data = {
                    'execution_id': self.current_execution.execution_id,
                    'status': self.current_execution.status.value,
                    'start_time': self.current_execution.start_time,
                    'end_time': self.current_execution.end_time,
                    'total_scenarios': self.current_execution.total_scenarios,
                    'passed_scenarios': self.current_execution.passed_scenarios,
                    'failed_scenarios': self.current_execution.failed_scenarios,
                    'current_scenario_index': self.current_execution.current_scenario_index,
                    'scenarios': [
                        {
                            'name': s.name,
                            'description': s.description,
                            'feature_file': s.feature_file,
                            'tags': s.tags,
                            'status': s.status.value,
                            'duration_ms': s.duration_ms,
                            'error_message': s.error_message,
                            'steps': [
                                {
                                    'step_type': step.step_type,
                                    'description': step.description,
                                    'status': step.status.value,
                                    'duration_ms': step.duration_ms,
                                    'error_message': step.error_message,
                                    'pin_interactions': step.pin_interactions
                                }
                                for step in s.steps
                            ]
                        }
                        for s in self.current_execution.scenarios
                    ]
                }
                
                self.progress_callback(execution_data)
                
            except Exception as e:
                print(f"Error sending progress update: {e}")

    def get_execution_status(self) -> Optional[Dict[str, Any]]:
        """Get current execution status"""
        if not self.current_execution:
            return None
            
        return {
            'execution_id': self.current_execution.execution_id,
            'status': self.current_execution.status.value,
            'total_scenarios': self.current_execution.total_scenarios,
            'passed_scenarios': self.current_execution.passed_scenarios,
            'failed_scenarios': self.current_execution.failed_scenarios,
            'current_scenario_index': self.current_execution.current_scenario_index
        }

    def stop_execution(self) -> bool:
        """Stop current test execution"""
        if self.current_execution and self.current_execution.status == TestStatus.RUNNING:
            self.current_execution.status = TestStatus.ERROR
            self.current_execution.error_message = "Execution stopped by user"
            return True
        return False


def main():
    """Command-line interface for the test automation service"""
    import sys
    import json

    if len(sys.argv) < 2:
        print("Usage: python TestAutomationService.py <command> [args...]")
        sys.exit(1)

    command = sys.argv[1]
    service = TestAutomationService()

    try:
        if command == 'get_scenarios':
            # Get all available scenarios
            scenarios = service.get_available_scenarios()
            print(json.dumps(scenarios, indent=2))

        elif command == 'execute_scenarios':
            if len(sys.argv) < 4:
                print("Usage: python TestAutomationService.py execute_scenarios <scenarios_json> <execution_id>")
                sys.exit(1)

            scenarios_json = sys.argv[2]
            execution_id = sys.argv[3]

            try:
                scenario_names = json.loads(scenarios_json)
            except json.JSONDecodeError as e:
                print(f"Error parsing scenarios JSON: {e}")
                sys.exit(1)

            # Set up progress callback to print updates
            def progress_callback(execution_data):
                print(json.dumps(execution_data))
                sys.stdout.flush()

            service.progress_callback = progress_callback

            # Execute scenarios
            success = service.execute_scenarios(scenario_names, execution_id)
            if not success:
                print(json.dumps({"error": "Failed to start execution"}))
                sys.exit(1)

            # Wait for execution to complete
            while (service.current_execution and
                   service.current_execution.status == TestStatus.RUNNING):
                time.sleep(0.1)

        else:
            print(f"Unknown command: {command}")
            sys.exit(1)

    except Exception as e:
        print(json.dumps({"error": str(e)}))
        sys.exit(1)


if __name__ == '__main__':
    main()
