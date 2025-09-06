#!/usr/bin/env python3
"""
Test runner for Arduino Command Logging functionality
Runs comprehensive tests for the Arduino command/response display feature
"""
import subprocess
import sys
import os
import time
from pathlib import Path

def run_command(cmd, description=""):
    """Run a command and return the result"""
    print(f"\n{'='*60}")
    print(f"Running: {description}")
    print(f"Command: {cmd}")
    print(f"{'='*60}")
    
    start_time = time.time()
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    end_time = time.time()
    
    print(f"Duration: {end_time - start_time:.2f} seconds")
    print(f"Return code: {result.returncode}")
    
    if result.stdout:
        print(f"\nSTDOUT:\n{result.stdout}")
    
    if result.stderr:
        print(f"\nSTDERR:\n{result.stderr}")
    
    return result

def main():
    """Main test runner function"""
    print("Arduino Command Logging Test Suite")
    print("=" * 60)
    
    # Change to the web-ui directory
    web_ui_dir = Path(__file__).parent
    os.chdir(web_ui_dir)
    
    # Test categories to run
    test_categories = [
        {
            'name': 'Arduino Command Logging Unit Tests',
            'command': 'python3 -m pytest tests/unit/test_arduino_command_logging.py -v -m arduino_logging',
            'description': 'Backend command logging functionality'
        },
        {
            'name': 'Arduino Command Log Component Tests',
            'command': 'python3 -m pytest tests/unit/test_arduino_command_log_component.py -v -m arduino_logging',
            'description': 'Frontend component logic'
        },
        {
            'name': 'Arduino Command Log Hook Tests',
            'command': 'python3 -m pytest tests/unit/test_arduino_command_log_hook.py -v -m arduino_logging',
            'description': 'React hook functionality'
        },
        {
            'name': 'Arduino Command Logging Integration Tests',
            'command': 'python3 -m pytest tests/integration/test_arduino_command_logging_integration.py -v -m arduino_logging',
            'description': 'End-to-end integration'
        },
        {
            'name': 'All Arduino Logging Tests',
            'command': 'python3 -m pytest -v -m arduino_logging',
            'description': 'Complete test suite'
        }
    ]
    
    # Results tracking
    results = []
    total_start_time = time.time()
    
    # Run each test category
    for category in test_categories:
        print(f"\n\n{'#'*80}")
        print(f"# {category['name']}")
        print(f"# {category['description']}")
        print(f"{'#'*80}")
        
        result = run_command(category['command'], category['name'])
        results.append({
            'name': category['name'],
            'success': result.returncode == 0,
            'returncode': result.returncode
        })
        
        if result.returncode != 0:
            print(f"\n❌ FAILED: {category['name']}")
        else:
            print(f"\n✅ PASSED: {category['name']}")
    
    # Summary
    total_end_time = time.time()
    total_duration = total_end_time - total_start_time
    
    print(f"\n\n{'='*80}")
    print("TEST SUMMARY")
    print(f"{'='*80}")
    print(f"Total Duration: {total_duration:.2f} seconds")
    print(f"Tests Run: {len(results)}")
    
    passed = sum(1 for r in results if r['success'])
    failed = len(results) - passed
    
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")
    
    print(f"\nDetailed Results:")
    for result in results:
        status = "✅ PASS" if result['success'] else "❌ FAIL"
        print(f"  {status} - {result['name']}")
    
    # Coverage report
    print(f"\n{'='*80}")
    print("COVERAGE REPORT")
    print(f"{'='*80}")
    
    coverage_result = run_command(
        'python -m pytest tests/unit/test_arduino_command_logging.py --cov=backend/src --cov-report=term-missing',
        'Coverage analysis for Arduino command logging'
    )
    
    # Final status
    if failed == 0:
        print(f"\n🎉 ALL TESTS PASSED! 🎉")
        print(f"Arduino Command Logging functionality is working correctly.")
        return 0
    else:
        print(f"\n💥 {failed} TEST(S) FAILED 💥")
        print(f"Please review the failed tests and fix any issues.")
        return 1

def run_specific_test(test_name):
    """Run a specific test by name"""
    test_files = {
        'backend': 'tests/unit/test_arduino_command_logging.py',
        'component': 'tests/unit/test_arduino_command_log_component.py',
        'hook': 'tests/unit/test_arduino_command_log_hook.py',
        'integration': 'tests/integration/test_arduino_command_logging_integration.py'
    }
    
    if test_name in test_files:
        cmd = f'python -m pytest {test_files[test_name]} -v'
        result = run_command(cmd, f'Running {test_name} tests')
        return result.returncode
    else:
        print(f"Unknown test name: {test_name}")
        print(f"Available tests: {', '.join(test_files.keys())}")
        return 1

def run_performance_tests():
    """Run performance-focused tests"""
    print("Running Performance Tests for Arduino Command Logging")
    print("=" * 60)
    
    performance_tests = [
        'python -m pytest tests/unit/test_arduino_command_logging.py::TestArduinoCommandLogging::test_concurrent_command_logging -v',
        'python -m pytest tests/unit/test_arduino_command_logging.py::TestArduinoCommandLogging::test_memory_management_in_logging -v',
        'python -m pytest tests/integration/test_arduino_command_logging_integration.py::TestArduinoCommandLoggingIntegration::test_high_frequency_command_handling -v',
        'python -m pytest tests/integration/test_arduino_command_logging_integration.py::TestArduinoCommandLoggingIntegration::test_concurrent_command_processing -v'
    ]
    
    for test_cmd in performance_tests:
        result = run_command(test_cmd, 'Performance test')
        if result.returncode != 0:
            return 1
    
    return 0

if __name__ == '__main__':
    if len(sys.argv) > 1:
        command = sys.argv[1]
        
        if command == 'specific' and len(sys.argv) > 2:
            test_name = sys.argv[2]
            exit_code = run_specific_test(test_name)
        elif command == 'performance':
            exit_code = run_performance_tests()
        elif command == 'help':
            print("Arduino Command Logging Test Runner")
            print("Usage:")
            print("  python run_arduino_logging_tests.py                    # Run all tests")
            print("  python run_arduino_logging_tests.py specific backend   # Run backend tests")
            print("  python run_arduino_logging_tests.py specific component # Run component tests")
            print("  python run_arduino_logging_tests.py specific hook      # Run hook tests")
            print("  python run_arduino_logging_tests.py specific integration # Run integration tests")
            print("  python run_arduino_logging_tests.py performance        # Run performance tests")
            print("  python run_arduino_logging_tests.py help               # Show this help")
            exit_code = 0
        else:
            print(f"Unknown command: {command}")
            exit_code = 1
    else:
        exit_code = main()
    
    sys.exit(exit_code)
