#!/usr/bin/env python3
"""
Web-UI Acceptance Test Environment Configuration

Sets up the test environment for web-ui acceptance testing with behave framework.
Configures browser automation, web-ui server connection, and test data management.

Author: Cannasol Technologies  
License: Proprietary
"""

import os
import sys
import time
import requests
import subprocess
import signal
from pathlib import Path
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

# Add project root to path
project_root = Path(__file__).parent.parent.parent.parent
sys.path.append(str(project_root))

class WebUITestEnvironment:
    def __init__(self):
        self.backend_process = None
        self.frontend_process = None
        self.driver = None
        self.backend_url = "http://localhost:3001"
        self.frontend_url = "http://localhost:3101"
        self.wait_timeout = 30

    def setup_browser(self):
        """Setup headless Chrome browser for testing"""
        chrome_options = Options()
        chrome_options.add_argument('--headless')
        chrome_options.add_argument('--no-sandbox')
        chrome_options.add_argument('--disable-dev-shm-usage')
        chrome_options.add_argument('--disable-gpu')
        chrome_options.add_argument('--window-size=1920,1080')
        
        try:
            self.driver = webdriver.Chrome(options=chrome_options)
            self.driver.implicitly_wait(10)
            return True
        except Exception as e:
            print(f"Failed to setup Chrome browser: {e}")
            return False

    def start_web_servers(self):
        """Start web-ui backend and frontend servers"""
        try:
            # Start backend
            backend_env = os.environ.copy()
            backend_env['HARDWARE_PRESENT'] = 'false'  # Use simulation mode
            backend_env['PORT'] = '3001'
            
            self.backend_process = subprocess.Popen(
                ['npm', 'run', 'dev'],
                cwd=str(project_root / 'web-ui' / 'backend'),
                env=backend_env,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            
            # Start frontend  
            frontend_env = os.environ.copy()
            frontend_env['PORT'] = '3101'
            
            self.frontend_process = subprocess.Popen(
                ['npm', 'run', 'dev'],
                cwd=str(project_root / 'web-ui' / 'frontend'),
                env=frontend_env,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            
            # Wait for servers to start
            self._wait_for_servers()
            return True
            
        except Exception as e:
            print(f"Failed to start web servers: {e}")
            return False

    def _wait_for_servers(self):
        """Wait for web servers to be ready"""
        print("Waiting for backend server...")
        for i in range(30):  # 30 second timeout
            try:
                response = requests.get(f"{self.backend_url}/api/health", timeout=2)
                if response.status_code == 200:
                    break
            except:
                pass
            time.sleep(1)
        else:
            raise Exception("Backend server failed to start")
            
        print("Waiting for frontend server...")
        for i in range(30):  # 30 second timeout
            try:
                response = requests.get(self.frontend_url, timeout=2)
                if response.status_code == 200:
                    break
            except:
                pass
            time.sleep(1)
        else:
            raise Exception("Frontend server failed to start")

    def stop_web_servers(self):
        """Stop web-ui servers"""
        if self.backend_process:
            self.backend_process.terminate()
            self.backend_process.wait()
        if self.frontend_process:
            self.frontend_process.terminate()
            self.frontend_process.wait()

    def cleanup_browser(self):
        """Cleanup browser resources"""
        if self.driver:
            self.driver.quit()

def before_all(context):
    """Setup test environment before all tests"""
    print("Setting up web-ui acceptance test environment...")
    
    context.test_env = WebUITestEnvironment()
    
    # Start web servers
    if not context.test_env.start_web_servers():
        raise Exception("Failed to start web servers")
    
    # Setup browser
    if not context.test_env.setup_browser():
        raise Exception("Failed to setup browser")
    
    context.driver = context.test_env.driver
    context.backend_url = context.test_env.backend_url
    context.frontend_url = context.test_env.frontend_url
    
    print("Web-UI acceptance test environment ready!")

def after_all(context):
    """Cleanup test environment after all tests"""
    print("Cleaning up web-ui acceptance test environment...")
    
    if hasattr(context, 'test_env'):
        context.test_env.cleanup_browser()
        context.test_env.stop_web_servers()
    
    print("Web-UI acceptance test environment cleaned up!")

def before_scenario(context, scenario):
    """Setup before each scenario"""
    # Navigate to the web-ui home page
    if hasattr(context, 'driver'):
        context.driver.get(context.frontend_url)
        # Wait for initial page load
        WebDriverWait(context.driver, 10).until(
            EC.presence_of_element_located((By.TAG_NAME, "body"))
        )

def after_scenario(context, scenario):
    """Cleanup after each scenario"""
    # Clear any browser state between scenarios
    if hasattr(context, 'driver'):
        context.driver.delete_all_cookies()