#!/usr/bin/env python3
"""
Test Automation Service Tests

Tests for the TestAutomationService class that handles BDD scenario execution
from the web UI.

Author: Cannasol Technologies
License: Proprietary
"""

import pytest
import json
import tempfile
import os
from pathlib import Path
from unittest.mock import Mock, patch, MagicMock

# Add the backend services to the path
import sys
sys.path.append(str(Path(__file__).parent.parent.parent / 'backend' / 'src' / 'services'))

from TestAutomationService import TestAutomationService, TestStatus, TestStep, TestScenario, TestExecution


class TestTestAutomationService:
    """Test cases for TestAutomationService"""

    def setup_method(self):
        """Set up test fixtures"""
        self.service = TestAutomationService()
        
        # Create a temporary directory for test feature files
        self.temp_dir = tempfile.mkdtemp()
        self.features_path = Path(self.temp_dir) / 'features'
        self.features_path.mkdir(exist_ok=True)
        
        # Override the features path
        self.service.features_path = self.features_path

    def teardown_method(self):
        """Clean up test fixtures"""
        import shutil
        shutil.rmtree(self.temp_dir)

    def create_test_feature_file(self, filename: str, content: str):
        """Helper to create test feature files"""
        feature_file = self.features_path / filename
        with open(feature_file, 'w') as f:
            f.write(content)
        return feature_file

    def test_parse_simple_feature_file(self):
        """Test parsing a simple feature file"""
        content = """
@hil @smoke
Feature: HIL Smoke Test
  Verify HIL wrapper connectivity and basic communication.
  
  Background:
    Given the HIL wrapper is connected and ready
  
  Scenario: HIL-001 Wrapper responds to PING
    Then the harness sanity check passes
    And the serial device is available for the harness
"""
        
        self.create_test_feature_file('smoke.feature', content)
        scenarios = self.service.get_available_scenarios()
        
        assert len(scenarios) == 1
        scenario = scenarios[0]
        assert scenario['name'] == 'HIL-001 Wrapper responds to PING'
        assert scenario['feature_file'] == 'smoke.feature'
        assert scenario['feature_name'] == 'HIL Smoke Test'
        assert 'hil' in scenario['tags']
        assert 'smoke' in scenario['tags']
        assert len(scenario['steps']) == 2

    def test_parse_gpio_feature_file(self):
        """Test parsing a GPIO functionality feature file"""
        content = """
Feature: GPIO Hardware Functionality
  As a firmware developer
  I want to verify GPIO pin control and monitoring
  So that I can ensure sonicator interface pins work correctly

  Background:
    Given the HIL framework is connected to ATmega32A
    And the GPIO HAL is loaded on the target

  @hil @hardware @gpio
  Scenario: Digital output control - START_4 pin
    When I set pin "START_4" to HIGH via HIL framework
    Then the pin voltage should be approximately 5.0V
    When I set pin "START_4" to LOW via HIL framework  
    Then the pin voltage should be approximately 0.0V
"""
        
        self.create_test_feature_file('gpio.feature', content)
        scenarios = self.service.get_available_scenarios()
        
        assert len(scenarios) == 1
        scenario = scenarios[0]
        assert scenario['name'] == 'Digital output control - START_4 pin'
        assert 'START_4' in scenario['steps'][0]['pin_interactions']
        assert 'hil' in scenario['tags']
        assert 'hardware' in scenario['tags']
        assert 'gpio' in scenario['tags']

    def test_extract_pin_interactions(self):
        """Test pin interaction extraction from step descriptions"""
        test_cases = [
            ('When I set pin "START_4" to HIGH via HIL framework', ['START_4']),
            ('Then pin "RESET_4" should still be LOW', ['RESET_4']),
            ('When I apply 2.5V to ADC channel "POWER_SENSE_4"', ['POWER_SENSE_4']),
            ('Given pin "OVERLOAD_4" is configured as input', ['OVERLOAD_4']),
            ('When I set STATUS_LED to HIGH', ['STATUS_LED']),
            ('Test with no pins mentioned', []),
            ('Multiple pins START_4 and RESET_4 test', ['START_4', 'RESET_4'])
        ]
        
        for description, expected_pins in test_cases:
            pins = self.service._extract_pin_interactions(description)
            assert pins == expected_pins, f"Failed for: {description}"

    def test_extract_tags_from_previous_lines(self):
        """Test tag extraction from feature file lines"""
        lines = [
            "Feature: Test Feature",
            "",
            "@hil @smoke @hardware",
            "@gpio",
            "Scenario: Test Scenario"
        ]
        
        tags = self.service._extract_tags_from_previous_lines(lines, 4)  # Index of scenario line
        expected_tags = ['hil', 'smoke', 'hardware', 'gpio']
        assert sorted(tags) == sorted(expected_tags)

    def test_execute_scenarios_invalid_names(self):
        """Test executing scenarios with invalid names"""
        success = self.service.execute_scenarios(['NonExistentScenario'], 'test-exec-1')
        assert not success

    def test_execute_scenarios_already_running(self):
        """Test executing scenarios when already running"""
        # Mock an execution in progress
        self.service.current_execution = TestExecution(
            execution_id='test-exec-1',
            scenarios=[],
            status=TestStatus.RUNNING
        )
        
        success = self.service.execute_scenarios(['TestScenario'], 'test-exec-2')
        assert not success

    @patch('threading.Thread')
    def test_execute_scenarios_success(self, mock_thread):
        """Test successful scenario execution start"""
        # Create a test feature file
        content = """
Feature: Test Feature
  Scenario: Test Scenario
    Given test step
    When test action
    Then test result
"""
        self.create_test_feature_file('test.feature', content)
        
        # Mock the thread
        mock_thread_instance = Mock()
        mock_thread.return_value = mock_thread_instance
        
        success = self.service.execute_scenarios(['Test Scenario'], 'test-exec-1')
        
        assert success
        assert self.service.current_execution is not None
        assert self.service.current_execution.execution_id == 'test-exec-1'
        mock_thread_instance.start.assert_called_once()

    def test_stop_execution_no_execution(self):
        """Test stopping execution when none is running"""
        success = self.service.stop_execution()
        assert not success

    def test_stop_execution_success(self):
        """Test successful execution stop"""
        self.service.current_execution = TestExecution(
            execution_id='test-exec-1',
            scenarios=[],
            status=TestStatus.RUNNING
        )
        
        success = self.service.stop_execution()
        assert success
        assert self.service.current_execution.status == TestStatus.ERROR
        assert 'stopped by user' in self.service.current_execution.error_message

    def test_get_execution_status_no_execution(self):
        """Test getting execution status when none exists"""
        status = self.service.get_execution_status()
        assert status is None

    def test_get_execution_status_with_execution(self):
        """Test getting execution status with active execution"""
        execution = TestExecution(
            execution_id='test-exec-1',
            scenarios=[],
            total_scenarios=5,
            passed_scenarios=3,
            failed_scenarios=1,
            current_scenario_index=4,
            status=TestStatus.RUNNING
        )
        self.service.current_execution = execution
        
        status = self.service.get_execution_status()
        assert status is not None
        assert status['execution_id'] == 'test-exec-1'
        assert status['total_scenarios'] == 5
        assert status['passed_scenarios'] == 3
        assert status['failed_scenarios'] == 1
        assert status['current_scenario_index'] == 4
        assert status['status'] == 'running'

    def test_progress_callback(self):
        """Test progress callback functionality"""
        callback_data = []
        
        def test_callback(data):
            callback_data.append(data)
        
        service = TestAutomationService(progress_callback=test_callback)
        
        # Create a mock execution
        execution = TestExecution(
            execution_id='test-exec-1',
            scenarios=[],
            status=TestStatus.RUNNING
        )
        service.current_execution = execution
        
        # Trigger progress update
        service._send_progress_update()
        
        assert len(callback_data) == 1
        assert callback_data[0]['execution_id'] == 'test-exec-1'
        assert callback_data[0]['status'] == 'running'

    def test_scenario_execution_simulation(self):
        """Test the scenario execution simulation"""
        # Create test scenario
        steps = [
            TestStep('Given', 'test setup', pin_interactions=['START_4']),
            TestStep('When', 'test action', pin_interactions=['RESET_4']),
            TestStep('Then', 'test result', pin_interactions=[])
        ]
        
        scenario = TestScenario(
            name='Test Scenario',
            description='Test Description',
            feature_file='test.feature',
            tags=['test'],
            steps=steps
        )
        
        # Execute scenario (this will use the simulation)
        self.service._execute_scenario(scenario)
        
        # Check that scenario completed
        assert scenario.status in [TestStatus.PASSED, TestStatus.FAILED]
        assert scenario.duration_ms is not None
        
        # Check that all steps have status
        for step in scenario.steps:
            assert step.status in [TestStatus.PASSED, TestStatus.FAILED]

    def test_command_line_interface_get_scenarios(self):
        """Test command line interface for getting scenarios"""
        # Create a test feature file
        content = """
Feature: CLI Test Feature
  Scenario: CLI Test Scenario
    Given test step
"""
        self.create_test_feature_file('cli_test.feature', content)
        
        # Test the main function with get_scenarios command
        with patch('sys.argv', ['TestAutomationService.py', 'get_scenarios']):
            with patch('builtins.print') as mock_print:
                from TestAutomationService import main
                main()
                
                # Check that scenarios were printed
                mock_print.assert_called()
                # The last call should be the JSON output
                output = mock_print.call_args[0][0]
                scenarios = json.loads(output)
                assert len(scenarios) == 1
                assert scenarios[0]['name'] == 'CLI Test Scenario'


if __name__ == '__main__':
    pytest.main([__file__])
