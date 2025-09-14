#!/usr/bin/env python3
"""
Unity Coverage Runner for Multi-Sonicator I/O Controller
Runs Unity tests with coverage reporting and generates dashboard-consumable reports
"""

import os
import sys
import subprocess
import json
import glob
from pathlib import Path
import argparse

class UnityCoverageRunner:
    def __init__(self, project_root=None):
        self.project_root = Path(project_root) if project_root else Path(__file__).parent.parent
        self.test_dir = self.project_root / "test" / "unit"
        self.coverage_dir = self.project_root / "coverage"
        self.src_dir = self.project_root / "src"
        
        # Create coverage directory
        self.coverage_dir.mkdir(exist_ok=True)
        
        self.modules = ["communication", "hal", "control", "sonicator"]
        self.coverage_data = {
            "timestamp": None,
            "modules": {},
            "overall": {
                "lines_covered": 0,
                "lines_total": 0,
                "coverage_percentage": 0.0,
                "meets_requirement": False
            }
        }
    
    def run_module_tests(self, module_name):
        """Run Unity tests for a specific module with coverage"""
        print(f"🧪 Running tests for {module_name} module...")
        
        module_dir = self.test_dir / module_name
        test_file = module_dir / f"test_{module_name}.c"
        
        if not test_file.exists():
            print(f"⚠️  Test file not found: {test_file}")
            return False
        
        # Find Unity library path
        unity_paths = list(self.project_root.glob(".pio/libdeps/*/Unity/src"))
        unity_include = str(unity_paths[0]) if unity_paths else ""
        unity_source = str(unity_paths[0] / "unity.c") if unity_paths else ""

        # Get mock files for the module
        mock_files = []
        mock_dir = self.project_root / "test" / "mocks"
        if module_name == "communication":
            mock_files.append(str(mock_dir / "modbus_mock.c"))
        elif module_name == "hal":
            mock_files.append(str(mock_dir / "hal_mock.c"))
        elif module_name == "control":
            mock_files.append(str(mock_dir / "control_mock.c"))
        elif module_name == "sonicator":
            mock_files.append(str(mock_dir / "sonicator_mock.c"))

        # Compile with coverage flags
        output_file = module_dir / f"test_{module_name}.out"
        compile_cmd = [
            "gcc",
            "-I", str(self.project_root / "include"),
            "-I", str(self.test_dir),
            "-I", str(self.src_dir),
            "-I", str(self.src_dir / "modules" / module_name),
            "-I", str(mock_dir),
            "-I", unity_include,
            "-DUNIT_TEST",
            "-DNATIVE_TEST",
            "-fprofile-arcs",
            "-ftest-coverage",
            "-O0",
            "-g",
            str(test_file),
            unity_source
        ] + mock_files + [
            "-o", str(output_file),
            "--coverage"
        ]
        
        try:
            result = subprocess.run(compile_cmd, cwd=module_dir, capture_output=True, text=True)
            if result.returncode != 0:
                print(f"❌ Compilation failed for {module_name}:")
                print(result.stderr)
                return False
            
            # Run the test
            result = subprocess.run([str(output_file)], cwd=module_dir, capture_output=True, text=True)
            if result.returncode != 0:
                print(f"❌ Test execution failed for {module_name}:")
                print(result.stdout)
                print(result.stderr)
                return False
            
            print(f"✅ Tests passed for {module_name}")
            print(result.stdout)
            
            # Generate coverage data
            self.generate_module_coverage(module_name, module_dir)
            return True
            
        except Exception as e:
            print(f"❌ Error running tests for {module_name}: {e}")
            return False
    
    def generate_module_coverage(self, module_name, module_dir):
        """Generate coverage data for a module"""
        print(f"📊 Generating coverage for {module_name}...")
        
        try:
            # Run gcov
            gcov_cmd = ["gcov", f"test_{module_name}.c"]
            subprocess.run(gcov_cmd, cwd=module_dir, capture_output=True)
            
            # Parse gcov output
            gcov_file = module_dir / f"test_{module_name}.c.gcov"
            if gcov_file.exists():
                lines_covered, lines_total = self.parse_gcov_file(gcov_file)
                
                coverage_percentage = (lines_covered / lines_total * 100) if lines_total > 0 else 0
                
                self.coverage_data["modules"][module_name] = {
                    "lines_covered": lines_covered,
                    "lines_total": lines_total,
                    "coverage_percentage": round(coverage_percentage, 2)
                }
                
                print(f"📈 {module_name}: {lines_covered}/{lines_total} lines ({coverage_percentage:.1f}%)")
            else:
                print(f"⚠️  No gcov file found for {module_name}")
                
        except Exception as e:
            print(f"❌ Error generating coverage for {module_name}: {e}")
    
    def parse_gcov_file(self, gcov_file):
        """Parse gcov file to extract coverage statistics"""
        lines_covered = 0
        lines_total = 0
        exclude = False
        
        try:
            with open(gcov_file, 'r') as f:
                for line in f:
                    raw = line.rstrip('\n')
                    # Respect exclusion markers embedded in gcov output
                    if 'GCOV_EXCL_START' in raw:
                        exclude = True
                        continue
                    if 'GCOV_EXCL_STOP' in raw:
                        exclude = False
                        continue

                    line = raw.strip()
                    if exclude:
                        continue
                    if ':' in line:
                        parts = line.split(':', 2)
                        if len(parts) >= 2:
                            count = parts[0].strip()
                            if count.isdigit():
                                lines_total += 1
                                if int(count) > 0:
                                    lines_covered += 1
                            elif count == '#####':
                                lines_total += 1  # Uncovered line
        except Exception as e:
            print(f"⚠️  Error parsing gcov file {gcov_file}: {e}")
        
        return lines_covered, lines_total
    
    def run_all_tests(self):
        """Run all module tests and generate comprehensive coverage report"""
        print("🚀 Starting Unity test suite with coverage...")
        
        success_count = 0
        for module in self.modules:
            if self.run_module_tests(module):
                success_count += 1
        
        # Calculate overall coverage
        self.calculate_overall_coverage()
        
        # Generate reports
        self.generate_coverage_json()
        self.generate_coverage_html()
        
        print(f"\n📊 Test Summary: {success_count}/{len(self.modules)} modules passed")
        print(f"📈 Overall Coverage: {self.coverage_data['overall']['coverage_percentage']:.1f}%")
        
        # Check 85% requirement
        if self.coverage_data['overall']['coverage_percentage'] >= 85.0:
            print("✅ Coverage requirement met (≥85%)")
            self.coverage_data['overall']['meets_requirement'] = True
        else:
            print("⚠️  Coverage requirement not met (<85%)")
            self.coverage_data['overall']['meets_requirement'] = False
        
        return success_count == len(self.modules)
    
    def calculate_overall_coverage(self):
        """Calculate overall coverage across all modules"""
        total_covered = sum(module['lines_covered'] for module in self.coverage_data['modules'].values())
        total_lines = sum(module['lines_total'] for module in self.coverage_data['modules'].values())
        
        self.coverage_data['overall']['lines_covered'] = total_covered
        self.coverage_data['overall']['lines_total'] = total_lines
        
        if total_lines > 0:
            overall_percentage = (total_covered / total_lines) * 100
            self.coverage_data['overall']['coverage_percentage'] = round(overall_percentage, 2)
    
    def generate_coverage_json(self):
        """Generate JSON coverage report for dashboard consumption"""
        import datetime
        
        self.coverage_data['timestamp'] = datetime.datetime.now().isoformat()
        
        json_file = self.coverage_dir / "coverage.json"
        with open(json_file, 'w') as f:
            json.dump(self.coverage_data, f, indent=2)
        
        print(f"📄 Coverage JSON report: {json_file}")
    
    def generate_coverage_html(self):
        """Generate HTML coverage report"""
        html_content = f"""
<!DOCTYPE html>
<html>
<head>
    <title>Unity Test Coverage Report - Multi-Sonicator I/O Controller</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .header {{ background: #f0f0f0; padding: 20px; border-radius: 5px; }}
        .summary {{ margin: 20px 0; }}
        .module {{ margin: 10px 0; padding: 10px; border: 1px solid #ddd; border-radius: 5px; }}
        .pass {{ background: #d4edda; }}
        .fail {{ background: #f8d7da; }}
        .coverage-bar {{ width: 100%; height: 20px; background: #f0f0f0; border-radius: 10px; }}
        .coverage-fill {{ height: 100%; background: #28a745; border-radius: 10px; }}
    </style>
</head>
<body>
    <div class="header">
        <h1>Unity Test Coverage Report</h1>
        <p>Multi-Sonicator I/O Controller - Generated: {self.coverage_data.get('timestamp', 'Unknown')}</p>
    </div>
    
    <div class="summary">
        <h2>Overall Coverage: {self.coverage_data['overall']['coverage_percentage']:.1f}%</h2>
        <div class="coverage-bar">
            <div class="coverage-fill" style="width: {self.coverage_data['overall']['coverage_percentage']}%"></div>
        </div>
        <p>Lines Covered: {self.coverage_data['overall']['lines_covered']} / {self.coverage_data['overall']['lines_total']}</p>
        <p>Requirement (≥85%): {'✅ MET' if self.coverage_data['overall']['meets_requirement'] else '❌ NOT MET'}</p>
    </div>
    
    <h2>Module Coverage</h2>
"""
        
        for module_name, module_data in self.coverage_data['modules'].items():
            status_class = "pass" if module_data['coverage_percentage'] >= 85 else "fail"
            html_content += f"""
    <div class="module {status_class}">
        <h3>{module_name.title()} Module</h3>
        <div class="coverage-bar">
            <div class="coverage-fill" style="width: {module_data['coverage_percentage']}%"></div>
        </div>
        <p>Coverage: {module_data['coverage_percentage']:.1f}% ({module_data['lines_covered']}/{module_data['lines_total']} lines)</p>
    </div>
"""
        
        html_content += """
</body>
</html>
"""
        
        html_file = self.coverage_dir / "coverage.html"
        with open(html_file, 'w') as f:
            f.write(html_content)
        
        print(f"📄 Coverage HTML report: {html_file}")

def main():
    parser = argparse.ArgumentParser(description="Unity Coverage Runner")
    parser.add_argument("--module", help="Run tests for specific module only")
    parser.add_argument("--project-root", help="Project root directory")
    
    args = parser.parse_args()
    
    runner = UnityCoverageRunner(args.project_root)
    
    if args.module:
        success = runner.run_module_tests(args.module)
        runner.calculate_overall_coverage()
        runner.generate_coverage_json()
        runner.generate_coverage_html()
    else:
        success = runner.run_all_tests()
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
