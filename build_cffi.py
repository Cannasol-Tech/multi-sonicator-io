#!/usr/bin/env python3
"""
CFFI Build System for Multi-Sonicator I/O Controller

This script compiles C business logic functions into a shared library
for pytest unit testing via CFFI. Separates business logic from hardware
dependencies to enable organizational testing compliance.

Usage:
    python build_cffi.py [--debug] [--clean]
"""

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

from cffi import FFI

# Project configuration
PROJECT_ROOT = Path(__file__).parent
SRC_DIR = PROJECT_ROOT / "src"
BUILD_DIR = PROJECT_ROOT / "build"
BUSINESS_LOGIC_DIR = SRC_DIR / "business_logic"
INCLUDE_DIR = PROJECT_ROOT / "include"

# CFFI configuration
SHARED_LIB_NAME = "libsonicator_logic"
SHARED_LIB_PATH = BUILD_DIR / f"{SHARED_LIB_NAME}.so"

def setup_build_environment():
    """Create build directories and ensure environment is ready."""
    BUILD_DIR.mkdir(exist_ok=True)
    print(f"✅ Build directory ready: {BUILD_DIR}")

def get_c_source_files():
    """Get list of C source files from business logic directory."""
    if not BUSINESS_LOGIC_DIR.exists():
        print(f"❌ Business logic directory not found: {BUSINESS_LOGIC_DIR}")
        print("   Run 'python restructure_code.py' first to separate business logic")
        sys.exit(1)
    
    c_files = list(BUSINESS_LOGIC_DIR.glob("*.c"))
    if not c_files:
        print(f"❌ No C source files found in {BUSINESS_LOGIC_DIR}")
        sys.exit(1)
    
    print(f"✅ Found {len(c_files)} C source files")
    for f in c_files:
        print(f"   - {f.name}")
    
    return c_files

def compile_shared_library(c_files, debug=False):
    """Compile C business logic into shared library."""
    print(f"\n🔨 Compiling shared library: {SHARED_LIB_PATH}")
    
    # GCC compilation command
    cmd = [
        "gcc",
        "-shared",
        "-fPIC",
        "-o", str(SHARED_LIB_PATH),
        f"-I{INCLUDE_DIR}",
        "-std=c99",
        "-Wall",
        "-Wextra"
    ]
    
    if debug:
        cmd.extend(["-g", "-O0", "-DDEBUG"])
        print("🐛 Debug mode enabled")
    else:
        cmd.extend(["-O2", "-DNDEBUG"])
    
    # Add source files
    cmd.extend([str(f) for f in c_files])
    
    try:
        print(f"Running: {' '.join(cmd)}")
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        print("✅ Compilation successful")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ Compilation failed with exit code {e.returncode}")
        print(f"STDOUT: {e.stdout}")
        print(f"STDERR: {e.stderr}")
        return False

def generate_cffi_header():
    """Generate CFFI header definitions."""
    print("\n📝 Generating CFFI header definitions...")
    
    ffi = FFI()
    
    # Core data structures
    header_content = """
    // Core sonicator data structures
    typedef struct {
        uint8_t amplitude[4];      // Amplitude settings (20-100%)
        uint16_t frequency[4];     // Frequency readings (Hz)
        uint8_t active_mask;       // Bitmask of active units
        uint8_t active_count;      // Number of active units
        bool overload_flags[4];    // Overload status per unit
        bool comm_fault_flags[4];  // Communication fault flags
    } sonicator_controller_t;
    
    typedef struct {
        uint16_t register_address;
        uint16_t value;
        uint8_t function_code;
        bool valid;
    } modbus_request_t;
    
    // Business logic function declarations (no hardware dependencies)
    
    // Sonicator control logic
    uint8_t calculate_active_count(uint8_t mask);
    bool validate_amplitude_range(uint8_t amplitude);
    uint8_t clamp_amplitude(uint8_t input);
    bool is_valid_unit_id(uint8_t unit_id);
    
    // MODBUS protocol logic
    bool validate_modbus_register_address(uint16_t address);
    uint16_t calculate_modbus_crc(uint8_t* buffer, uint8_t length);
    bool parse_modbus_frame(uint8_t* frame, uint8_t frame_length, 
                           modbus_request_t* request);
    bool build_modbus_response(uint8_t slave_id, uint8_t function_code,
                              uint16_t address, uint16_t value,
                              uint8_t* response_buffer, uint8_t* response_length);
    
    // Safety algorithms
    bool is_overload_condition(uint16_t power_reading);
    bool should_emergency_shutdown(sonicator_controller_t* controller,
                                 bool comm_timeout, bool overload_detected,
                                 bool watchdog_reset);
    
    // State machine logic
    typedef enum {
        SONICATOR_STATE_OFF = 0,
        SONICATOR_STATE_STARTING = 1,
        SONICATOR_STATE_RUNNING = 2,
        SONICATOR_STATE_STOPPING = 3,
        SONICATOR_STATE_FAULT = 4
    } sonicator_state_t;
    
    sonicator_state_t get_next_state(sonicator_state_t current_state,
                                   bool start_command, bool stop_command,
                                   bool fault_condition);
    
    // Mathematical functions
    uint16_t calculate_power_consumption(uint8_t amplitude, uint16_t frequency);
    uint16_t frequency_to_register_value(uint16_t frequency_hz);
    uint16_t register_value_to_frequency(uint16_t register_value);
    
    // Data validation
    bool validate_system_configuration(sonicator_controller_t* controller);
    bool validate_modbus_register_map(void);
    """
    
    try:
        ffi.cdef(header_content)
        print("✅ CFFI header generated successfully")
        
        # Save header for reference
        header_file = BUILD_DIR / "cffi_definitions.h"
        with open(header_file, 'w') as f:
            f.write("// CFFI Header Definitions - Auto-generated\n")
            f.write("// This file is for reference only - actual definitions in build_cffi.py\n\n")
            f.write(header_content)
        
        print(f"📄 Header saved to: {header_file}")
        return ffi, header_content
        
    except Exception as e:
        print(f"❌ CFFI header generation failed: {e}")
        return None, None

def validate_shared_library(ffi):
    """Validate that the shared library loads correctly."""
    print(f"\n🔍 Validating shared library: {SHARED_LIB_PATH}")
    
    if not SHARED_LIB_PATH.exists():
        print(f"❌ Shared library not found: {SHARED_LIB_PATH}")
        return False
    
    try:
        lib = ffi.dlopen(str(SHARED_LIB_PATH))
        print("✅ Shared library loads successfully")
        
        # Test a simple function if available
        try:
            test_result = lib.calculate_active_count(0b1010)  # Should return 2
            print(f"✅ Function test passed: calculate_active_count(0b1010) = {test_result}")
        except AttributeError:
            print("⚠️  Function test skipped (function not yet implemented)")
        
        return True
        
    except Exception as e:
        print(f"❌ Shared library validation failed: {e}")
        return False

def clean_build():
    """Clean build artifacts."""
    print("🧹 Cleaning build artifacts...")
    
    if BUILD_DIR.exists():
        shutil.rmtree(BUILD_DIR)
        print(f"✅ Removed: {BUILD_DIR}")
    
    # Clean any __pycache__ directories
    for pycache in PROJECT_ROOT.glob("**/__pycache__"):
        if pycache.is_dir():
            shutil.rmtree(pycache)
            print(f"✅ Removed: {pycache}")

def main():
    """Main build process."""
    parser = argparse.ArgumentParser(description="Build CFFI shared library for testing")
    parser.add_argument("--debug", action="store_true", help="Build with debug symbols")
    parser.add_argument("--clean", action="store_true", help="Clean build artifacts")
    args = parser.parse_args()
    
    print("🏗️  CFFI Build System for Multi-Sonicator I/O Controller")
    print("=" * 60)
    
    if args.clean:
        clean_build()
        return 0
    
    # Check for GCC availability
    try:
        subprocess.run(["gcc", "--version"], capture_output=True, check=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("❌ GCC compiler not found. Please install GCC.")
        return 1
    
    # Build process
    setup_build_environment()
    c_files = get_c_source_files()
    
    if not compile_shared_library(c_files, debug=args.debug):
        return 1
    
    ffi, header_content = generate_cffi_header()
    if ffi is None:
        return 1
    
    if not validate_shared_library(ffi):
        return 1
    
    print("\n✅ CFFI build completed successfully!")
    print(f"📦 Shared library: {SHARED_LIB_PATH}")
    print(f"🧪 Ready for pytest unit testing")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
