#!/usr/bin/env python3
"""
PlatformIO Post-build Hook for CFFI Integration

This script updates the CFFI shared library after firmware compilation
to ensure testing stays in sync with the latest code changes.
"""

import os
import sys
from pathlib import Path

try:
    Import("env")
    PROJECT_ROOT = Path(env.get("PROJECT_DIR"))
except:
    # Fallback for direct execution
    PROJECT_ROOT = Path(__file__).parent.parent

def post_build_cffi():
    """Update CFFI shared library after firmware build."""
    print("🔄 Post-build CFFI update...")
    
    build_script = PROJECT_ROOT / "build_cffi.py"
    if build_script.exists():
        import subprocess
        result = subprocess.run([
            sys.executable, str(build_script)
        ], cwd=PROJECT_ROOT, capture_output=True, text=True)
        
        if result.returncode == 0:
            print("✅ CFFI shared library updated after firmware build")
        else:
            print(f"⚠️  CFFI post-build update failed: {result.stderr}")
    else:
        print("⚠️  build_cffi.py not found, skipping post-build CFFI update")

# Execute the post-build update
post_build_cffi()
