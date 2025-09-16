/**
 * @file simple_register_map.h
 * @title Simplified MODBUS Register Map using Enum-Based Addressing
 * @company Axovia AI
 * @date 2025-09-16
 * @brief Simplified MODBUS register map implementation using enum-based addressing and flat array storage
 * @version 1.1.0
 *
 * @details
 * This file provides a simplified register map implementation using an enumerated type for
 * register addresses and a flat array for storage. This approach replaces the complex
 * struct-based register map with a simpler, more efficient design that is easier to maintain
 * and extend while providing the same functionality.
 *
 * The simplified register map offers several advantages over the traditional struct approach:
 * - **Direct Address Access**: Registers can be accessed directly by enum constants
 * - **Flat Storage**: All registers stored in a single contiguous array for efficiency
 * - **Type Safety**: Enum prevents invalid register address access
 * - **Maintainability**: Easier to add, remove, or modify register definitions
 * - **Performance**: Faster access with compile-time address resolution
 *
 * @section overview Overview
 *
 * The simplified register map consists of three main components:
 * - **Enumerated Addresses**: Named constants for all register addresses
 * - **Flat Array Storage**: Contiguous memory block for register values
 * - **Access Functions**: Simple read/write functions with validation
 *
 * This design maintains full compatibility with MODBUS RTU protocol while simplifying
 * the internal implementation and improving code maintainability.
 *
 * @section register_organization Register Organization
 *
 * Registers are organized into logical groups:
 * - **System Status Registers (0x0000-0x000F)**: Read-only system-wide status
 * - **Global Control Registers (0x0010-0x001F)**: Read/write global control
 * - **Per-Sonicator Registers (0x0100-0x041F)**: Individual sonicator control and status
 *
 * Each sonicator occupies 32 registers (0x20 stride) with control and status blocks.
 *
 * @section usage Usage
 *
 * To use the simplified register map:
 *
 * @code{.c}
 * // Initialize the register system
 * if (!simple_register_init()) {
 *     // Handle initialization failure
 * }
 *
 * // Write to a register
 * simple_register_write(REG_SON1_AMPLITUDE_SP, 75);
 *
 * // Read from a register
 * uint16_t amplitude = simple_register_read(REG_SON1_AMPLITUDE_SP);
 *
 * // Check if address is valid
 * if (is_valid_simple_register_address(REG_SYSTEM_STATUS)) {
 *     uint16_t status = simple_register_read(REG_SYSTEM_STATUS);
 * }
 * @endcode
 *
 * @section register_addresses Register Addresses
 *
 * The enum @c simple_register_address_t defines all valid register addresses:
 * - **REG_SYSTEM_STATUS**: Overall system status bitfield
 * - **REG_ACTIVE_COUNT**: Number of currently active sonicators
 * - **REG_ACTIVE_MASK**: Bitmask of running sonicator units
 * - **REG_SONx_***: Sonicator-specific registers (x = 1-4)
 *
 * @section memory_efficiency Memory Efficiency
 *
 * The flat array approach provides optimal memory usage:
 * - **Contiguous Storage**: All registers in a single array
 * - **Fixed Size**: Known memory footprint at compile time
 * - **No Overhead**: Minimal metadata compared to struct-based approach
 * - **Cache Friendly**: Sequential access patterns for better performance
 *
 * @section data_integrity Data Integrity
 *
 * The implementation ensures data integrity through:
 * - **Address Validation**: All access validated against valid ranges
 * - **Type Safety**: Enum prevents invalid address usage
 * - **Initialization**: All registers initialized to safe defaults
 * - **Bounds Checking**: Automatic validation of array access
 *
 * @section performance_considerations Performance Considerations
 *
 * The simplified register map is optimized for embedded systems:
 * - **Compile-time Resolution**: Enum values resolved at compile time
 * - **Direct Array Access**: Minimal indirection for register access
 * - **Inline Validation**: Fast address validation functions
 * - **Minimal Overhead**: Low runtime cost for register operations
 *
 * @section migration_guide Migration from Struct-Based Map
 *
 * To migrate from the previous struct-based register map:
 *
 * @code{.c}
 * // Old approach (struct-based)
 * modbus_register_map_t* map = register_manager_get_map();
 * map->system_status.system_status = 0x0001;
 * uint16_t status = map->system_status.system_status;
 *
 * // New approach (enum-based)
 * simple_register_write(REG_SYSTEM_STATUS, 0x0001);
 * uint16_t status = simple_register_read(REG_SYSTEM_STATUS);
 * @endcode
 *
 * @section error_handling Error Handling
 *
 * The API provides robust error handling:
 * - **Invalid Addresses**: Writes to invalid addresses are ignored
 * - **Read Defaults**: Invalid address reads return 0
 * - **Initialization**: Failed initialization returns false
 * - **Validation**: Address validation functions available
 *
 * @warning This simplified register map is designed for the Multi-Sonicator-IO system
 * @warning Address ranges and register definitions are specific to this application
 * @warning Always validate register addresses before access in production code
 *
 * @see simple_register_map.c Implementation file with function definitions
 * @see modbus_registers.h Original MODBUS register structure definitions
 * @see docs/modbus-protocol-guide.md MODBUS RTU protocol documentation
 *
 * @note The enum values correspond directly to MODBUS register addresses
 * @note All registers are initialized to safe default values on startup
 * @note The flat array storage provides optimal memory layout for embedded systems
 * @note Address validation is performed on all read/write operations
 */

#ifndef SIMPLE_REGISTER_MAP_H
#define SIMPLE_REGISTER_MAP_H

#include <stdint.h>
#include <stdbool.h>
#include "modbus_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumerated register addresses for all MODBUS registers
 * 
 * This enum provides named constants for all register addresses in the system,
 * replacing the previous complex struct-based approach with a simple flat array.
 */
typedef enum {
    // ========================================================================
    // SYSTEM STATUS REGISTERS (0x0000-0x000F) - Read Only
    // ========================================================================
    REG_SYSTEM_STATUS        = 0x0000,  //< Overall system status bitfield
    REG_ACTIVE_COUNT         = 0x0001,  //< Number of active sonicators (0-4)
    REG_ACTIVE_MASK          = 0x0002,  //< Bitmask of running units (bits 0-3)
    REG_WATCHDOG_STATUS      = 0x0003,  //< Watchdog timer status and health
    REG_COMM_ERRORS          = 0x0004,  //< Communication error count (cumulative)
    REG_PREV_ACTIVE_MASK     = 0x0005,  //< Previous active units mask (diagnostic)
    REG_LAST_SHUTDOWN_REASON = 0x0006,  //< Last shutdown reason code (0-255)
    REG_SYSTEM_UPTIME_LO     = 0x0007,  //< System uptime low 16 bits (seconds)
    REG_SYSTEM_UPTIME_HI     = 0x0008,  //< System uptime high 16 bits (seconds)
    REG_SONICATOR_PRESENCE_MASK = 0x0009, //< Bitmask of sonicators present (bits 0-3)
    // 0x000A-0x000F: Reserved for future system status registers

    // ========================================================================
    // GLOBAL CONTROL REGISTERS (0x0010-0x001F) - Read/Write
    // ========================================================================
    REG_GLOBAL_ENABLE        = 0x0010,  //< Global system enable (0=disabled, 1=enabled)
    REG_EMERGENCY_STOP       = 0x0011,  //< Emergency stop command (1=activate)
    REG_SYSTEM_RESET         = 0x0012,  //< System reset command (0xA5C3=activate)
    REG_TEST_START_INHIBIT   = 0x0013,  //< Per-unit start inhibit mask (bits 0-3)
    REG_GLOBAL_AMPLITUDE     = 0x0014,  //< Global amplitude setpoint (optional)
    // 0x0015-0x001F: Reserved for future global control

    // ========================================================================
    // SONICATOR 1 REGISTERS (ID=0, 0x0100-0x011F)
    // ========================================================================
    // Sonicator 1 Control (0x0100-0x010F) - Read/Write
    REG_SON1_START_STOP      = 0x0100,  //< Start/Stop control
    REG_SON1_AMPLITUDE_SP    = 0x0101,  //< Amplitude setpoint (20-100%)
    REG_SON1_OVERLOAD_RESET  = 0x0102,  //< Overload reset command
    // 0x0103-0x010F: Reserved for future control

    // Sonicator 1 Status (0x0110-0x011F) - Read Only
    REG_SON1_POWER_RAW_ADC   = 0x0110,  //< Raw ADC power reading (0-1023)
    REG_SON1_FREQ_DIV10_HZ   = 0x0111,  //< Operating frequency ÷10 (Hz÷10)
    REG_SON1_STATUS_FLAGS    = 0x0112,  //< Status flags
    REG_SON1_AMPLITUDE_ACT   = 0x0113,  //< Actual amplitude reading
    REG_SON1_PREV_STATE      = 0x0114,  //< Previous state enum
    REG_SON1_PERSISTED_AMPLITUDE = 0x0115, //< Last setpoint before shutdown
    REG_SON1_LAST_FAULT_CODE = 0x0116,  //< Last fault code
    REG_SON1_LAST_STATE_TIMESTAMP_LO = 0x0117, //< Low 16 bits of timestamp
    // 0x0118-0x011F: Reserved for future status

    // ========================================================================
    // SONICATOR 2 REGISTERS (ID=1, 0x0120-0x013F)
    // ========================================================================
    // Sonicator 2 Control (0x0120-0x012F) - Read/Write
    REG_SON2_START_STOP      = 0x0120,  //< Start/Stop control
    REG_SON2_AMPLITUDE_SP    = 0x0121,  //< Amplitude setpoint (20-100%)
    REG_SON2_OVERLOAD_RESET  = 0x0122,  //< Overload reset command
    // 0x0123-0x012F: Reserved for future control

    // Sonicator 2 Status (0x0130-0x013F) - Read Only
    REG_SON2_POWER_RAW_ADC   = 0x0130,  //< Raw ADC power reading (0-1023)
    REG_SON2_FREQ_DIV10_HZ   = 0x0131,  //< Operating frequency ÷10 (Hz÷10)
    REG_SON2_STATUS_FLAGS    = 0x0132,  //< Status flags
    REG_SON2_AMPLITUDE_ACT   = 0x0133,  //< Actual amplitude reading
    REG_SON2_PREV_STATE      = 0x0134,  //< Previous state enum
    REG_SON2_PERSISTED_AMPLITUDE = 0x0135, //< Last setpoint before shutdown
    REG_SON2_LAST_FAULT_CODE = 0x0136,  //< Last fault code
    REG_SON2_LAST_STATE_TIMESTAMP_LO = 0x0137, //< Low 16 bits of timestamp
    // 0x0138-0x013F: Reserved for future status

    // ========================================================================
    // SONICATOR 3 REGISTERS (ID=2, 0x0140-0x015F)
    // ========================================================================
    // Sonicator 3 Control (0x0140-0x014F) - Read/Write
    REG_SON3_START_STOP      = 0x0140,  //< Start/Stop control
    REG_SON3_AMPLITUDE_SP    = 0x0141,  //< Amplitude setpoint (20-100%)
    REG_SON3_OVERLOAD_RESET  = 0x0142,  //< Overload reset command
    // 0x0143-0x014F: Reserved for future control

    // Sonicator 3 Status (0x0150-0x015F) - Read Only
    REG_SON3_POWER_RAW_ADC   = 0x0150,  //< Raw ADC power reading (0-1023)
    REG_SON3_FREQ_DIV10_HZ   = 0x0151,  //< Operating frequency ÷10 (Hz÷10)
    REG_SON3_STATUS_FLAGS    = 0x0152,  //< Status flags
    REG_SON3_AMPLITUDE_ACT   = 0x0153,  //< Actual amplitude reading
    REG_SON3_PREV_STATE      = 0x0154,  //< Previous state enum
    REG_SON3_PERSISTED_AMPLITUDE = 0x0155, //< Last setpoint before shutdown
    REG_SON3_LAST_FAULT_CODE = 0x0156,  //< Last fault code
    REG_SON3_LAST_STATE_TIMESTAMP_LO = 0x0157, //< Low 16 bits of timestamp
    // 0x0158-0x015F: Reserved for future status

    // ========================================================================
    // SONICATOR 4 REGISTERS (ID=3, 0x0160-0x017F)
    // ========================================================================
    // Sonicator 4 Control (0x0160-0x016F) - Read/Write
    REG_SON4_START_STOP      = 0x0160,  //< Start/Stop control
    REG_SON4_AMPLITUDE_SP    = 0x0161,  //< Amplitude setpoint (20-100%)
    REG_SON4_OVERLOAD_RESET  = 0x0162,  //< Overload reset command
    // 0x0163-0x016F: Reserved for future control

    // Sonicator 4 Status (0x0170-0x017F) - Read Only
    REG_SON4_POWER_RAW_ADC   = 0x0170,  //< Raw ADC power reading (0-1023)
    REG_SON4_FREQ_DIV10_HZ   = 0x0171,  //< Operating frequency ÷10 (Hz÷10)
    REG_SON4_STATUS_FLAGS    = 0x0172,  //< Status flags
    REG_SON4_AMPLITUDE_ACT   = 0x0173,  //< Actual amplitude reading
    REG_SON4_PREV_STATE      = 0x0174,  //< Previous state enum
    REG_SON4_PERSISTED_AMPLITUDE = 0x0175, //< Last setpoint before shutdown
    REG_SON4_LAST_FAULT_CODE = 0x0176,  //< Last fault code
    REG_SON4_LAST_STATE_TIMESTAMP_LO = 0x0177, //< Low 16 bits of timestamp
    // 0x0178-0x017F: Reserved for future status

    // Additional sonicators would continue in the same pattern
    // Up to 32 sonicators supported (0x0100-0x041F range)
    
    // Maximum register address + 1 for array sizing
    REG_MAX_ADDRESS          = 0x0420   //< One past the highest valid register address
} simple_register_address_t;

// Maximum number of registers in the system
#define MAX_REGISTERS REG_MAX_ADDRESS

/**
 * @brief Initialize the simplified register storage system
 * @return true on success, false on failure
 */
bool simple_register_init(void);

/**
 * @brief Read a register value by address
 * @param address Register address to read
 * @return Register value at the specified address
 */
uint16_t simple_register_read(simple_register_address_t address);

/**
 * @brief Write a register value by address
 * @param address Register address to write
 * @param value Value to write to the register
 */
void simple_register_write(simple_register_address_t address, uint16_t value);

/**
 * @brief Check if a register address is valid
 * @param address Register address to validate
 * @return true if address is valid, false otherwise
 */
static inline bool is_valid_simple_register_address(simple_register_address_t address) {
    return (address >= REG_SYSTEM_STATUS && address < REG_MAX_ADDRESS);
}

#ifdef __cplusplus
}
#endif

#endif // SIMPLE_REGISTER_MAP_H