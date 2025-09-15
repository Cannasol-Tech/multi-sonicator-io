#!/usr/bin/env python3
"""
CI Test Runner
Simulates the CI pipeline locally for testing and validation
"""

import os
import sys
import subprocess
import json
from pathlib import Path
from datetime import datetime

class CITestRunner:
    def __init__(self, project_root=None):
        self.project_root = Path(project_root) if project_root else Path(__file__).parent.parent
        self.results = {
            "timestamp": datetime.now().isoformat(),
            "stages": {},
            "overall_status": "UNKNOWN"
        }
    
    def run_command(self, command, stage_name, timeout=300):
        """Run a command and capture results"""
        print(f"🔄 Running {stage_name}...")
        
        try:
            result = subprocess.run(
                command,
                shell=True,
                cwd=self.project_root,
                capture_output=True,
                text=True,
                timeout=timeout
            )
            
            success = result.returncode == 0
            
            self.results["stages"][stage_name] = {
                "status": "PASS" if success else "FAIL",
                "return_code": result.returncode,
                "stdout": result.stdout,
                "stderr": result.stderr,
                "command": command
            }
            
            if success:
                print(f"✅ {stage_name} - PASSED")
            else:
                print(f"❌ {stage_name} - FAILED")
                print(f"   Error: {result.stderr[:200]}...")
            
            return success
            
        except subprocess.TimeoutExpired:
            print(f"⏰ {stage_name} - TIMEOUT")
            self.results["stages"][stage_name] = {
                "status": "TIMEOUT",
                "return_code": -1,
                "error": "Command timed out"
            }
            return False
            
        except Exception as e:
            print(f"💥 {stage_name} - ERROR: {e}")
            self.results["stages"][stage_name] = {
                "status": "ERROR",
                "return_code": -1,
                "error": str(e)
            }
            return False
    
    def run_validation_stage(self):
        """Run configuration validation stage"""
        print("\n🔍 STAGE 1: Configuration Validation")
        
        commands = [
            ("make validate-config", "Config Validation"),
            ("python3 scripts/manage_pending_scenarios.py", "Pending Scenarios Analysis")
        ]
        
        all_passed = True
        for command, name in commands:
            if not self.run_command(command, name):
                all_passed = False
        
        return all_passed
    
    def run_unit_tests_stage(self):
        """Run unit tests stage"""
        print("\n🧪 STAGE 2: Unit Tests")
        
        commands = [
            ("make test-unit", "Unit Tests with Coverage"),
            ("make generate-traceability-report", "Traceability Report Generation")
        ]
        
        all_passed = True
        for command, name in commands:
            if not self.run_command(command, name):
                all_passed = False
        
        # Check coverage requirement
        coverage_file = self.project_root / "coverage" / "coverage.json"
        if coverage_file.exists():
            try:
                with open(coverage_file, 'r') as f:
                    data = json.load(f)
                coverage = data['overall']['coverage_percentage']
                
                if coverage >= 85.0:
                    print(f"✅ Coverage Requirement - PASSED ({coverage:.1f}%)")
                    self.results["stages"]["Coverage Check"] = {
                        "status": "PASS",
                        "coverage": coverage
                    }
                else:
                    print(f"❌ Coverage Requirement - FAILED ({coverage:.1f}% < 85%)")
                    self.results["stages"]["Coverage Check"] = {
                        "status": "FAIL",
                        "coverage": coverage
                    }
                    all_passed = False
            except Exception as e:
                print(f"⚠️ Could not check coverage: {e}")
        
        return all_passed
    
    def run_acceptance_tests_stage(self):
        """Run acceptance tests stage (mock for CI)"""
        print("\n🎭 STAGE 3: Acceptance Tests (Mock)")
        
        # For CI, we validate BDD structure and syntax, ignoring undefined step implementations
        print("🔄 Running BDD Syntax Validation...")
        
        try:
            result = subprocess.run(
                ["python3", "-m", "behave", "--dry-run", "--tags=not @hil", "features/"],
                cwd=self.project_root / "test/acceptance",
                capture_output=True,
                text=True,
                timeout=120
            )
            
            # For BDD syntax validation, we check if features can be parsed, not if steps are implemented
            # Look for parsing errors rather than undefined step errors
            has_syntax_errors = False
            output = result.stdout + result.stderr
            
            # Check for actual syntax/parsing errors (not just undefined steps)
            syntax_error_indicators = [
                "SyntaxError",
                "ParseError", 
                "ConfigurationError",
                "feature file not found",
                "failed to load feature",
                "BadFeatureError"
            ]
            
            for error_type in syntax_error_indicators:
                if error_type.lower() in output.lower():
                    has_syntax_errors = True
                    break
            
            # Check if features were found and parsed
            features_found = "Feature:" in output or "Scenario:" in output
            
            if has_syntax_errors:
                print("❌ BDD Syntax Validation - FAILED")
                print(f"   Syntax errors found in BDD features")
                self.results["stages"]["BDD Syntax Validation"] = {
                    "status": "FAIL",
                    "error": "Syntax errors in BDD features",
                    "output": output[:500]
                }
                return False
            elif not features_found:
                print("❌ BDD Syntax Validation - FAILED") 
                print(f"   No BDD features found or parsed")
                self.results["stages"]["BDD Syntax Validation"] = {
                    "status": "FAIL",
                    "error": "No BDD features found",
                    "output": output[:500]
                }
                return False
            else:
                print("✅ BDD Syntax Validation - PASSED")
                print("   BDD features are syntactically valid (undefined steps are expected in CI)")
                self.results["stages"]["BDD Syntax Validation"] = {
                    "status": "PASS",
                    "note": "Syntax valid, undefined steps ignored for CI",
                    "return_code": result.returncode
                }
                return True
                
        except Exception as e:
            print(f"❌ BDD Syntax Validation - FAILED")
            print(f"   Exception during validation: {e}")
            self.results["stages"]["BDD Syntax Validation"] = {
                "status": "FAIL",
                "error": f"Exception: {str(e)}"
            }
            return False
    
    def run_integration_stage(self):
        """Run integration and reporting stage"""
        print("\n📊 STAGE 4: Integration Report")
        
        # Generate final reports
        success = self.run_command(
            "python3 scripts/generate_traceability_report.py",
            "Final Integration Report"
        )
        
        return success
    
    def generate_summary(self):
        """Generate test summary"""
        print("\n" + "="*60)
        print("🏁 CI PIPELINE SUMMARY")
        print("="*60)
        
        total_stages = len(self.results["stages"])
        passed_stages = sum(1 for stage in self.results["stages"].values() 
                           if stage["status"] == "PASS")
        
        print(f"📊 Stages: {passed_stages}/{total_stages} passed")
        
        # Overall status
        if passed_stages == total_stages:
            self.results["overall_status"] = "PASS"
            print("🎉 Overall Status: ✅ PASS")
        else:
            self.results["overall_status"] = "FAIL"
            print("💥 Overall Status: ❌ FAIL")
        
        print("\n📋 Stage Details:")
        for stage_name, stage_result in self.results["stages"].items():
            status_icon = "✅" if stage_result["status"] == "PASS" else "❌"
            print(f"   {status_icon} {stage_name}: {stage_result['status']}")
        
        # Coverage information
        if "Coverage Check" in self.results["stages"]:
            coverage = self.results["stages"]["Coverage Check"].get("coverage", 0)
            print(f"\n📈 Code Coverage: {coverage:.1f}%")
        
        # Save results
        results_file = self.project_root / "ci_results.json"
        with open(results_file, 'w') as f:
            json.dump(self.results, f, indent=2)
        print(f"💾 Results saved to: {results_file}")
        
        return self.results["overall_status"] == "PASS"
    
    def run_full_pipeline(self):
        """Run the complete CI pipeline"""
        print("🚀 Starting CI Test Pipeline")
        print(f"📁 Project Root: {self.project_root}")
        print(f"⏰ Started: {self.results['timestamp']}")
        
        stages = [
            ("Validation", self.run_validation_stage),
            ("Unit Tests", self.run_unit_tests_stage),
            ("Acceptance Tests", self.run_acceptance_tests_stage),
            ("Integration", self.run_integration_stage)
        ]
        
        for stage_name, stage_func in stages:
            try:
                stage_func()
            except KeyboardInterrupt:
                print(f"\n⚠️ Pipeline interrupted during {stage_name}")
                self.results["overall_status"] = "INTERRUPTED"
                break
            except Exception as e:
                print(f"\n💥 Unexpected error in {stage_name}: {e}")
                self.results["stages"][f"{stage_name} Error"] = {
                    "status": "ERROR",
                    "error": str(e)
                }
        
        return self.generate_summary()

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description="Run CI test pipeline locally")
    parser.add_argument("--stage", choices=["validation", "unit", "acceptance", "integration"],
                       help="Run specific stage only")
    parser.add_argument("--project-root", help="Project root directory")
    
    args = parser.parse_args()
    
    runner = CITestRunner(args.project_root)
    
    if args.stage:
        # Run specific stage
        stage_map = {
            "validation": runner.run_validation_stage,
            "unit": runner.run_unit_tests_stage,
            "acceptance": runner.run_acceptance_tests_stage,
            "integration": runner.run_integration_stage
        }
        
        success = stage_map[args.stage]()
        runner.generate_summary()
        sys.exit(0 if success else 1)
    else:
        # Run full pipeline
        success = runner.run_full_pipeline()
        sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
