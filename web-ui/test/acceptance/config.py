#!/usr/bin/env python3
"""
Web-UI Acceptance Test Configuration

Configuration file for web-ui acceptance testing with behave framework.
Contains settings for browser automation, test execution, and reporting.

Author: Cannasol Technologies
License: Proprietary
"""

import os
from pathlib import Path

# Project root directory
PROJECT_ROOT = Path(__file__).parent.parent.parent.parent

# Web-UI server configuration
WEB_UI_CONFIG = {
    'backend_port': 3001,
    'frontend_port': 3101,
    'backend_url': 'http://localhost:3001',
    'frontend_url': 'http://localhost:3101',
    'startup_timeout': 30,
    'test_timeout': 60
}

# Browser configuration
BROWSER_CONFIG = {
    'headless': os.getenv('HEADLESS', 'true').lower() == 'true',
    'window_size': (1920, 1080),
    'implicit_wait': 10,
    'page_load_timeout': 30,
    'browser_options': [
        '--no-sandbox',
        '--disable-dev-shm-usage',
        '--disable-gpu',
        '--disable-web-security',
        '--allow-running-insecure-content'
    ]
}

# Test execution configuration
TEST_CONFIG = {
    'max_scenarios': 50,
    'scenario_timeout': 300,  # 5 minutes per scenario
    'step_timeout': 30,
    'retry_attempts': 3,
    'parallel_execution': False
}

# Mock/simulation configuration
MOCK_CONFIG = {
    'hardware_present': False,
    'simulation_mode': True,
    'mock_hardware_responses': True,
    'mock_delay_ms': 100
}

# Reporting configuration
REPORTING_CONFIG = {
    'output_dir': PROJECT_ROOT / 'web-ui' / 'test' / 'results',
    'junit_file': 'web-ui-acceptance-junit.xml',
    'html_report': 'web-ui-acceptance-report.html',
    'json_report': 'web-ui-acceptance-report.json',
    'include_screenshots': True,
    'screenshot_on_failure': True
}

# Logging configuration
LOGGING_CONFIG = {
    'level': 'INFO',
    'format': '[%(asctime)s] %(levelname)s: %(message)s',
    'file': PROJECT_ROOT / 'web-ui' / 'test' / 'logs' / 'acceptance.log'
}