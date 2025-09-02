#!/usr/bin/env python3
"""
Development Environment Setup Script

This script sets up the complete development environment for CFFI-based
testing compliance with the Multi-Sonicator I/O Controller project.

Usage:
    python setup_dev_environment.py [--full] [--test-only]
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path

PROJECT_ROOT = Path(__file__).parent
REQUIREMENTS_FILE = PROJECT_ROOT / "requirements-testing.txt"

def check_python_version():
    """Ensure Python version is compatible."""
    if sys.version_info < (3, 7):
        print("❌ Python 3.7+ required for CFFI testing")
        print(f"   Current version: {sys.version}")
        return False
    
    print(f"✅ Python version: {sys.version.split()[0]}")
    return True

def check_gcc_available():
    """Check if GCC compiler is available."""
    try:
        result = subprocess.run(["gcc", "--version"], capture_output=True, text=True)
        if result.returncode == 0:
            version_line = result.stdout.split('\n')[0]
            print(f"✅ GCC compiler: {version_line}")
            return True
    except FileNotFoundError:
        pass
    
    print("❌ GCC compiler not found")
    print("   Install GCC:")
    print("   - macOS: xcode-select --install")
    print("   - Ubuntu: sudo apt-get install gcc")
    print("   - CentOS: sudo yum install gcc")
    return False

def install_python_dependencies():
    """Install Python dependencies for testing."""
    print("\n📦 Installing Python dependencies...")
    
    if not REQUIREMENTS_FILE.exists():
        print(f"❌ Requirements file not found: {REQUIREMENTS_FILE}")
        return False
    
    try:
        result = subprocess.run([
            sys.executable, "-m", "pip", "install", "-r", str(REQUIREMENTS_FILE)
        ], check=True, capture_output=True, text=True)
        
        print("✅ Python dependencies installed successfully")
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"❌ Failed to install dependencies: {e.stderr}")
        return False

def setup_code_structure():
    """Set up the separated code structure."""
    print("\n🏗️  Setting up code structure...")
    
    restructure_script = PROJECT_ROOT / "restructure_code.py"
    if not restructure_script.exists():
        print(f"❌ Restructure script not found: {restructure_script}")
        return False
    
    try:
        result = subprocess.run([
            sys.executable, str(restructure_script)
        ], cwd=PROJECT_ROOT, check=True, capture_output=True, text=True)
        
        print("✅ Code structure set up successfully")
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"❌ Failed to set up code structure: {e.stderr}")
        return False

def build_cffi_library():
    """Build the CFFI shared library."""
    print("\n🔨 Building CFFI shared library...")
    
    build_script = PROJECT_ROOT / "build_cffi.py"
    if not build_script.exists():
        print(f"❌ CFFI build script not found: {build_script}")
        return False
    
    try:
        result = subprocess.run([
            sys.executable, str(build_script)
        ], cwd=PROJECT_ROOT, check=True, capture_output=True, text=True)
        
        print("✅ CFFI shared library built successfully")
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"❌ Failed to build CFFI library: {e.stderr}")
        print("   This is expected if business logic C files are not yet implemented")
        return False

def run_initial_tests():
    """Run initial test suite to validate setup."""
    print("\n🧪 Running initial test validation...")
    
    # Check if pytest is available
    try:
        result = subprocess.run([
            sys.executable, "-m", "pytest", "--version"
        ], capture_output=True, text=True, check=True)
        
        print(f"✅ pytest available: {result.stdout.strip()}")
        
    except subprocess.CalledProcessError:
        print("❌ pytest not available")
        return False
    
    # Try to run tests (may fail if C functions not implemented)
    test_dir = PROJECT_ROOT / "test" / "unit"
    if test_dir.exists():
        try:
            result = subprocess.run([
                sys.executable, "-m", "pytest", str(test_dir), "-v", "--tb=short"
            ], cwd=PROJECT_ROOT, capture_output=True, text=True)
            
            if result.returncode == 0:
                print("✅ Initial tests passed")
                return True
            else:
                print("⚠️  Some tests failed (expected if C functions not implemented)")
                print("   Test framework is correctly set up")
                return True
                
        except Exception as e:
            print(f"⚠️  Test execution failed: {e}")
            return True  # Not critical for setup
    
    print("⚠️  No tests found yet - framework ready for implementation")
    return True

def check_platformio():
    """Check if PlatformIO is available for firmware builds."""
    print("\n🔍 Checking PlatformIO availability...")
    
    try:
        result = subprocess.run(["platformio", "--version"], 
                              capture_output=True, text=True, check=True)
        print(f"✅ PlatformIO: {result.stdout.strip()}")
        return True
        
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("⚠️  PlatformIO not found")
        print("   Install PlatformIO for firmware development:")
        print("   pip install platformio")
        return False

def create_vscode_settings():
    """Create VS Code settings for the project."""
    print("\n⚙️  Creating VS Code settings...")
    
    vscode_dir = PROJECT_ROOT / ".vscode"
    vscode_dir.mkdir(exist_ok=True)
    
    # Python path settings
    settings_json = vscode_dir / "settings.json"
    settings = {
        "python.defaultInterpreterPath": sys.executable,
        "python.testing.pytestEnabled": True,
        "python.testing.pytestArgs": ["test/unit"],
        "python.testing.autoTestDiscoverOnSaveEnabled": True,
        "files.associations": {
            "*.h": "c",
            "*.c": "c"
        },
        "C_Cpp.default.includePath": [
            "${workspaceFolder}/include",
            "${workspaceFolder}/include/business_logic",
            "${workspaceFolder}/include/hardware"
        ],
        "C_Cpp.default.defines": [
            "CFFI_TESTING"
        ]
    }
    
    import json
    with open(settings_json, 'w') as f:
        json.dump(settings, f, indent=2)
    
    print(f"✅ VS Code settings created: {settings_json}")

def print_next_steps():
    """Print next steps for development."""
    print("\n" + "="*60)
    print("🎉 Development environment setup complete!")
    print("="*60)
    
    print("\n📋 Next Steps:")
    print("1. Implement C business logic functions in src/business_logic/")
    print("2. Add corresponding pytest test cases in test/unit/")
    print("3. Run 'make test-unit' to validate implementations")
    print("4. Implement hardware interface in src/hardware/")
    print("5. Run 'make test-hil' for hardware-in-the-loop testing")
    
    print("\n🛠️  Available Commands:")
    print("   make cffi           # Build CFFI shared library")
    print("   make test-unit      # Run pytest unit tests")
    print("   make test           # Run all tests")
    print("   make build          # Build ATmega32A firmware")
    print("   make clean          # Clean all build artifacts")
    
    print("\n📚 Key Files:")
    print("   src/business_logic/  # Pure C business logic (for CFFI)")
    print("   src/hardware/        # Hardware-dependent code (for HIL)")
    print("   test/unit/          # pytest unit tests")
    print("   build_cffi.py        # CFFI build script")
    print("   Makefile             # Build automation")

def main():
    """Main setup process."""
    parser = argparse.ArgumentParser(description="Set up development environment")
    parser.add_argument("--full", action="store_true", 
                       help="Full setup including PlatformIO checks")
    parser.add_argument("--test-only", action="store_true",
                       help="Set up testing environment only")
    args = parser.parse_args()
    
    print("🚀 Multi-Sonicator I/O Controller Development Setup")
    print("=" * 60)
    
    success = True
    
    # Core requirements
    if not check_python_version():
        return 1
    
    if not check_gcc_available():
        success = False
    
    # Python dependencies
    if not install_python_dependencies():
        success = False
    
    # Code structure
    if not setup_code_structure():
        success = False
    
    # CFFI build (may fail if C code not implemented)
    build_cffi_library()  # Don't fail on this
    
    # Test validation
    if not run_initial_tests():
        success = False
    
    # Optional checks
    if args.full and not args.test_only:
        check_platformio()
    
    # VS Code integration
    create_vscode_settings()
    
    if success:
        print_next_steps()
        return 0
    else:
        print("\n❌ Setup completed with some issues")
        print("   Review the errors above and resolve before proceeding")
        return 1

if __name__ == "__main__":
    sys.exit(main())
