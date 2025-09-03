/**
 * @file modbus_registers.h
 * @brief MODBUS Register Map Definitions for Multi-Sonicator System
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Defines the complete MODBUS register map as specified in Epic 1 Story 3.
 * Implements register structures for system status, global control, and
 * per-sonicator control/monitoring registers (0x0000-0x041F ranges).
 * 
 * Register Map:
 * - 0x0000-0x000F: System Status Registers (Read Only)
 * - 0x0010-0x001F: Global Control Registers (Read/Write)
 * - 0x0100-0x041F: Per-Sonicator Registers (32 sonicators max)
 *   - 0xN00-0xN0F: Control registers for sonicator N
 *   - 0xN10-0xN1F: Status registers for sonicator N
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

// System Status Register Addresses (Read Only)
#define MODBUS_REG_SYSTEM_STATUS        0x0000  ///< Overall system status
#define MODBUS_REG_ACTIVE_COUNT         0x0001  ///< Number of active sonicators
#define MODBUS_REG_ACTIVE_MASK          0x0002  ///< Bitmask of running units
#define MODBUS_REG_WATCHDOG_STATUS      0x0003  ///< Watchdog timer status
#define MODBUS_REG_COMM_ERRORS          0x0004  ///< Communication error count

// Global Control Register Addresses (Read/Write)
#define MODBUS_REG_GLOBAL_ENABLE        0x0010  ///< Global system enable
#define MODBUS_REG_EMERGENCY_STOP       0x0011  ///< Emergency stop command
#define MODBUS_REG_SYSTEM_RESET         0x0012  ///< System reset command

// Per-Sonicator Register Base Addresses
#define MODBUS_REG_SONICATOR_BASE       0x0100  ///< Base address for sonicator registers
#define MODBUS_REG_SONICATOR_STRIDE     0x0020  ///< Address stride between sonicators

// Per-Sonicator Control Register Offsets (Read/Write)
#define MODBUS_REG_SON_START_STOP       0x00    ///< Start/Stop control
#define MODBUS_REG_SON_AMPLITUDE_SP     0x01    ///< Amplitude setpoint (20-100%)
#define MODBUS_REG_SON_OVERLOAD_RESET   0x02    ///< Overload reset command

// Per-Sonicator Status Register Offsets (Read Only)
#define MODBUS_REG_SON_POWER_WATTS      0x10    ///< Power consumption (Watts)
#define MODBUS_REG_SON_FREQUENCY_HZ     0x11    ///< Operating frequency (Hz)
#define MODBUS_REG_SON_STATUS_FLAGS     0x12    ///< Status flags
#define MODBUS_REG_SON_AMPLITUDE_ACT    0x13    ///< Actual amplitude reading

// Maximum sonicator count (based on register range 0x0100-0x041F)
#define MODBUS_MAX_SONICATORS           4

// ============================================================================
// SYSTEM STATUS BIT DEFINITIONS
// ============================================================================

// System Status Register Bits (0x0000)
#define SYSTEM_STATUS_OK                0x0001  ///< System operating normally
#define SYSTEM_STATUS_FAULT             0x0002  ///< System fault detected
#define SYSTEM_STATUS_EMERGENCY_STOP    0x0004  ///< Emergency stop active
#define SYSTEM_STATUS_COMM_FAULT        0x0008  ///< Communication fault
#define SYSTEM_STATUS_OVERTEMP          0x0010  ///< Over temperature detected

// Sonicator Status Flags Bits (0xN12)
#define SON_STATUS_RUNNING              0x0001  ///< Sonicator is running
#define SON_STATUS_OVERLOAD             0x0002  ///< Overload detected
#define SON_STATUS_FREQ_LOCK            0x0004  ///< Frequency lock achieved
#define SON_STATUS_COMM_FAULT           0x0008  ///< Communication fault
#define SON_STATUS_OVER_TEMP            0x0010  ///< Over temperature
#define SON_STATUS_FAULT                0x0020  ///< General fault

// ============================================================================
// REGISTER STRUCTURE DEFINITIONS
// ============================================================================

/**
 * @brief System Status Registers (0x0000-0x000F) - Read Only
 */
typedef struct {
    uint16_t system_status;        ///< 0x0000: Overall system status
    uint16_t active_count;         ///< 0x0001: Number of active sonicators
    uint16_t active_mask;          ///< 0x0002: Bitmask of running units
    uint16_t watchdog_status;      ///< 0x0003: Watchdog timer status
    uint16_t comm_errors;          ///< 0x0004: Communication error count
    uint16_t reserved[11];         ///< 0x0005-0x000F: Reserved for future use
} system_status_registers_t;

/**
 * @brief Global Control Registers (0x0010-0x001F) - Read/Write
 */
typedef struct {
    uint16_t global_enable;        ///< 0x0010: Global system enable
    uint16_t emergency_stop;       ///< 0x0011: Emergency stop command
    uint16_t system_reset;         ///< 0x0012: System reset command
    uint16_t reserved[13];         ///< 0x0013-0x001F: Reserved for future use
} global_control_registers_t;

/**
 * @brief Per-Sonicator Registers (0xN00-0xN1F)
 * Each sonicator has control (0xN00-0xN0F) and status (0xN10-0xN1F) blocks
 */
typedef struct {
    // Control Registers (0xN00-0xN0F) - Read/Write
    uint16_t start_stop;           ///< 0xN00: Start/Stop control
    uint16_t amplitude_setpoint;   ///< 0xN01: Amplitude setpoint (20-100%)
    uint16_t overload_reset;       ///< 0xN02: Overload reset command
    uint16_t reserved_ctrl[13];    ///< 0xN03-0xN0F: Reserved for future use
    
    // Status Registers (0xN10-0xN1F) - Read Only
    uint16_t power_watts;          ///< 0xN10: Power consumption (Watts)
    uint16_t frequency_hz;         ///< 0xN11: Operating frequency (Hz)
    uint16_t status_flags;         ///< 0xN12: Status flags
    uint16_t amplitude_actual;     ///< 0xN13: Actual amplitude reading
    uint16_t reserved_status[12];  ///< 0xN14-0xN1F: Reserved for future use
} sonicator_registers_t;

/**
 * @brief Complete MODBUS Register Map
 */
typedef struct {
    system_status_registers_t system_status;    ///< System status registers
    global_control_registers_t global_control;  ///< Global control registers
    sonicator_registers_t sonicators[MODBUS_MAX_SONICATORS]; ///< Per-sonicator registers
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
