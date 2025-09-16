#!/usr/bin/env python3
"""
Version Bump Script for Multi-Sonicator CI/CD Pipeline
Handles semantic versioning and version updates
"""

import os
import sys
import re
import json
import argparse
from datetime import datetime

def get_current_version():
    """
    Get current version from various sources
    """
    # Try to get from system_config.h
    config_path = 'include/system_config.h'
    if os.path.exists(config_path):
        with open(config_path, 'r') as f:
            content = f.read()
            match = re.search(r'#define\s+PROJECT_VERSION\s+"([^"]+)"', content)
            if match:
                return match.group(1)
    
    # Default version
    return "1.0.0"

def update_version_in_config(new_version):
    """
    Update version in system_config.h
    """
    config_path = 'include/system_config.h'
    if not os.path.exists(config_path):
        return False
    
    with open(config_path, 'r') as f:
        content = f.read()
    
    # Update PROJECT_VERSION
    content = re.sub(
        r'#define\s+PROJECT_VERSION\s+"[^"]+"',
        f'#define PROJECT_VERSION "{new_version}"',
        content
    )
    
    with open(config_path, 'w') as f:
        f.write(content)
    
    return True

def parse_version(version_string):
    """
    Parse version string into major, minor, patch
    """
    # Remove 'v' prefix if present
    if version_string.startswith('v'):
        version_string = version_string[1:]
    
    match = re.match(r'^(\d+)\.(\d+)\.(\d+)(?:-(.+))?$', version_string)
    if not match:
        raise ValueError(f"Invalid version format: {version_string}")
    
    return {
        'major': int(match.group(1)),
        'minor': int(match.group(2)),
        'patch': int(match.group(3)),
        'prerelease': match.group(4) if match.group(4) else None
    }

def increment_version(current_version, increment_type):
    """
    Increment version based on type (major, minor, patch)
    """
    version_parts = parse_version(current_version)
    
    if increment_type == 'major':
        version_parts['major'] += 1
        version_parts['minor'] = 0
        version_parts['patch'] = 0
    elif increment_type == 'minor':
        version_parts['minor'] += 1
        version_parts['patch'] = 0
    elif increment_type == 'patch':
        version_parts['patch'] += 1
    else:
        raise ValueError(f"Invalid increment type: {increment_type}")
    
    # Clear prerelease for stable releases
    version_parts['prerelease'] = None
    
    return format_version(version_parts)

def format_version(version_parts):
    """
    Format version parts into string
    """
    version = f"{version_parts['major']}.{version_parts['minor']}.{version_parts['patch']}"
    if version_parts['prerelease']:
        version += f"-{version_parts['prerelease']}"
    return version

def main():
    parser = argparse.ArgumentParser(description='Version management for Multi-Sonicator')
    parser.add_argument('--current', action='store_true', help='Show current version')
    parser.add_argument('--increment', choices=['major', 'minor', 'patch'], help='Increment version')
    parser.add_argument('--set', help='Set specific version')
    parser.add_argument('--prerelease', help='Add prerelease suffix')
    
    args = parser.parse_args()
    
    current_version = get_current_version()
    
    if args.current:
        print(f"Current version: {current_version}")
        return 0
    
    new_version = current_version
    
    if args.increment:
        new_version = increment_version(current_version, args.increment)
    elif args.set:
        new_version = args.set
    
    if args.prerelease:
        version_parts = parse_version(new_version)
        version_parts['prerelease'] = args.prerelease
        new_version = format_version(version_parts)
    
    if new_version != current_version:
        print(f"Updating version from {current_version} to {new_version}")
        if update_version_in_config(new_version):
            print("Version updated successfully")
        else:
            print("Failed to update version")
            return 1
    
    # Generate version info file
    version_info = {
        'version': new_version,
        'build_date': datetime.now().isoformat(),
        'git_commit': os.getenv('GITHUB_SHA', 'unknown'),
        'git_branch': os.getenv('GITHUB_REF_NAME', 'unknown')
    }
    
    with open('version.json', 'w') as f:
        json.dump(version_info, f, indent=2)
    
    print(f"Version info written to version.json")
    return 0

if __name__ == "__main__":
    sys.exit(main())