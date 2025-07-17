#!/usr/bin/env python3
"""
Generic Performance Benchmarking Tool
Measures performance metrics for any project's build and test processes
"""

import os
import sys
import time
import json
import yaml
import statistics
import subprocess
import argparse
from datetime import datetime
from pathlib import Path
import psutil
import platform

class PerformanceBenchmark:
    def __init__(self, config_path=None, project_path=None):
        """
        Initialize performance benchmarking tool
        
        Args:
            config_path: Path to configuration file
            project_path: Path to project root directory
        """
        self.project_path = Path(project_path) if project_path else Path.cwd()
        self.config = self.load_config(config_path)
        self.metrics = {
            'build_times': [],
            'test_times': [],
            'simulation_times': [],
            'memory_usage': [],
            'cpu_usage': [],
            'communication_stats': {},
            'environment_info': self.get_environment_info()
        }
        self.baseline_metrics = None
        
    def load_config(self, config_path):
        """Load configuration from file or use defaults"""
        default_config = {
            'project_type': 'generic',  # platformio, cmake, npm, python, etc.
            'build': {
                'environments': ['test'],
                'iterations': 3,
                'clean_build': True,
                'parallel_jobs': 1
            },
            'test': {
                'iterations': 3,
                'timeout': 300,
                'parallel': True
            },
            'monitoring': {
                'sample_interval': 1.0,
                'memory_threshold': '512MB',
                'cpu_threshold': 80
            },
            'thresholds': {
                'build_time_warning': 300,  # seconds
                'build_time_error': 600,
                'test_time_warning': 120,
                'test_time_error': 300
            }
        }
        
        if config_path and Path(config_path).exists():
            with open(config_path, 'r') as f:
                if config_path.endswith('.yaml') or config_path.endswith('.yml'):
                    user_config = yaml.safe_load(f)
                else:
                    user_config = json.load(f)
            # Merge with defaults
            default_config.update(user_config)
            
        return default_config
    
    def get_environment_info(self):
        """Get system environment information"""
        return {
            'platform': platform.platform(),
            'architecture': platform.architecture()[0],
            'processor': platform.processor(),
            'python_version': platform.python_version(),
            'cpu_count': psutil.cpu_count(),
            'total_memory': psutil.virtual_memory().total,
            'timestamp': datetime.now().isoformat()
        }
    
    def get_build_commands(self):
        """Get build commands based on project type"""
        project_type = self.config['project_type']
        
        commands = {
            'platformio': {
                'clean': ['pio', 'run', '--target', 'clean'],
                'build': ['pio', 'run'],
                'test': ['pio', 'test']
            },
            'cmake': {
                'clean': ['make', 'clean'],
                'build': ['make', '-j{}'.format(self.config['build']['parallel_jobs'])],
                'test': ['make', 'test']
            },
            'npm': {
                'clean': ['npm', 'run', 'clean'],
                'build': ['npm', 'run', 'build'],
                'test': ['npm', 'test']
            },
            'python': {
                'clean': ['python', 'setup.py', 'clean'],
                'build': ['python', 'setup.py', 'build'],
                'test': ['python', '-m', 'pytest']
            },
            'generic': {
                'clean': ['echo', 'No clean command configured'],
                'build': ['echo', 'No build command configured'],
                'test': ['echo', 'No test command configured']
            }
        }
        
        return commands.get(project_type, commands['generic'])
    
    def monitor_resources(self, duration=1.0):
        """Monitor CPU and memory usage"""
        process = psutil.Process()
        cpu_percent = process.cpu_percent(interval=duration)
        memory_info = process.memory_info()
        
        return {
            'cpu_percent': cpu_percent,
            'memory_rss': memory_info.rss,
            'memory_vms': memory_info.vms,
            'timestamp': time.time()
        }
    
    def run_build_benchmark(self, environment=None, iterations=None):
        """Benchmark build performance"""
        iterations = iterations or self.config['build']['iterations']
        environments = [environment] if environment else self.config['build']['environments']
        commands = self.get_build_commands()
        
        print(f"Running build benchmark for {environments} ({iterations} iterations)")
        
        for env in environments:
            env_metrics = []
            
            for i in range(iterations):
                print(f"  Build iteration {i+1}/{iterations} for {env}")
                
                # Monitor resources during build
                resource_samples = []
                start_time = time.time()
                
                try:
                    # Clean build if configured
                    if self.config['build']['clean_build']:
                        clean_cmd = commands['clean'].copy()
                        if self.config['project_type'] == 'platformio':
                            clean_cmd.extend(['-e', env])
                        subprocess.run(clean_cmd, cwd=self.project_path, 
                                     capture_output=True, text=True, timeout=60)
                    
                    # Build
                    build_cmd = commands['build'].copy()
                    if self.config['project_type'] == 'platformio':
                        build_cmd.extend(['-e', env])
                    
                    # Start monitoring
                    build_process = subprocess.Popen(
                        build_cmd, cwd=self.project_path,
                        stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True
                    )
                    
                    # Monitor resources while building
                    while build_process.poll() is None:
                        resource_samples.append(self.monitor_resources(
                            self.config['monitoring']['sample_interval']
                        ))
                    
                    build_process.wait()
                    end_time = time.time()
                    
                    build_time = end_time - start_time
                    env_metrics.append({
                        'iteration': i + 1,
                        'build_time': build_time,
                        'success': build_process.returncode == 0,
                        'resource_samples': resource_samples,
                        'max_memory': max([s['memory_rss'] for s in resource_samples]) if resource_samples else 0,
                        'avg_cpu': statistics.mean([s['cpu_percent'] for s in resource_samples]) if resource_samples else 0
                    })
                    
                    print(f"    Build time: {build_time:.2f}s, Success: {build_process.returncode == 0}")
                    
                except subprocess.TimeoutExpired:
                    print(f"    Build timeout after {self.config['thresholds']['build_time_error']}s")
                    env_metrics.append({
                        'iteration': i + 1,
                        'build_time': self.config['thresholds']['build_time_error'],
                        'success': False,
                        'timeout': True
                    })
                
            self.metrics['build_times'].append({
                'environment': env,
                'metrics': env_metrics,
                'avg_time': statistics.mean([m['build_time'] for m in env_metrics]),
                'success_rate': sum([1 for m in env_metrics if m['success']]) / len(env_metrics)
            })
    
    def run_test_benchmark(self, test_filter=None, iterations=None):
        """Benchmark test performance"""
        iterations = iterations or self.config['test']['iterations']
        commands = self.get_build_commands()
        
        print(f"Running test benchmark ({iterations} iterations)")
        
        for i in range(iterations):
            print(f"  Test iteration {i+1}/{iterations}")
            start_time = time.time()
            
            try:
                test_cmd = commands['test'].copy()
                if test_filter and self.config['project_type'] == 'platformio':
                    test_cmd.extend(['--filter', test_filter])
                
                result = subprocess.run(
                    test_cmd, cwd=self.project_path,
                    capture_output=True, text=True,
                    timeout=self.config['test']['timeout']
                )
                
                end_time = time.time()
                test_time = end_time - start_time
                
                self.metrics['test_times'].append({
                    'iteration': i + 1,
                    'test_time': test_time,
                    'success': result.returncode == 0,
                    'output_lines': len(result.stdout.split('\n'))
                })
                
                print(f"    Test time: {test_time:.2f}s, Success: {result.returncode == 0}")
                
            except subprocess.TimeoutExpired:
                print(f"    Test timeout after {self.config['test']['timeout']}s")
                self.metrics['test_times'].append({
                    'iteration': i + 1,
                    'test_time': self.config['test']['timeout'],
                    'success': False,
                    'timeout': True
                })
    
    def analyze_results(self):
        """Analyze benchmark results and generate insights"""
        analysis = {
            'summary': {},
            'warnings': [],
            'recommendations': []
        }
        
        # Build analysis
        if self.metrics['build_times']:
            build_times = []
            for env_data in self.metrics['build_times']:
                build_times.extend([m['build_time'] for m in env_data['metrics']])
            
            if build_times:
                analysis['summary']['build'] = {
                    'avg_time': statistics.mean(build_times),
                    'min_time': min(build_times),
                    'max_time': max(build_times),
                    'std_dev': statistics.stdev(build_times) if len(build_times) > 1 else 0
                }
                
                # Check thresholds
                avg_time = analysis['summary']['build']['avg_time']
                if avg_time > self.config['thresholds']['build_time_error']:
                    analysis['warnings'].append(f"Build time ({avg_time:.1f}s) exceeds error threshold")
                elif avg_time > self.config['thresholds']['build_time_warning']:
                    analysis['warnings'].append(f"Build time ({avg_time:.1f}s) exceeds warning threshold")
        
        # Test analysis
        if self.metrics['test_times']:
            test_times = [t['test_time'] for t in self.metrics['test_times']]
            analysis['summary']['test'] = {
                'avg_time': statistics.mean(test_times),
                'min_time': min(test_times),
                'max_time': max(test_times),
                'success_rate': sum([1 for t in self.metrics['test_times'] if t['success']]) / len(test_times)
            }
            
            # Check thresholds
            avg_time = analysis['summary']['test']['avg_time']
            if avg_time > self.config['thresholds']['test_time_error']:
                analysis['warnings'].append(f"Test time ({avg_time:.1f}s) exceeds error threshold")
            elif avg_time > self.config['thresholds']['test_time_warning']:
                analysis['warnings'].append(f"Test time ({avg_time:.1f}s) exceeds warning threshold")
        
        # Generate recommendations
        if analysis['summary'].get('build', {}).get('avg_time', 0) > 60:
            analysis['recommendations'].append("Consider enabling parallel builds to reduce build time")
        
        if analysis['summary'].get('test', {}).get('success_rate', 1) < 0.95:
            analysis['recommendations'].append("Test success rate is low - investigate failing tests")
        
        return analysis
    
    def generate_report(self, output_path='performance_report.json'):
        """Generate comprehensive performance report"""
        analysis = self.analyze_results()
        
        report = {
            'metadata': {
                'generated_at': datetime.now().isoformat(),
                'project_path': str(self.project_path),
                'config': self.config,
                'environment': self.metrics['environment_info']
            },
            'metrics': self.metrics,
            'analysis': analysis
        }
        
        # Save report
        with open(output_path, 'w') as f:
            json.dump(report, f, indent=2)
        
        # Print summary
        print("\n" + "="*50)
        print("PERFORMANCE BENCHMARK REPORT")
        print("="*50)
        
        if 'build' in analysis['summary']:
            build = analysis['summary']['build']
            print(f"Build Performance:")
            print(f"  Average time: {build['avg_time']:.2f}s")
            print(f"  Min time: {build['min_time']:.2f}s")
            print(f"  Max time: {build['max_time']:.2f}s")
            print(f"  Std deviation: {build['std_dev']:.2f}s")
        
        if 'test' in analysis['summary']:
            test = analysis['summary']['test']
            print(f"\nTest Performance:")
            print(f"  Average time: {test['avg_time']:.2f}s")
            print(f"  Success rate: {test['success_rate']*100:.1f}%")
        
        if analysis['warnings']:
            print(f"\n⚠️  WARNINGS:")
            for warning in analysis['warnings']:
                print(f"  - {warning}")
        
        if analysis['recommendations']:
            print(f"\n💡 RECOMMENDATIONS:")
            for rec in analysis['recommendations']:
                print(f"  - {rec}")
        
        print(f"\nFull report saved to: {output_path}")
        return report

def main():
    parser = argparse.ArgumentParser(description='Generic Performance Benchmarking Tool')
    parser.add_argument('--config', '-c', help='Configuration file path')
    parser.add_argument('--project-path', '-p', help='Project root directory', default='.')
    parser.add_argument('--output', '-o', help='Output report file', default='performance_report.json')
    parser.add_argument('--build-only', action='store_true', help='Run build benchmarks only')
    parser.add_argument('--test-only', action='store_true', help='Run test benchmarks only')
    parser.add_argument('--environment', '-e', help='Specific environment to test')
    parser.add_argument('--iterations', '-i', type=int, help='Number of iterations')
    
    args = parser.parse_args()
    
    # Initialize benchmarking tool
    benchmark = PerformanceBenchmark(
        config_path=args.config,
        project_path=args.project_path
    )
    
    # Run benchmarks
    if not args.test_only:
        benchmark.run_build_benchmark(
            environment=args.environment,
            iterations=args.iterations
        )
    
    if not args.build_only:
        benchmark.run_test_benchmark(iterations=args.iterations)
    
    # Generate report
    benchmark.generate_report(args.output)

if __name__ == '__main__':
    main()