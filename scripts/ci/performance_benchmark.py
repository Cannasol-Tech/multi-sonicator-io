#!/usr/bin/env python3
"""
Performance Benchmarking Script for Multi-Sonicator CI/CD Pipeline
Measures performance metrics for simulation vs hardware testing
"""

import os
import sys
import time
import json
import statistics
import subprocess
import argparse
from datetime import datetime

class PerformanceBenchmark:
    def __init__(self):
        self.metrics = {
            'build_times': [],
            'test_times': [],
            'simulation_times': [],
            'memory_usage': [],
            'cpu_usage': [],
            'communication_stats': {}
        }
        self.baseline_metrics = None
        
    def run_build_benchmark(self, environment='test_desktop', iterations=3):
        """Benchmark build performance"""
        print(f"Running build benchmark for {environment} ({iterations} iterations)")
        
        build_times = []
        for i in range(iterations):
            print(f"  Build iteration {i+1}/{iterations}")
            start_time = time.time()
            
            # Clean build
            result = subprocess.run(
                ['pio', 'run', '-e', environment, '--target', 'clean'],
                capture_output=True, text=True
            )
            
            # Build
            result = subprocess.run(
                ['pio', 'run', '-e', environment],
                capture_output=True, text=True
            )
            
            end_time = time.time()
            build_time = end_time - start_time
            build_times.append(build_time)
            
            if result.returncode != 0:
                print(f"    Build failed: {result.stderr}")
                continue
                
            print(f"    Build time: {build_time:.2f}s")
        
        self.metrics['build_times'] = build_times
        return build_times
    
    def run_test_benchmark(self, test_filter=None, iterations=3):
        """Benchmark test performance"""
        print(f"Running test benchmark ({iterations} iterations)")
        
        test_times = []
        for i in range(iterations):
            print(f"  Test iteration {i+1}/{iterations}")
            start_time = time.time()
            
            # Run tests
            cmd = ['pio', 'test', '-e', 'test_desktop', '--verbose']
            if test_filter:
                cmd.extend(['--filter', test_filter])
            
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            end_time = time.time()
            test_time = end_time - start_time
            test_times.append(test_time)
            
            print(f"    Test time: {test_time:.2f}s")
            
            # Parse test output for additional metrics
            if result.returncode == 0:
                self.parse_test_output(result.stdout)
        
        self.metrics['test_times'] = test_times
        return test_times
    
    def run_simulation_benchmark(self, scenario_count=5):
        """Benchmark simulation performance"""
        print(f"Running simulation benchmark ({scenario_count} scenarios)")
        
        simulation_times = []
        scenarios = [
            'SCENARIO_NORMAL_OPERATION',
            'SCENARIO_OVERLOAD_RECOVERY',
            'SCENARIO_FREQUENCY_DRIFT',
            'SCENARIO_EMERGENCY_STOP',
            'SCENARIO_POWER_RAMPING'
        ]
        
        for i, scenario in enumerate(scenarios[:scenario_count]):
            print(f"  Scenario {i+1}/{scenario_count}: {scenario}")
            start_time = time.time()
            
            # Run simulation test
            result = subprocess.run([
                'pio', 'test', '-e', 'test_desktop', 
                '--filter', 'test_simulation_integration',
                '--verbose'
            ], capture_output=True, text=True)
            
            end_time = time.time()
            simulation_time = end_time - start_time
            simulation_times.append(simulation_time)
            
            print(f"    Simulation time: {simulation_time:.2f}s")
        
        self.metrics['simulation_times'] = simulation_times
        return simulation_times
    
    def measure_memory_usage(self):
        """Measure memory usage during compilation"""
        print("Measuring memory usage...")
        
        # This is a simplified version - real implementation would use psutil
        # or system monitoring tools
        try:
            result = subprocess.run(
                ['free', '-m'], capture_output=True, text=True
            )
            
            if result.returncode == 0:
                lines = result.stdout.split('\n')
                for line in lines:
                    if 'Mem:' in line:
                        parts = line.split()
                        total_mem = int(parts[1])
                        used_mem = int(parts[2])
                        free_mem = int(parts[3])
                        
                        self.metrics['memory_usage'] = {
                            'total': total_mem,
                            'used': used_mem,
                            'free': free_mem,
                            'usage_percent': (used_mem / total_mem) * 100
                        }
                        break
        except Exception as e:
            print(f"Memory measurement failed: {e}")
    
    def parse_test_output(self, output):
        """Parse test output for performance metrics"""
        # Look for test execution times, memory usage, etc.
        lines = output.split('\n')
        
        for line in lines:
            if 'Test ran in' in line:
                # Extract test execution time
                pass
            elif 'Memory' in line:
                # Extract memory usage
                pass
    
    def calculate_statistics(self):
        """Calculate statistical metrics"""
        stats = {}
        
        for metric_name, values in self.metrics.items():
            if isinstance(values, list) and values:
                stats[metric_name] = {
                    'mean': statistics.mean(values),
                    'median': statistics.median(values),
                    'stdev': statistics.stdev(values) if len(values) > 1 else 0,
                    'min': min(values),
                    'max': max(values),
                    'count': len(values)
                }
        
        return stats
    
    def generate_report(self, output_file='performance_report.json'):
        """Generate performance report"""
        stats = self.calculate_statistics()
        
        report = {
            'timestamp': datetime.now().isoformat(),
            'metrics': self.metrics,
            'statistics': stats,
            'environment': {
                'platform': os.uname().sysname,
                'python_version': sys.version,
                'pwd': os.getcwd()
            }
        }
        
        # Add performance analysis
        report['analysis'] = self.analyze_performance(stats)
        
        # Write report
        with open(output_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"Performance report generated: {output_file}")
        return report
    
    def analyze_performance(self, stats):
        """Analyze performance metrics and provide insights"""
        analysis = {
            'summary': '',
            'recommendations': [],
            'performance_grade': 'A',
            'bottlenecks': []
        }
        
        # Analyze build times
        if 'build_times' in stats:
            build_stats = stats['build_times']
            if build_stats['mean'] > 60:  # > 1 minute
                analysis['bottlenecks'].append("Build time exceeds 1 minute")
                analysis['recommendations'].append("Consider build optimization")
                analysis['performance_grade'] = 'C'
            elif build_stats['mean'] > 30:  # > 30 seconds
                analysis['performance_grade'] = 'B'
        
        # Analyze test times
        if 'test_times' in stats:
            test_stats = stats['test_times']
            if test_stats['mean'] > 300:  # > 5 minutes
                analysis['bottlenecks'].append("Test execution exceeds 5 minutes")
                analysis['recommendations'].append("Consider test parallelization")
                analysis['performance_grade'] = 'C'
        
        # Analyze simulation performance
        if 'simulation_times' in stats:
            sim_stats = stats['simulation_times']
            if sim_stats['mean'] > 30:  # > 30 seconds per scenario
                analysis['bottlenecks'].append("Simulation scenarios are slow")
                analysis['recommendations'].append("Optimize simulation update frequency")
        
        # Generate summary
        total_time = 0
        if 'build_times' in stats:
            total_time += stats['build_times']['mean']
        if 'test_times' in stats:
            total_time += stats['test_times']['mean']
        
        analysis['summary'] = f"Total pipeline time: {total_time:.1f}s, Grade: {analysis['performance_grade']}"
        
        return analysis
    
    def compare_with_baseline(self, baseline_file='baseline_performance.json'):
        """Compare current performance with baseline"""
        if not os.path.exists(baseline_file):
            print(f"Baseline file not found: {baseline_file}")
            return None
        
        with open(baseline_file, 'r') as f:
            baseline = json.load(f)
        
        comparison = {
            'baseline_date': baseline.get('timestamp', 'unknown'),
            'current_date': datetime.now().isoformat(),
            'changes': {}
        }
        
        current_stats = self.calculate_statistics()
        baseline_stats = baseline.get('statistics', {})
        
        for metric in ['build_times', 'test_times', 'simulation_times']:
            if metric in current_stats and metric in baseline_stats:
                current_mean = current_stats[metric]['mean']
                baseline_mean = baseline_stats[metric]['mean']
                
                change_percent = ((current_mean - baseline_mean) / baseline_mean) * 100
                comparison['changes'][metric] = {
                    'current': current_mean,
                    'baseline': baseline_mean,
                    'change_percent': change_percent,
                    'improved': change_percent < 0
                }
        
        return comparison

def main():
    parser = argparse.ArgumentParser(description='Performance benchmarking for Multi-Sonicator')
    parser.add_argument('--build-iterations', type=int, default=3, help='Number of build iterations')
    parser.add_argument('--test-iterations', type=int, default=3, help='Number of test iterations')
    parser.add_argument('--test-filter', help='Test filter pattern')
    parser.add_argument('--output', default='performance_report.json', help='Output file')
    parser.add_argument('--baseline', help='Baseline file for comparison')
    parser.add_argument('--save-baseline', action='store_true', help='Save as baseline')
    
    args = parser.parse_args()
    
    benchmark = PerformanceBenchmark()
    
    print("Multi-Sonicator Performance Benchmark")
    print("=" * 40)
    
    # Run benchmarks
    benchmark.run_build_benchmark(iterations=args.build_iterations)
    benchmark.run_test_benchmark(test_filter=args.test_filter, iterations=args.test_iterations)
    benchmark.run_simulation_benchmark()
    benchmark.measure_memory_usage()
    
    # Generate report
    report = benchmark.generate_report(args.output)
    
    # Compare with baseline if provided
    if args.baseline:
        comparison = benchmark.compare_with_baseline(args.baseline)
        if comparison:
            print("\nBaseline Comparison:")
            for metric, data in comparison['changes'].items():
                change = data['change_percent']
                status = "✓" if data['improved'] else "✗"
                print(f"  {metric}: {change:+.1f}% {status}")
    
    # Save as baseline if requested
    if args.save_baseline:
        baseline_file = 'baseline_performance.json'
        with open(baseline_file, 'w') as f:
            json.dump(report, f, indent=2)
        print(f"Baseline saved: {baseline_file}")
    
    # Print summary
    print(f"\nPerformance Summary:")
    print(f"  Grade: {report['analysis']['performance_grade']}")
    print(f"  {report['analysis']['summary']}")
    
    if report['analysis']['recommendations']:
        print("\nRecommendations:")
        for rec in report['analysis']['recommendations']:
            print(f"  - {rec}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())