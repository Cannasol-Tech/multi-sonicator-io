#!/usr/bin/env python3
"""
Company Standards Synchronization Script

Downloads only the docs/sop/ directory from the Axovia-AI/agile-flow repository
to keep company standards up to date without pulling the entire project.

Usage:
    python3 scripts/sync_company_standards.py [--check-only]
"""

import os
import sys
import json
import shutil
import tempfile
import subprocess
from pathlib import Path
from urllib.request import urlopen, Request
from urllib.error import URLError, HTTPError

# Configuration - Update these values for your organization
REPO_OWNER = "Axovia-AI"  # Change to your organization
REPO_NAME = "agile-flow"  # Change to your standards repository
STANDARDS_SOURCE_PATH = "docs/sop"
STANDARDS_TARGET_PATH = "docs/sop"
GITHUB_API_BASE = "https://api.github.com"
GITHUB_RAW_BASE = "https://raw.githubusercontent.com"

# Fallback: Use local standards if remote repository is not available
FALLBACK_TO_LOCAL = True
LOCAL_STANDARDS_PATH = "docs/sop"

def log(message, level="INFO"):
    """Simple logging function"""
    print(f"[{level}] {message}")

def get_latest_commit_sha():
    """Get the latest commit SHA from the main branch"""
    try:
        url = f"{GITHUB_API_BASE}/repos/{REPO_OWNER}/{REPO_NAME}/branches/main"
        req = Request(url)
        req.add_header('User-Agent', 'Multi-Sonicator-IO-Standards-Sync/1.0')
        
        with urlopen(req) as response:
            data = json.loads(response.read().decode())
            return data['commit']['sha']
    except (URLError, HTTPError, KeyError) as e:
        log(f"Failed to get latest commit SHA: {e}", "ERROR")
        return None

def get_directory_contents(commit_sha, path):
    """Get the contents of a directory from GitHub API"""
    try:
        url = f"{GITHUB_API_BASE}/repos/{REPO_OWNER}/{REPO_NAME}/contents/{path}?ref={commit_sha}"
        req = Request(url)
        req.add_header('User-Agent', 'Multi-Sonicator-IO-Standards-Sync/1.0')
        
        with urlopen(req) as response:
            return json.loads(response.read().decode())
    except (URLError, HTTPError) as e:
        log(f"Failed to get directory contents for {path}: {e}", "ERROR")
        return None

def download_file(file_url, target_path):
    """Download a file from GitHub"""
    try:
        req = Request(file_url)
        req.add_header('User-Agent', 'Multi-Sonicator-IO-Standards-Sync/1.0')
        
        with urlopen(req) as response:
            content = response.read()
            
        # Ensure target directory exists
        target_path.parent.mkdir(parents=True, exist_ok=True)
        
        # Write file
        with open(target_path, 'wb') as f:
            f.write(content)
        
        log(f"Downloaded: {target_path}")
        return True
    except (URLError, HTTPError) as e:
        log(f"Failed to download {file_url}: {e}", "ERROR")
        return False

def sync_standards_directory(commit_sha, source_path="", target_base=None):
    """Recursively sync the standards directory"""
    if target_base is None:
        target_base = Path(STANDARDS_TARGET_PATH)
    
    full_source_path = f"{STANDARDS_SOURCE_PATH}/{source_path}".strip("/")
    contents = get_directory_contents(commit_sha, full_source_path)
    
    if not contents:
        return False
    
    success = True
    for item in contents:
        item_name = item['name']
        item_type = item['type']
        
        if item_type == 'file':
            # Download file
            file_url = f"{GITHUB_RAW_BASE}/{REPO_OWNER}/{REPO_NAME}/{commit_sha}/{full_source_path}/{item_name}"
            target_file = target_base / source_path / item_name
            if not download_file(file_url, target_file):
                success = False
        
        elif item_type == 'dir':
            # Recursively sync subdirectory
            sub_path = f"{source_path}/{item_name}".strip("/")
            if not sync_standards_directory(commit_sha, sub_path, target_base):
                success = False
    
    return success

def get_current_commit_info():
    """Get current commit info from local standards"""
    info_file = Path(STANDARDS_TARGET_PATH) / ".sync_info.json"
    if info_file.exists():
        try:
            with open(info_file, 'r') as f:
                return json.load(f)
        except (json.JSONDecodeError, IOError):
            pass
    return None

def save_commit_info(commit_sha):
    """Save current commit info to local standards"""
    info_file = Path(STANDARDS_TARGET_PATH) / ".sync_info.json"
    info_file.parent.mkdir(parents=True, exist_ok=True)
    
    info = {
        "commit_sha": commit_sha,
        "sync_timestamp": subprocess.check_output(['date', '-u', '+%Y-%m-%dT%H:%M:%SZ']).decode().strip(),
        "source_repo": f"{REPO_OWNER}/{REPO_NAME}",
        "source_path": STANDARDS_SOURCE_PATH
    }
    
    with open(info_file, 'w') as f:
        json.dump(info, f, indent=2)

def validate_root_directory():
    """Validate root directory compliance"""
    log("🔍 Validating root directory structure...")
    
    # Required root files
    required_files = {
        '.gitignore': 'Version control exclusions',
        '.npmignore': 'NPM package exclusions', 
        'Makefile': 'Development workflow targets',
        'LICENSE': 'Project license',
        'README.md': 'Project documentation'
    }
    
    # Optional files
    optional_files = {'.gitmodules': 'Git submodule definitions'}
    
    # Prohibited files (examples)
    prohibited_patterns = [
        'package.json', 'package-lock.json', 'requirements.txt',
        'pytest.ini', 'behave.ini', 'setup.cfg', 'platformio.ini'
    ]
    
    violations = []
    warnings = []
    
    # Check for required files
    for file, description in required_files.items():
        if not Path(file).exists():
            violations.append(f"Missing required file: {file} ({description})")
        else:
            log(f"✅ Found required file: {file}")
    
    # Check for prohibited files in root
    for pattern in prohibited_patterns:
        if Path(pattern).exists():
            warnings.append(f"Prohibited file in root: {pattern} (should be in config/ or appropriate subdirectory)")
    
    # Report violations
    if violations:
        log("❌ Root directory violations found:", "ERROR")
        for violation in violations:
            log(f"   - {violation}", "ERROR")
    
    if warnings:
        log("⚠️ Root directory warnings:", "WARNING")
        for warning in warnings:
            log(f"   - {warning}", "WARNING")
    
    return len(violations) == 0

def validate_core_directories():
    """Validate core directory structure"""
    log("🔍 Validating core directory structure...")
    
    # Required core directories
    required_dirs = {
        'docs': 'Documentation',
        'src': 'Source code',
        'bin': 'Binary/executable files'
    }
    
    # Required docs subdirectories per company standards
    required_docs_subdirs = {
        'docs/architecture': 'Architecture documentation',
        'docs/planning': 'Planning documents',
        'docs/requirements': 'Requirements documentation', 
        'docs/sop': 'Project standards',
        'docs/agile': 'Agile development artifacts',
        'docs/agent-reports': 'AI agent reports',
        'docs/testing': 'Testing documentation'
    }
    
    violations = []
    
    # Check core directories
    for dir_path, description in required_dirs.items():
        if not Path(dir_path).is_dir():
            violations.append(f"Missing required directory: {dir_path} ({description})")
        else:
            log(f"✅ Found required directory: {dir_path}")
    
    # Check docs subdirectories
    for dir_path, description in required_docs_subdirs.items():
        if not Path(dir_path).is_dir():
            violations.append(f"Missing required docs subdirectory: {dir_path} ({description})")
        else:
            log(f"✅ Found required docs subdirectory: {dir_path}")
            
            # Check for README.md in each subdirectory
            readme_path = Path(dir_path) / 'README.md'
            if not readme_path.exists():
                violations.append(f"Missing README.md in {dir_path}")
            else:
                log(f"✅ Found README.md in {dir_path}")
    
    if violations:
        log("❌ Directory structure violations found:", "ERROR")
        for violation in violations:
            log(f"   - {violation}", "ERROR")
    
    return len(violations) == 0

def validate_standards_files():
    """Validate standards files integrity"""
    log("🔍 Validating standards files integrity...")
    
    standards_path = Path(STANDARDS_TARGET_PATH)
    if not standards_path.exists():
        log("❌ Standards directory not found", "ERROR")
        return False
    
    # Check for key standards files
    key_files = [
        'project-structure.md',
        'coding-style.md', 
        'sw-testing-standard.md'
    ]
    
    violations = []
    
    for file in key_files:
        file_path = standards_path / file
        if not file_path.exists():
            violations.append(f"Missing key standards file: {file}")
        else:
            # Basic content validation
            try:
                with open(file_path, 'r') as f:
                    content = f.read()
                    if len(content.strip()) < 100:
                        violations.append(f"Standards file too short (possible corruption): {file}")
                    else:
                        log(f"✅ Standards file validated: {file}")
            except Exception as e:
                violations.append(f"Cannot read standards file {file}: {e}")
    
    if violations:
        log("❌ Standards files validation failed:", "ERROR")
        for violation in violations:
            log(f"   - {violation}", "ERROR")
    
    return len(violations) == 0

def comprehensive_standards_check():
    """Perform comprehensive standards compliance check"""
    log("🔍 Starting comprehensive standards compliance check...")
    
    # Component checks
    checks = [
        ("Root Directory Compliance", validate_root_directory),
        ("Core Directory Structure", validate_core_directories),
        ("Standards Files Integrity", validate_standards_files)
    ]
    
    results = []
    overall_success = True
    
    for check_name, check_func in checks:
        log(f"\n📋 {check_name}:")
        try:
            result = check_func()
            results.append((check_name, result, None))
            if not result:
                overall_success = False
        except Exception as e:
            log(f"❌ {check_name} failed with exception: {e}", "ERROR")
            results.append((check_name, False, str(e)))
            overall_success = False
    
    # Summary report
    log("\n📊 Standards Compliance Summary:")
    for check_name, success, error in results:
        status = "✅ PASS" if success else "❌ FAIL"
        log(f"   {status} - {check_name}")
        if error:
            log(f"     Error: {error}")
    
    if overall_success:
        log("\n✅ All standards compliance checks passed!")
    else:
        log("\n❌ Standards compliance violations found. Please review and fix issues above.", "ERROR")
    
    return overall_success

def check_standards_status():
    """Check if standards are up to date and validate compliance"""
    log("🔍 Checking company standards status...")

    # Get latest commit
    latest_sha = get_latest_commit_sha()
    if not latest_sha:
        if FALLBACK_TO_LOCAL:
            log("⚠️ Remote repository not available, checking local standards", "WARNING")
            current_info = get_current_commit_info()
            if current_info and current_info.get('source') == 'local':
                log("✅ Using local standards (remote not available)")
                # Still perform comprehensive validation
                return comprehensive_standards_check()
            else:
                log("📥 Local standards sync required")
                return False
        else:
            log("❌ Failed to check remote repository", "ERROR")
            return False
    
    # Get current local commit
    current_info = get_current_commit_info()
    
    if not current_info:
        log("📥 No local standards found - sync required")
        return False
    
    current_sha = current_info.get('commit_sha')
    if current_sha == latest_sha:
        log("✅ Standards are up to date")
    else:
        log("📥 Updates available from central repository")
        log(f"   Current: {current_sha[:8] if current_sha else 'unknown'}")
        log(f"   Latest:  {latest_sha[:8]}")
    
    # Perform comprehensive validation regardless of version status
    return comprehensive_standards_check()

def copy_local_standards():
    """Copy local standards as fallback"""
    log("📚 Using local standards as fallback...")

    source_path = Path(LOCAL_STANDARDS_PATH)
    target_path = Path(STANDARDS_TARGET_PATH)

    if not source_path.exists():
        log(f"❌ Local standards not found at {source_path}", "ERROR")
        return False

    # Clean target directory
    if target_path.exists():
        shutil.rmtree(target_path)

    # Copy local standards
    shutil.copytree(source_path, target_path)

    # Save info
    info = {
        "source": "local",
        "sync_timestamp": subprocess.check_output(['date', '-u', '+%Y-%m-%dT%H:%M:%SZ']).decode().strip(),
        "source_path": str(source_path)
    }

    info_file = target_path / ".sync_info.json"
    with open(info_file, 'w') as f:
        json.dump(info, f, indent=2)

    log("✅ Local standards copied successfully")
    return True

def sync_standards():
    """Sync company standards from central repository"""
    log(f"📚 Syncing company standards from {REPO_OWNER}/{REPO_NAME}...")

    # Get latest commit
    latest_sha = get_latest_commit_sha()
    if not latest_sha:
        if FALLBACK_TO_LOCAL:
            log("⚠️ Remote repository not available, falling back to local standards", "WARNING")
            return copy_local_standards()
        else:
            log("❌ Failed to get latest commit information", "ERROR")
            return False
    
    # Create target directory
    target_path = Path(STANDARDS_TARGET_PATH)
    if target_path.exists():
        log(f"🧹 Cleaning existing standards directory: {target_path}")
        shutil.rmtree(target_path)
    
    # Sync standards directory
    log(f"📥 Downloading standards from commit {latest_sha[:8]}...")
    if not sync_standards_directory(latest_sha):
        log("❌ Failed to sync standards", "ERROR")
        return False
    
    # Save commit info
    save_commit_info(latest_sha)
    
    log("✅ Company standards synchronized successfully")
    log(f"📁 Standards available in: {target_path}")
    return True

def main():
    """Main function"""
    if len(sys.argv) > 1 and sys.argv[1] == "--check-only":
        success = check_standards_status()
        sys.exit(0 if success else 1)
    else:
        success = sync_standards()
        sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
