#!/usr/bin/env python3
"""
Test Automation Test Configuration

Pytest configuration and fixtures for test automation interface tests.

Author: Cannasol Technologies
License: Proprietary
"""

import pytest
import tempfile
import shutil
from pathlib import Path
from unittest.mock import Mock, patch


@pytest.fixture
def temp_features_dir():
    """Create a temporary directory for test feature files"""
    temp_dir = tempfile.mkdtemp()
    features_path = Path(temp_dir) / 'features'
    features_path.mkdir(exist_ok=True)
    
    yield features_path
    
    # Cleanup
    shutil.rmtree(temp_dir)


@pytest.fixture
def sample_feature_content():
    """Sample feature file content for testing"""
    return """
@hil @smoke
Feature: HIL Smoke Test
  Verify HIL wrapper connectivity and basic communication.
  
  Background:
    Given the HIL wrapper is connected and ready
  
  Scenario: HIL-001 Wrapper responds to PING
    Then the harness sanity check passes
    And the serial device is available for the harness

  @hil @hardware @gpio
  Scenario: Digital output control - START_4 pin
    When I set pin "START_4" to HIGH via HIL framework
    Then the pin voltage should be approximately 5.0V
    When I set pin "START_4" to LOW via HIL framework  
    Then the pin voltage should be approximately 0.0V
"""


@pytest.fixture
def mock_hil_controller():
    """Mock HIL controller for testing"""
    mock_controller = Mock()
    mock_controller.program_firmware.return_value = True
    return mock_controller


@pytest.fixture
def mock_hardware_interface():
    """Mock hardware interface for testing"""
    mock_interface = Mock()
    mock_interface.send_command.return_value = "PONG"
    mock_interface.isConnected.return_value = True
    mock_interface.getSerialPort.return_value = "/dev/ttyUSB0"
    return mock_interface


@pytest.fixture
def sample_test_execution():
    """Sample test execution data for testing"""
    return {
        'execution_id': 'test-exec-123',
        'status': 'running',
        'start_time': 1625097600000,  # 2021-07-01 00:00:00 UTC
        'end_time': None,
        'total_scenarios': 2,
        'passed_scenarios': 1,
        'failed_scenarios': 0,
        'current_scenario_index': 1,
        'scenarios': [
            {
                'name': 'Test Scenario 1',
                'description': 'First test scenario',
                'feature_file': 'test.feature',
                'tags': ['hil', 'smoke'],
                'status': 'passed',
                'duration_ms': 5000,
                'steps': [
                    {
                        'step_type': 'Given',
                        'description': 'Given test setup',
                        'status': 'passed',
                        'duration_ms': 1000,
                        'pin_interactions': ['START_4']
                    },
                    {
                        'step_type': 'When',
                        'description': 'When test action',
                        'status': 'passed',
                        'duration_ms': 2000,
                        'pin_interactions': ['RESET_4']
                    }
                ]
            },
            {
                'name': 'Test Scenario 2',
                'description': 'Second test scenario',
                'feature_file': 'test.feature',
                'tags': ['hil', 'gpio'],
                'status': 'running',
                'duration_ms': None,
                'steps': [
                    {
                        'step_type': 'Then',
                        'description': 'Then test result',
                        'status': 'running',
                        'duration_ms': None,
                        'pin_interactions': ['POWER_SENSE_4']
                    }
                ]
            }
        ]
    }


@pytest.fixture
def mock_websocket():
    """Mock WebSocket for testing"""
    mock_ws = Mock()
    mock_ws.send = Mock()
    mock_ws.close = Mock()
    return mock_ws


@pytest.fixture
def mock_progress_callback():
    """Mock progress callback for testing"""
    callback_data = []
    
    def callback(data):
        callback_data.append(data)
    
    callback.data = callback_data
    return callback


# Test data constants
TEST_SCENARIOS = [
    {
        'name': 'HIL-001 Wrapper responds to PING',
        'description': 'Test HIL wrapper connectivity',
        'feature_file': 'hil_smoke.feature',
        'feature_name': 'HIL Smoke Test',
        'tags': ['hil', 'smoke'],
        'steps': [
            {
                'step_type': 'Then',
                'description': 'Then the harness sanity check passes',
                'pin_interactions': []
            },
            {
                'step_type': 'And',
                'description': 'And the serial device is available for the harness',
                'pin_interactions': []
            }
        ]
    },
    {
        'name': 'Digital output control - START_4 pin',
        'description': 'Test GPIO pin control',
        'feature_file': 'gpio_functionality.feature',
        'feature_name': 'GPIO Hardware Functionality',
        'tags': ['hil', 'hardware', 'gpio'],
        'steps': [
            {
                'step_type': 'When',
                'description': 'When I set pin "START_4" to HIGH via HIL framework',
                'pin_interactions': ['START_4']
            },
            {
                'step_type': 'Then',
                'description': 'Then the pin voltage should be approximately 5.0V',
                'pin_interactions': ['START_4']
            }
        ]
    }
]

TEST_TAGS = ['hil', 'smoke', 'hardware', 'gpio']

TEST_FEATURES = ['hil_smoke.feature', 'gpio_functionality.feature']
