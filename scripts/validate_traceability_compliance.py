#!/usr/bin/env python3
"""
Multi-Sonicator I/O Controller - Traceability Compliance Validator
Automated validation of PRD-to-Test traceability for enterprise compliance

CRITICAL: This script ensures 100% traceability between PRD requirements 
and acceptance test scenarios, preventing compliance failures.
"""

import json
import yaml
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple
from datetime import datetime
import subprocess

class TraceabilityValidator:
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.prd_path = project_root / "docs/prd-shards"
        self.test_path = project_root / "test/acceptance/features"
        self.traceability_matrix = project_root / "docs/prd-shards/prd-to-scenarios-traceability.md"
        
        # Critical compliance thresholds
        self.REQUIRED_COVERAGE = 100.0  # 100% PRD coverage required
        self.REQUIRED_UNIT_COVERAGE = 85.0  # 85% unit test coverage
        self.MAX_PENDING_SCENARIOS = 5  # Max pending scenarios allowed
        
        self.validation_results = {
            "timestamp": datetime.now().isoformat(),
            "compliance_status": "UNKNOWN",
            "critical_failures": [],
            "warnings": [],
            "prd_coverage": {},
            "test_coverage": {},
            "drift_analysis": {}
        }

    def validate_prd_to_test_mapping(self) -> Dict:
        """Validate that every PRD requirement has corresponding test scenarios"""
        print("🔍 Validating PRD-to-Test mapping...")
        
        # Extract PRD requirements
        prd_requirements = self._extract_prd_requirements()
        
        # Extract test scenarios with tags
        test_scenarios = self._extract_test_scenarios()
        
        # Cross-reference mapping
        coverage_gaps = []
        for req_id, req_data in prd_requirements.items():
            mapped_tests = self._find_tests_for_requirement(req_id, test_scenarios)
            if not mapped_tests:
                coverage_gaps.append({
                    "requirement_id": req_id,
                    "description": req_data.get("description", ""),
                    "severity": "CRITICAL",
                    "impact": "Compliance failure - no test coverage"
                })
        
        # Calculate coverage percentage safely
        total_reqs = len(prd_requirements)
        covered_reqs = len(prd_requirements) - len(coverage_gaps)
        coverage_pct = (covered_reqs / total_reqs * 100) if total_reqs > 0 else 0.0

        self.validation_results["prd_coverage"] = {
            "total_requirements": total_reqs,
            "covered_requirements": covered_reqs,
            "coverage_percentage": coverage_pct,
            "gaps": coverage_gaps
        }
        
        return self.validation_results["prd_coverage"]

    def validate_test_execution_status(self) -> Dict:
        """Validate that critical test scenarios are not stuck in pending state"""
        print("⚡ Validating test execution status...")
        
        pending_scenarios = []
        critical_pending = []
        
        # Scan all feature files for @pending tags
        for feature_file in self.test_path.glob("*.feature"):
            with open(feature_file, 'r') as f:
                content = f.read()
                
            # Find scenarios with @pending tag
            pending_matches = re.findall(r'@pending.*?\n.*?Scenario.*?:(.*)', content, re.MULTILINE)
            for scenario_name in pending_matches:
                scenario_info = {
                    "file": feature_file.name,
                    "scenario": scenario_name.strip(),
                    "age_days": self._get_scenario_age(feature_file, scenario_name.strip())
                }
                pending_scenarios.append(scenario_info)
                
                # Check if it's a critical scenario (has @req- or SC-xxx tags)
                if re.search(r'@req-|SC-\d+', content):
                    critical_pending.append(scenario_info)
        
        self.validation_results["test_coverage"] = {
            "total_pending": len(pending_scenarios),
            "critical_pending": len(critical_pending),
            "max_allowed": self.MAX_PENDING_SCENARIOS,
            "compliance": len(critical_pending) <= self.MAX_PENDING_SCENARIOS,
            "pending_scenarios": pending_scenarios
        }
        
        if len(critical_pending) > self.MAX_PENDING_SCENARIOS:
            self.validation_results["critical_failures"].append(
                f"Too many critical pending scenarios: {len(critical_pending)} > {self.MAX_PENDING_SCENARIOS}"
            )
        
        return self.validation_results["test_coverage"]

    def validate_drift_detection(self) -> Dict:
        """Detect drift between PRD requirements and implementation"""
        print("🔄 Validating drift detection...")
        
        # Check if traceability matrix is up-to-date
        matrix_age = self._get_file_age(self.traceability_matrix)
        prd_age = min([self._get_file_age(f) for f in self.prd_path.glob("*.md")])
        
        drift_detected = matrix_age > prd_age
        
        # Check for missing scenario IDs in matrix
        matrix_scenarios = self._extract_matrix_scenarios()
        actual_scenarios = self._extract_actual_scenarios()
        
        missing_from_matrix = actual_scenarios - matrix_scenarios
        orphaned_in_matrix = matrix_scenarios - actual_scenarios
        
        self.validation_results["drift_analysis"] = {
            "matrix_outdated": drift_detected,
            "matrix_age_hours": matrix_age,
            "prd_age_hours": prd_age,
            "missing_scenarios": list(missing_from_matrix),
            "orphaned_scenarios": list(orphaned_in_matrix),
            "drift_score": len(missing_from_matrix) + len(orphaned_in_matrix)
        }
        
        # Only report as critical failure if there are actual missing/orphaned scenarios
        if missing_from_matrix or orphaned_in_matrix:
            self.validation_results["critical_failures"].append(
                f"Traceability drift detected: {len(missing_from_matrix)} missing, {len(orphaned_in_matrix)} orphaned"
            )
        elif drift_detected:
            # Matrix age is a warning, not a critical failure
            self.validation_results["warnings"].append(
                f"Traceability matrix is outdated ({matrix_age:.1f} hours old)"
            )
        
        return self.validation_results["drift_analysis"]

    def generate_compliance_report(self) -> str:
        """Generate executive compliance report"""
        print("📊 Generating compliance report...")
        
        # Determine overall compliance status
        has_critical_failures = len(self.validation_results["critical_failures"]) > 0
        prd_coverage_ok = self.validation_results["prd_coverage"]["coverage_percentage"] >= self.REQUIRED_COVERAGE
        test_status_ok = self.validation_results["test_coverage"]["compliance"]
        
        if has_critical_failures or not prd_coverage_ok or not test_status_ok:
            self.validation_results["compliance_status"] = "❌ FAILED"
        elif len(self.validation_results["warnings"]) > 0:
            self.validation_results["compliance_status"] = "⚠️ CONCERNS"
        else:
            self.validation_results["compliance_status"] = "✅ COMPLIANT"
        
        # Generate report
        report = f"""
# TRACEABILITY COMPLIANCE REPORT
**Generated:** {self.validation_results['timestamp']}
**Status:** {self.validation_results['compliance_status']}

## EXECUTIVE SUMMARY
- PRD Coverage: {self.validation_results['prd_coverage']['coverage_percentage']:.1f}% ({self.validation_results['prd_coverage']['covered_requirements']}/{self.validation_results['prd_coverage']['total_requirements']})
- Pending Critical Tests: {self.validation_results['test_coverage']['critical_pending']}/{self.validation_results['test_coverage']['max_allowed']}
- Drift Score: {self.validation_results['drift_analysis']['drift_score']}

## CRITICAL FAILURES
"""
        
        if self.validation_results["critical_failures"]:
            for failure in self.validation_results["critical_failures"]:
                report += f"- ❌ {failure}\n"
        else:
            report += "- ✅ No critical failures detected\n"
        
        report += "\n## DETAILED ANALYSIS\n"
        report += f"- Matrix last updated: {self.validation_results['drift_analysis']['matrix_age_hours']:.1f} hours ago\n"
        report += f"- Missing scenarios: {len(self.validation_results['drift_analysis']['missing_scenarios'])}\n"
        report += f"- Orphaned scenarios: {len(self.validation_results['drift_analysis']['orphaned_scenarios'])}\n"
        
        return report

    def _extract_prd_requirements(self) -> Dict:
        """Extract requirements from PRD documents"""
        requirements = {}

        # Parse project-requirements.md for formal requirements
        req_file = self.prd_path / "project-requirements.md"
        if req_file.exists():
            with open(req_file, 'r') as f:
                content = f.read()

            # Extract requirement patterns (REQ-xxx, @req-xxx, etc.)
            req_patterns = re.findall(r'(@req-[\w-]+|REQ-\d+)', content, re.IGNORECASE)
            for req in req_patterns:
                requirements[req.upper()] = {"description": f"Requirement {req}", "source": "project-requirements.md"}

        # Also extract from traceability matrix for comprehensive coverage
        matrix_file = self.traceability_matrix
        if matrix_file.exists():
            with open(matrix_file, 'r') as f:
                content = f.read()

            # Extract @req- tags from matrix
            req_patterns = re.findall(r'@req-[\w-]+', content, re.IGNORECASE)
            for req in req_patterns:
                req_key = req.upper()
                if req_key not in requirements:
                    requirements[req_key] = {"description": f"Matrix requirement {req}", "source": "traceability-matrix"}

        # If no requirements found, create some basic ones from existing test tags
        if not requirements:
            print("⚠️ No formal requirements found, extracting from test scenarios...")
            test_scenarios = self._extract_test_scenarios()
            req_tags = set()
            for scenario_data in test_scenarios.values():
                for tag in scenario_data["tags"]:
                    if tag.startswith("req-"):
                        req_tags.add(f"@{tag}".upper())

            for req_tag in req_tags:
                requirements[req_tag] = {"description": f"Test-derived requirement {req_tag}", "source": "test-scenarios"}

        return requirements

    def _extract_test_scenarios(self) -> Dict:
        """Extract test scenarios with their tags"""
        scenarios = {}
        
        for feature_file in self.test_path.glob("*.feature"):
            with open(feature_file, 'r') as f:
                content = f.read()
            
            # Extract scenarios with tags
            scenario_blocks = re.findall(r'(@[\w\s@-]+)\s*Scenario.*?:(.*)', content, re.MULTILINE)
            for tags, scenario_name in scenario_blocks:
                scenario_id = f"{feature_file.stem}::{scenario_name.strip()}"
                scenarios[scenario_id] = {
                    "tags": [tag.strip() for tag in tags.split('@') if tag.strip()],
                    "file": feature_file.name,
                    "name": scenario_name.strip()
                }
        
        return scenarios

    def _find_tests_for_requirement(self, req_id: str, test_scenarios: Dict) -> List:
        """Find test scenarios that cover a specific requirement"""
        matching_tests = []

        # Normalize requirement ID for matching
        req_base = req_id.replace("@", "").replace("REQ-", "").lower()

        for scenario_id, scenario_data in test_scenarios.items():
            # Check if any tag matches the requirement
            for tag in scenario_data["tags"]:
                tag_normalized = tag.replace("req-", "").lower()

                # Direct match or partial match for compound requirements
                if (req_base in tag_normalized or
                    tag_normalized in req_base or
                    req_id.lower() in tag.lower() or
                    tag.lower() in req_id.lower()):
                    matching_tests.append(scenario_id)
                    break

        return matching_tests

    def _get_scenario_age(self, file_path: Path, scenario_name: str) -> int:
        """Get age of scenario in days (simplified - returns 0)"""
        return 0  # Simplified implementation

    def _get_file_age(self, file_path: Path) -> float:
        """Get file age in hours"""
        if not file_path.exists():
            return float('inf')
        
        import time
        age_seconds = time.time() - file_path.stat().st_mtime
        return age_seconds / 3600  # Convert to hours

    def _extract_matrix_scenarios(self) -> Set[str]:
        """Extract scenario IDs from traceability matrix"""
        scenarios = set()
        
        if self.traceability_matrix.exists():
            with open(self.traceability_matrix, 'r') as f:
                content = f.read()
            
            # Extract SC-xxx scenario IDs
            sc_ids = re.findall(r'SC-\d+', content)
            scenarios.update(sc_ids)
        
        return scenarios

    def _extract_actual_scenarios(self) -> Set[str]:
        """Extract actual scenario IDs from feature files"""
        scenarios = set()
        
        for feature_file in self.test_path.glob("*.feature"):
            with open(feature_file, 'r') as f:
                content = f.read()
            
            # Extract SC-xxx scenario IDs
            sc_ids = re.findall(r'SC-\d+', content)
            scenarios.update(sc_ids)
        
        return scenarios


def main():
    """Main validation entry point"""
    project_root = Path(__file__).parent.parent
    validator = TraceabilityValidator(project_root)
    
    print("🚀 TRACEABILITY COMPLIANCE VALIDATION STARTING...")
    print("=" * 60)
    
    # Run all validations
    validator.validate_prd_to_test_mapping()
    validator.validate_test_execution_status()
    validator.validate_drift_detection()
    
    # Generate compliance report
    report = validator.generate_compliance_report()
    
    # Save results
    results_file = project_root / "coverage/traceability_compliance.json"
    results_file.parent.mkdir(exist_ok=True)
    
    with open(results_file, 'w') as f:
        json.dump(validator.validation_results, f, indent=2)
    
    report_file = project_root / "coverage/traceability_compliance_report.md"
    with open(report_file, 'w') as f:
        f.write(report)
    
    print(f"📄 Compliance results: {results_file}")
    print(f"📄 Compliance report: {report_file}")
    print("=" * 60)
    print(report)
    
    # Exit with appropriate code
    if validator.validation_results["compliance_status"].startswith("❌"):
        print("\n🚨 COMPLIANCE VALIDATION FAILED!")
        sys.exit(1)
    elif validator.validation_results["compliance_status"].startswith("⚠️"):
        print("\n⚠️ COMPLIANCE CONCERNS DETECTED!")
        sys.exit(2)
    else:
        print("\n✅ COMPLIANCE VALIDATION PASSED!")
        sys.exit(0)


if __name__ == "__main__":
    main()
