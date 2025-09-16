/**
 * @file simple_register_map.c
 * @title Simplified MODBUS Register Map Implementation
 * @company Axovia AI
 * @date 2025-09-16
 * @brief Implementation of the simplified MODBUS register map using enum-based addressing and flat array storage
 * @version 1.1.0
 *
 * @details
 * This file provides the implementation for the simplified register map using an enumerated type
 * for register addresses and a flat array for storage. The implementation focuses on efficiency,
 * reliability, and ease of maintenance while providing full MODBUS RTU compatibility.
 *
 * The implementation includes:
 * - **Flat Array Storage**: Contiguous memory block for all register values
 * - **Initialization Logic**: Safe defaults and proper system startup
 * - **Access Functions**: Validated read/write operations with error handling
 * - **Memory Management**: Efficient use of embedded system resources
 *
 * @section implementation_overview Implementation Overview
 *
 * The implementation uses a flat array approach for optimal performance:
 * - **Static Array**: Fixed-size array for all register storage
 * - **Direct Access**: Array indexing using enum values
 * - **Validation Layer**: Address validation before all operations
 * - **Initialization**: One-time setup with safe defaults
 *
 * This approach provides better cache performance and simpler memory management
 * compared to complex struct-based implementations.
 *
 * @section memory_layout Memory Layout
 *
 * The register storage is organized as a contiguous array:
 *
 * @code{.c}
 * static uint16_t register_storage[MAX_REGISTERS] = {0};
 * @endcode
 *
 * - **Size**: MAX_REGISTERS (0x0420 = 1056 registers)
 * - **Type**: uint16_t for 16-bit MODBUS register compatibility
 * - **Initialization**: Zero-initialized at startup
 * - **Access**: Direct indexing using enum values
 *
 * @section initialization Initialization Process
 *
 * The initialization process sets up the register system:
 *
 * @code{.c}
 * bool simple_register_init(void) {
 *     // Clear all registers to 0
 *     memset(register_storage, 0, sizeof(register_storage));
 *     
 *     // Set system status defaults
 *     register_storage[REG_SYSTEM_STATUS] = 0x0001;
 *     
 *     // Initialize sonicator defaults
 *     // ... per-sonicator initialization
 *     
 *     registers_initialized = true;
 *     return true;
 * }
 * @endcode
 *
 * Initialization includes:
 * - **Memory Clearing**: All registers set to 0
 * - **System Defaults**: Safe system status values
 * - **Sonicator Setup**: Default values for all sonicator registers
 * - **Flag Setting**: Initialization completion flag
 *
 * @section read_write_operations Read/Write Operations
 *
 * All operations include validation and error handling:
 *
 * @code{.c}
 * uint16_t simple_register_read(simple_register_address_t address) {
 *     if (!registers_initialized) {
 *         simple_register_init();
 *     }
 *     
 *     if (!is_valid_simple_register_address(address)) {
 *         return 0;
 *     }
 *     
 *     return register_storage[address];
 * }
 * @endcode
 *
 * Key features:
 * - **Lazy Initialization**: Auto-initialize if not already done
 * - **Address Validation**: Check validity before access
 * - **Error Handling**: Return safe defaults for invalid operations
 * - **Thread Safety**: Designed for single-threaded embedded use
 *
 * @section sonicator_initialization Sonicator Initialization
 *
 * Each sonicator is initialized with safe defaults:
 *
 * @code{.c}
 * // Control registers (0x0100 + i*0x20)
 * register_storage[ctrl_base + 0] = 0;     // Stopped
 * register_storage[ctrl_base + 1] = 50;    // 50% amplitude
 * register_storage[ctrl_base + 2] = 0;     // No reset
 *
 * // Status registers (0x0110 + i*0x20)
 * register_storage[stat_base + 0] = 0;     // Power 0
 * register_storage[stat_base + 1] = 20000; // 20kHz frequency
 * register_storage[stat_base + 2] = 0;     // No flags
 * // ... additional status registers
 * @endcode
 *
 * @section error_handling Error Handling
 *
 * The implementation provides robust error handling:
 * - **Invalid Addresses**: Operations on invalid addresses are ignored or return 0
 * - **Uninitialized Access**: Automatic initialization on first access
 * - **Memory Safety**: Bounds checking prevents array overruns
 * - **Fail-Safe Defaults**: Safe values returned for error conditions
 *
 * @section performance_characteristics Performance Characteristics
 *
 * The flat array approach provides excellent performance:
 * - **Access Time**: O(1) direct array access
 * - **Memory Usage**: Fixed, predictable footprint
 * - **Cache Performance**: Sequential access patterns
 * - **Validation Overhead**: Minimal inline checks
 *
 * @section memory_usage Memory Usage
 *
 * Memory footprint is optimized for embedded systems:
 * - **Register Array**: 1056 × 2 = 2112 bytes
 * - **Initialization Flag**: 1 byte
 * - **Total**: ~2113 bytes
 * - **No Dynamic Allocation**: All memory static
 *
 * @section thread_safety Thread Safety
 *
 * The implementation is designed for embedded systems:
 * - **Single Thread**: Assumes single-threaded execution
 * - **No Synchronization**: No mutexes or locks
 * - **Atomic Operations**: Read/write operations are atomic on most platforms
 * - **Interrupt Safe**: Can be used in interrupt contexts
 *
 * @section testing_and_validation Testing and Validation
 *
 * The implementation supports comprehensive testing:
 * - **Unit Tests**: Individual function validation
 * - **Integration Tests**: Full system validation
 * - **Boundary Tests**: Edge case and error condition testing
 * - **Performance Tests**: Timing and resource usage validation
 *
 * @warning This implementation assumes single-threaded operation
 * @warning Address validation is critical for system safety
 * @warning Always initialize before use in production code
 *
 * @see simple_register_map.h Header file with API declarations
 * @see modbus_registers.h MODBUS register structure definitions
 * @see docs/register-map-architecture.md Register map design documentation
 *
 * @note The enum values are used directly as array indices
 * @note All registers are initialized to safe default values
 * @note Memory layout is optimized for embedded system constraints
 * @note Validation is performed on all read/write operations
 */

#include "simple_register_map.h"
#include <string.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

// Flat array storage for all registers
static uint16_t register_storage[MAX_REGISTERS] = {0};

// Flag to track if the register system has been initialized
static bool registers_initialized = false;

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

bool simple_register_init(void) {
    // Initialize all registers to 0
    memset(register_storage, 0, sizeof(register_storage));
    
    // Set default system status
    register_storage[REG_SYSTEM_STATUS] = 0x0001; // SYSTEM_STATUS_OK
    register_storage[REG_ACTIVE_COUNT] = 0;
    register_storage[REG_ACTIVE_MASK] = 0;
    register_storage[REG_WATCHDOG_STATUS] = 1; // Watchdog OK
    register_storage[REG_COMM_ERRORS] = 0;
    register_storage[REG_PREV_ACTIVE_MASK] = 0; // None active before boot (default)
    register_storage[REG_LAST_SHUTDOWN_REASON] = 0; // 0=normal default
    
    // Initialize all sonicators to safe defaults
    for (int i = 0; i < 4; i++) {
        // Calculate base addresses for each sonicator
        uint16_t ctrl_base = 0x0100 + (i * 0x20);  // Control registers start
        uint16_t stat_base = 0x0110 + (i * 0x20);  // Status registers start
        
        // Control registers
        register_storage[ctrl_base + 0] = 0;          // Stopped
        register_storage[ctrl_base + 1] = 50;         // 50% default amplitude
        register_storage[ctrl_base + 2] = 0;          // No overload reset
        
        // Status registers
        register_storage[stat_base + 0] = 0;          // Power 0
        register_storage[stat_base + 1] = 20000;      // 20kHz default
        register_storage[stat_base + 2] = 0;          // No status flags
        register_storage[stat_base + 3] = 0;          // Actual amplitude 0
        register_storage[stat_base + 4] = 0;          // STOPPED prev state
        register_storage[stat_base + 5] = 50;         // Persisted amplitude 50%
        register_storage[stat_base + 6] = 0;          // No fault code
        register_storage[stat_base + 7] = 0;          // Timestamp 0
    }
    
    registers_initialized = true;
    return true;
}

uint16_t simple_register_read(simple_register_address_t address) {
    // If not initialized, initialize first
    if (!registers_initialized) {
        simple_register_init();
    }
    
    // Validate address
    if (!is_valid_simple_register_address(address)) {
        return 0; // Return 0 for invalid addresses
    }
    
    return register_storage[address];
}

void simple_register_write(simple_register_address_t address, uint16_t value) {
    // If not initialized, initialize first
    if (!registers_initialized) {
        simple_register_init();
    }
    
    // Validate address
    if (!is_valid_simple_register_address(address)) {
        return; // Ignore writes to invalid addresses
    }
    
    register_storage[address] = value;
}