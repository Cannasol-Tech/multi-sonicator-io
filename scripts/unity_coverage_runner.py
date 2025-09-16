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
        # Try to resolve Unity framework paths (installed by PlatformIO)
        unity_paths = list(self.project_root.glob(".pio/libdeps/*/Unity/src"))
        self.unity_include_dir = unity_paths[0] if unity_paths else None
        self.unity_source_file = (unity_paths[0] / "unity.c") if unity_paths else None
        # Prefer Apple/LLVM gcov when available (clang --coverage emits llvm format)
        try:
            xcrun_llvm = subprocess.run(["xcrun", "--find", "llvm-cov"], capture_output=True, text=True)
            if xcrun_llvm.returncode == 0 and xcrun_llvm.stdout.strip():
                self.gcov_cmd = ["xcrun", "llvm-cov", "gcov"]
            else:
                self.gcov_cmd = ["gcov"]
        except Exception:
            self.gcov_cmd = ["gcov"]
        # Additional C++ unit tests not covered by the module C test harness
        self.cpp_tests = [
            {
                "name": "multi_sonicator",
                "test_file": self.project_root / "test" / "unit" / "control" / "test_multi_sonicator.cpp",
                "sources": [
                    self.project_root / "src" / "modules" / "control" / "multi_sonicator.cpp",
                    self.project_root / "test" / "mocks" / "Arduino.cpp",
                ],
                "include_dirs": [
                    self.project_root / "include",
                    self.project_root / "test" / "unit",
                    self.project_root / "src",
                    self.project_root / "src" / "modules" / "control",
                    self.project_root / "test" / "mocks",
                ],
            }
        ]
        # Additional C++ suite for HIL framework coverage (Story 0.2)
        self.cpp_tests.append({
            "name": "hil_framework",
            "test_file": self.project_root / "test" / "unit" / "test_hil_framework.cpp",
            "sources": [
                # HAL core
                self.project_root / "src" / "modules" / "hal" / "hal.cpp",
                self.project_root / "src" / "modules" / "hal" / "gpio.cpp",
                self.project_root / "src" / "modules" / "hal" / "adc.cpp",
                self.project_root / "src" / "modules" / "hal" / "pwm.cpp",
                self.project_root / "src" / "modules" / "hal" / "timer.cpp",
                self.project_root / "src" / "modules" / "hal" / "uart.cpp",
                # Register manager minimal linkage
                self.project_root / "src" / "modules" / "communication" / "modbus_register_manager.cpp",
                # Multi-sonicator for API exercised in tests
                self.project_root / "src" / "modules" / "control" / "multi_sonicator.cpp",
                # Arduino mocks
                self.project_root / "test" / "mocks" / "Arduino.cpp",
            ],
            "include_dirs": [
                self.project_root / "include",
                self.project_root / "test" / "unit",
                self.project_root / "src",
                self.project_root / "src" / "modules" / "hal",
                self.project_root / "src" / "modules" / "communication",
                self.project_root / "src" / "modules" / "control",
                self.project_root / "test" / "mocks",
            ],
        })
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
        """Run Unity tests for a specific module with proper source coverage."""
        print(f"🧪 Running tests for {module_name} module...")

        module_dir = self.test_dir / module_name
        test_file = module_dir / f"test_{module_name}.c"

        if not test_file.exists():
            print(f"⚠️  Test file not found: {test_file}")
            return False

        unity_include = str(self.unity_include_dir) if self.unity_include_dir else ""
        unity_source = str(self.unity_source_file) if self.unity_source_file else ""

        # Use C tests with mocks; compile with gcc and link unity + mocks
        mock_dir = self.project_root / "test" / "mocks"
        mock_files = []
        if module_name == "communication":
            mock_files.append(str(mock_dir / "modbus_mock.c"))
        elif module_name == "hal":
            mock_files.append(str(mock_dir / "hal_mock.c"))
        elif module_name == "control":
            mock_files.append(str(mock_dir / "control_mock.c"))
        elif module_name == "sonicator":
            mock_files.append(str(mock_dir / "sonicator_mock.c"))

        compiled_c_files = [str(test_file)] + mock_files

        output_file = module_dir / f"test_{module_name}.out"
        clang_mode = (self.gcov_cmd[:2] == ["xcrun", "llvm-cov"])  # prefer clang instrumentation on macOS
        compiler = "clang" if clang_mode else "gcc"
        cmd = [
            compiler,
            "-I", str(self.project_root / "include"),
            "-I", str(self.test_dir),
            "-I", str(self.src_dir),
            "-I", str(self.src_dir / "modules" / module_name),
            "-I", str(mock_dir),
        ]
        if unity_include:
            cmd += ["-I", unity_include]
        cmd += [
            "-DUNIT_TEST",
            "-DNATIVE_TEST",
            "-O0",
            "-g",
        ]
        if clang_mode:
            cmd += ["-fprofile-instr-generate", "-fcoverage-mapping"]
        else:
            cmd += ["-fprofile-arcs", "-ftest-coverage"]
        cmd += [str(test_file)]
        if unity_source:
            cmd.append(unity_source)
        cmd += mock_files + ["-o", str(output_file)]
        if not clang_mode:
            cmd += ["--coverage"]

        try:
            result = subprocess.run(cmd, cwd=module_dir, capture_output=True, text=True)
            if result.returncode != 0:
                print(f"❌ Compilation failed for {module_name}:")
                print(result.stderr)
                return False

            # Run the test (set LLVM profile file when using clang instrumentation)
            run_env = os.environ.copy()
            if clang_mode:
                run_env["LLVM_PROFILE_FILE"] = f"{output_file.stem}_%p.profraw"
            result = subprocess.run([str(output_file)], cwd=module_dir, env=run_env, capture_output=True, text=True)
            if result.returncode != 0:
                print(f"❌ Test execution failed for {module_name}:")
                print(result.stdout)
                print(result.stderr)
                return False

            print(f"✅ Tests passed for {module_name}")
            print(result.stdout)

            # Generate coverage data
            if clang_mode:
                # Merge profiles and generate a report
                profdata = module_dir / f"{output_file.stem}.profdata"
                # Merge any profraw generated (expand glob in Python)
                import glob as _glob
                profraws = _glob.glob(str(module_dir / "*.profraw"))
                if profraws:
                    subprocess.run(["xcrun", "llvm-profdata", "merge", "-sparse", *[Path(p).name for p in profraws], "-o", str(profdata)], cwd=module_dir, capture_output=True)
                report = subprocess.run([
                    "xcrun", "llvm-cov", "report", str(output_file),
                    f"-instr-profile={profdata}",
                    "-ignore-filename-regex=Unity/src/unity.c|unity.c|/Unity/|/unity_"
                ], cwd=module_dir, capture_output=True, text=True)
                lines_cov, lines_total = self.parse_llvm_cov_report(report.stdout)
                pct = (lines_cov / lines_total * 100) if lines_total else 0.0
                self.coverage_data["modules"][module_name] = {
                    "lines_covered": lines_cov,
                    "lines_total": lines_total,
                    "coverage_percentage": round(pct, 2),
                }
                print(f"📈 {module_name}: {lines_cov}/{lines_total} lines ({pct:.1f}%)")
            else:
                # Fallback to gcov aggregation
                self.generate_module_coverage(module_name, module_dir, compiled_c_files)
            return True

        except Exception as e:
            print(f"❌ Error running tests for {module_name}: {e}")
            return False
    
    def generate_module_coverage(self, module_name, module_dir, sources):
        """Generate coverage data for a module by running gcov on compiled C sources."""
        print(f"📊 Generating coverage for {module_name}...")
        try:
            total_lines = 0
            total_covered = 0
            for src in sources:
                src_path = Path(src)
                if not src_path.suffix in (".c", ".cpp"):
                    continue
                subprocess.run(self.gcov_cmd + [src_path.name], cwd=module_dir, capture_output=True)
                gcov_file = module_dir / f"{src_path.name}.gcov"
                if gcov_file.exists():
                    covered, lines = self.parse_gcov_file(gcov_file)
                    total_lines += lines
                    total_covered += covered
            pct = (total_covered / total_lines * 100) if total_lines else 0.0
            self.coverage_data["modules"][module_name] = {
                "lines_covered": total_covered,
                "lines_total": total_lines,
                "coverage_percentage": round(pct, 2),
            }
            print(f"📈 {module_name}: {total_covered}/{total_lines} lines ({pct:.1f}%)")
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
        total_count = 0
        for module in self.modules:
            total_count += 1
            if self.run_module_tests(module):
                success_count += 1

        # Run additional C++ tests for Story 4.1 (multi_sonicator) only when enabled
        run_cpp = os.environ.get("UNITY_CPP", "0") in ("1", "true", "True")
        if run_cpp:
            cpp_passed, cpp_total = self.run_cpp_tests()
        else:
            cpp_passed, cpp_total = 0, 0
        success_count += cpp_passed
        total_count += cpp_total
        
        # Calculate overall coverage
        self.calculate_overall_coverage()
        
        # Generate reports
        self.generate_coverage_json()
        self.generate_coverage_html()
        
        print(f"\n📊 Test Summary: {success_count}/{total_count} test suites passed")
        print(f"📈 Overall Coverage: {self.coverage_data['overall']['coverage_percentage']:.1f}%")
        
        # Check 85% requirement
        if self.coverage_data['overall']['coverage_percentage'] >= 85.0:
            print("✅ Coverage requirement met (≥85%)")
            self.coverage_data['overall']['meets_requirement'] = True
        else:
            print("⚠️  Coverage requirement not met (<85%)")
            self.coverage_data['overall']['meets_requirement'] = False
        
        return success_count == total_count

    def run_cpp_tests(self):
        """Compile and run additional C++ Unity tests (e.g., Story 4.1 multi_sonicator)."""
        passed = 0
        total = 0
        # Optional filter to run a subset (comma-separated names)
        only = os.environ.get("UNITY_CPP_FILTER")
        selected = self.cpp_tests
        if only:
            wanted = {x.strip() for x in only.split(',') if x.strip()}
            selected = [cfg for cfg in self.cpp_tests if cfg["name"] in wanted]

        for cfg in selected:
            name = cfg["name"]
            test_file = cfg["test_file"]
            sources = cfg["sources"]
            include_dirs = cfg["include_dirs"]

            if not test_file.exists():
                continue

            total += 1
            print(f"🧪 Running C++ tests for {name}...")
            # Build command
            output_file = test_file.parent / f"{test_file.stem}.out"
            cmd = [
                "g++",
                "-std=c++11",
                "-DUNIT_TEST",
                "-DNATIVE_TEST",
                "-O0",
                "-g",
                "-fprofile-arcs",
                "-ftest-coverage",
            ]
            for inc in include_dirs:
                cmd += ["-I", str(inc)]
            # Add Unity include if available
            if self.unity_include_dir:
                cmd += ["-I", str(self.unity_include_dir)]
            cmd += [
                str(test_file),
            ] + [str(s) for s in sources] + ([str(self.unity_source_file)] if self.unity_source_file else []) + [
                "-o", str(output_file),
                "--coverage",
            ]

            try:
                result = subprocess.run(cmd, cwd=test_file.parent, capture_output=True, text=True)
                if result.returncode != 0:
                    print(f"❌ Compilation failed for {name} C++ tests:")
                    print(result.stderr)
                    continue

                # Execute
                result = subprocess.run([str(output_file)], cwd=test_file.parent, capture_output=True, text=True)
                if result.returncode != 0:
                    print(f"❌ Test execution failed for {name}:")
                    print(result.stdout)
                    print(result.stderr)
                    continue

                print(f"✅ Tests passed for {name}")
                print(result.stdout)

                # Coverage: run gcov for primary source(s)
                for src in sources:
                    if src.suffix in (".cpp", ".c") and src.name != "Arduino.cpp":
                        self.generate_cpp_coverage(name, src, test_file.parent)
                passed += 1
            except Exception as e:
                print(f"❌ Error running C++ tests for {name}: {e}")
        return passed, total

    def generate_cpp_coverage(self, module_name, source_path: Path, workdir: Path):
        """Run gcov on a specific C/C++ source and record coverage under module_name."""
        try:
            subprocess.run(self.gcov_cmd + [str(source_path.name)], cwd=workdir, capture_output=True)
            gcov_file = workdir / f"{source_path.name}.gcov"
            if gcov_file.exists():
                lines_covered, lines_total = self.parse_gcov_file(gcov_file)
                pct = (lines_covered / lines_total * 100) if lines_total > 0 else 0
                # Aggregate: if module already exists, sum
                prev = self.coverage_data["modules"].get(module_name, {"lines_covered":0, "lines_total":0})
                agg = {
                    "lines_covered": prev["lines_covered"] + lines_covered,
                    "lines_total": prev["lines_total"] + lines_total,
                }
                agg["coverage_percentage"] = round((agg["lines_covered"]/agg["lines_total"]*100) if agg["lines_total"] else 0.0, 2)
                self.coverage_data["modules"][module_name] = agg
                print(f"📈 {module_name}: {agg['lines_covered']}/{agg['lines_total']} lines ({agg['coverage_percentage']:.1f}%)")
            else:
                print(f"⚠️  No gcov produced for {source_path.name}")
        except Exception as e:
            print(f"❌ Error generating C++ coverage for {module_name}: {e}")
    
    def parse_llvm_cov_report(self, report_text: str):
        """Parse llvm-cov report output to extract covered and total lines from TOTAL row."""
        lines_cov = 0
        lines_total = 0
        for line in report_text.splitlines():
            parts = line.strip().split()
            if len(parts) >= 5 and parts[0] == 'TOTAL':
                # llvm-cov report TOTAL row columns: Filename Regions Missed Regions ... Lines Missed ...
                # We will derive lines_covered = Lines - Missed
                try:
                    # Columns end with: Branches Missed Branches Cover
                    # Lines and Missed Lines are the 6th and 5th from the end respectively
                    lines_val = int(parts[-6])
                    missed_val = int(parts[-5])
                    lines_total = lines_val
                    lines_cov = max(0, lines_val - missed_val)
                    break
                except Exception:
                    continue
        return lines_cov, lines_total
    
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
