#!/usr/bin/env python3
"""
Report Generation Script for Multi-Sonicator CI/CD Pipeline
Generates test reports, build summaries, and documentation
"""

import os
import sys
import json
import xml.etree.ElementTree as ET
from datetime import datetime
import argparse

def generate_test_report(test_results_dir, output_file):
    """
    Generate HTML test report from JUnit XML files
    """
    if not os.path.exists(test_results_dir):
        print(f"Test results directory not found: {test_results_dir}")
        return False
    
    # Find all JUnit XML files
    junit_files = []
    for root, dirs, files in os.walk(test_results_dir):
        for file in files:
            if file.endswith('.xml'):
                junit_files.append(os.path.join(root, file))
    
    if not junit_files:
        print("No JUnit XML files found")
        return False
    
    # Parse test results
    total_tests = 0
    total_failures = 0
    total_errors = 0
    test_suites = []
    
    for junit_file in junit_files:
        try:
            tree = ET.parse(junit_file)
            root = tree.getroot()
            
            # Extract test suite information
            suite_info = {
                'name': root.get('name', 'Unknown'),
                'tests': int(root.get('tests', 0)),
                'failures': int(root.get('failures', 0)),
                'errors': int(root.get('errors', 0)),
                'time': float(root.get('time', 0)),
                'testcases': []
            }
            
            # Extract individual test cases
            for testcase in root.findall('testcase'):
                case_info = {
                    'name': testcase.get('name', 'Unknown'),
                    'classname': testcase.get('classname', 'Unknown'),
                    'time': float(testcase.get('time', 0)),
                    'status': 'PASS'
                }
                
                # Check for failures or errors
                if testcase.find('failure') is not None:
                    case_info['status'] = 'FAIL'
                    case_info['failure'] = testcase.find('failure').text
                elif testcase.find('error') is not None:
                    case_info['status'] = 'ERROR'
                    case_info['error'] = testcase.find('error').text
                
                suite_info['testcases'].append(case_info)
            
            test_suites.append(suite_info)
            total_tests += suite_info['tests']
            total_failures += suite_info['failures']
            total_errors += suite_info['errors']
            
        except Exception as e:
            print(f"Error parsing {junit_file}: {e}")
            continue
    
    # Generate HTML report
    html_content = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>Multi-Sonicator Test Report</title>
        <style>
            body {{ font-family: Arial, sans-serif; margin: 20px; }}
            .header {{ background-color: #f0f0f0; padding: 20px; border-radius: 5px; }}
            .summary {{ margin: 20px 0; }}
            .suite {{ margin: 20px 0; border: 1px solid #ddd; border-radius: 5px; }}
            .suite-header {{ background-color: #e0e0e0; padding: 10px; font-weight: bold; }}
            .testcase {{ padding: 10px; border-bottom: 1px solid #eee; }}
            .pass {{ color: green; }}
            .fail {{ color: red; }}
            .error {{ color: orange; }}
            .failure-detail {{ background-color: #ffe6e6; padding: 10px; margin: 10px 0; border-radius: 3px; }}
        </style>
    </head>
    <body>
        <div class="header">
            <h1>Multi-Sonicator Test Report</h1>
            <p>Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
        </div>
        
        <div class="summary">
            <h2>Test Summary</h2>
            <p>Total Tests: {total_tests}</p>
            <p>Passed: {total_tests - total_failures - total_errors}</p>
            <p>Failed: {total_failures}</p>
            <p>Errors: {total_errors}</p>
            <p>Success Rate: {((total_tests - total_failures - total_errors) / total_tests * 100 if total_tests > 0 else 0):.1f}%</p>
        </div>
    """
    
    # Add test suite details
    for suite in test_suites:
        html_content += f"""
        <div class="suite">
            <div class="suite-header">
                {suite['name']} - {suite['tests']} tests, {suite['failures']} failures, {suite['errors']} errors
            </div>
        """
        
        for testcase in suite['testcases']:
            status_class = testcase['status'].lower()
            html_content += f"""
            <div class="testcase">
                <span class="{status_class}">[{testcase['status']}]</span> 
                {testcase['name']} ({testcase['time']:.3f}s)
            """
            
            if testcase['status'] == 'FAIL' and 'failure' in testcase:
                html_content += f"""
                <div class="failure-detail">
                    <strong>Failure:</strong><br>
                    <pre>{testcase['failure']}</pre>
                </div>
                """
            elif testcase['status'] == 'ERROR' and 'error' in testcase:
                html_content += f"""
                <div class="failure-detail">
                    <strong>Error:</strong><br>
                    <pre>{testcase['error']}</pre>
                </div>
                """
            
            html_content += "</div>"
        
        html_content += "</div>"
    
    html_content += """
    </body>
    </html>
    """
    
    # Write report to file
    with open(output_file, 'w') as f:
        f.write(html_content)
    
    print(f"Test report generated: {output_file}")
    return True

def generate_build_summary(build_dir, output_file):
    """
    Generate build summary report
    """
    build_info = {
        'timestamp': datetime.now().isoformat(),
        'environments': []
    }
    
    # Check for build artifacts
    if os.path.exists(build_dir):
        for env_dir in os.listdir(build_dir):
            env_path = os.path.join(build_dir, env_dir)
            if os.path.isdir(env_path):
                firmware_path = os.path.join(env_path, 'firmware.hex')
                if os.path.exists(firmware_path):
                    env_info = {
                        'name': env_dir,
                        'firmware_size': os.path.getsize(firmware_path),
                        'firmware_exists': True
                    }
                else:
                    env_info = {
                        'name': env_dir,
                        'firmware_size': 0,
                        'firmware_exists': False
                    }
                build_info['environments'].append(env_info)
    
    # Write build summary
    with open(output_file, 'w') as f:
        json.dump(build_info, f, indent=2)
    
    print(f"Build summary generated: {output_file}")
    return True

def main():
    parser = argparse.ArgumentParser(description='Generate CI/CD reports')
    parser.add_argument('--test-results', help='Test results directory')
    parser.add_argument('--build-dir', help='Build directory', default='.pio/build')
    parser.add_argument('--output-dir', help='Output directory', default='reports')
    
    args = parser.parse_args()
    
    # Create output directory
    os.makedirs(args.output_dir, exist_ok=True)
    
    success = True
    
    # Generate test report if test results are available
    if args.test_results:
        test_report_file = os.path.join(args.output_dir, 'test_report.html')
        if not generate_test_report(args.test_results, test_report_file):
            success = False
    
    # Generate build summary
    build_summary_file = os.path.join(args.output_dir, 'build_summary.json')
    if not generate_build_summary(args.build_dir, build_summary_file):
        success = False
    
    if success:
        print("All reports generated successfully")
        return 0
    else:
        print("Some reports failed to generate")
        return 1

if __name__ == "__main__":
    sys.exit(main())