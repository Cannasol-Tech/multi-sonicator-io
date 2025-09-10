#!/usr/bin/env python3
"""
CI Report Generator
Generates comprehensive reports for CI/CD pipeline including test results, build artifacts, and coverage analysis
"""

import os
import sys
import json
import argparse
import xml.etree.ElementTree as ET
from pathlib import Path
from datetime import datetime
import subprocess

class CIReportGenerator:
    def __init__(self, output_dir="reports"):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.timestamp = datetime.now().isoformat()
        
    def generate_test_report(self, test_results_dir):
        """Generate comprehensive test report from JUnit XML files"""
        test_dir = Path(test_results_dir)
        if not test_dir.exists():
            print(f"⚠️ Test results directory not found: {test_results_dir}")
            return
            
        report = {
            "timestamp": self.timestamp,
            "test_suites": [],
            "summary": {
                "total_tests": 0,
                "passed": 0,
                "failed": 0,
                "skipped": 0,
                "execution_time": 0.0
            }
        }
        
        # Process all XML test result files
        for xml_file in test_dir.glob("*.xml"):
            try:
                tree = ET.parse(xml_file)
                root = tree.getroot()
                
                suite_info = {
                    "name": xml_file.stem,
                    "file": str(xml_file),
                    "tests": int(root.get("tests", 0)),
                    "failures": int(root.get("failures", 0)),
                    "errors": int(root.get("errors", 0)),
                    "skipped": int(root.get("skipped", 0)),
                    "time": float(root.get("time", 0.0)),
                    "test_cases": []
                }
                
                # Extract individual test cases
                for testcase in root.findall(".//testcase"):
                    case_info = {
                        "name": testcase.get("name", "unknown"),
                        "classname": testcase.get("classname", ""),
                        "time": float(testcase.get("time", 0.0)),
                        "status": "passed"
                    }
                    
                    # Check for failures or errors
                    if testcase.find("failure") is not None:
                        case_info["status"] = "failed"
                        case_info["failure"] = testcase.find("failure").text
                    elif testcase.find("error") is not None:
                        case_info["status"] = "error"
                        case_info["error"] = testcase.find("error").text
                    elif testcase.find("skipped") is not None:
                        case_info["status"] = "skipped"
                        
                    suite_info["test_cases"].append(case_info)
                
                report["test_suites"].append(suite_info)
                
                # Update summary
                report["summary"]["total_tests"] += suite_info["tests"]
                report["summary"]["failed"] += suite_info["failures"] + suite_info["errors"]
                report["summary"]["skipped"] += suite_info["skipped"]
                report["summary"]["execution_time"] += suite_info["time"]
                
            except ET.ParseError as e:
                print(f"⚠️ Failed to parse {xml_file}: {e}")
                continue
        
        # Calculate passed tests
        report["summary"]["passed"] = (report["summary"]["total_tests"] - 
                                     report["summary"]["failed"] - 
                                     report["summary"]["skipped"])
        
        # Write JSON report
        report_file = self.output_dir / "test_report.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
            
        # Generate HTML summary
        self._generate_test_html_report(report)
        
        print(f"✅ Test report generated: {report_file}")
        return report
        
    def generate_build_report(self, build_dir):
        """Generate build artifact report"""
        build_path = Path(build_dir)
        if not build_path.exists():
            print(f"⚠️ Build directory not found: {build_dir}")
            return
            
        report = {
            "timestamp": self.timestamp,
            "build_artifacts": [],
            "environments": {},
            "summary": {
                "total_artifacts": 0,
                "total_size_bytes": 0
            }
        }
        
        # Scan for firmware files
        for env_dir in build_path.iterdir():
            if env_dir.is_dir():
                env_info = {
                    "name": env_dir.name,
                    "artifacts": []
                }
                
                # Look for common firmware files
                for pattern in ["*.hex", "*.bin", "*.elf"]:
                    for artifact in env_dir.glob(pattern):
                        if artifact.is_file():
                            size = artifact.stat().st_size
                            artifact_info = {
                                "name": artifact.name,
                                "path": str(artifact.relative_to(build_path)),
                                "size_bytes": size,
                                "size_human": self._format_bytes(size),
                                "modified": datetime.fromtimestamp(artifact.stat().st_mtime).isoformat()
                            }
                            
                            env_info["artifacts"].append(artifact_info)
                            report["build_artifacts"].append(artifact_info)
                            report["summary"]["total_size_bytes"] += size
                            report["summary"]["total_artifacts"] += 1
                
                if env_info["artifacts"]:
                    report["environments"][env_dir.name] = env_info
        
        # Write report
        report_file = self.output_dir / "build_report.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
            
        print(f"✅ Build report generated: {report_file}")
        return report
        
    def generate_coverage_report(self, coverage_dir=None):
        """Generate coverage report summary"""
        if coverage_dir:
            coverage_path = Path(coverage_dir)
        else:
            coverage_path = Path("coverage")
            
        report = {
            "timestamp": self.timestamp,
            "coverage_available": coverage_path.exists(),
            "summary": {
                "line_coverage": 0.0,
                "branch_coverage": 0.0,
                "function_coverage": 0.0
            }
        }
        
        if coverage_path.exists():
            # Look for coverage files
            for coverage_file in coverage_path.glob("*.json"):
                try:
                    with open(coverage_file, 'r') as f:
                        coverage_data = json.load(f)
                        if "summary" in coverage_data:
                            report["summary"].update(coverage_data["summary"])
                            break
                except (json.JSONDecodeError, KeyError):
                    continue
        
        # Write report
        report_file = self.output_dir / "coverage_report.json"
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
            
        print(f"✅ Coverage report generated: {report_file}")
        return report
        
    def _generate_test_html_report(self, report):
        """Generate HTML test report"""
        html_content = f"""
<!DOCTYPE html>
<html>
<head>
    <title>Multi-Sonicator CI Test Report</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .summary {{ background: #f5f5f5; padding: 15px; border-radius: 5px; margin-bottom: 20px; }}
        .passed {{ color: green; }}
        .failed {{ color: red; }}
        .skipped {{ color: orange; }}
        table {{ border-collapse: collapse; width: 100%; }}
        th, td {{ border: 1px solid #ddd; padding: 8px; text-align: left; }}
        th {{ background-color: #f2f2f2; }}
    </style>
</head>
<body>
    <h1>Multi-Sonicator CI Test Report</h1>
    <p>Generated: {report['timestamp']}</p>
    
    <div class="summary">
        <h2>Summary</h2>
        <p><strong>Total Tests:</strong> {report['summary']['total_tests']}</p>
        <p><strong class="passed">Passed:</strong> {report['summary']['passed']}</p>
        <p><strong class="failed">Failed:</strong> {report['summary']['failed']}</p>
        <p><strong class="skipped">Skipped:</strong> {report['summary']['skipped']}</p>
        <p><strong>Execution Time:</strong> {report['summary']['execution_time']:.2f}s</p>
    </div>
    
    <h2>Test Suites</h2>
    <table>
        <tr>
            <th>Suite</th>
            <th>Tests</th>
            <th>Passed</th>
            <th>Failed</th>
            <th>Skipped</th>
            <th>Time (s)</th>
        </tr>
"""
        
        for suite in report['test_suites']:
            passed = suite['tests'] - suite['failures'] - suite['errors'] - suite['skipped']
            html_content += f"""
        <tr>
            <td>{suite['name']}</td>
            <td>{suite['tests']}</td>
            <td class="passed">{passed}</td>
            <td class="failed">{suite['failures'] + suite['errors']}</td>
            <td class="skipped">{suite['skipped']}</td>
            <td>{suite['time']:.2f}</td>
        </tr>
"""
        
        html_content += """
    </table>
</body>
</html>
"""
        
        html_file = self.output_dir / "test_report.html"
        with open(html_file, 'w') as f:
            f.write(html_content)
            
    def _format_bytes(self, bytes_value):
        """Format bytes in human readable format"""
        for unit in ['B', 'KB', 'MB', 'GB']:
            if bytes_value < 1024.0:
                return f"{bytes_value:.1f} {unit}"
            bytes_value /= 1024.0
        return f"{bytes_value:.1f} TB"

def main():
    parser = argparse.ArgumentParser(description='Generate CI/CD pipeline reports')
    parser.add_argument('--test-results', help='Path to test results directory')
    parser.add_argument('--build-dir', help='Path to build directory')
    parser.add_argument('--coverage-dir', help='Path to coverage directory')
    parser.add_argument('--output-dir', default='reports', help='Output directory for reports')
    
    args = parser.parse_args()
    
    generator = CIReportGenerator(args.output_dir)
    
    print("🔄 Generating CI/CD reports...")
    
    if args.test_results:
        generator.generate_test_report(args.test_results)
        
    if args.build_dir:
        generator.generate_build_report(args.build_dir)
        
    generator.generate_coverage_report(args.coverage_dir)
    
    print("✅ All reports generated successfully")

if __name__ == "__main__":
    main()
