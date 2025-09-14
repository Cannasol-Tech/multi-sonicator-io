#!/usr/bin/env python3
"""
Project Structure Validation Script

Validates that the Multi-Sonicator I/O Controller project structure
follows embedded development standards and BMad-Core requirements.
"""

import os
import sys
from pathlib import Path

def validate_project_structure():
    """Validate that all required directories and files exist."""
    project_root = Path(__file__).parent.parent
    
    required_dirs = [
        "src",
        "include", 
        "test/unit",
        "test/acceptance",
        "docs",
        "docs/stories",
        "docs/sop",
        "scripts",
        ".github/workflows"
    ]
    
    required_files = [
        "platformio.ini",
        "Makefile", 
        "README.md",
        "requirements-testing.txt",
        "docs/prd.md",
        "docs/architecture.md",
        "docs/sop/sw-testing-standard.md",
        "src/main.cpp",
        "include/config.h",
        "include/types.h"
    ]
    
    missing_dirs = []
    missing_files = []
    
    print("🔍 Validating project structure...")
    
    # Check required directories
    for dir_path in required_dirs:
        full_path = project_root / dir_path
        if not full_path.exists():
            missing_dirs.append(dir_path)
        else:
            print(f"✅ Directory exists: {dir_path}")
    
    # Check required files
    for file_path in required_files:
        full_path = project_root / file_path
        if not full_path.exists():
            missing_files.append(file_path)
        else:
            print(f"✅ File exists: {file_path}")
    
    # Report results
    if missing_dirs or missing_files:
        print("\n❌ Project structure validation FAILED")
        
        if missing_dirs:
            print(f"\nMissing directories ({len(missing_dirs)}):")
            for dir_path in missing_dirs:
                print(f"  - {dir_path}")
        
        if missing_files:
            print(f"\nMissing files ({len(missing_files)}):")
            for file_path in missing_files:
                print(f"  - {file_path}")
                
        return False
    else:
        print("\n✅ Project structure validation PASSED")
        return True

def validate_platformio_config():
    """Validate PlatformIO configuration for ATmega32A."""
    project_root = Path(__file__).parent.parent
    platformio_ini = project_root / "platformio.ini"
    
    if not platformio_ini.exists():
        print("❌ platformio.ini not found")
        return False
    
    print("🔍 Validating PlatformIO configuration...")
    
    required_settings = [
        "platform = atmelavr",
        "board = ATmega32", 
        "framework = arduino",
        "board_build.mcu = atmega32a",
        "board_build.f_cpu = 16000000L"
    ]
    
    content = platformio_ini.read_text()
    missing_settings = []
    
    for setting in required_settings:
        if setting not in content:
            missing_settings.append(setting)
        else:
            print(f"✅ Found: {setting}")
    
    if missing_settings:
        print(f"\n❌ PlatformIO validation FAILED")
        print(f"Missing settings ({len(missing_settings)}):")
        for setting in missing_settings:
            print(f"  - {setting}")
        return False
    else:
        print("✅ PlatformIO configuration validation PASSED")
        return True

def validate_makefile_targets():
    """Validate that required Make targets exist."""
    project_root = Path(__file__).parent.parent
    makefile = project_root / "Makefile"
    
    if not makefile.exists():
        print("❌ Makefile not found")
        return False
    
    print("🔍 Validating Makefile targets...")
    
    required_targets = [
        "test-unit",
        "test-acceptance", 
        "test-integration",
        "build",
        "clean"
    ]
    
    content = makefile.read_text()
    missing_targets = []
    
    for target in required_targets:
        if f"{target}:" not in content:
            missing_targets.append(target)
        else:
            print(f"✅ Found target: {target}")
    
    if missing_targets:
        print(f"\n❌ Makefile validation FAILED")
        print(f"Missing targets ({len(missing_targets)}):")
        for target in missing_targets:
            print(f"  - {target}")
        return False
    else:
        print("✅ Makefile targets validation PASSED")
        return True

def main():
    """Main validation function."""
    print("🚀 Multi-Sonicator I/O Controller - Project Structure Validation")
    print("=" * 60)
    
    validations = [
        validate_project_structure,
        validate_platformio_config,
        validate_makefile_targets
    ]
    
    results = []
    for validation in validations:
        results.append(validation())
        print()  # Add spacing between validations
    
    # Summary
    passed = sum(results)
    total = len(results)
    
    print("=" * 60)
    print(f"📊 VALIDATION SUMMARY: {passed}/{total} checks passed")
    
    if all(results):
        print("🎉 All validations PASSED - Project structure is compliant!")
        sys.exit(0)
    else:
        print("💥 Some validations FAILED - Please fix the issues above")
        sys.exit(1)

if __name__ == "__main__":
    main()
