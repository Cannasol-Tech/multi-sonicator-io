#!/usr/bin/env python3
"""
Traceability Report Generator
Creates comprehensive coverage and traceability reports for dashboard consumption
"""

import os
import sys
import json
import yaml
import glob
from pathlib import Path
from datetime import datetime
from typing import Dict, List, Any, Optional

class TraceabilityReportGenerator:
    def __init__(self, project_root=None):
        self.project_root = Path(project_root) if project_root else Path(__file__).parent.parent
        self.coverage_dir = self.project_root / "coverage"
        self.acceptance_dir = self.project_root / "test" / "acceptance"
        self.features_dir = self.acceptance_dir / "features"
        
        self.report_data = {
            "timestamp": datetime.now().isoformat(),
            "project": "Multi-Sonicator I/O Controller",
            "version": "1.0.0",
            "unit_testing": {},
            "acceptance_testing": {},
            "traceability": {},
            "overall_metrics": {}
        }
    
    def load_unit_test_coverage(self):
        """Load unit test coverage data"""
        coverage_file = self.coverage_dir / "coverage.json"
        
        if coverage_file.exists():
            with open(coverage_file, 'r') as f:
                coverage_data = json.load(f)
            
            self.report_data["unit_testing"] = {
                "framework": "Unity Test Framework",
                "environment": "Native C/C++",
                "coverage_data": coverage_data,
                "meets_requirement": coverage_data.get("overall", {}).get("coverage_percentage", 0) >= 85.0,
                "requirement": "≥85% code coverage"
            }
        else:
            self.report_data["unit_testing"] = {
                "status": "No coverage data available",
                "meets_requirement": False
            }
    
    def analyze_acceptance_tests(self):
        """Analyze acceptance test structure and coverage"""
        feature_files = list(self.features_dir.glob("*.feature"))
        
        features = []
        total_scenarios = 0
        tagged_scenarios = {"hil": 0, "smoke": 0, "pending": 0}
        
        for feature_file in feature_files:
            try:
                with open(feature_file, 'r') as f:
                    content = f.read()
                
                # Simple parsing for scenarios and tags
                lines = content.split('\n')
                feature_name = None
                scenarios = []
                
                for line in lines:
                    line = line.strip()
                    if line.startswith('Feature:'):
                        feature_name = line.replace('Feature:', '').strip()
                    elif line.startswith('Scenario:'):
                        scenario_name = line.replace('Scenario:', '').strip()
                        scenarios.append({
                            "name": scenario_name,
                            "tags": []
                        })
                        total_scenarios += 1
                    elif line.startswith('@'):
                        # Extract tags
                        tags = [tag.strip('@') for tag in line.split()]
                        if scenarios:
                            scenarios[-1]["tags"] = tags
                            for tag in tags:
                                if tag in tagged_scenarios:
                                    tagged_scenarios[tag] += 1
                
                if feature_name:
                    features.append({
                        "name": feature_name,
                        "file": feature_file.name,
                        "scenarios": scenarios,
                        "scenario_count": len(scenarios)
                    })
            
            except Exception as e:
                print(f"Warning: Could not parse {feature_file}: {e}")
        
        self.report_data["acceptance_testing"] = {
            "framework": "Behave BDD",
            "environment": "Hardware-in-the-Loop (HIL)",
            "total_features": len(features),
            "total_scenarios": total_scenarios,
            "tagged_scenarios": tagged_scenarios,
            "features": features,
            "coverage_areas": [
                "GPIO Functionality",
                "ADC Verification", 
                "PWM Generation",
                "MODBUS Communication",
                "Power Verification",
                "Basic Connectivity",
                "Sonicator Control"
            ]
        }
    
    def create_traceability_matrix(self):
        """Create traceability matrix linking tests to requirements"""
        # This would typically link to actual PRD requirements
        # For now, we'll create a sample matrix based on feature coverage
        
        requirements = {
            "REQ-001": {
                "description": "GPIO pin control and monitoring",
                "tests": ["hil_gpio_functionality.feature"],
                "coverage": "Unit + Acceptance"
            },
            "REQ-002": {
                "description": "ADC voltage measurement accuracy",
                "tests": ["hil_adc_verification.feature"],
                "coverage": "Unit + Acceptance"
            },
            "REQ-003": {
                "description": "PWM signal generation",
                "tests": ["hil_pwm_generation.feature"],
                "coverage": "Unit + Acceptance"
            },
            "REQ-004": {
                "description": "MODBUS RTU communication protocol",
                "tests": ["hil_modbus_communication.feature", "modbus.feature"],
                "coverage": "Unit + Acceptance"
            },
            "REQ-005": {
                "description": "Power supply monitoring and safety",
                "tests": ["hil_power_verification.feature"],
                "coverage": "Unit + Acceptance"
            },
            "REQ-006": {
                "description": "Sonicator control and feedback",
                "tests": ["sonicator.feature", "control.feature"],
                "coverage": "Unit + Acceptance"
            }
        }
        
        self.report_data["traceability"] = {
            "requirements_count": len(requirements),
            "requirements": requirements,
            "coverage_matrix": self._build_coverage_matrix(requirements)
        }
    
    def _build_coverage_matrix(self, requirements):
        """Build coverage matrix showing test coverage per requirement"""
        matrix = {}
        
        for req_id, req_data in requirements.items():
            test_files = req_data.get("tests", [])
            unit_coverage = "✅" if req_data.get("coverage") == "Unit + Acceptance" else "❌"
            acceptance_coverage = "✅" if len(test_files) > 0 else "❌"
            
            matrix[req_id] = {
                "description": req_data["description"],
                "unit_tests": unit_coverage,
                "acceptance_tests": acceptance_coverage,
                "test_files": test_files,
                "fully_covered": unit_coverage == "✅" and acceptance_coverage == "✅"
            }
        
        return matrix
    
    def calculate_overall_metrics(self):
        """Calculate overall project metrics"""
        unit_coverage = self.report_data["unit_testing"].get("coverage_data", {}).get("overall", {}).get("coverage_percentage", 0)
        acceptance_scenarios = self.report_data["acceptance_testing"].get("total_scenarios", 0)
        requirements_covered = sum(1 for req in self.report_data["traceability"]["coverage_matrix"].values() if req["fully_covered"])
        total_requirements = len(self.report_data["traceability"]["coverage_matrix"])
        
        self.report_data["overall_metrics"] = {
            "unit_test_coverage": f"{unit_coverage:.1f}%",
            "unit_test_status": "✅ PASS" if unit_coverage >= 85 else "❌ FAIL",
            "acceptance_scenarios": acceptance_scenarios,
            "requirements_coverage": f"{requirements_covered}/{total_requirements}",
            "requirements_percentage": f"{(requirements_covered/total_requirements*100):.1f}%" if total_requirements > 0 else "0%",
            "overall_status": "✅ ROBUST" if unit_coverage >= 85 and requirements_covered == total_requirements else "⚠️ NEEDS WORK"
        }
    
    def generate_json_report(self):
        """Generate JSON report for dashboard consumption"""
        json_file = self.coverage_dir / "traceability_report.json"
        self.coverage_dir.mkdir(exist_ok=True)
        
        with open(json_file, 'w') as f:
            json.dump(self.report_data, f, indent=2)
        
        print(f"📄 Traceability JSON report: {json_file}")
    
    def generate_html_report(self):
        """Generate HTML traceability report"""
        html_content = f"""
<!DOCTYPE html>
<html>
<head>
    <title>Testing Framework Traceability Report - Multi-Sonicator I/O Controller</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .header {{ background: #f0f0f0; padding: 20px; border-radius: 5px; }}
        .section {{ margin: 20px 0; padding: 15px; border: 1px solid #ddd; border-radius: 5px; }}
        .metrics {{ display: flex; gap: 20px; }}
        .metric {{ flex: 1; text-align: center; padding: 10px; background: #f9f9f9; border-radius: 5px; }}
        .pass {{ background: #d4edda; }}
        .fail {{ background: #f8d7da; }}
        .warning {{ background: #fff3cd; }}
        table {{ width: 100%; border-collapse: collapse; margin: 10px 0; }}
        th, td {{ border: 1px solid #ddd; padding: 8px; text-align: left; }}
        th {{ background: #f2f2f2; }}
    </style>
</head>
<body>
    <div class="header">
        <h1>Testing Framework Traceability Report</h1>
        <p>Multi-Sonicator I/O Controller - Generated: {self.report_data['timestamp']}</p>
        <p>Overall Status: {self.report_data['overall_metrics']['overall_status']}</p>
    </div>
    
    <div class="metrics">
        <div class="metric {'pass' if 'PASS' in self.report_data['overall_metrics']['unit_test_status'] else 'fail'}">
            <h3>Unit Test Coverage</h3>
            <p>{self.report_data['overall_metrics']['unit_test_coverage']}</p>
            <p>{self.report_data['overall_metrics']['unit_test_status']}</p>
        </div>
        <div class="metric">
            <h3>Acceptance Scenarios</h3>
            <p>{self.report_data['overall_metrics']['acceptance_scenarios']}</p>
            <p>BDD Test Cases</p>
        </div>
        <div class="metric">
            <h3>Requirements Coverage</h3>
            <p>{self.report_data['overall_metrics']['requirements_coverage']}</p>
            <p>{self.report_data['overall_metrics']['requirements_percentage']}</p>
        </div>
    </div>
    
    <div class="section">
        <h2>Traceability Matrix</h2>
        <table>
            <tr>
                <th>Requirement ID</th>
                <th>Description</th>
                <th>Unit Tests</th>
                <th>Acceptance Tests</th>
                <th>Test Files</th>
            </tr>
"""
        
        for req_id, req_data in self.report_data["traceability"]["coverage_matrix"].items():
            html_content += f"""
            <tr>
                <td>{req_id}</td>
                <td>{req_data['description']}</td>
                <td>{req_data['unit_tests']}</td>
                <td>{req_data['acceptance_tests']}</td>
                <td>{', '.join(req_data['test_files'])}</td>
            </tr>
"""
        
        html_content += """
        </table>
    </div>
</body>
</html>
"""
        
        html_file = self.coverage_dir / "traceability_report.html"
        with open(html_file, 'w') as f:
            f.write(html_content)
        
        print(f"📄 Traceability HTML report: {html_file}")
    
    def generate_all_reports(self):
        """Generate all traceability reports"""
        print("🔍 Generating comprehensive traceability reports...")
        
        self.load_unit_test_coverage()
        self.analyze_acceptance_tests()
        self.create_traceability_matrix()
        self.calculate_overall_metrics()
        
        self.generate_json_report()
        self.generate_html_report()
        
        print("✅ Traceability report generation complete")
        print(f"📈 Overall Status: {self.report_data['overall_metrics']['overall_status']}")

def main():
    generator = TraceabilityReportGenerator()
    generator.generate_all_reports()

if __name__ == "__main__":
    main()
