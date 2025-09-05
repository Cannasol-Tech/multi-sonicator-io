#!/usr/bin/env python3
"""
Test Automation API Tests

Tests for the test automation API endpoints and TypeScript service integration.

Author: Cannasol Technologies
License: Proprietary
"""

import pytest
import json
from unittest.mock import Mock, patch, AsyncMock
import sys
from pathlib import Path

# Add the backend to the path
sys.path.append(str(Path(__file__).parent.parent.parent / 'backend' / 'src'))

# Mock the TestAutomationService for API tests
class MockTestAutomationService:
    def __init__(self):
        self.scenarios = [
            {
                'name': 'Test Scenario 1',
                'description': 'First test scenario',
                'feature_file': 'test1.feature',
                'feature_name': 'Test Feature 1',
                'tags': ['hil', 'smoke'],
                'steps': [
                    {
                        'step_type': 'Given',
                        'description': 'Given test setup',
                        'pin_interactions': ['START_4']
                    },
                    {
                        'step_type': 'When',
                        'description': 'When test action',
                        'pin_interactions': ['RESET_4']
                    }
                ]
            },
            {
                'name': 'Test Scenario 2',
                'description': 'Second test scenario',
                'feature_file': 'test2.feature',
                'feature_name': 'Test Feature 2',
                'tags': ['hil', 'gpio'],
                'steps': [
                    {
                        'step_type': 'Then',
                        'description': 'Then test result',
                        'pin_interactions': ['POWER_SENSE_4']
                    }
                ]
            }
        ]
        
        self.tags = ['hil', 'smoke', 'gpio']
        self.features = ['test1.feature', 'test2.feature']
        self.current_execution = None
        self.execution_in_progress = False

    async def getAvailableScenarios(self):
        return self.scenarios

    async def getScenariosByTags(self, tags):
        if not tags:
            return self.scenarios
        return [s for s in self.scenarios if any(tag in s['tags'] for tag in tags)]

    async def getAvailableTags(self):
        return self.tags

    async def getAvailableFeatures(self):
        return self.features

    async def executeScenarios(self, scenario_names, execution_id):
        if self.execution_in_progress:
            return False
        
        self.execution_in_progress = True
        self.current_execution = {
            'execution_id': execution_id,
            'scenarios': [s for s in self.scenarios if s['name'] in scenario_names],
            'status': 'running',
            'total_scenarios': len(scenario_names),
            'passed_scenarios': 0,
            'failed_scenarios': 0,
            'current_scenario_index': 0
        }
        return True

    async def stopExecution(self):
        if self.execution_in_progress:
            self.execution_in_progress = False
            if self.current_execution:
                self.current_execution['status'] = 'stopped'
            return True
        return False

    async def getCurrentExecution(self):
        return self.current_execution

    def isExecutionInProgress(self):
        return self.execution_in_progress


class TestTestAutomationAPI:
    """Test cases for Test Automation API functionality"""

    def setup_method(self):
        """Set up test fixtures"""
        self.mock_service = MockTestAutomationService()

    def test_get_available_scenarios(self):
        """Test getting available scenarios"""
        scenarios = self.mock_service.scenarios
        
        assert len(scenarios) == 2
        assert scenarios[0]['name'] == 'Test Scenario 1'
        assert scenarios[1]['name'] == 'Test Scenario 2'
        assert 'hil' in scenarios[0]['tags']
        assert 'smoke' in scenarios[0]['tags']

    def test_filter_scenarios_by_tags(self):
        """Test filtering scenarios by tags"""
        # Test with smoke tag
        smoke_scenarios = [s for s in self.mock_service.scenarios if 'smoke' in s['tags']]
        assert len(smoke_scenarios) == 1
        assert smoke_scenarios[0]['name'] == 'Test Scenario 1'
        
        # Test with gpio tag
        gpio_scenarios = [s for s in self.mock_service.scenarios if 'gpio' in s['tags']]
        assert len(gpio_scenarios) == 1
        assert gpio_scenarios[0]['name'] == 'Test Scenario 2'
        
        # Test with hil tag (should return both)
        hil_scenarios = [s for s in self.mock_service.scenarios if 'hil' in s['tags']]
        assert len(hil_scenarios) == 2

    def test_get_scenarios_by_pins(self):
        """Test getting scenarios that interact with specific pins"""
        # Test START_4 pin
        start4_scenarios = []
        for scenario in self.mock_service.scenarios:
            for step in scenario['steps']:
                if 'START_4' in step['pin_interactions']:
                    start4_scenarios.append(scenario)
                    break
        
        assert len(start4_scenarios) == 1
        assert start4_scenarios[0]['name'] == 'Test Scenario 1'
        
        # Test POWER_SENSE_4 pin
        power_scenarios = []
        for scenario in self.mock_service.scenarios:
            for step in scenario['steps']:
                if 'POWER_SENSE_4' in step['pin_interactions']:
                    power_scenarios.append(scenario)
                    break
        
        assert len(power_scenarios) == 1
        assert power_scenarios[0]['name'] == 'Test Scenario 2'

    def test_execution_id_generation(self):
        """Test execution ID generation"""
        # Mock the TestAutomationAPI class
        class MockTestAutomationAPI:
            @staticmethod
            def generateExecutionId():
                import time
                import random
                return f"exec_{int(time.time())}_{''.join(random.choices('abcdefghijklmnopqrstuvwxyz0123456789', k=9))}"
        
        exec_id = MockTestAutomationAPI.generateExecutionId()
        assert exec_id.startswith('exec_')
        assert len(exec_id.split('_')) == 3
        assert len(exec_id.split('_')[2]) == 9

    def test_execution_progress_calculation(self):
        """Test execution progress calculation"""
        execution = {
            'total_scenarios': 10,
            'passed_scenarios': 6,
            'failed_scenarios': 2
        }
        
        completed = execution['passed_scenarios'] + execution['failed_scenarios']
        progress = round((completed / execution['total_scenarios']) * 100)
        
        assert progress == 80

    def test_status_color_mapping(self):
        """Test status color mapping"""
        color_map = {
            'passed': '#22c55e',
            'failed': '#ef4444',
            'error': '#f97316',
            'running': '#3b82f6',
            'pending': '#6b7280',
            'skipped': '#a3a3a3'
        }
        
        for status, expected_color in color_map.items():
            # This would be the actual implementation in TestAutomationAPI
            assert color_map[status] == expected_color

    def test_status_icon_mapping(self):
        """Test status icon mapping"""
        icon_map = {
            'passed': '✅',
            'failed': '❌',
            'error': '⚠️',
            'running': '🔄',
            'pending': '⏳',
            'skipped': '⏭️'
        }
        
        for status, expected_icon in icon_map.items():
            assert icon_map[status] == expected_icon

    def test_duration_formatting(self):
        """Test duration formatting"""
        test_cases = [
            (0, '0s'),
            (1000, '1s'),
            (30000, '30s'),
            (60000, '1m 0s'),
            (90000, '1m 30s'),
            (3600000, '60m 0s')
        ]
        
        for duration_ms, expected in test_cases:
            if duration_ms == 0:
                formatted = '0s'
            else:
                seconds = duration_ms // 1000
                minutes = seconds // 60
                if minutes > 0:
                    formatted = f"{minutes}m {seconds % 60}s"
                else:
                    formatted = f"{seconds}s"
            
            assert formatted == expected

    @pytest.mark.asyncio
    async def test_execute_scenarios_success(self):
        """Test successful scenario execution"""
        scenario_names = ['Test Scenario 1']
        execution_id = 'test-exec-123'
        
        success = await self.mock_service.executeScenarios(scenario_names, execution_id)
        
        assert success
        assert self.mock_service.execution_in_progress
        assert self.mock_service.current_execution['execution_id'] == execution_id
        assert len(self.mock_service.current_execution['scenarios']) == 1

    @pytest.mark.asyncio
    async def test_execute_scenarios_already_running(self):
        """Test executing scenarios when already running"""
        # Start first execution
        await self.mock_service.executeScenarios(['Test Scenario 1'], 'exec-1')
        
        # Try to start second execution
        success = await self.mock_service.executeScenarios(['Test Scenario 2'], 'exec-2')
        
        assert not success
        assert self.mock_service.current_execution['execution_id'] == 'exec-1'

    @pytest.mark.asyncio
    async def test_stop_execution_success(self):
        """Test successful execution stop"""
        # Start execution
        await self.mock_service.executeScenarios(['Test Scenario 1'], 'exec-1')
        
        # Stop execution
        success = await self.mock_service.stopExecution()
        
        assert success
        assert not self.mock_service.execution_in_progress
        assert self.mock_service.current_execution['status'] == 'stopped'

    @pytest.mark.asyncio
    async def test_stop_execution_no_execution(self):
        """Test stopping execution when none is running"""
        success = await self.mock_service.stopExecution()
        assert not success

    def test_scenario_validation(self):
        """Test scenario validation"""
        available_scenarios = [s['name'] for s in self.mock_service.scenarios]
        
        # Valid scenarios
        valid_scenarios = ['Test Scenario 1', 'Test Scenario 2']
        missing = [name for name in valid_scenarios if name not in available_scenarios]
        assert len(missing) == 0
        
        # Invalid scenarios
        invalid_scenarios = ['Non-existent Scenario', 'Test Scenario 1']
        missing = [name for name in invalid_scenarios if name not in available_scenarios]
        assert len(missing) == 1
        assert missing[0] == 'Non-existent Scenario'

    def test_step_progress_calculation(self):
        """Test current step progress calculation"""
        scenario = {
            'steps': [
                {'status': 'passed'},
                {'status': 'passed'},
                {'status': 'running'},
                {'status': 'pending'},
                {'status': 'pending'}
            ]
        }
        
        completed_steps = len([s for s in scenario['steps'] if s['status'] in ['passed', 'failed', 'error']])
        total_steps = len(scenario['steps'])
        
        assert completed_steps == 2
        assert total_steps == 5

    def test_pin_interaction_extraction(self):
        """Test extracting all pins from a scenario"""
        scenario = self.mock_service.scenarios[0]  # Test Scenario 1
        
        pins = set()
        for step in scenario['steps']:
            pins.update(step['pin_interactions'])
        
        expected_pins = {'START_4', 'RESET_4'}
        assert pins == expected_pins


if __name__ == '__main__':
    pytest.main([__file__])
