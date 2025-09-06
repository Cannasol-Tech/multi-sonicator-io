#!/usr/bin/env python3
"""
Company Standards Synchronization Script

Downloads only the docs/standards/ directory from the Axovia-AI/axovia-flow repository
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
REPO_NAME = "axovia-flow"  # Change to your standards repository
STANDARDS_SOURCE_PATH = "docs/standards"
STANDARDS_TARGET_PATH = ".axovia-flow/company-standards"
GITHUB_API_BASE = "https://api.github.com"
GITHUB_RAW_BASE = "https://raw.githubusercontent.com"

# Fallback: Use local standards if remote repository is not available
FALLBACK_TO_LOCAL = True
LOCAL_STANDARDS_PATH = "docs/standards"

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

def check_standards_status():
    """Check if standards are up to date"""
    log("🔍 Checking company standards status...")

    # Get latest commit
    latest_sha = get_latest_commit_sha()
    if not latest_sha:
        if FALLBACK_TO_LOCAL:
            log("⚠️ Remote repository not available, checking local standards", "WARNING")
            current_info = get_current_commit_info()
            if current_info and current_info.get('source') == 'local':
                log("✅ Using local standards (remote not available)")
                return True
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
        return True
    else:
        log("📥 Updates available from central repository")
        log(f"   Current: {current_sha[:8] if current_sha else 'unknown'}")
        log(f"   Latest:  {latest_sha[:8]}")
        return False

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
