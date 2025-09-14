#!/usr/bin/env python3
"""
Generate Full Executive Report for Manual Test Execution
Combines unit test results with acceptance test results for complete release reporting.
"""

import json
import argparse
from pathlib import Path
from datetime import datetime
import subprocess
import os


def get_git_info():
    """Get git commit and tag information."""
    try:
        commit = subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode().strip()
        try:
            # Try to get current tag
            tag = subprocess.check_output(['git', 'describe', '--tags', '--exact-match']).decode().strip()
        except subprocess.CalledProcessError:
            # If no exact tag, use latest tag + commit
            try:
                tag = subprocess.check_output(['git', 'describe', '--tags']).decode().strip()
            except subprocess.CalledProcessError:
                tag = "v0.0.0-dev"
        return commit[:7], tag
    except subprocess.CalledProcessError:
        return "unknown", "v0.0.0-dev"


def combine_test_results(unit_results, acceptance_results, output_dir):
    """Combine unit and acceptance test results into full executive report."""
    commit, tag = get_git_info()
    timestamp = datetime.utcnow().isoformat() + 'Z'
    
    output_dir = Path(output_dir)
    output_dir.mkdir(exist_ok=True)
    
    # Load unit test results
    unit_data = {}
    if unit_results and Path(unit_results).exists():
        try:
            with open(unit_results, 'r') as f:
                unit_data = json.load(f)
        except Exception as e:
            print(f"Warning: Could not load unit test results: {e}")
    
    # Load acceptance test results
    acceptance_data = {}
    if acceptance_results and Path(acceptance_results).exists():
        try:
            with open(acceptance_results, 'r') as f:
                acceptance_data = json.load(f)
        except Exception as e:
            print(f"Warning: Could not load acceptance test results: {e}")
    
    # Combine scenarios
    all_scenarios = []
    all_scenarios.extend(unit_data.get('scenarios', []))
    all_scenarios.extend(acceptance_data.get('scenarios', []))
    
    # Combine requirements
    all_requirements = []
    all_requirements.extend(unit_data.get('requirements', []))
    all_requirements.extend(acceptance_data.get('requirements', []))
    
    # Calculate combined summary
    unit_summary = unit_data.get('summary', {"total": 0, "passed": 0, "failed": 0, "skipped": 0, "durationMs": 0})
    acceptance_summary = acceptance_data.get('summary', {"total": 0, "passed": 0, "failed": 0, "skipped": 0, "durationMs": 0})
    
    combined_summary = {
        "total": unit_summary["total"] + acceptance_summary["total"],
        "passed": unit_summary["passed"] + acceptance_summary["passed"],
        "failed": unit_summary["failed"] + acceptance_summary["failed"],
        "skipped": unit_summary["skipped"] + acceptance_summary["skipped"],
        "durationMs": unit_summary["durationMs"] + acceptance_summary["durationMs"]
    }
    
    # Generate combined executive report
    executive_report = {
        "version": "1.0.0",
        "owner": "Cannasol-Tech",
        "repo": "multi-sonicator-io",
        "releaseTag": tag,
        "commit": commit,
        "createdAt": timestamp,
        "testType": "complete-manual",
        "summary": combined_summary,
        "scenarios": all_scenarios,
        "requirements": all_requirements,
        "testBreakdown": {
            "unit": unit_summary,
            "acceptance": acceptance_summary
        }
    }
    
    with open(output_dir / "executive-report-complete.json", 'w') as f:
        json.dump(executive_report, f, indent=2)
    
    print(f"✅ Generated executive-report-complete.json ({combined_summary['total']} total scenarios)")
    
    # Generate human-readable markdown report
    coverage_pct = 0.0
    if unit_data.get('coverage'):
        coverage_pct = unit_data['coverage'].get('overall', {}).get('coverage_percentage', 0.0)
    
    markdown_content = f"""# Multi-Sonicator I/O Controller - Complete Test Report {tag}

## Executive Summary

**Release**: {tag}  
**Commit**: {commit}  
**Generated**: {timestamp}
**Test Type**: Complete Manual Execution (Unit + Acceptance)

## Combined Test Results

- **Total Test Scenarios**: {combined_summary['total']}
- **Passed**: {combined_summary['passed']} ✅
- **Failed**: {combined_summary['failed']} ❌
- **Skipped**: {combined_summary['skipped']} ⏭️
- **Total Duration**: {combined_summary['durationMs'] / 1000:.1f} seconds

## Test Breakdown

### Unit Tests
- **Scenarios**: {unit_summary['total']}
- **Passed**: {unit_summary['passed']} ✅
- **Failed**: {unit_summary['failed']} ❌
- **Coverage**: {coverage_pct:.1f}%
- **Coverage Requirement (≥85%)**: {"✅ MET" if coverage_pct >= 85.0 else "❌ NOT MET"}

### Acceptance Tests
- **Scenarios**: {acceptance_summary['total']}
- **Passed**: {acceptance_summary['passed']} ✅
- **Failed**: {acceptance_summary['failed']} ❌
- **Framework**: Behave BDD with HIL hardware

## Quality Gates

| Gate | Requirement | Status |
|------|------------|--------|
| Unit Test Coverage | ≥85% | {"✅ PASS" if coverage_pct >= 85.0 else "❌ FAIL"} |
| Acceptance Tests | All critical scenarios pass | {"✅ PASS" if acceptance_summary['failed'] == 0 else "❌ FAIL"} |
| Overall Test Pass Rate | ≥95% | {"✅ PASS" if (combined_summary['passed'] / max(combined_summary['total'], 1)) >= 0.95 else "❌ FAIL"} |

## Release Readiness

{"✅ **READY FOR RELEASE**" if (coverage_pct >= 85.0 and acceptance_summary['failed'] == 0 and (combined_summary['passed'] / max(combined_summary['total'], 1)) >= 0.95) else "❌ **NOT READY FOR RELEASE**"}

{"All quality gates passed. This release meets the comprehensive testing requirements." if (coverage_pct >= 85.0 and acceptance_summary['failed'] == 0 and (combined_summary['passed'] / max(combined_summary['total'], 1)) >= 0.95) else "One or more quality gates failed. Review failed tests before release."}

## Testing Approach

This report combines:
- **Unit Tests**: Automated execution with mocking (CI-compatible)
- **Acceptance Tests**: Manual execution with real HIL hardware
- **Complete Coverage**: Both automated and manual validation

---
*Generated by Multi-Sonicator Complete Test Suite*
"""
    
    with open(output_dir / "executive-report-complete.md", 'w') as f:
        f.write(markdown_content)
    
    print(f"✅ Generated executive-report-complete.md (human-readable)")
    print(f"\n🎯 Complete test artifacts generated in {output_dir}/")
    print("Ready for release validation and GitHub release upload!")


def main():
    parser = argparse.ArgumentParser(description="Generate complete executive report from unit and acceptance test results")
    parser.add_argument("--unit-results", 
                       help="Path to unit test executive report JSON")
    parser.add_argument("--acceptance-results", 
                       help="Path to acceptance test executive report JSON")
    parser.add_argument("--output", default="final",
                       help="Output directory for artifacts (default: final)")
    
    args = parser.parse_args()
    
    if not args.unit_results and not args.acceptance_results:
        print("Error: At least one of --unit-results or --acceptance-results must be provided")
        return 1
    
    combine_test_results(args.unit_results, args.acceptance_results, args.output)
    return 0


if __name__ == "__main__":
    exit(main())
