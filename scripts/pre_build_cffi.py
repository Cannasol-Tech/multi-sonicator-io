#!/usr/bin/env python3
"""
PlatformIO Pre-build Hook for CFFI Integration

This script ensures the CFFI shared library is available before firmware builds.
It checks if business logic files have changed and rebuilds if necessary.
"""

import os
import sys
from pathlib import Path

Import("env")

PROJECT_ROOT = Path(env.get("PROJECT_DIR"))
BUSINESS_LOGIC_DIR = PROJECT_ROOT / "src" / "business_logic"
BUILD_DIR = PROJECT_ROOT / "build"
SHARED_LIB_PATH = BUILD_DIR / "libsonicator_logic.so"

def check_cffi_build_needed():
    """Check if CFFI shared library needs to be rebuilt."""
    if not SHARED_LIB_PATH.exists():
        return True
    
    if not BUSINESS_LOGIC_DIR.exists():
        return False  # No business logic files yet
    
    # Check if any business logic files are newer than shared library
    lib_mtime = SHARED_LIB_PATH.stat().st_mtime
    
    for c_file in BUSINESS_LOGIC_DIR.glob("*.c"):
        if c_file.stat().st_mtime > lib_mtime:
            return True
    
    return False

def build_cffi_if_needed():
    """Build CFFI shared library if needed."""
    if check_cffi_build_needed():
        print("🏗️  CFFI shared library needs updating...")
        
        build_script = PROJECT_ROOT / "build_cffi.py"
        if build_script.exists():
            import subprocess
            result = subprocess.run([
                sys.executable, str(build_script)
            ], cwd=PROJECT_ROOT)
            
            if result.returncode == 0:
                print("✅ CFFI shared library updated successfully")
            else:
                print("⚠️  CFFI build failed, continuing with firmware build")
        else:
            print("⚠️  build_cffi.py not found, skipping CFFI build")
    else:
        print("✅ CFFI shared library is up to date")

# Execute the check
print("🔍 Checking CFFI build status...")
build_cffi_if_needed()
