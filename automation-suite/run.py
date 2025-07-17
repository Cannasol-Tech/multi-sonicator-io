#!/usr/bin/env python3
"""
Automation Suite Integration Script
Provides a unified interface to all automation suite tools
"""

import os
import sys
import json
import argparse
import subprocess
from pathlib import Path
from datetime import datetime

class AutomationSuite:
    def __init__(self, project_path=None):
        self.project_path = Path(project_path) if project_path else Path.cwd()
        self.suite_path = self.project_path / 'automation-suite'
        
        if not self.suite_path.exists():
            raise FileNotFoundError(f"Automation suite not found at {self.suite_path}")
    
    def run_tool(self, tool_path, args=None):
        """Run an automation suite tool"""
        cmd = [sys.executable, str(tool_path)]
        if args:
            cmd.extend(args)
        
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, cwd=self.project_path)
        return result.returncode == 0
    
    def detect_hardware(self, force_simulation=False):
        """Run hardware detection"""
        print("🔍 Running hardware detection...")
        tool_path = self.suite_path / 'ci-tools' / 'hardware_detection.py'
        args = ['--output', 'hardware_detection.json']
        
        if force_simulation:
            args.append('--force-simulation')
        
        return self.run_tool(tool_path, args)
    
    def run_performance_benchmark(self, config=None, environment=None):
        """Run performance benchmarks"""
        print("📊 Running performance benchmarks...")
        tool_path = self.suite_path / 'ci-tools' / 'performance_benchmark.py'
        args = ['--output', 'performance_report.json']
        
        if config:
            args.extend(['--config', config])
        if environment:
            args.extend(['--environment', environment])
        
        return self.run_tool(tool_path, args)
    
    def run_tests(self, scenarios=None, config=None):
        """Run test scenarios"""
        print("🧪 Running test scenarios...")
        tool_path = self.suite_path / 'testing' / 'scenario_runner.py'
        args = ['--output', 'test_results.json']
        
        if scenarios:
            args.extend(['--scenarios', scenarios])
        if config:
            args.extend(['--config', config])
        
        return self.run_tool(tool_path, args)
    
    def generate_documentation(self, formats=None):
        """Generate documentation"""
        print("📚 Generating documentation...")
        tool_path = self.suite_path / 'documentation' / 'generate_docs.py'
        args = ['--output-dir', 'generated-docs']
        
        if formats:
            args.extend(['--formats'] + formats)
        
        return self.run_tool(tool_path, args)
    
    def run_full_pipeline(self, config_file=None):
        """Run the complete automation pipeline"""
        print("🚀 Starting full automation pipeline...")
        print("=" * 60)
        
        start_time = datetime.now()
        results = {}
        
        # Step 1: Hardware Detection
        results['hardware_detection'] = self.detect_hardware()
        
        # Step 2: Performance Benchmarks
        results['performance_benchmark'] = self.run_performance_benchmark(config=config_file)
        
        # Step 3: Test Scenarios
        scenarios_file = self.suite_path / 'examples' / 'integration-scenarios.yaml'
        results['test_scenarios'] = self.run_tests(scenarios=str(scenarios_file), config=config_file)
        
        # Step 4: Documentation Generation
        results['documentation'] = self.generate_documentation(formats=['markdown', 'html'])
        
        # Generate summary report
        end_time = datetime.now()
        duration = end_time - start_time
        
        print("\n" + "=" * 60)
        print("AUTOMATION PIPELINE SUMMARY")
        print("=" * 60)
        print(f"Execution time: {duration}")
        print(f"Start time: {start_time}")
        print(f"End time: {end_time}")
        print()
        
        for step, success in results.items():
            status = "✅ PASS" if success else "❌ FAIL"
            print(f"{status} {step.replace('_', ' ').title()}")
        
        overall_success = all(results.values())
        print(f"\nOverall result: {'✅ SUCCESS' if overall_success else '❌ FAILURE'}")
        
        # Save summary
        summary = {
            'timestamp': start_time.isoformat(),
            'duration_seconds': duration.total_seconds(),
            'results': results,
            'overall_success': overall_success
        }
        
        with open('automation_summary.json', 'w') as f:
            json.dump(summary, f, indent=2)
        
        print(f"\nSummary saved to: automation_summary.json")
        return overall_success

def main():
    parser = argparse.ArgumentParser(description='Automation Suite Integration Tool')
    
    # Global options
    parser.add_argument('--project-path', '-p', help='Project root directory', default='.')
    parser.add_argument('--config', '-c', help='Configuration file path')
    
    # Command selection
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Hardware detection
    hw_parser = subparsers.add_parser('detect', help='Run hardware detection')
    hw_parser.add_argument('--force-simulation', action='store_true', help='Force simulation mode')
    
    # Performance benchmarks
    perf_parser = subparsers.add_parser('benchmark', help='Run performance benchmarks')
    perf_parser.add_argument('--environment', '-e', help='Build environment to test')
    
    # Test scenarios
    test_parser = subparsers.add_parser('test', help='Run test scenarios')
    test_parser.add_argument('--scenarios', '-s', help='Test scenarios file')
    
    # Documentation generation
    docs_parser = subparsers.add_parser('docs', help='Generate documentation')
    docs_parser.add_argument('--formats', nargs='+', choices=['markdown', 'html'], 
                           default=['markdown'], help='Output formats')
    
    # Full pipeline
    full_parser = subparsers.add_parser('all', help='Run complete automation pipeline')
    
    # Setup command
    setup_parser = subparsers.add_parser('setup', help='Setup automation suite dependencies')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    try:
        if args.command == 'setup':
            # Install dependencies
            requirements_file = Path(args.project_path) / 'automation-suite' / 'requirements.txt'
            if requirements_file.exists():
                print("Installing automation suite dependencies...")
                result = subprocess.run([
                    sys.executable, '-m', 'pip', 'install', '-r', str(requirements_file)
                ])
                return result.returncode
            else:
                print(f"Requirements file not found: {requirements_file}")
                return 1
        
        # Initialize automation suite
        suite = AutomationSuite(project_path=args.project_path)
        
        if args.command == 'detect':
            success = suite.detect_hardware(force_simulation=args.force_simulation)
        elif args.command == 'benchmark':
            success = suite.run_performance_benchmark(
                config=args.config, 
                environment=args.environment
            )
        elif args.command == 'test':
            success = suite.run_tests(scenarios=args.scenarios, config=args.config)
        elif args.command == 'docs':
            success = suite.generate_documentation(formats=args.formats)
        elif args.command == 'all':
            success = suite.run_full_pipeline(config_file=args.config)
        else:
            print(f"Unknown command: {args.command}")
            return 1
        
        return 0 if success else 1
        
    except Exception as e:
        print(f"Error: {e}")
        return 1

if __name__ == '__main__':
    sys.exit(main())