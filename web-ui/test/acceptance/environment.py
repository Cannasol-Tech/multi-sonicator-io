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
import subprocess
import signal
from pathlib import Path

# Add project root to path
project_root = Path(__file__).parent.parent.parent.parent
sys.path.append(str(project_root))

# Try to import optional dependencies
try:
    import requests
    HAS_REQUESTS = True
except ImportError:
    HAS_REQUESTS = False

try:
    from selenium import webdriver
    from selenium.webdriver.chrome.options import Options
    from selenium.webdriver.common.by import By
    from selenium.webdriver.support.ui import WebDriverWait
    from selenium.webdriver.support import expected_conditions as EC
    HAS_SELENIUM = True
except ImportError:
    HAS_SELENIUM = False

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
        if not HAS_SELENIUM:
            print("Selenium not available - skipping browser setup")
            return False
            
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
            
            # Wait for servers to start (if requests is available)
            if HAS_REQUESTS:
                self._wait_for_servers()
            else:
                print("Requests not available - skipping server readiness check")
                time.sleep(5)  # Basic wait
            return True
            
        except Exception as e:
            print(f"Failed to start web servers: {e}")
            return False

    def _wait_for_servers(self):
        """Wait for web servers to be ready"""
        if not HAS_REQUESTS:
            return
            
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
    
    # Check for required dependencies
    if not HAS_SELENIUM:
        print("⚠️  Selenium not available - browser automation tests will be skipped")
        context.skip_browser_tests = True
    else:
        context.skip_browser_tests = False
    
    if not HAS_REQUESTS:
        print("⚠️  Requests not available - HTTP API tests will be limited")
    
    context.test_env = WebUITestEnvironment()
    
    # Setup browser only if selenium is available
    if HAS_SELENIUM:
        # Start web servers
        if not context.test_env.start_web_servers():
            print("⚠️  Failed to start web servers - using mock mode")
            
        # Setup browser  
        if not context.test_env.setup_browser():
            print("⚠️  Failed to setup browser - browser tests will be skipped")
            context.skip_browser_tests = True
        else:
            context.driver = context.test_env.driver
            context.backend_url = context.test_env.backend_url
            context.frontend_url = context.test_env.frontend_url
    
    print("Web-UI acceptance test environment setup complete!")

def after_all(context):
    """Cleanup test environment after all tests"""
    print("Cleaning up web-ui acceptance test environment...")
    
    if hasattr(context, 'test_env'):
        context.test_env.cleanup_browser()
        context.test_env.stop_web_servers()
    
    print("Web-UI acceptance test environment cleaned up!")

def before_scenario(context, scenario):
    """Setup before each scenario"""
    # Skip browser-dependent scenarios if browser is not available
    if hasattr(context, 'skip_browser_tests') and context.skip_browser_tests:
        if any(tag in ['@ui-loading', '@tag-filtering', '@single-test', '@multiple-tests'] 
               for tag in scenario.tags):
            scenario.skip("Browser automation not available")
            return
    
    # Navigate to the web-ui home page if browser is available
    if hasattr(context, 'driver') and context.driver:
        try:
            context.driver.get(context.frontend_url)
            # Wait for initial page load
            if HAS_SELENIUM:
                WebDriverWait(context.driver, 10).until(
                    EC.presence_of_element_located((By.TAG_NAME, "body"))
                )
        except Exception as e:
            print(f"Failed to navigate to web-ui: {e}")

def after_scenario(context, scenario):
    """Cleanup after each scenario"""
    # Clear any browser state between scenarios
    if hasattr(context, 'driver') and context.driver:
        try:
            context.driver.delete_all_cookies()
        except Exception as e:
            print(f"Failed to clear browser state: {e}")