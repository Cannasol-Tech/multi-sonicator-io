#!/usr/bin/env python3
"""
Coverage Report Generation Script for Multi-Sonicator I/O Controller
Generates comprehensive coverage reports for Unity native tests
"""

import os
import sys
import subprocess
import json
from pathlib import Path

def generate_coverage_report():
    """Generate coverage report using gcov and lcov"""
    
    print("🔍 Generating coverage report...")
    
    # Get project root directory
    project_root = Path(__file__).parent.parent
    test_dir = project_root / "test" / "unit"
    coverage_dir = project_root / "coverage"
    
    # Create coverage directory
    coverage_dir.mkdir(exist_ok=True)
    
    try:
        # Run gcov on all .gcda files
        gcda_files = list(project_root.rglob("*.gcda"))
        if not gcda_files:
            print("⚠️  No coverage data files found (.gcda)")
            return False
            
        print(f"📊 Found {len(gcda_files)} coverage data files")
        
        # Generate gcov reports
        for gcda_file in gcda_files:
            cmd = ["gcov", str(gcda_file)]
            subprocess.run(cmd, cwd=gcda_file.parent, capture_output=True)
        
        # Generate lcov report if lcov is available
        try:
            # Initialize lcov
            lcov_info = coverage_dir / "coverage.info"
            cmd = ["lcov", "--capture", "--directory", str(project_root), 
                   "--output-file", str(lcov_info)]
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                # Generate HTML report
                html_dir = coverage_dir / "html"
                cmd = ["genhtml", str(lcov_info), "--output-directory", str(html_dir)]
                subprocess.run(cmd, capture_output=True)
                print(f"✅ HTML coverage report generated: {html_dir}/index.html")
            else:
                print("⚠️  lcov not available, using basic gcov reports")
                
        except FileNotFoundError:
            print("⚠️  lcov not installed, using basic gcov reports")
        
        # Generate JSON summary for dashboard
        generate_coverage_json(coverage_dir)
        
        print("✅ Coverage report generation complete")
        return True
        
    except Exception as e:
        print(f"❌ Error generating coverage report: {e}")
        return False

def generate_coverage_json(coverage_dir):
    """Generate JSON coverage summary for dashboard consumption"""
    
    coverage_data = {
        "timestamp": subprocess.check_output(["date", "-Iseconds"]).decode().strip(),
        "modules": {},
        "overall": {
            "lines_covered": 0,
            "lines_total": 0,
            "coverage_percentage": 0.0
        }
    }
    
    # Parse gcov files for coverage data
    gcov_files = list(Path(".").rglob("*.gcov"))
    
    for gcov_file in gcov_files:
        module_name = gcov_file.stem.replace(".gcov", "")
        
        lines_covered = 0
        lines_total = 0
        
        try:
            with open(gcov_file, 'r') as f:
                for line in f:
                    if line.strip().startswith(('-:', '#####', '    ')):
                        continue
                    if ':' in line:
                        parts = line.split(':', 2)
                        if len(parts) >= 2:
                            count = parts[0].strip()
                            if count.isdigit():
                                lines_total += 1
                                if int(count) > 0:
                                    lines_covered += 1
                                    
        except Exception as e:
            print(f"⚠️  Error parsing {gcov_file}: {e}")
            continue
            
        if lines_total > 0:
            coverage_percentage = (lines_covered / lines_total) * 100
            coverage_data["modules"][module_name] = {
                "lines_covered": lines_covered,
                "lines_total": lines_total,
                "coverage_percentage": round(coverage_percentage, 2)
            }
            
            coverage_data["overall"]["lines_covered"] += lines_covered
            coverage_data["overall"]["lines_total"] += lines_total
    
    # Calculate overall coverage
    if coverage_data["overall"]["lines_total"] > 0:
        overall_percentage = (coverage_data["overall"]["lines_covered"] / 
                            coverage_data["overall"]["lines_total"]) * 100
        coverage_data["overall"]["coverage_percentage"] = round(overall_percentage, 2)
    
    # Write JSON report
    json_file = coverage_dir / "coverage.json"
    with open(json_file, 'w') as f:
        json.dump(coverage_data, f, indent=2)
    
    print(f"📊 Coverage JSON report: {json_file}")
    print(f"📈 Overall coverage: {coverage_data['overall']['coverage_percentage']:.1f}%")
    
    # Check if we meet 85% requirement
    if coverage_data["overall"]["coverage_percentage"] >= 85.0:
        print("✅ Coverage requirement met (≥85%)")
    else:
        print("⚠️  Coverage requirement not met (<85%)")

if __name__ == "__main__":
    success = generate_coverage_report()
    sys.exit(0 if success else 1)
