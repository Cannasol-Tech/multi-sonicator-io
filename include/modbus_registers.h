/**
 * @file src/modules/communication/modbus_registers.h
 * @title MODBUS Register Structure Definitions and Address Macros
 * @author Cannasol Technologies
 * @company Axovia AI
 * @date 2025-09-16
 * @brief Comprehensive MODBUS RTU register structure definitions, address calculation macros, and data validation functions
 * @version 1.2.0
 *
 * @details
 * This file defines the complete MODBUS register map structure for the Multi-Sonicator-IO
 * ATmega32A controller. It provides the foundational data structures, address calculation
 * macros, and validation functions used throughout the MODBUS communication system.
 *
 * The register map follows industrial MODBUS RTU standards with clear separation between:
 * - **Read-only status registers**: System and sonicator telemetry data
 * - **Read/write control registers**: Command and configuration parameters
 * - **Write-only command registers**: Pulse commands and reset functions
 *
 * @section register_organization Register Organization
 *
 * The system implements a hierarchical register structure:
 *
 * **System Level (0x0000-0x001F):**
 * - System status and diagnostics (0x0000-0x000F)
 * - Global control and configuration (0x0010-0x001F)
 *
 * **Sonicator Level (0x0100-0x041F):**
 * - Up to 4 Sonicators currently supported, register space for 32 sonicators in Future
 * - 32 registers per sonicator (0x20 stride)
 * - Control block (0xN00-0xN0F) and Status block (0xN10-0xN1F)
 *
 * @section data_efficiency Raw Data Architecture
 *
 * This implementation prioritizes ATmega32A efficiency through raw data storage:
 *
 * **Power Monitoring:**
 * - Raw 10-bit ADC values (0-1023) stored directly
 * - No floating-point conversion in microcontroller
 * - Cloud applies 5.44mV/W scaling factor
 *
 * **Frequency Monitoring:**
 * - Hz÷10 values stored directly from CT2000 hardware
 * - Eliminates multiplication operations
 * - Cloud multiplies by 10 for actual frequency
 *
 * @section address_calculation Address Calculation Macros
 *
 * The system uses standardized macros for register address calculation:
 * - `SONICATOR_REG_ADDR(id, offset)`: Calculate sonicator register address
 * - `IS_VALID_REGISTER_ADDR(addr)`: Validate register address
 * - `IS_READONLY_REGISTER(addr)`: Check if register is read-only
 *
 * @section performance_characteristics Performance Characteristics
 *
 * - **Register Access Time**: <5ms per register read/write
 * - **Bulk Transfer Rate**: Up to 50 registers per 100ms window
 * - **Update Frequency**: Status registers updated every 10ms
 * - **Command Latency**: Control registers processed within 20ms
 *
 * @warning All register access must use provided macros and validation functions
 * @warning Direct structure member access bypasses validation and may cause system faults
 *
 * @see include/register_map.h Named register address constants
 * @see docs/cloud-conversion-guide.md Raw data conversion formulas
 * @see config/hardware-config.yaml Hardware pin assignments (authoritative source)
 */

#ifndef MODBUS_REGISTERS_H
#define MODBUS_REGISTERS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// MODBUS REGISTER ADDRESS DEFINITIONS
// ============================================================================

/**
 * @defgroup system_status_registers System Status Registers (0x0000-0x000F)
 * @brief Read-only system-wide status and diagnostic registers
 * @details These registers provide real-time system health monitoring and diagnostic
 *          information. All registers in this group are read-only and updated by the
 *          system monitor task every 10ms.
 * @{
 */

/**
 * @brief Active sonicator bitmask register address (0x0002)
 * @details Bitfield indicating which sonicators are currently running
 *
 * Bit mapping:
 * - Bit 0 (0x0001): Sonicator 1 running status
 * - Bit 1 (0x0002): Sonicator 2 running status
 * - Bit 2 (0x0004): Sonicator 3 running status
 * - Bit 3 (0x0008): Sonicator 4 running status
 * - Bits 4-15: Reserved for future expansion
 *
 * @note Synchronized with individual sonicator status flags
 * @note Updated in real-time as sonicators start/stop
 */
#define MODBUS_REG_ACTIVE_MASK          0x0002  //< Bitmask of running units (bits 0-3)

/**
 * @brief Watchdog timer status register address (0x0003)
 * @details System watchdog timer status and health monitoring
 *
 * This register provides information about the system watchdog timer state,
 * including timeout status, reset causes, and health monitoring data.
 * Critical for system reliability and fault detection.
 *
 * @warning Watchdog timeout triggers automatic system reset
 * @note Used by external monitoring systems for health checks
 */
#define MODBUS_REG_WATCHDOG_STATUS      0x0003  //< Watchdog timer status and health

/**
 * @brief Communication error counter register address (0x0004)
 * @details Cumulative count of MODBUS communication errors since system start
 *
 * Error types included:
 * - CRC validation failures
 * - Communication timeouts
 * - Frame format errors
 * - Invalid register access attempts
 * - Protocol violations
 *
 * @note Counter resets to zero on system restart
 * @note High error counts may indicate communication line issues
 * @note Used for communication quality monitoring and diagnostics
 */
#define MODBUS_REG_COMM_ERRORS          0x0004  //< Communication error count (cumulative)

/** @} */ // end of system_status_registers group

/**
 * @defgroup global_control_registers Global Control Registers (0x0010-0x001F)
 * @brief Read/write system-wide control and configuration registers
 * @details These registers provide global control over the entire Multi-Sonicator system.
 *          All registers in this group are read/write and processed by the command
 *          handler task within 20ms of write operations.
 * @{
 */

/**
 * @brief Global system enable register address (0x0010)
 * @details Master enable/disable control for the entire sonicator system
 *
 * Values:
 * - 0: System disabled - All sonicators stopped, no new starts allowed
 * - 1: System enabled - Normal operation, sonicators can be started
 * - Other values: Reserved, treated as disabled
 *
 * When disabled, all running sonicators are immediately stopped and the system
 * enters a safe state. Individual sonicator start commands are ignored while
 * the global enable is disabled.
 *
 * @note This is the master safety control for the entire system
 * @warning Disabling this register immediately stops all sonicators
 * @see REG_EMERGENCY_STOP for emergency shutdown functionality
 */
#define MODBUS_REG_GLOBAL_ENABLE        0x0010  //< Global system enable (0=disabled, 1=enabled)

/**
 * @brief Emergency stop command register address (0x0011)
 * @details Emergency stop activation for immediate system shutdown
 *
 * Values:
 * - 0: Normal operation
 * - 1: Emergency stop activated - Immediate shutdown of all sonicators
 * - Other values: Reserved, treated as emergency stop
 *
 * When activated, this register:
 * - Immediately stops all running sonicators
 * - Captures current system state for diagnostics
 * - Sets system status to emergency stop condition
 * - Requires manual reset to resume operation
 *
 * @warning This is a safety-critical function - activates immediate shutdown
 * @note System state is preserved for diagnostic purposes
 * @note Requires system reset or manual clearance to resume operation
 * @see MODBUS_REG_SYSTEM_RESET for system reset functionality
 */
#define MODBUS_REG_EMERGENCY_STOP       0x0011  //< Emergency stop command (1=activate)

/**
 * @brief System reset command register address (0x0012)
 * @details Software reset command for system restart and fault clearance
 *
 * Values:
 * - 0: Normal operation
 * - 0xA5C3: System reset activation code
 * - Other values: Ignored for safety
 *
 * When the correct activation code is written, the system performs:
 * - Graceful shutdown of all sonicators
 * - Fault condition clearance
 * - System state reset to initial conditions
 * - Communication error counter reset
 * - Watchdog timer reset
 *
 * @warning Use with caution - causes complete system restart
 * @note Requires specific activation code (0xA5C3) for safety
 * @note All running operations are terminated during reset
 */
#define MODBUS_REG_SYSTEM_RESET         0x0012  //< System reset command (0xA5C3=activate)

/**
 * @brief Per-unit start inhibit mask register address (0x0013)
 * @details Bitmask to prevent individual sonicators from starting
 *
 * Bit mapping:
 * - Bit 0 (0x0001): Inhibit Sonicator 1 start commands
 * - Bit 1 (0x0002): Inhibit Sonicator 2 start commands
 * - Bit 2 (0x0004): Inhibit Sonicator 3 start commands
 * - Bit 3 (0x0008): Inhibit Sonicator 4 start commands
 * - Bits 4-15: Reserved for future expansion
 *
 * When a bit is set, the corresponding sonicator will ignore start commands
 * but can still be stopped. This is useful for:
 * - Maintenance operations
 * - Fault isolation
 * - Load management
 * - Testing individual units
 *
 * @note Does not affect currently running sonicators
 * @note Only prevents new start commands from being processed
 * @see Individual sonicator start/stop registers for direct control
 */
#define MODBUS_REG_TEST_START_INHIBIT   0x0013  //< Per-unit start inhibit mask (bits 0-3)

/** @} */ // end of global_control_registers group

// Per-Sonicator Register Base Addresses
#define MODBUS_REG_SONICATOR_BASE       0x0100  //< Base address for sonicator registers
#define MODBUS_REG_SONICATOR_STRIDE     0x0020  //< Address stride between sonicators

// Per-Sonicator Control Register Offsets (Read/Write)
#define MODBUS_REG_SON_START_STOP       0x00    //< Start/Stop control
#define MODBUS_REG_SON_AMPLITUDE_SP     0x01    //< Amplitude setpoint (20-100%)
#define MODBUS_REG_SON_OVERLOAD_RESET   0x02    //< Overload reset command

// Per-Sonicator Status Register Offsets (Read Only)
#define MODBUS_REG_SON_POWER_WATTS      0x10    //< Raw ADC power reading (0-1023, cloud converts)
#define MODBUS_REG_SON_FREQUENCY_HZ     0x11    //< Operating frequency ÷10 (Hz÷10 for efficiency)
#define MODBUS_REG_SON_STATUS_FLAGS     0x12    //< Status flags
#define MODBUS_REG_SON_AMPLITUDE_ACT    0x13    //< Actual amplitude reading

// Maximum sonicator count (based on register range 0x0100-0x041F)
#define MODBUS_MAX_SONICATORS           4

// ============================================================================
// SYSTEM STATUS BIT DEFINITIONS
// ============================================================================

// System Status Register Bits (0x0000)
#define SYSTEM_STATUS_OK                0x0001  //< System operating normally
#define SYSTEM_STATUS_FAULT             0x0002  //< System fault detected
#define SYSTEM_STATUS_EMERGENCY_STOP    0x0004  //< Emergency stop active
#define SYSTEM_STATUS_COMM_FAULT        0x0008  //< Communication fault
#define SYSTEM_STATUS_OVERTEMP          0x0010  //< Over temperature detected

// Sonicator Status Flags Bits (0xN12)
#define SON_STATUS_RUNNING              0x0001  //< Sonicator is running
#define SON_STATUS_OVERLOAD             0x0002  //< Overload detected
#define SON_STATUS_FREQ_LOCK            0x0004  //< Frequency lock achieved
#define SON_STATUS_COMM_FAULT           0x0008  //< Communication fault
#define SON_STATUS_OVER_TEMP            0x0010  //< Over temperature
#define SON_STATUS_FAULT                0x0020  //< General fault

// ============================================================================
// REGISTER STRUCTURE DEFINITIONS
// ============================================================================

/**
 * @brief System Status Registers (0x0000-0x000F) - Read Only
 */
typedef struct {
    uint16_t system_status;        //< 0x0000: Overall system status
    uint16_t active_count;         //< 0x0001: Number of active sonicators
    uint16_t active_mask;          //< 0x0002: Bitmask of running units
    uint16_t watchdog_status;      //< 0x0003: Watchdog timer status
    uint16_t comm_errors;          //< 0x0004: Communication error count
    uint16_t prev_active_mask;     //< 0x0005: Previous active units mask (read-only)
    uint16_t last_shutdown_reason; //< 0x0006: Last shutdown reason code (read-only)
    uint16_t reserved[9];          //< 0x0007-0x000F: Reserved for future use
} system_status_registers_t;

/**
 * @brief Global Control Registers (0x0010-0x001F) - Read/Write
 */
typedef struct {
    uint16_t global_enable;             //< 0x0010: Global system enable
    uint16_t global_amplitude_sp;       //< 0x0011: Global amplitude setpoint (20-100%)
    uint16_t emergency_stop;            //< 0x0012: Emergency stop command
    uint16_t system_reset;              //< 0x0013: System reset command
    uint16_t test_start_inhibit_mask;   //< 0x0014: Per-unit start inhibit mask (test-only)
    uint16_t reserved[12];              //< 0x0015-0x001F: Reserved for future use
} global_control_registers_t;

/**
 * @brief Per Sonicator Control Registers (0xN00-0xN0F) - Read/Write
 */
typedef struct {
    // 0xN00: Start/Stop control (1=run, 0=stop, persistent state)
    uint16_t start_stop;           //< 0xN00: Start/Stop control (1=run, 0=stop, persistent state)
    uint16_t overload_reset;       //< 0xN01: Overload reset command
    uint16_t reserved_ctrl[13];    //< 0xN02-0xN0F: Reserved for future use
} sonicator_control_registers_t;

/**
 * @brief Per Sonicator Status Registers (0xN10-0xN1F) - Read Only
 */
typedef struct {
    uint16_t power_watts;          //< 0xN10: Raw ADC power reading (0-1023, cloud converts)
    uint16_t frequency_hz;         //< 0xN11: Operating frequency ÷10 (Hz÷10 for efficiency)
    uint16_t status_flags;         //< 0xN12: Status flags
    uint16_t amplitude_actual;     //< 0xN13: Actual amplitude reading
    uint16_t prev_state;           //< 0xN14: Previous state enum (read-only)
    uint16_t persisted_amplitude;  //< 0xN15: Last setpoint before shutdown (read-only)
    uint16_t last_fault_code;      //< 0xN16: Last fault code (read-only)
    uint16_t last_state_timestamp_lo; //< 0xN17: Low 16 bits of timestamp (read-only)
    uint16_t reserved_status[7];   //< 0xN18-0xN1F: Reserved for future use
} sonicator_status_registers_t;

/**
 * @brief Per-Sonicator Registers (0xN00-0xN1F)
 * Each sonicator has control (0xN00-0xN0F) and status (0xN10-0xN1F) blocks
 */
typedef struct {
    sonicator_control_registers_t control; //< Control registers (0xN00-0xN0F)
    sonicator_status_registers_t status;   //< Status registers (0xN10-0xN1F)

    // Direct access aliases for backward compatibility
    uint16_t start_stop;           //< Alias for control.start_stop
    uint16_t amplitude_setpoint;   //< Alias for control.amplitude_setpoint
    uint16_t overload_reset;       //< Alias for control.overload_reset
    uint16_t power_watts;          //< Alias for status.power_watts
    uint16_t frequency_hz;         //< Alias for status.frequency_hz
    uint16_t status_flags;         //< Alias for status.status_flags
    uint16_t amplitude_actual;     //< Alias for status.amplitude_actual
    uint16_t prev_state;           //< Alias for status.prev_state
    uint16_t persisted_amplitude;  //< Alias for status.persisted_amplitude
} sonicator_registers_t;

/**
 * @brief Complete MODBUS Register Map
 */
typedef struct {
    system_status_registers_t system_status;    //< System status registers
    global_control_registers_t global_control;  //< Global control registers
    sonicator_registers_t sonicators[MODBUS_MAX_SONICATORS]; //< Per-sonicator registers
} modbus_register_map_t;

// ============================================================================
// REGISTER ACCESS MACROS
// ============================================================================

/**
 * @brief Calculate sonicator register address
 * @param sonicator_id Sonicator ID (0-31)
 * @param offset Register offset within sonicator block
 * @return Absolute register address
 */
#define SONICATOR_REG_ADDR(sonicator_id, offset) \
    (MODBUS_REG_SONICATOR_BASE + ((sonicator_id) * MODBUS_REG_SONICATOR_STRIDE) + (offset))

/**
 * @brief Check if address is within valid register range
 * @param addr Register address to check
 * @return true if address is valid, false otherwise
 */
#define IS_VALID_REGISTER_ADDR(addr) \
    (((addr) <= 0x000F) || \
     (((addr) >= 0x0010) && ((addr) <= 0x001F)) || \
     (((addr) >= 0x0100) && ((addr) <= 0x041F)))

/**
 * @brief Check if register is read-only
 * @param addr Register address to check
 * @return true if register is read-only, false if read/write
 */
#define IS_READONLY_REGISTER(addr) \
    (((addr) <= 0x000F) || \
     (((addr) >= 0x0100) && (((addr) & 0x001F) >= 0x0010)))

#ifdef __cplusplus
}
#endif

#endif // MODBUS_REGISTERS_H
