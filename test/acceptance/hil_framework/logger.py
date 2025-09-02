#!/usr/bin/env python3
"""
HIL Logger - Logging and reporting for Hardware-in-the-Loop testing

This module provides structured logging and reporting capabilities for HIL testing
with integration for Behave test reporting.

Author: Cannasol Technologies  
License: Proprietary
"""

import logging
import os
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Optional


class HILLogger:
    """HIL test logging and reporting system"""
    
    def __init__(self, log_level=logging.INFO, log_file: Optional[str] = None):
        """Initialize HIL logger"""
        self.logger = logging.getLogger('hil_framework')
        self.logger.setLevel(log_level)
        
        # Prevent duplicate handlers
        if not self.logger.handlers:
            self._setup_logging(log_file)
            
    def _setup_logging(self, log_file: Optional[str] = None):
        """Setup logging configuration"""
        formatter = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        
        # Console handler
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setLevel(logging.INFO)
        console_handler.setFormatter(formatter)
        self.logger.addHandler(console_handler)
        
        # File handler
        if log_file:
            log_path = Path(log_file)
        else:
            # Default log file in test directory
            log_dir = Path(__file__).parent.parent / 'logs'
            log_dir.mkdir(exist_ok=True)
            timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
            log_path = log_dir / f'hil_test_{timestamp}.log'
            
        file_handler = logging.FileHandler(log_path)
        file_handler.setLevel(logging.DEBUG)
        file_handler.setFormatter(formatter)
        self.logger.addHandler(file_handler)
        
        self.info(f"HIL logging initialized - log file: {log_path}")
        
    def debug(self, message: str):
        """Log debug message"""
        self.logger.debug(message)
        
    def info(self, message: str):
        """Log info message"""
        self.logger.info(message)
        
    def warning(self, message: str):
        """Log warning message"""
        self.logger.warning(message)
        
    def error(self, message: str):
        """Log error message"""
        self.logger.error(message)
        
    def critical(self, message: str):
        """Log critical message"""
        self.logger.critical(message)
        
    def test_start(self, test_name: str):
        """Log test start"""
        self.info(f"🧪 TEST START: {test_name}")
        
    def test_pass(self, test_name: str):
        """Log test pass"""
        self.info(f"✅ TEST PASS: {test_name}")
        
    def test_fail(self, test_name: str, error: str):
        """Log test failure"""
        self.error(f"❌ TEST FAIL: {test_name} - {error}")
        
    def hardware_event(self, event: str, details: str = ""):
        """Log hardware event"""
        self.info(f"🔧 HW EVENT: {event} {details}")
        
    def scenario_start(self, scenario_name: str):
        """Log scenario start"""
        self.info(f"📋 SCENARIO START: {scenario_name}")
        
    def scenario_end(self, scenario_name: str, result: str):
        """Log scenario end"""
        icon = "✅" if result.lower() == "pass" else "❌"
        self.info(f"{icon} SCENARIO {result.upper()}: {scenario_name}")
        
    def step_executed(self, step_name: str, result: str = "PASS"):
        """Log step execution"""
        icon = "✅" if result.upper() == "PASS" else "❌"
        self.info(f"{icon} STEP {result.upper()}: {step_name}")
        
    def measurement(self, parameter: str, value: float, unit: str = "", expected: str = ""):
        """Log measurement result"""
        expected_str = f" (expected: {expected})" if expected else ""
        self.info(f"📊 MEASUREMENT: {parameter} = {value}{unit}{expected_str}")
        
    def communication_log(self, direction: str, data: str):
        """Log communication data"""
        arrow = "→" if direction.upper() == "TX" else "←"
        self.debug(f"📡 COMM {arrow} {data}")


# Global logger instance for easy access
hil_logger = HILLogger()
