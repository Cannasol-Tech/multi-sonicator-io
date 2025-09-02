#!/usr/bin/env python3
"""
Code Restructuring Script for CFFI Compliance

This script restructures the existing C codebase to separate business logic
from hardware-dependent code, enabling pytest unit testing via CFFI while
maintaining HIL testing capabilities.

Usage:
    python restructure_code.py [--dry-run] [--force]
"""

import argparse
import os
import shutil
import sys
from pathlib import Path

# Project structure
PROJECT_ROOT = Path(__file__).parent
SRC_DIR = PROJECT_ROOT / "src"
INCLUDE_DIR = PROJECT_ROOT / "include"

# New directory structure
BUSINESS_LOGIC_DIR = SRC_DIR / "business_logic"
HARDWARE_DIR = SRC_DIR / "hardware"
TEST_DIR = PROJECT_ROOT / "test" / "unit"

def create_directory_structure():
    """Create the new directory structure for separated code."""
    print("📁 Creating new directory structure...")
    
    directories = [
        BUSINESS_LOGIC_DIR,
        HARDWARE_DIR,
        TEST_DIR,
        INCLUDE_DIR / "business_logic",
        INCLUDE_DIR / "hardware"
    ]
    
    for dir_path in directories:
        dir_path.mkdir(parents=True, exist_ok=True)
        print(f"✅ Created: {dir_path}")

def create_business_logic_modules():
    """Create business logic C modules (no hardware dependencies)."""
    print("\n🧠 Creating business logic modules...")
    
    # 1. MODBUS Protocol Logic
    modbus_logic_c = BUSINESS_LOGIC_DIR / "modbus_protocol.c"
    modbus_logic_h = INCLUDE_DIR / "business_logic" / "modbus_protocol.h"
    
    with open(modbus_logic_h, 'w') as f:
        f.write("""/**
 * @file modbus_protocol.h
 * @brief MODBUS RTU protocol business logic (hardware-independent)
 * @author Cannasol Technologies
 * @date 2025-09-02
 * 
 * Pure business logic for MODBUS protocol handling without hardware dependencies.
 * Suitable for CFFI unit testing via pytest.
 */

#ifndef MODBUS_PROTOCOL_H
#define MODBUS_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

// MODBUS register map constants
#define MODBUS_SLAVE_ID 2
#define MODBUS_AMPLITUDE_BASE_REGISTER 40001
#define MODBUS_START_STOP_BASE_REGISTER 40005
#define MODBUS_POWER_BASE_REGISTER 40009
#define MODBUS_FREQUENCY_BASE_REGISTER 40013
#define MODBUS_STATUS_BASE_REGISTER 40017

// MODBUS function codes
#define MODBUS_FC_READ_HOLDING_REGISTERS 0x03
#define MODBUS_FC_WRITE_SINGLE_REGISTER 0x06
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS 0x10

// Data structures
typedef struct {
    uint16_t register_address;
    uint16_t value;
    uint8_t function_code;
    bool valid;
} modbus_request_t;

// Business logic functions (no hardware dependencies)
bool validate_modbus_register_address(uint16_t address);
uint16_t calculate_modbus_crc(uint8_t* buffer, uint8_t length);
bool parse_modbus_frame(uint8_t* frame, uint8_t frame_length, modbus_request_t* request);
bool build_modbus_response(uint8_t slave_id, uint8_t function_code,
                          uint16_t address, uint16_t value,
                          uint8_t* response_buffer, uint8_t* response_length);
bool validate_modbus_register_map(void);

#endif // MODBUS_PROTOCOL_H
""")
    
    with open(modbus_logic_c, 'w') as f:
        f.write("""/**
 * @file modbus_protocol.c
 * @brief MODBUS RTU protocol business logic implementation
 */

#include "business_logic/modbus_protocol.h"

// CRC-16 lookup table for MODBUS
static const uint16_t crc16_table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    // ... (truncated for brevity - full table needed in real implementation)
    0x6E17, 0xAED6, 0xAF96, 0x6F57, 0xADB5, 0x6D74, 0x6C34, 0xACF5
};

bool validate_modbus_register_address(uint16_t address) {
    // Amplitude control registers (40001-40004)
    if (address >= MODBUS_AMPLITUDE_BASE_REGISTER && 
        address <= MODBUS_AMPLITUDE_BASE_REGISTER + 3) {
        return true;
    }
    
    // Start/Stop control registers (40005-40008)
    if (address >= MODBUS_START_STOP_BASE_REGISTER && 
        address <= MODBUS_START_STOP_BASE_REGISTER + 3) {
        return true;
    }
    
    // Power monitoring registers (40009-40012)
    if (address >= MODBUS_POWER_BASE_REGISTER && 
        address <= MODBUS_POWER_BASE_REGISTER + 3) {
        return true;
    }
    
    // Frequency monitoring registers (40013-40016)
    if (address >= MODBUS_FREQUENCY_BASE_REGISTER && 
        address <= MODBUS_FREQUENCY_BASE_REGISTER + 3) {
        return true;
    }
    
    // Status registers (40017-40020)
    if (address >= MODBUS_STATUS_BASE_REGISTER && 
        address <= MODBUS_STATUS_BASE_REGISTER + 3) {
        return true;
    }
    
    return false;
}

uint16_t calculate_modbus_crc(uint8_t* buffer, uint8_t length) {
    uint16_t crc = 0xFFFF;
    
    for (uint8_t i = 0; i < length; i++) {
        uint8_t table_index = (crc ^ buffer[i]) & 0xFF;
        crc = (crc >> 8) ^ crc16_table[table_index];
    }
    
    return crc;
}

bool parse_modbus_frame(uint8_t* frame, uint8_t frame_length, modbus_request_t* request) {
    if (!frame || !request || frame_length < 8) {
        return false;
    }
    
    // Extract slave ID (should match our slave ID)
    uint8_t slave_id = frame[0];
    if (slave_id != MODBUS_SLAVE_ID) {
        return false;
    }
    
    // Extract function code
    request->function_code = frame[1];
    
    // Extract register address (big-endian)
    request->register_address = (frame[2] << 8) | frame[3];
    
    // Extract value (big-endian) 
    request->value = (frame[4] << 8) | frame[5];
    
    // Validate CRC
    uint16_t received_crc = (frame[frame_length - 1] << 8) | frame[frame_length - 2];
    uint16_t calculated_crc = calculate_modbus_crc(frame, frame_length - 2);
    
    if (received_crc != calculated_crc) {
        return false;
    }
    
    // Validate register address
    request->valid = validate_modbus_register_address(request->register_address);
    
    return true;
}

bool build_modbus_response(uint8_t slave_id, uint8_t function_code,
                          uint16_t address, uint16_t value,
                          uint8_t* response_buffer, uint8_t* response_length) {
    if (!response_buffer || !response_length) {
        return false;
    }
    
    // Build response frame
    response_buffer[0] = slave_id;
    response_buffer[1] = function_code;
    response_buffer[2] = (address >> 8) & 0xFF;
    response_buffer[3] = address & 0xFF;
    response_buffer[4] = (value >> 8) & 0xFF;
    response_buffer[5] = value & 0xFF;
    
    // Calculate and append CRC
    uint16_t crc = calculate_modbus_crc(response_buffer, 6);
    response_buffer[6] = crc & 0xFF;
    response_buffer[7] = (crc >> 8) & 0xFF;
    
    *response_length = 8;
    return true;
}

bool validate_modbus_register_map(void) {
    // Validate that our register map doesn't have overlaps
    uint16_t registers[] = {
        MODBUS_AMPLITUDE_BASE_REGISTER,
        MODBUS_START_STOP_BASE_REGISTER,
        MODBUS_POWER_BASE_REGISTER,
        MODBUS_FREQUENCY_BASE_REGISTER,
        MODBUS_STATUS_BASE_REGISTER
    };
    
    // Each register block has 4 registers (one per sonicator unit)
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (registers[i] + 3 >= registers[j]) {
                return false; // Overlap detected
            }
        }
    }
    
    return true;
}
""")
    
    print(f"✅ Created: {modbus_logic_h}")
    print(f"✅ Created: {modbus_logic_c}")
    
    # 2. Sonicator Controller Logic
    controller_logic_c = BUSINESS_LOGIC_DIR / "sonicator_controller.c"
    controller_logic_h = INCLUDE_DIR / "business_logic" / "sonicator_controller.h"
    
    with open(controller_logic_h, 'w') as f:
        f.write("""/**
 * @file sonicator_controller.h
 * @brief Sonicator control business logic (hardware-independent)
 * @author Cannasol Technologies
 * @date 2025-09-02
 * 
 * Pure business logic for sonicator control without hardware dependencies.
 * Suitable for CFFI unit testing via pytest.
 */

#ifndef SONICATOR_CONTROLLER_H
#define SONICATOR_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_SONICATOR_UNITS 4
#define MIN_AMPLITUDE_PERCENT 20
#define MAX_AMPLITUDE_PERCENT 100

// Data structures
typedef struct {
    uint8_t amplitude[MAX_SONICATOR_UNITS];      // Amplitude settings (20-100%)
    uint16_t frequency[MAX_SONICATOR_UNITS];     // Frequency readings (Hz)
    uint8_t active_mask;                         // Bitmask of active units
    uint8_t active_count;                        // Number of active units
    bool overload_flags[MAX_SONICATOR_UNITS];    // Overload status per unit
    bool comm_fault_flags[MAX_SONICATOR_UNITS];  // Communication fault flags
} sonicator_controller_t;

typedef enum {
    SONICATOR_STATE_OFF = 0,
    SONICATOR_STATE_STARTING = 1,
    SONICATOR_STATE_RUNNING = 2,
    SONICATOR_STATE_STOPPING = 3,
    SONICATOR_STATE_FAULT = 4
} sonicator_state_t;

// Business logic functions (no hardware dependencies)
uint8_t calculate_active_count(uint8_t mask);
bool validate_amplitude_range(uint8_t amplitude);
uint8_t clamp_amplitude(uint8_t input);
bool is_valid_unit_id(uint8_t unit_id);
sonicator_state_t get_next_state(sonicator_state_t current_state,
                               bool start_command, bool stop_command,
                               bool fault_condition);
bool validate_system_configuration(sonicator_controller_t* controller);

// Mathematical functions
uint16_t calculate_power_consumption(uint8_t amplitude, uint16_t frequency);
uint16_t frequency_to_register_value(uint16_t frequency_hz);
uint16_t register_value_to_frequency(uint16_t register_value);

#endif // SONICATOR_CONTROLLER_H
""")
    
    with open(controller_logic_c, 'w') as f:
        f.write("""/**
 * @file sonicator_controller.c
 * @brief Sonicator control business logic implementation
 */

#include "business_logic/sonicator_controller.h"

uint8_t calculate_active_count(uint8_t mask) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_SONICATOR_UNITS; i++) {
        if (mask & (1 << i)) {
            count++;
        }
    }
    return count;
}

bool validate_amplitude_range(uint8_t amplitude) {
    return (amplitude >= MIN_AMPLITUDE_PERCENT && amplitude <= MAX_AMPLITUDE_PERCENT);
}

uint8_t clamp_amplitude(uint8_t input) {
    if (input < MIN_AMPLITUDE_PERCENT) {
        return MIN_AMPLITUDE_PERCENT;
    }
    if (input > MAX_AMPLITUDE_PERCENT) {
        return MAX_AMPLITUDE_PERCENT;
    }
    return input;
}

bool is_valid_unit_id(uint8_t unit_id) {
    return (unit_id < MAX_SONICATOR_UNITS);
}

sonicator_state_t get_next_state(sonicator_state_t current_state,
                               bool start_command, bool stop_command,
                               bool fault_condition) {
    // Fault condition always takes priority
    if (fault_condition) {
        return SONICATOR_STATE_FAULT;
    }
    
    switch (current_state) {
        case SONICATOR_STATE_OFF:
            if (start_command) {
                return SONICATOR_STATE_STARTING;
            }
            break;
            
        case SONICATOR_STATE_STARTING:
            if (stop_command) {
                return SONICATOR_STATE_STOPPING;
            }
            // Assume transition to running after startup sequence
            return SONICATOR_STATE_RUNNING;
            
        case SONICATOR_STATE_RUNNING:
            if (stop_command) {
                return SONICATOR_STATE_STOPPING;
            }
            break;
            
        case SONICATOR_STATE_STOPPING:
            // Transition to off after stop sequence
            return SONICATOR_STATE_OFF;
            
        case SONICATOR_STATE_FAULT:
            // Only reset command can clear fault (not implemented here)
            break;
    }
    
    return current_state; // No state change
}

bool validate_system_configuration(sonicator_controller_t* controller) {
    if (!controller) {
        return false;
    }
    
    // Validate amplitude settings
    for (uint8_t i = 0; i < MAX_SONICATOR_UNITS; i++) {
        if (!validate_amplitude_range(controller->amplitude[i])) {
            return false;
        }
    }
    
    // Validate active count matches active mask
    uint8_t calculated_count = calculate_active_count(controller->active_mask);
    if (calculated_count != controller->active_count) {
        return false;
    }
    
    return true;
}

uint16_t calculate_power_consumption(uint8_t amplitude, uint16_t frequency) {
    // Simplified power calculation: Power = k * amplitude^2 * frequency
    // where k is a calibration constant
    const uint16_t k = 10; // Calibration constant (adjust based on real measurements)
    
    uint32_t power = (uint32_t)k * amplitude * amplitude * frequency / 1000;
    
    // Clamp to uint16_t range
    if (power > 65535) {
        return 65535;
    }
    
    return (uint16_t)power;
}

uint16_t frequency_to_register_value(uint16_t frequency_hz) {
    // Convert frequency in Hz to MODBUS register value
    // Assuming direct mapping for now (could be scaled)
    return frequency_hz;
}

uint16_t register_value_to_frequency(uint16_t register_value) {
    // Convert MODBUS register value to frequency in Hz
    // Assuming direct mapping for now (could be scaled)
    return register_value;
}
""")
    
    print(f"✅ Created: {controller_logic_h}")
    print(f"✅ Created: {controller_logic_c}")
    
    # 3. Safety Algorithms
    safety_logic_c = BUSINESS_LOGIC_DIR / "safety_algorithms.c"
    safety_logic_h = INCLUDE_DIR / "business_logic" / "safety_algorithms.h"
    
    with open(safety_logic_h, 'w') as f:
        f.write("""/**
 * @file safety_algorithms.h
 * @brief Safety algorithm business logic (hardware-independent)
 * @author Cannasol Technologies
 * @date 2025-09-02
 * 
 * Pure safety algorithm logic without hardware dependencies.
 * Suitable for CFFI unit testing via pytest.
 */

#ifndef SAFETY_ALGORITHMS_H
#define SAFETY_ALGORITHMS_H

#include <stdint.h>
#include <stdbool.h>
#include "business_logic/sonicator_controller.h"

// Safety thresholds
#define OVERLOAD_POWER_THRESHOLD_WATTS 2000
#define MAX_SAFE_FREQUENCY_HZ 50000
#define MIN_SAFE_FREQUENCY_HZ 10000

// Business logic functions (no hardware dependencies)
bool is_overload_condition(uint16_t power_reading);
bool should_emergency_shutdown(sonicator_controller_t* controller,
                             bool comm_timeout, bool overload_detected,
                             bool watchdog_reset);
bool is_safe_frequency(uint16_t frequency);
bool validate_safety_limits(sonicator_controller_t* controller);

#endif // SAFETY_ALGORITHMS_H
""")
    
    with open(safety_logic_c, 'w') as f:
        f.write("""/**
 * @file safety_algorithms.c
 * @brief Safety algorithm business logic implementation
 */

#include "business_logic/safety_algorithms.h"

bool is_overload_condition(uint16_t power_reading) {
    return power_reading > OVERLOAD_POWER_THRESHOLD_WATTS;
}

bool should_emergency_shutdown(sonicator_controller_t* controller,
                             bool comm_timeout, bool overload_detected,
                             bool watchdog_reset) {
    if (!controller) {
        return true; // Safe default
    }
    
    // Emergency shutdown conditions
    if (comm_timeout || overload_detected || watchdog_reset) {
        return true;
    }
    
    // Check individual unit overload flags
    for (uint8_t i = 0; i < MAX_SONICATOR_UNITS; i++) {
        if (controller->overload_flags[i]) {
            return true;
        }
    }
    
    // Check for unsafe frequency conditions
    for (uint8_t i = 0; i < MAX_SONICATOR_UNITS; i++) {
        if (!is_safe_frequency(controller->frequency[i])) {
            return true;
        }
    }
    
    return false;
}

bool is_safe_frequency(uint16_t frequency) {
    if (frequency == 0) {
        return true; // Unit is off
    }
    
    return (frequency >= MIN_SAFE_FREQUENCY_HZ && frequency <= MAX_SAFE_FREQUENCY_HZ);
}

bool validate_safety_limits(sonicator_controller_t* controller) {
    if (!controller) {
        return false;
    }
    
    // Validate all amplitude settings are within safe range
    for (uint8_t i = 0; i < MAX_SONICATOR_UNITS; i++) {
        if (!validate_amplitude_range(controller->amplitude[i])) {
            return false;
        }
    }
    
    // Validate all frequencies are within safe range
    for (uint8_t i = 0; i < MAX_SONICATOR_UNITS; i++) {
        if (!is_safe_frequency(controller->frequency[i])) {
            return false;
        }
    }
    
    return true;
}
""")
    
    print(f"✅ Created: {safety_logic_h}")
    print(f"✅ Created: {safety_logic_c}")

def create_hardware_wrapper_interface():
    """Create hardware interface wrapper for HIL testing."""
    print("\n🔧 Creating hardware interface modules...")
    
    hardware_interface_h = INCLUDE_DIR / "hardware" / "hardware_interface.h"
    hardware_interface_c = HARDWARE_DIR / "hardware_interface.c"
    
    with open(hardware_interface_h, 'w') as f:
        f.write("""/**
 * @file hardware_interface.h
 * @brief Hardware abstraction layer interface
 * @author Cannasol Technologies
 * @date 2025-09-02
 * 
 * Hardware-dependent functions for HIL testing.
 * This layer interfaces with actual ATmega32A hardware.
 */

#ifndef HARDWARE_INTERFACE_H
#define HARDWARE_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>
#include "business_logic/sonicator_controller.h"

// Hardware initialization
bool hardware_init(void);
bool hardware_reset(void);

// GPIO control
bool set_sonicator_start_pin(uint8_t unit_id, bool state);
bool set_sonicator_stop_pin(uint8_t unit_id, bool state);
bool set_amplitude_pwm(uint8_t unit_id, uint8_t amplitude_percent);

// ADC readings
uint16_t read_power_consumption(uint8_t unit_id);
uint16_t read_frequency(uint8_t unit_id);
bool read_overload_flag(uint8_t unit_id);

// UART/Serial communication
bool uart_init(uint32_t baud_rate);
bool uart_send_data(uint8_t* data, uint8_t length);
uint8_t uart_receive_data(uint8_t* buffer, uint8_t max_length);

// System monitoring
bool is_watchdog_reset(void);
uint16_t get_system_voltage(void);
uint8_t get_system_temperature(void);

#endif // HARDWARE_INTERFACE_H
""")
    
    with open(hardware_interface_c, 'w') as f:
        f.write("""/**
 * @file hardware_interface.c
 * @brief Hardware abstraction layer implementation
 * 
 * This file will contain the actual ATmega32A hardware interface code.
 * For now, it provides stubs that can be implemented during HIL testing.
 */

#include "hardware/hardware_interface.h"
#include <Arduino.h>

// TODO: Include actual pin definitions from config.h
// #include "config.h"

bool hardware_init(void) {
    // TODO: Initialize GPIO pins, ADC, UART, timers
    // This will be implemented with actual ATmega32A register programming
    return true;
}

bool hardware_reset(void) {
    // TODO: Perform hardware reset sequence
    return true;
}

bool set_sonicator_start_pin(uint8_t unit_id, bool state) {
    // TODO: Set appropriate GPIO pin for sonicator start signal
    // Will use pin definitions from config.h
    return true;
}

bool set_sonicator_stop_pin(uint8_t unit_id, bool state) {
    // TODO: Set appropriate GPIO pin for sonicator stop signal
    return true;
}

bool set_amplitude_pwm(uint8_t unit_id, uint8_t amplitude_percent) {
    // TODO: Generate PWM signal for amplitude control
    // Will use timer/PWM registers on ATmega32A
    return true;
}

uint16_t read_power_consumption(uint8_t unit_id) {
    // TODO: Read ADC channel for power measurement
    // Will return actual ADC reading converted to watts
    return 1500; // Stub value
}

uint16_t read_frequency(uint8_t unit_id) {
    // TODO: Read frequency from sonicator feedback
    // May use input capture or frequency counter
    return 20000; // Stub value (20kHz)
}

bool read_overload_flag(uint8_t unit_id) {
    // TODO: Read overload detection pin
    return false; // Stub value
}

bool uart_init(uint32_t baud_rate) {
    // TODO: Initialize UART with specified baud rate
    // Will configure UART0 registers on ATmega32A
    return true;
}

bool uart_send_data(uint8_t* data, uint8_t length) {
    // TODO: Send data via UART
    return true;
}

uint8_t uart_receive_data(uint8_t* buffer, uint8_t max_length) {
    // TODO: Receive data via UART
    return 0; // Stub: no data received
}

bool is_watchdog_reset(void) {
    // TODO: Check watchdog reset flag
    return false;
}

uint16_t get_system_voltage(void) {
    // TODO: Read system voltage via ADC
    return 5000; // Stub: 5.0V in mV
}

uint8_t get_system_temperature(void) {
    // TODO: Read system temperature
    return 25; // Stub: 25°C
}
""")
    
    print(f"✅ Created: {hardware_interface_h}")
    print(f"✅ Created: {hardware_interface_c}")

def create_pytest_integration_files():
    """Create pytest integration and configuration files."""
    print("\n🧪 Creating pytest integration files...")
    
    # Create pytest configuration
    pytest_ini = PROJECT_ROOT / "pytest.ini"
    with open(pytest_ini, 'w') as f:
        f.write("""[tool:pytest]
testpaths = test/unit
python_files = test_*.py
python_classes = Test*
python_functions = test_*
addopts = 
    --verbose
    --tb=short
    --strict-markers
    --disable-warnings
    --cov=src/business_logic
    --cov-report=term-missing
    --cov-report=json:coverage.json
    --cov-report=html:htmlcov
    --cov-fail-under=90
    --junit-xml=unit-test-results.xml

markers =
    unit: Unit tests for business logic
    integration: Integration tests
    slow: Slow running tests
    requires_hardware: Tests that require physical hardware
""")
    
    # Create CFFI helper module
    cffi_helper = TEST_DIR / "cffi_helper.py"
    with open(cffi_helper, 'w') as f:
        f.write('''"""
CFFI Helper Module for pytest Unit Tests

This module provides CFFI interface setup for testing C business logic
functions in Python pytest without hardware dependencies.
"""

import os
import sys
from pathlib import Path
from cffi import FFI

# Add project root to path
PROJECT_ROOT = Path(__file__).parent.parent.parent
sys.path.insert(0, str(PROJECT_ROOT))

# Build paths
BUILD_DIR = PROJECT_ROOT / "build"
SHARED_LIB_PATH = BUILD_DIR / "libsonicator_logic.so"

def get_cffi_interface():
    """Get CFFI interface for sonicator business logic."""
    ffi = FFI()
    
    # Define C structures and functions for CFFI
    ffi.cdef("""
    // Core data structures
    typedef struct {
        uint8_t amplitude[4];      
        uint16_t frequency[4];     
        uint8_t active_mask;       
        uint8_t active_count;      
        bool overload_flags[4];    
        bool comm_fault_flags[4];  
    } sonicator_controller_t;
    
    typedef struct {
        uint16_t register_address;
        uint16_t value;
        uint8_t function_code;
        bool valid;
    } modbus_request_t;
    
    typedef enum {
        SONICATOR_STATE_OFF = 0,
        SONICATOR_STATE_STARTING = 1,
        SONICATOR_STATE_RUNNING = 2,
        SONICATOR_STATE_STOPPING = 3,
        SONICATOR_STATE_FAULT = 4
    } sonicator_state_t;
    
    // Sonicator control functions
    uint8_t calculate_active_count(uint8_t mask);
    bool validate_amplitude_range(uint8_t amplitude);
    uint8_t clamp_amplitude(uint8_t input);
    bool is_valid_unit_id(uint8_t unit_id);
    sonicator_state_t get_next_state(sonicator_state_t current_state,
                                   bool start_command, bool stop_command,
                                   bool fault_condition);
    bool validate_system_configuration(sonicator_controller_t* controller);
    
    // MODBUS protocol functions
    bool validate_modbus_register_address(uint16_t address);
    uint16_t calculate_modbus_crc(uint8_t* buffer, uint8_t length);
    bool parse_modbus_frame(uint8_t* frame, uint8_t frame_length, 
                           modbus_request_t* request);
    bool build_modbus_response(uint8_t slave_id, uint8_t function_code,
                              uint16_t address, uint16_t value,
                              uint8_t* response_buffer, uint8_t* response_length);
    bool validate_modbus_register_map(void);
    
    // Safety algorithm functions
    bool is_overload_condition(uint16_t power_reading);
    bool should_emergency_shutdown(sonicator_controller_t* controller,
                                 bool comm_timeout, bool overload_detected,
                                 bool watchdog_reset);
    bool is_safe_frequency(uint16_t frequency);
    bool validate_safety_limits(sonicator_controller_t* controller);
    
    // Mathematical functions
    uint16_t calculate_power_consumption(uint8_t amplitude, uint16_t frequency);
    uint16_t frequency_to_register_value(uint16_t frequency_hz);
    uint16_t register_value_to_frequency(uint16_t register_value);
    """)
    
    # Load shared library
    if not SHARED_LIB_PATH.exists():
        raise FileNotFoundError(
            f"Shared library not found: {SHARED_LIB_PATH}\\n"
            f"Run 'python build_cffi.py' first to build the library."
        )
    
    lib = ffi.dlopen(str(SHARED_LIB_PATH))
    
    return ffi, lib

def ensure_build_ready():
    """Ensure the CFFI build is ready for testing."""
    if not SHARED_LIB_PATH.exists():
        print(f"Building CFFI shared library...")
        import subprocess
        result = subprocess.run([
            sys.executable, str(PROJECT_ROOT / "build_cffi.py")
        ], capture_output=True, text=True)
        
        if result.returncode != 0:
            raise RuntimeError(f"CFFI build failed: {result.stderr}")
    
    return True
''')
    
    # Create sample unit test
    sample_test = TEST_DIR / "test_sonicator_controller.py"
    with open(sample_test, 'w') as f:
        f.write('''"""
Unit tests for sonicator controller business logic using CFFI.

These tests validate the pure business logic without hardware dependencies,
ensuring organizational compliance with pytest testing standards.
"""

import pytest
from cffi_helper import get_cffi_interface, ensure_build_ready

@pytest.fixture(scope="module")
def cffi_lib():
    """Fixture to provide CFFI library interface."""
    ensure_build_ready()
    ffi, lib = get_cffi_interface()
    return ffi, lib

class TestSonicatorBusinessLogic:
    """Unit tests for pure business logic - no hardware involved."""
    
    def test_active_count_calculation(self, cffi_lib):
        """Test active sonicator count calculation (FR9)."""
        ffi, lib = cffi_lib
        
        assert lib.calculate_active_count(0b0000) == 0  # No units
        assert lib.calculate_active_count(0b0001) == 1  # Unit 1 only
        assert lib.calculate_active_count(0b1010) == 2  # Units 2,4
        assert lib.calculate_active_count(0b1111) == 4  # All units
    
    def test_amplitude_clamping_logic(self, cffi_lib):
        """Test amplitude clamping business rule (FR3)."""
        ffi, lib = cffi_lib
        
        assert lib.clamp_amplitude(10) == 20   # Below minimum → clamp to 20%
        assert lib.clamp_amplitude(150) == 100 # Above maximum → clamp to 100%
        assert lib.clamp_amplitude(50) == 50   # Valid range → unchanged
        assert lib.clamp_amplitude(20) == 20   # Minimum boundary
        assert lib.clamp_amplitude(100) == 100 # Maximum boundary
    
    def test_amplitude_validation(self, cffi_lib):
        """Test amplitude range validation."""
        ffi, lib = cffi_lib
        
        assert lib.validate_amplitude_range(20) == True   # Minimum valid
        assert lib.validate_amplitude_range(100) == True  # Maximum valid
        assert lib.validate_amplitude_range(50) == True   # Mid-range valid
        assert lib.validate_amplitude_range(19) == False  # Below minimum
        assert lib.validate_amplitude_range(101) == False # Above maximum
    
    def test_unit_id_validation(self, cffi_lib):
        """Test sonicator unit ID validation."""
        ffi, lib = cffi_lib
        
        assert lib.is_valid_unit_id(0) == True   # First unit
        assert lib.is_valid_unit_id(3) == True   # Last unit (0-indexed)
        assert lib.is_valid_unit_id(4) == False  # Beyond range
        assert lib.is_valid_unit_id(255) == False # Way beyond range

class TestModbusProtocol:
    """Unit tests for MODBUS protocol logic."""
    
    def test_modbus_register_validation(self, cffi_lib):
        """Test MODBUS register address validation logic (FR8)."""
        ffi, lib = cffi_lib
        
        # Amplitude registers (40001-40004)
        assert lib.validate_modbus_register_address(40001) == True
        assert lib.validate_modbus_register_address(40004) == True
        
        # Start/Stop registers (40005-40008)  
        assert lib.validate_modbus_register_address(40005) == True
        assert lib.validate_modbus_register_address(40008) == True
        
        # Invalid addresses
        assert lib.validate_modbus_register_address(50000) == False
        assert lib.validate_modbus_register_address(30000) == False
        assert lib.validate_modbus_register_address(0) == False
    
    def test_modbus_register_map_validation(self, cffi_lib):
        """Test that MODBUS register map has no overlaps."""
        ffi, lib = cffi_lib
        
        assert lib.validate_modbus_register_map() == True

class TestSafetyAlgorithms:
    """Unit tests for safety algorithms - pure logic, no hardware."""
    
    def test_overload_detection_algorithm(self, cffi_lib):
        """Test overload detection logic with different power readings."""
        ffi, lib = cffi_lib
        
        assert lib.is_overload_condition(1500) == False  # Normal power
        assert lib.is_overload_condition(2500) == True   # Overload detected
        assert lib.is_overload_condition(0) == False     # Zero power OK
        assert lib.is_overload_condition(2000) == False  # At threshold
        assert lib.is_overload_condition(2001) == True   # Just above threshold
    
    def test_frequency_safety_validation(self, cffi_lib):
        """Test frequency safety range validation."""
        ffi, lib = cffi_lib
        
        assert lib.is_safe_frequency(0) == True      # Off state
        assert lib.is_safe_frequency(20000) == True  # Normal frequency
        assert lib.is_safe_frequency(5000) == False  # Too low
        assert lib.is_safe_frequency(60000) == False # Too high
        assert lib.is_safe_frequency(10000) == True  # Minimum safe
        assert lib.is_safe_frequency(50000) == True  # Maximum safe

class TestStateMachine:
    """Unit tests for sonicator state machine logic."""
    
    def test_state_transitions(self, cffi_lib):
        """Test state machine transitions."""
        ffi, lib = cffi_lib
        
        # OFF → STARTING on start command
        next_state = lib.get_next_state(0, True, False, False)  # OFF, start=True
        assert next_state == 1  # STARTING
        
        # RUNNING → STOPPING on stop command  
        next_state = lib.get_next_state(2, False, True, False)  # RUNNING, stop=True
        assert next_state == 3  # STOPPING
        
        # Any state → FAULT on fault condition
        next_state = lib.get_next_state(2, False, False, True)  # RUNNING, fault=True
        assert next_state == 4  # FAULT
""")
    
    print(f"✅ Created: {pytest_ini}")
    print(f"✅ Created: {cffi_helper}")
    print(f"✅ Created: {sample_test}")

def create_makefile_integration():
    """Update Makefile to include CFFI build targets."""
    print("\n📝 Creating Makefile integration...")
    
    makefile = PROJECT_ROOT / "Makefile"
    
    makefile_content = """# Multi-Sonicator I/O Controller Makefile
# Supports both firmware build and Python testing via CFFI

.PHONY: help build test test-unit test-acceptance test-hil clean cffi install-deps

# Default target
help:
	@echo "Multi-Sonicator I/O Controller Build System"
	@echo "Available targets:"
	@echo "  build          - Build firmware for ATmega32A"
	@echo "  cffi           - Build CFFI shared library for testing"
	@echo "  test           - Run all tests (unit + acceptance)"
	@echo "  test-unit      - Run pytest unit tests only"
	@echo "  test-acceptance - Run BDD acceptance tests"
	@echo "  test-hil       - Run hardware-in-the-loop tests"
	@echo "  clean          - Clean all build artifacts"
	@echo "  install-deps   - Install Python dependencies"

# Firmware build using PlatformIO
build:
	@echo "🔨 Building firmware for ATmega32A..."
	platformio run

upload:
	@echo "📤 Uploading firmware to ATmega32A..."
	platformio run --target upload

# CFFI build for pytest testing
cffi:
	@echo "🏗️  Building CFFI shared library..."
	python build_cffi.py

cffi-debug:
	@echo "🐛 Building CFFI shared library with debug symbols..."
	python build_cffi.py --debug

# Python dependency installation
install-deps:
	@echo "📦 Installing Python dependencies..."
	pip install cffi pytest pytest-cov behave

# Unit testing with pytest (organizational compliance)
test-unit: cffi
	@echo "🧪 Running pytest unit tests..."
	python -m pytest test/unit/ \\
		--cov=src/business_logic \\
		--cov-report=json:coverage.json \\
		--cov-report=html:htmlcov \\
		--cov-report=term-missing \\
		--junit-xml=unit-test-results.xml \\
		--cov-fail-under=90 \\
		--verbose

# BDD acceptance testing
test-acceptance:
	@echo "🎭 Running BDD acceptance tests..."
	@python scripts/detect_hardware.py --check-arduino || (echo "❌ Hardware required for acceptance tests" && exit 1)
	behave test/acceptance \\
		--junit \\
		--junit-directory=acceptance-results \\
		-D profile=simulation

# Hardware-in-the-loop testing  
test-hil:
	@echo "🔧 Running HIL tests on real hardware..."
	@python scripts/detect_hardware.py --check-arduino || (echo "❌ Hardware required for HIL tests" && exit 1)
	behave test/acceptance \\
		--junit \\
		--junit-directory=hil-results \\
		-D profile=hil

# Run all tests
test: test-unit test-acceptance

# Generate release artifacts
generate-release-artifacts: test
	@echo "📊 Generating release artifacts..."
	mkdir -p final/
	python scripts/release/generate_executive_report.py \\
		--acceptance-results acceptance-results/ \\
		--unit-results unit-test-results.xml \\
		--coverage coverage.json \\
		--output final/

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts..."
	python build_cffi.py --clean
	platformio run --target clean
	rm -rf .pio/
	rm -rf htmlcov/
	rm -rf .pytest_cache/
	rm -rf **/__pycache__/
	rm -f coverage.json
	rm -f unit-test-results.xml
	rm -rf acceptance-results/
	rm -rf hil-results/
	rm -rf final/

# Development environment setup
setup-dev: install-deps cffi
	@echo "🛠️  Development environment ready!"
	@echo "✅ CFFI shared library built"
	@echo "✅ Python dependencies installed"
	@echo "Run 'make test-unit' to validate setup"

# Continuous integration target
ci: setup-dev test generate-release-artifacts
	@echo "🚀 CI pipeline completed successfully!"
"""
    
    with open(makefile, 'w') as f:
        f.write(makefile_content)
    
    print(f"✅ Created: {makefile}")

def create_requirements_file():
    """Create Python requirements file for CFFI dependencies."""
    print("\n📋 Creating Python requirements file...")
    
    requirements = PROJECT_ROOT / "requirements-testing.txt"
    with open(requirements, 'w') as f:
        f.write("""# Python dependencies for CFFI-based testing
# Multi-Sonicator I/O Controller Testing Stack

# CFFI for C code integration
cffi>=1.15.0,<2.0.0

# pytest testing framework (organizational standard)
pytest>=7.0.0
pytest-cov>=4.0.0
pytest-xdist>=3.0.0

# BDD testing
behave>=1.2.6

# Utilities
pathlib2>=2.3.0;python_version<"3.4"
typing>=3.7.0;python_version<"3.5"

# Development utilities
black>=22.0.0
isort>=5.0.0
flake8>=4.0.0
""")
    
    print(f"✅ Created: {requirements}")

def main():
    """Main restructuring process."""
    parser = argparse.ArgumentParser(description="Restructure code for CFFI compliance")
    parser.add_argument("--dry-run", action="store_true", help="Show what would be done")
    parser.add_argument("--force", action="store_true", help="Overwrite existing files")
    args = parser.parse_args()
    
    print("🏗️  Code Restructuring for CFFI Testing Compliance")
    print("=" * 60)
    
    if args.dry_run:
        print("🔍 DRY RUN MODE - No files will be created")
        return 0
    
    # Check if directories already exist
    if BUSINESS_LOGIC_DIR.exists() and not args.force:
        print(f"❌ Directory {BUSINESS_LOGIC_DIR} already exists")
        print("   Use --force to overwrite or remove manually")
        return 1
    
    try:
        create_directory_structure()
        create_business_logic_modules()
        create_hardware_wrapper_interface()
        create_pytest_integration_files()
        create_makefile_integration()
        create_requirements_file()
        
        print("\n✅ Code restructuring completed successfully!")
        print("\n📋 Next steps:")
        print("1. Run 'make install-deps' to install Python dependencies")
        print("2. Run 'make cffi' to build the CFFI shared library")
        print("3. Run 'make test-unit' to validate the setup")
        print("4. Implement actual C function bodies in business_logic/ modules")
        print("5. Add more pytest test cases based on your requirements")
        
        return 0
        
    except Exception as e:
        print(f"❌ Restructuring failed: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
