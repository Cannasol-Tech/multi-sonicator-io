#!/usr/bin/env python3
"""
Pending Scenarios Manager
Automatically manages @pending tags for unimplemented feature scenarios
while maintaining traceability
"""

import os
import sys
import re
import yaml
from pathlib import Path
from typing import Dict, List, Set, Tuple

class PendingScenariosManager:
    def __init__(self, project_root=None):
        self.project_root = Path(project_root) if project_root else Path(__file__).parent.parent
        self.features_dir = self.project_root / "test" / "acceptance" / "features"
        self.config_file = self.project_root / "test" / "acceptance" / "pending_scenarios.yaml"
        
        # Load configuration
        self.config = self.load_config()
        
    def load_config(self) -> Dict:
        """Load pending scenarios configuration"""
        default_config = {
            "auto_skip_patterns": [
                "not yet implemented",
                "TODO:",
                "FIXME:",
                "placeholder"
            ],
            "implementation_status": {
                "gpio": "implemented",
                "adc": "implemented", 
                "pwm": "implemented",
                "modbus": "implemented",
                "power": "implemented",
                "connectivity": "implemented",
                "sonicator": "partial",
                "advanced_features": "pending"
            },
            "skip_by_feature": {
                "advanced_diagnostics": True,
                "performance_profiling": True,
                "automated_calibration": False
            }
        }
        
        if self.config_file.exists():
            try:
                with open(self.config_file, 'r') as f:
                    loaded_config = yaml.safe_load(f)
                    default_config.update(loaded_config)
            except Exception as e:
                print(f"Warning: Could not load config file: {e}")
        
        return default_config
    
    def save_config(self):
        """Save configuration to file"""
        try:
            with open(self.config_file, 'w') as f:
                yaml.dump(self.config, f, default_flow_style=False, indent=2)
        except Exception as e:
            print(f"Error saving config: {e}")
    
    def scan_feature_files(self) -> List[Tuple[Path, Dict]]:
        """Scan all feature files and extract scenario information"""
        feature_files = []
        
        for feature_file in self.features_dir.glob("*.feature"):
            try:
                with open(feature_file, 'r') as f:
                    content = f.read()
                
                feature_info = self.parse_feature_file(content, feature_file)
                feature_files.append((feature_file, feature_info))
                
            except Exception as e:
                print(f"Warning: Could not parse {feature_file}: {e}")
        
        return feature_files
    
    def parse_feature_file(self, content: str, file_path: Path) -> Dict:
        """Parse a feature file and extract scenario information"""
        lines = content.split('\n')
        
        feature_info = {
            "name": None,
            "scenarios": [],
            "tags": set(),
            "file_path": file_path
        }
        
        current_scenario = None
        in_scenario = False
        
        for line_num, line in enumerate(lines, 1):
            line = line.strip()
            
            if line.startswith('Feature:'):
                feature_info["name"] = line.replace('Feature:', '').strip()
            
            elif line.startswith('@'):
                # Extract tags
                tags = [tag.strip('@') for tag in line.split()]
                if in_scenario and current_scenario:
                    current_scenario["tags"].update(tags)
                else:
                    feature_info["tags"].update(tags)
            
            elif line.startswith('Scenario:'):
                if current_scenario:
                    feature_info["scenarios"].append(current_scenario)
                
                current_scenario = {
                    "name": line.replace('Scenario:', '').strip(),
                    "line_number": line_num,
                    "tags": set(),
                    "steps": [],
                    "should_skip": False,
                    "skip_reason": None
                }
                in_scenario = True
            
            elif in_scenario and current_scenario:
                if line.startswith(('Given', 'When', 'Then', 'And', 'But')):
                    current_scenario["steps"].append(line)
                    
                    # Check for auto-skip patterns
                    for pattern in self.config["auto_skip_patterns"]:
                        if pattern.lower() in line.lower():
                            current_scenario["should_skip"] = True
                            current_scenario["skip_reason"] = f"Contains pattern: {pattern}"
                            break
        
        # Add the last scenario
        if current_scenario:
            feature_info["scenarios"].append(current_scenario)
        
        return feature_info
    
    def determine_skip_status(self, feature_info: Dict) -> Dict:
        """Determine which scenarios should be skipped based on configuration"""
        feature_name = feature_info["name"].lower() if feature_info["name"] else ""
        
        for scenario in feature_info["scenarios"]:
            # Check if already marked as pending
            if "pending" in scenario["tags"]:
                scenario["should_skip"] = True
                scenario["skip_reason"] = "Already marked as @pending"
                continue
            
            # Check implementation status by feature type
            for feature_type, status in self.config["implementation_status"].items():
                if feature_type in feature_name:
                    if status == "pending":
                        scenario["should_skip"] = True
                        scenario["skip_reason"] = f"Feature {feature_type} not implemented"
                    elif status == "partial":
                        # Only skip if scenario has specific unimplemented indicators
                        if any(pattern in " ".join(scenario["steps"]).lower() 
                               for pattern in self.config["auto_skip_patterns"]):
                            scenario["should_skip"] = True
                            scenario["skip_reason"] = f"Partial implementation - scenario not ready"
                    break
            
            # Check skip by feature flags
            for feature_flag, should_skip in self.config["skip_by_feature"].items():
                if should_skip and feature_flag in feature_name:
                    scenario["should_skip"] = True
                    scenario["skip_reason"] = f"Feature flag {feature_flag} disabled"
                    break
        
        return feature_info
    
    def update_feature_file(self, file_path: Path, feature_info: Dict) -> bool:
        """Update feature file with @pending tags where needed"""
        try:
            with open(file_path, 'r') as f:
                lines = f.readlines()
            
            modified = False
            
            for scenario in feature_info["scenarios"]:
                if scenario["should_skip"] and "pending" not in scenario["tags"]:
                    # Find the scenario line and add @pending tag before it
                    scenario_line = scenario["line_number"] - 1
                    
                    # Look for existing tags on the line before
                    tag_line = scenario_line - 1
                    while tag_line >= 0 and lines[tag_line].strip().startswith('@'):
                        tag_line -= 1
                    
                    # Insert @pending tag
                    indent = len(lines[scenario_line]) - len(lines[scenario_line].lstrip())
                    pending_tag = ' ' * indent + f"@pending  # {scenario['skip_reason']}\n"
                    lines.insert(scenario_line, pending_tag)
                    modified = True
            
            if modified:
                with open(file_path, 'w') as f:
                    f.writelines(lines)
                return True
                
        except Exception as e:
            print(f"Error updating {file_path}: {e}")
        
        return False
    
    def generate_skip_report(self, feature_files: List[Tuple[Path, Dict]]) -> Dict:
        """Generate report of skipped scenarios"""
        report = {
            "total_features": len(feature_files),
            "total_scenarios": 0,
            "skipped_scenarios": 0,
            "skip_reasons": {},
            "features": []
        }
        
        for file_path, feature_info in feature_files:
            feature_report = {
                "name": feature_info["name"],
                "file": file_path.name,
                "total_scenarios": len(feature_info["scenarios"]),
                "skipped_scenarios": 0,
                "scenarios": []
            }
            
            for scenario in feature_info["scenarios"]:
                report["total_scenarios"] += 1
                
                scenario_report = {
                    "name": scenario["name"],
                    "skipped": scenario["should_skip"],
                    "reason": scenario.get("skip_reason")
                }
                
                if scenario["should_skip"]:
                    report["skipped_scenarios"] += 1
                    feature_report["skipped_scenarios"] += 1
                    
                    reason = scenario.get("skip_reason", "Unknown")
                    report["skip_reasons"][reason] = report["skip_reasons"].get(reason, 0) + 1
                
                feature_report["scenarios"].append(scenario_report)
            
            report["features"].append(feature_report)
        
        return report
    
    def run_analysis(self, update_files=False):
        """Run complete analysis and optionally update files"""
        print("🔍 Scanning BDD feature files for pending scenarios...")
        
        feature_files = self.scan_feature_files()
        
        # Determine skip status for each feature
        for i, (file_path, feature_info) in enumerate(feature_files):
            feature_files[i] = (file_path, self.determine_skip_status(feature_info))
        
        # Generate report
        report = self.generate_skip_report(feature_files)
        
        # Update files if requested
        updated_files = 0
        if update_files:
            print("📝 Updating feature files with @pending tags...")
            for file_path, feature_info in feature_files:
                if self.update_feature_file(file_path, feature_info):
                    updated_files += 1
        
        # Print summary
        print(f"\n📊 Pending Scenarios Analysis:")
        print(f"   Total Features: {report['total_features']}")
        print(f"   Total Scenarios: {report['total_scenarios']}")
        print(f"   Skipped Scenarios: {report['skipped_scenarios']}")
        print(f"   Skip Percentage: {(report['skipped_scenarios']/report['total_scenarios']*100):.1f}%")
        
        if report['skip_reasons']:
            print(f"\n📋 Skip Reasons:")
            for reason, count in report['skip_reasons'].items():
                print(f"   • {reason}: {count} scenarios")
        
        if update_files:
            print(f"\n✅ Updated {updated_files} feature files")
        
        return report

def main():
    import argparse
    
    parser = argparse.ArgumentParser(description="Manage pending BDD scenarios")
    parser.add_argument("--update", action="store_true", 
                       help="Update feature files with @pending tags")
    parser.add_argument("--config", help="Path to configuration file")
    
    args = parser.parse_args()
    
    manager = PendingScenariosManager()
    report = manager.run_analysis(update_files=args.update)
    
    # Save updated configuration
    manager.save_config()

if __name__ == "__main__":
    main()
