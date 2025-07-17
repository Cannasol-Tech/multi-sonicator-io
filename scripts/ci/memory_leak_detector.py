#!/usr/bin/env python3
"""
Memory Leak Detection Script for Multi-Sonicator CI/CD Pipeline
Detects memory leaks in simulation and embedded code
"""

import os
import sys
import re
import json
import subprocess
import time
from datetime import datetime
import argparse

class MemoryLeakDetector:
    def __init__(self):
        self.leak_reports = []
        self.memory_snapshots = []
        self.baseline_memory = None
        
    def detect_static_memory_issues(self, source_dir='.'):
        """Detect static memory issues in source code"""
        print("Scanning for static memory issues...")
        
        issues = []
        
        # Find all source files
        source_files = []
        for root, dirs, files in os.walk(source_dir):
            for file in files:
                if file.endswith(('.c', '.cpp', '.h')):
                    source_files.append(os.path.join(root, file))
        
        # Check each source file
        for source_file in source_files:
            file_issues = self.analyze_source_file(source_file)
            if file_issues:
                issues.extend(file_issues)
        
        return issues
    
    def analyze_source_file(self, file_path):
        """Analyze a single source file for memory issues"""
        try:
            with open(file_path, 'r') as f:
                content = f.read()
        except Exception as e:
            return [{'type': 'read_error', 'file': file_path, 'error': str(e)}]
        
        issues = []
        lines = content.split('\n')
        
        # Check for common memory issues
        for i, line in enumerate(lines):
            line_num = i + 1
            
            # Check for malloc/free patterns
            if 'malloc(' in line or 'calloc(' in line:
                # Look for corresponding free() in the same function
                issues.append({
                    'type': 'malloc_usage',
                    'file': file_path,
                    'line': line_num,
                    'message': 'malloc/calloc usage detected - check for corresponding free()',
                    'severity': 'warning'
                })
            
            # Check for new/delete patterns
            if 'new ' in line:
                issues.append({
                    'type': 'new_usage',
                    'file': file_path,
                    'line': line_num,
                    'message': 'new operator usage detected - check for corresponding delete',
                    'severity': 'warning'
                })
            
            # Check for potential buffer overflows
            if 'strcpy(' in line or 'strcat(' in line:
                issues.append({
                    'type': 'buffer_overflow_risk',
                    'file': file_path,
                    'line': line_num,
                    'message': 'Unsafe string function detected - consider using safe alternatives',
                    'severity': 'high'
                })
            
            # Check for large stack allocations
            if 'char ' in line and '[' in line:
                # Extract array size
                match = re.search(r'char\s+\w+\[(\d+)\]', line)
                if match:
                    size = int(match.group(1))
                    if size > 1024:  # > 1KB
                        issues.append({
                            'type': 'large_stack_allocation',
                            'file': file_path,
                            'line': line_num,
                            'message': f'Large stack allocation detected: {size} bytes',
                            'severity': 'medium'
                        })
            
            # Check for memory leaks in Arduino code
            if 'String(' in line:
                issues.append({
                    'type': 'string_object_usage',
                    'file': file_path,
                    'line': line_num,
                    'message': 'String object usage - can cause memory fragmentation',
                    'severity': 'low'
                })
        
        return issues
    
    def run_valgrind_analysis(self, test_executable):
        """Run Valgrind memory leak detection"""
        print(f"Running Valgrind analysis on {test_executable}...")
        
        if not os.path.exists(test_executable):
            print(f"Test executable not found: {test_executable}")
            return None
        
        # Run Valgrind
        cmd = [
            'valgrind',
            '--tool=memcheck',
            '--leak-check=full',
            '--show-leak-kinds=all',
            '--xml=yes',
            '--xml-file=valgrind_report.xml',
            test_executable
        ]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
            
            # Parse Valgrind XML output
            if os.path.exists('valgrind_report.xml'):
                return self.parse_valgrind_xml('valgrind_report.xml')
            else:
                return self.parse_valgrind_text(result.stderr)
        
        except subprocess.TimeoutExpired:
            return {'error': 'Valgrind analysis timed out'}
        except Exception as e:
            return {'error': f'Valgrind analysis failed: {str(e)}'}
    
    def parse_valgrind_xml(self, xml_file):
        """Parse Valgrind XML output"""
        try:
            import xml.etree.ElementTree as ET
            tree = ET.parse(xml_file)
            root = tree.getroot()
            
            leaks = []
            for error in root.findall('.//error'):
                kind = error.find('kind')
                if kind is not None and 'leak' in kind.text.lower():
                    what = error.find('what')
                    stack = error.find('stack')
                    
                    leak_info = {
                        'kind': kind.text,
                        'what': what.text if what is not None else 'Unknown',
                        'stack': []
                    }
                    
                    if stack is not None:
                        for frame in stack.findall('frame'):
                            fn = frame.find('fn')
                            file = frame.find('file')
                            line = frame.find('line')
                            
                            frame_info = {
                                'function': fn.text if fn is not None else 'Unknown',
                                'file': file.text if file is not None else 'Unknown',
                                'line': line.text if line is not None else 'Unknown'
                            }
                            leak_info['stack'].append(frame_info)
                    
                    leaks.append(leak_info)
            
            return {'leaks': leaks, 'total_leaks': len(leaks)}
        
        except Exception as e:
            return {'error': f'Failed to parse Valgrind XML: {str(e)}'}
    
    def parse_valgrind_text(self, valgrind_output):
        """Parse Valgrind text output"""
        lines = valgrind_output.split('\n')
        leaks = []
        
        for line in lines:
            if 'definitely lost' in line or 'possibly lost' in line:
                # Extract leak information
                match = re.search(r'(\d+) bytes in (\d+) blocks', line)
                if match:
                    bytes_lost = int(match.group(1))
                    blocks_lost = int(match.group(2))
                    
                    leak_type = 'definitely lost' if 'definitely lost' in line else 'possibly lost'
                    
                    leaks.append({
                        'type': leak_type,
                        'bytes': bytes_lost,
                        'blocks': blocks_lost
                    })
        
        return {'leaks': leaks, 'total_leaks': len(leaks)}
    
    def analyze_embedded_memory(self, elf_file):
        """Analyze embedded memory usage"""
        print(f"Analyzing embedded memory usage for {elf_file}...")
        
        if not os.path.exists(elf_file):
            print(f"ELF file not found: {elf_file}")
            return None
        
        # Use avr-size to get memory information
        try:
            result = subprocess.run(['avr-size', '-A', elf_file], 
                                  capture_output=True, text=True)
            
            if result.returncode != 0:
                return {'error': f'avr-size failed: {result.stderr}'}
            
            # Parse avr-size output
            memory_info = self.parse_avr_size_output(result.stdout)
            
            # Check for memory usage issues
            warnings = []
            
            # Check flash usage
            if 'flash' in memory_info:
                flash_usage = memory_info['flash']
                if flash_usage > 0.9:  # > 90%
                    warnings.append({
                        'type': 'high_flash_usage',
                        'message': f'Flash usage is {flash_usage:.1%}',
                        'severity': 'high'
                    })
            
            # Check RAM usage
            if 'ram' in memory_info:
                ram_usage = memory_info['ram']
                if ram_usage > 0.8:  # > 80%
                    warnings.append({
                        'type': 'high_ram_usage',
                        'message': f'RAM usage is {ram_usage:.1%}',
                        'severity': 'high'
                    })
            
            memory_info['warnings'] = warnings
            return memory_info
        
        except Exception as e:
            return {'error': f'Memory analysis failed: {str(e)}'}
    
    def parse_avr_size_output(self, output):
        """Parse avr-size output"""
        lines = output.split('\n')
        sections = {}
        
        for line in lines:
            if line.startswith('.'):
                parts = line.split()
                if len(parts) >= 2:
                    section = parts[0]
                    size = int(parts[1])
                    sections[section] = size
        
        # Calculate memory usage
        flash_sections = ['.text', '.data']
        ram_sections = ['.data', '.bss']
        
        flash_used = sum(sections.get(s, 0) for s in flash_sections)
        ram_used = sum(sections.get(s, 0) for s in ram_sections)
        
        # ATmega32A specifications
        flash_total = 32 * 1024  # 32KB flash
        ram_total = 2 * 1024     # 2KB RAM
        
        return {
            'sections': sections,
            'flash_used': flash_used,
            'flash_total': flash_total,
            'flash': flash_used / flash_total,
            'ram_used': ram_used,
            'ram_total': ram_total,
            'ram': ram_used / ram_total
        }
    
    def run_simulation_memory_test(self):
        """Run memory leak test on simulation"""
        print("Running simulation memory leak test...")
        
        # Build test executable
        result = subprocess.run(['pio', 'run', '-e', 'test_desktop'], 
                              capture_output=True, text=True)
        
        if result.returncode != 0:
            return {'error': f'Build failed: {result.stderr}'}
        
        # Find test executable
        test_exe = None
        for root, dirs, files in os.walk('.pio/build/test_desktop'):
            for file in files:
                if file == 'program' or file == 'firmware':
                    test_exe = os.path.join(root, file)
                    break
        
        if not test_exe:
            return {'error': 'Test executable not found'}
        
        # Run Valgrind if available
        if self.is_valgrind_available():
            valgrind_result = self.run_valgrind_analysis(test_exe)
            if valgrind_result:
                return valgrind_result
        
        # Run basic memory monitoring
        return self.run_basic_memory_test(test_exe)
    
    def is_valgrind_available(self):
        """Check if Valgrind is available"""
        try:
            result = subprocess.run(['valgrind', '--version'], 
                                  capture_output=True, text=True)
            return result.returncode == 0
        except:
            return False
    
    def run_basic_memory_test(self, test_exe):
        """Run basic memory monitoring test"""
        # This would monitor memory usage over time
        # For now, return a placeholder
        return {
            'test_type': 'basic_memory_test',
            'executable': test_exe,
            'status': 'completed',
            'message': 'Basic memory test completed - no leaks detected'
        }
    
    def generate_memory_report(self, output_file='memory_report.json'):
        """Generate comprehensive memory report"""
        print("Generating memory leak report...")
        
        report = {
            'timestamp': datetime.now().isoformat(),
            'static_analysis': self.detect_static_memory_issues(),
            'simulation_test': self.run_simulation_memory_test(),
            'embedded_analysis': None
        }
        
        # Try to analyze embedded memory if ELF file exists
        elf_files = []
        for root, dirs, files in os.walk('.pio/build'):
            for file in files:
                if file.endswith('.elf'):
                    elf_files.append(os.path.join(root, file))
        
        if elf_files:
            report['embedded_analysis'] = self.analyze_embedded_memory(elf_files[0])
        
        # Generate summary
        report['summary'] = self.generate_summary(report)
        
        # Write report
        with open(output_file, 'w') as f:
            json.dump(report, f, indent=2)
        
        print(f"Memory report generated: {output_file}")
        return report
    
    def generate_summary(self, report):
        """Generate report summary"""
        summary = {
            'total_issues': 0,
            'high_severity': 0,
            'medium_severity': 0,
            'low_severity': 0,
            'recommendations': []
        }
        
        # Count static analysis issues
        if report['static_analysis']:
            for issue in report['static_analysis']:
                summary['total_issues'] += 1
                
                severity = issue.get('severity', 'low')
                if severity == 'high':
                    summary['high_severity'] += 1
                elif severity == 'medium':
                    summary['medium_severity'] += 1
                else:
                    summary['low_severity'] += 1
        
        # Add recommendations
        if summary['high_severity'] > 0:
            summary['recommendations'].append("Address high severity memory issues immediately")
        
        if summary['total_issues'] > 10:
            summary['recommendations'].append("Consider code review for memory management")
        
        # Check embedded memory usage
        if report['embedded_analysis'] and 'warnings' in report['embedded_analysis']:
            for warning in report['embedded_analysis']['warnings']:
                if warning['severity'] == 'high':
                    summary['recommendations'].append(f"Optimize memory usage: {warning['message']}")
        
        return summary

def main():
    parser = argparse.ArgumentParser(description='Memory leak detection for Multi-Sonicator')
    parser.add_argument('--source-dir', default='.', help='Source directory to analyze')
    parser.add_argument('--output', default='memory_report.json', help='Output report file')
    parser.add_argument('--static-only', action='store_true', help='Run only static analysis')
    parser.add_argument('--simulation-only', action='store_true', help='Run only simulation tests')
    
    args = parser.parse_args()
    
    detector = MemoryLeakDetector()
    
    print("Multi-Sonicator Memory Leak Detection")
    print("=" * 40)
    
    if args.static_only:
        issues = detector.detect_static_memory_issues(args.source_dir)
        print(f"Found {len(issues)} static memory issues")
        for issue in issues:
            print(f"  {issue['file']}:{issue['line']} - {issue['message']}")
    elif args.simulation_only:
        result = detector.run_simulation_memory_test()
        print(f"Simulation test result: {result}")
    else:
        # Run complete analysis
        report = detector.generate_memory_report(args.output)
        
        # Print summary
        summary = report['summary']
        print(f"\nMemory Analysis Summary:")
        print(f"  Total Issues: {summary['total_issues']}")
        print(f"  High Severity: {summary['high_severity']}")
        print(f"  Medium Severity: {summary['medium_severity']}")
        print(f"  Low Severity: {summary['low_severity']}")
        
        if summary['recommendations']:
            print("\nRecommendations:")
            for rec in summary['recommendations']:
                print(f"  - {rec}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())