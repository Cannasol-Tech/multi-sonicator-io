/**
 * @file include/register_map.h
 * @title MODBUS Register Address Map for Multi-Sonicator I/O Controller
 * @author Cannasol Technologies
 * @company Axovia AI
 * @date 2025-09-16
 * @brief Comprehensive MODBUS RTU register address definitions and convenience macros for sonicator control and monitoring
 * @version 1.1.0
 *
 * @details
 * This file provides the complete MODBUS register address map for the Multi-Sonicator-IO
 * ATmega32A controller. It defines canonical register addresses for all sonicator control
 * and monitoring functions, following industrial MODBUS RTU standards.
 *
 * The register map is organized into three main sections:
 * - **System Status Registers (0x0000-0x000F)**: Read-only system-wide status and diagnostics
 * - **Global Control Registers (0x0010-0x001F)**: Read/write global system control commands
 * - **Per-Sonicator Registers (0x0100-0x041F)**: Individual sonicator control and status (32 registers per unit)
 *
 * @section register_architecture Register Architecture
 *
 * The system supports up to 32 sonicators with a 32-register stride (0x20) per unit:
 * - **Control Block (0xN00-0xN0F)**: Read/write control registers
 * - **Status Block (0xN10-0xN1F)**: Read-only status and telemetry registers
 *
 * Currently implemented sonicators:
 * - Sonicator 1 (ID=0): 0x0100-0x011F
 * - Sonicator 2 (ID=1): 0x0120-0x013F
 * - Sonicator 3 (ID=2): 0x0140-0x015F
 * - Sonicator 4 (ID=3): 0x0160-0x017F
 *
 * @section data_efficiency Raw Data Efficiency
 *
 * This implementation prioritizes ATmega32A efficiency by storing raw sensor data:
 * - **Power registers**: Raw 10-bit ADC values (0-1023) instead of converted watts
 * - **Frequency registers**: Hz÷10 values directly from CT2000 hardware
 * - **Cloud conversion**: All scaling and unit conversions performed in PLC/cloud layer
 *
 * @section hardware_mapping Hardware Pin Mapping
 *
 * All pin assignments are defined in `config/hardware-config.yaml` as the **SOLE SOURCE OF TRUTH**.
 * This register map provides the MODBUS interface to those hardware signals.
 *
 * @section performance_requirements Performance Requirements
 *
 * - **Response Time**: <100ms per NFR2 requirements
 * - **Update Rate**: ≥10Hz telemetry refresh per NFR9
 * - **Communication**: MODBUS RTU at 115200 baud, 8N1, Slave ID=2
 * - **Latency**: ≤200ms end-to-end command latency per NFR1
 *
 * @section usage_examples Usage Examples
 *
 * @code{.c}
 * // Read sonicator 1 power (raw ADC)
 * uint16_t power_adc = modbus_read_register(REG_SON1_POWER_RAW_ADC);
 *
 * // Start sonicator 2 with 75% amplitude
 * modbus_write_register(REG_SON2_AMPLITUDE_SP, 75);
 * modbus_write_register(REG_SON2_START_STOP, 1);
 *
 * // Emergency stop all sonicators
 * modbus_write_register(REG_EMERGENCY_STOP, 1);
 *
 * // Using convenience macros
 * uint16_t son3_freq = modbus_read_register(REG_SONICATOR_FREQ_DIV10_HZ(2));
 * @endcode
 *
 * @warning All register addresses must be validated using IS_VALID_REGISTER_ADDR() before access
 * @warning Write operations to read-only registers will be ignored or return error codes
 *
 * @see config/hardware-config.yaml Hardware pin assignments (authoritative source)
 * @see src/modules/communication/modbus_registers.h MODBUS register structure definitions
 * @see docs/cloud-conversion-guide.md Raw data conversion formulas for cloud processing
 *
 * @note This file follows Axovia AI documentation standards and generates comprehensive
 *       API documentation via Doxygen for integration teams and cloud developers.
 */

#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

#include <stdint.h>
#include "modules/communication/modbus_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Canonical MODBUS register addresses for complete Multi-Sonicator system
 *
 * @details
 * This enumeration provides named constants for all MODBUS register addresses in the
 * Multi-Sonicator-IO system. Each register is documented with its address, access type,
 * data format, and functional description.
 *
 * @section register_naming_convention Register Naming Convention
 *
 * Register names follow a structured pattern:
 * - **REG_**: All register names start with this prefix
 * - **SONx_**: Sonicator-specific registers include unit number (1-4)
 * - **Function**: Descriptive name indicating register purpose
 * - **Type**: Suffix indicating data type (RAW_ADC, DIV10_HZ, FLAGS, etc.)
 *
 * @section sonicator_id_mapping Sonicator ID Mapping
 *
 * - **Physical Unit 1** = sonicator_id 0 (registers 0x0100-0x011F)
 * - **Physical Unit 2** = sonicator_id 1 (registers 0x0120-0x013F)
 * - **Physical Unit 3** = sonicator_id 2 (registers 0x0140-0x015F)
 * - **Physical Unit 4** = sonicator_id 3 (registers 0x0160-0x017F)
 *
 * @section register_stride Register Stride Architecture
 *
 * Each sonicator occupies exactly 32 registers (0x20 stride) organized as:
 * - **Control Block (0xN00-0xN0F)**: Read/write control registers
 * - **Status Block (0xN10-0xN1F)**: Read-only status and telemetry registers
 *
 * This design allows for future expansion up to 32 sonicators while maintaining
 * consistent addressing and efficient register access patterns.
 *
 * @section data_types Register Data Types
 *
 * - **uint16_t**: All MODBUS registers are 16-bit unsigned integers
 * - **Raw ADC**: 0-1023 range representing 10-bit ADC readings
 * - **Percentage**: 0-100 range for amplitude setpoints
 * - **Frequency**: Hz÷10 values for efficient storage
 * - **Bitfields**: Status flags packed into single registers
 *
 * @warning Register addresses are calculated using SONICATOR_REG_ADDR() macro with zero-based IDs
 * @warning All register access must be validated using IS_VALID_REGISTER_ADDR() function
 *
 * @note Pin assignments are defined in config/hardware-config.yaml as the authoritative source
 */
typedef enum {
    // ========================================================================
    // SYSTEM STATUS REGISTERS (0x0000-0x000F) - Read Only
    // ========================================================================

    /**
     * @brief Overall system status register (0x0000)
     * @details Bitfield containing system-wide status indicators
     *
     * Bit definitions:
     * - Bit 0 (0x0001): SYSTEM_STATUS_OK - System operating normally
     * - Bit 1 (0x0002): SYSTEM_STATUS_FAULT - System fault detected
     * - Bit 2 (0x0004): SYSTEM_STATUS_EMERGENCY_STOP - Emergency stop active
     * - Bit 3 (0x0008): SYSTEM_STATUS_COMM_FAULT - Communication fault
     * - Bit 4 (0x0010): SYSTEM_STATUS_OVERTEMP - Over temperature detected
     * - Bits 5-15: Reserved for future system status flags
     *
     * @note Read-only register updated by system monitor every 10ms
     * @see REG_EMERGENCY_STOP for emergency stop control
     */
    REG_SYSTEM_STATUS        = MODBUS_REG_SYSTEM_STATUS,       //< 0x0000: Overall system status bitfield

    /**
     * @brief Number of currently active sonicators (0x0001)
     * @details Count of sonicators currently in running state (0-4)
     *
     * @note Updated in real-time as sonicators start/stop
     * @see REG_ACTIVE_MASK for individual sonicator status
     */
    REG_ACTIVE_COUNT         = MODBUS_REG_ACTIVE_COUNT,        //< 0x0001: Number of active sonicators (0-4)

    /**
     * @brief Bitmask of currently running sonicators (0x0002)
     * @details Bitfield indicating which sonicators are currently active
     *
     * Bit definitions:
     * - Bit 0 (0x0001): Sonicator 1 running status
     * - Bit 1 (0x0002): Sonicator 2 running status
     * - Bit 2 (0x0004): Sonicator 3 running status
     * - Bit 3 (0x0008): Sonicator 4 running status
     * - Bits 4-15: Reserved for future sonicator expansion
     *
     * @note Updated in real-time, synchronized with individual status flags
     */
    REG_ACTIVE_MASK          = MODBUS_REG_ACTIVE_MASK,         //< 0x0002: Bitmask of running units (bits 0-3)

    /**
     * @brief Watchdog timer status register (0x0003)
     * @details System watchdog timer status and configuration
     *
     * @note Used for system health monitoring and fault detection
     * @warning Watchdog timeout triggers system reset
     */
    REG_WATCHDOG_STATUS      = MODBUS_REG_WATCHDOG_STATUS,     //< 0x0003: Watchdog timer status

    /**
     * @brief Communication error counter (0x0004)
     * @details Cumulative count of MODBUS communication errors
     *
     * Includes:
     * - CRC errors
     * - Timeout errors
     * - Frame errors
     * - Invalid register access attempts
     *
     * @note Counter resets on system restart
     * @note High error count may indicate communication issues
     */
    REG_COMM_ERRORS          = MODBUS_REG_COMM_ERRORS,         //< 0x0004: Communication error count

    /**
     * @brief Previous active sonicator mask (0x0005)
     * @details Snapshot of active sonicators before last emergency stop or system fault
     *
     * Used for:
     * - Diagnostic purposes
     * - Automatic restart after fault clearance
     * - System state recovery
     *
     * @note Captured during emergency stop or fault conditions
     * @see REG_LAST_SHUTDOWN_REASON for shutdown cause
     */
    REG_PREV_ACTIVE_MASK     = 0x0005,                         //< 0x0005: Previous active units mask (diagnostic)

    /**
     * @brief Last system shutdown reason code (0x0006)
     * @details Enumerated reason for last system shutdown or fault
     *
     * Reason codes:
     * - 0: Normal shutdown
     * - 1: Watchdog timeout
     * - 2: Over temperature
     * - 3: Emergency stop activated
     * - 4: Communication fault
     * - 5: Power supply fault
     * - 6-255: Reserved for future fault codes
     *
     * @note Persists across system restarts for diagnostic purposes
     */
    REG_LAST_SHUTDOWN_REASON = 0x0006,                         //< 0x0006: Last shutdown reason code (0-255)

    /**
     * @brief System uptime low 16 bits (0x0007)
     * @details Lower 16 bits of system uptime in seconds
     *
     * Combined with REG_SYSTEM_UPTIME_HI provides 32-bit uptime counter
     * Maximum uptime: ~136 years before rollover
     *
     * @note Updated every second by system timer
     * @see REG_SYSTEM_UPTIME_HI for upper 16 bits
     */
    REG_SYSTEM_UPTIME_LO     = 0x0007,                         //< 0x0007: System uptime low 16 bits (seconds)

    /**
     * @brief System uptime high 16 bits (0x0008)
     * @details Upper 16 bits of system uptime in seconds
     *
     * @note Combined with REG_SYSTEM_UPTIME_LO for full 32-bit uptime
     * @note Useful for system reliability monitoring
     */
    REG_SYSTEM_UPTIME_HI     = 0x0008,                         //< 0x0008: System uptime high 16 bits (seconds)

    /**
     * @brief Sonicator presence bitmask (0x0009)
     * @details Bitmask indicating which sonicators are physically present
     *
     * Bit definitions:
     * - Bit 0 (0x0001): Sonicator 1 present
     * - Bit 1 (0x0002): Sonicator 2 present
     * - Bit 2 (0x0004): Sonicator 3 present
     * - Bit 3 (0x0008): Sonicator 4 present
     * - Bits 4-15: Reserved for future sonicator expansion
     *
     * @note Updated during system initialization
     * @note Useful for dynamic system configuration
     */
    REG_SONCIATOR_PRESENCE_MASK = 0x0009,                       //< 0x0009: Bitmask of sonicators present (bits 0-3)

    // 0x000A-0x000F: Reserved for future system status registers

    // ========================================================================
    // GLOBAL CONTROL REGISTERS (0x0010-0x001F) - Read/Write
    // ========================================================================
    REG_GLOBAL_ENABLE        = MODBUS_REG_GLOBAL_ENABLE,       //< 0x0010: Global system enable
    REG_EMERGENCY_STOP       = MODBUS_REG_EMERGENCY_STOP,      //< 0x0011: Emergency stop command
    REG_SYSTEM_RESET         = MODBUS_REG_SYSTEM_RESET,        //< 0x0012: System reset command
    REG_TEST_START_INHIBIT   = MODBUS_REG_TEST_START_INHIBIT,  //< 0x0013: Per-unit start inhibit mask
    REG_GLOBAL_AMPLITUDE     = 0x0014,                         //< 0x0014: Global amplitude setpoint (optional)
    // 0x0015-0x001F: Reserved for future global control

    // ========================================================================
    // SONICATOR 1 REGISTERS (ID=0, 0x0100-0x011F)
    // ========================================================================

    // Sonicator 1 Control (0x0100-0x010F) - Read/Write
    REG_SON1_START_STOP      = SONICATOR_REG_ADDR(0, MODBUS_REG_SON_START_STOP),     //< 0x0100: Start/Stop control
    REG_SON1_AMPLITUDE_SP    = SONICATOR_REG_ADDR(0, MODBUS_REG_SON_AMPLITUDE_SP),   //< 0x0101: Amplitude setpoint (20-100%)
    REG_SON1_OVERLOAD_RESET  = SONICATOR_REG_ADDR(0, MODBUS_REG_SON_OVERLOAD_RESET), //< 0x0102: Overload reset command
    // 0x0103-0x010F: Reserved for future control

    // Sonicator 1 Status (0x0110-0x011F) - Read Only
    REG_SON1_POWER_RAW_ADC   = SONICATOR_REG_ADDR(0, MODBUS_REG_SON_POWER_WATTS),    //< 0x0110: Raw ADC power reading (0-1023)
    REG_SON1_FREQ_DIV10_HZ   = SONICATOR_REG_ADDR(0, MODBUS_REG_SON_FREQUENCY_HZ),   //< 0x0111: Operating frequency ÷10 (Hz÷10)
    REG_SON1_STATUS_FLAGS    = SONICATOR_REG_ADDR(0, MODBUS_REG_SON_STATUS_FLAGS),   //< 0x0112: Status flags
    REG_SON1_AMPLITUDE_ACT   = SONICATOR_REG_ADDR(0, MODBUS_REG_SON_AMPLITUDE_ACT),  //< 0x0113: Actual amplitude reading
    REG_SON1_PREV_STATE      = SONICATOR_REG_ADDR(0, 0x14),                          //< 0x0114: Previous state enum
    REG_SON1_PERSISTED_AMPLITUDE = SONICATOR_REG_ADDR(0, 0x15),                      //< 0x0115: Last setpoint before shutdown
    REG_SON1_LAST_FAULT_CODE = SONICATOR_REG_ADDR(0, 0x16),                          //< 0x0116: Last fault code
    REG_SON1_LAST_STATE_TIMESTAMP_LO = SONICATOR_REG_ADDR(0, 0x17),                  //< 0x0117: Low 16 bits of timestamp
    // 0x0118-0x011F: Reserved for future status

    // ========================================================================
    // SONICATOR 2 REGISTERS (ID=1, 0x0120-0x013F)
    // ========================================================================

    // Sonicator 2 Control (0x0120-0x012F) - Read/Write
    REG_SON2_START_STOP      = SONICATOR_REG_ADDR(1, MODBUS_REG_SON_START_STOP),     //< 0x0120: Start/Stop control
    REG_SON2_AMPLITUDE_SP    = SONICATOR_REG_ADDR(1, MODBUS_REG_SON_AMPLITUDE_SP),   //< 0x0121: Amplitude setpoint (20-100%)
    REG_SON2_OVERLOAD_RESET  = SONICATOR_REG_ADDR(1, MODBUS_REG_SON_OVERLOAD_RESET), //< 0x0122: Overload reset command
    // 0x0123-0x012F: Reserved for future control

    // Sonicator 2 Status (0x0130-0x013F) - Read Only
    REG_SON2_POWER_RAW_ADC   = SONICATOR_REG_ADDR(1, MODBUS_REG_SON_POWER_WATTS),    //< 0x0130: Raw ADC power reading (0-1023)
    REG_SON2_FREQ_DIV10_HZ   = SONICATOR_REG_ADDR(1, MODBUS_REG_SON_FREQUENCY_HZ),   //< 0x0131: Operating frequency ÷10 (Hz÷10)
    REG_SON2_STATUS_FLAGS    = SONICATOR_REG_ADDR(1, MODBUS_REG_SON_STATUS_FLAGS),   //< 0x0132: Status flags
    REG_SON2_AMPLITUDE_ACT   = SONICATOR_REG_ADDR(1, MODBUS_REG_SON_AMPLITUDE_ACT),  //< 0x0133: Actual amplitude reading
    REG_SON2_PREV_STATE      = SONICATOR_REG_ADDR(1, 0x14),                          //< 0x0134: Previous state enum
    REG_SON2_PERSISTED_AMPLITUDE = SONICATOR_REG_ADDR(1, 0x15),                      //< 0x0135: Last setpoint before shutdown
    REG_SON2_LAST_FAULT_CODE = SONICATOR_REG_ADDR(1, 0x16),                          //< 0x0136: Last fault code
    REG_SON2_LAST_STATE_TIMESTAMP_LO = SONICATOR_REG_ADDR(1, 0x17),                  //< 0x0137: Low 16 bits of timestamp
    // 0x0138-0x013F: Reserved for future status

    // ========================================================================
    // SONICATOR 3 REGISTERS (ID=2, 0x0140-0x015F)
    // ========================================================================

    // Sonicator 3 Control (0x0140-0x014F) - Read/Write
    REG_SON3_START_STOP      = SONICATOR_REG_ADDR(2, MODBUS_REG_SON_START_STOP),     //< 0x0140: Start/Stop control
    REG_SON3_AMPLITUDE_SP    = SONICATOR_REG_ADDR(2, MODBUS_REG_SON_AMPLITUDE_SP),   //< 0x0141: Amplitude setpoint (20-100%)
    REG_SON3_OVERLOAD_RESET  = SONICATOR_REG_ADDR(2, MODBUS_REG_SON_OVERLOAD_RESET), //< 0x0142: Overload reset command
    // 0x0143-0x014F: Reserved for future control

    // Sonicator 3 Status (0x0150-0x015F) - Read Only
    REG_SON3_POWER_RAW_ADC   = SONICATOR_REG_ADDR(2, MODBUS_REG_SON_POWER_WATTS),    //< 0x0150: Raw ADC power reading (0-1023)
    REG_SON3_FREQ_DIV10_HZ   = SONICATOR_REG_ADDR(2, MODBUS_REG_SON_FREQUENCY_HZ),   //< 0x0151: Operating frequency ÷10 (Hz÷10)
    REG_SON3_STATUS_FLAGS    = SONICATOR_REG_ADDR(2, MODBUS_REG_SON_STATUS_FLAGS),   //< 0x0152: Status flags
    REG_SON3_AMPLITUDE_ACT   = SONICATOR_REG_ADDR(2, MODBUS_REG_SON_AMPLITUDE_ACT),  //< 0x0153: Actual amplitude reading
    REG_SON3_PREV_STATE      = SONICATOR_REG_ADDR(2, 0x14),                          //< 0x0154: Previous state enum
    REG_SON3_PERSISTED_AMPLITUDE = SONICATOR_REG_ADDR(2, 0x15),                      //< 0x0155: Last setpoint before shutdown
    REG_SON3_LAST_FAULT_CODE = SONICATOR_REG_ADDR(2, 0x16),                          //< 0x0156: Last fault code
    REG_SON3_LAST_STATE_TIMESTAMP_LO = SONICATOR_REG_ADDR(2, 0x17),                  //< 0x0157: Low 16 bits of timestamp
    // 0x0158-0x015F: Reserved for future status

    // ========================================================================
    // SONICATOR 4 REGISTERS (ID=3, 0x0160-0x017F)
    // ========================================================================

    // Sonicator 4 Control (0x0160-0x016F) - Read/Write
    REG_SON4_START_STOP      = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_START_STOP),     //< 0x0160: Start/Stop control
    REG_SON4_AMPLITUDE_SP    = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_AMPLITUDE_SP),   //< 0x0161: Amplitude setpoint (20-100%)
    REG_SON4_OVERLOAD_RESET  = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_OVERLOAD_RESET), //< 0x0162: Overload reset command
    // 0x0163-0x016F: Reserved for future control

    // Sonicator 4 Status (0x0170-0x017F) - Read Only
    REG_SON4_POWER_RAW_ADC   = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_POWER_WATTS),    //< 0x0170: Raw ADC power reading (0-1023)
    REG_SON4_FREQ_DIV10_HZ   = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_FREQUENCY_HZ),   //< 0x0171: Operating frequency ÷10 (Hz÷10)
    REG_SON4_STATUS_FLAGS    = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_STATUS_FLAGS),   //< 0x0172: Status flags
    REG_SON4_AMPLITUDE_ACT   = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_AMPLITUDE_ACT),  //< 0x0173: Actual amplitude reading
    REG_SON4_PREV_STATE      = SONICATOR_REG_ADDR(3, 0x14),                          //< 0x0174: Previous state enum
    REG_SON4_PERSISTED_AMPLITUDE = SONICATOR_REG_ADDR(3, 0x15),                      //< 0x0175: Last setpoint before shutdown
    REG_SON4_LAST_FAULT_CODE = SONICATOR_REG_ADDR(3, 0x16),                          //< 0x0176: Last fault code
    REG_SON4_LAST_STATE_TIMESTAMP_LO = SONICATOR_REG_ADDR(3, 0x17),                  //< 0x0177: Low 16 bits of timestamp
    // 0x0178-0x017F: Reserved for future status

} register_address_t;

// ============================================================================
// CONVENIENCE MACROS FOR REGISTER ACCESS
// ============================================================================

/**
 * @brief Get start/stop register address for a sonicator
 * @param sonicator_id Sonicator ID (0-3)
 * @return Register address for start/stop control
 */
#define REG_SONICATOR_START_STOP(sonicator_id) \
    SONICATOR_REG_ADDR((sonicator_id), MODBUS_REG_SON_START_STOP)

/**
 * @brief Get amplitude setpoint register address for a sonicator
 * @param sonicator_id Sonicator ID (0-3)
 * @return Register address for amplitude setpoint
 */
#define REG_SONICATOR_AMPLITUDE_SP(sonicator_id) \
    SONICATOR_REG_ADDR((sonicator_id), MODBUS_REG_SON_AMPLITUDE_SP)

/**
 * @brief Get overload reset register address for a sonicator
 * @param sonicator_id Sonicator ID (0-3)
 * @return Register address for overload reset command
 */
#define REG_SONICATOR_OVERLOAD_RESET(sonicator_id) \
    SONICATOR_REG_ADDR((sonicator_id), MODBUS_REG_SON_OVERLOAD_RESET)

/**
 * @brief Get raw ADC power register address for a sonicator
 * @param sonicator_id Sonicator ID (0-3)
 * @return Register address for raw ADC power reading (0-1023)
 */
#define REG_SONICATOR_POWER_RAW_ADC(sonicator_id) \
    SONICATOR_REG_ADDR((sonicator_id), MODBUS_REG_SON_POWER_WATTS)

/**
 * @brief Get frequency ÷10 register address for a sonicator
 * @param sonicator_id Sonicator ID (0-3)
 * @return Register address for frequency ÷10 reading (more efficient)
 */
#define REG_SONICATOR_FREQ_DIV10_HZ(sonicator_id) \
    SONICATOR_REG_ADDR((sonicator_id), MODBUS_REG_SON_FREQUENCY_HZ)

/**
 * @brief Get status flags register address for a sonicator
 * @param sonicator_id Sonicator ID (0-3)
 * @return Register address for status flags
 */
#define REG_SONICATOR_STATUS_FLAGS(sonicator_id) \
    SONICATOR_REG_ADDR((sonicator_id), MODBUS_REG_SON_STATUS_FLAGS)

/**
 * @brief Get actual amplitude register address for a sonicator
 * @param sonicator_id Sonicator ID (0-3)
 * @return Register address for actual amplitude reading
 */
#define REG_SONICATOR_AMPLITUDE_ACT(sonicator_id) \
    SONICATOR_REG_ADDR((sonicator_id), MODBUS_REG_SON_AMPLITUDE_ACT)

// ============================================================================
// REGISTER VALIDATION FUNCTIONS
// ============================================================================

/**
 * @brief Check if a register address is valid for the system
 * @param address Register address to validate
 * @return true if address is valid, false otherwise
 */
static inline bool is_valid_register_address(uint16_t address) {
    return IS_VALID_REGISTER_ADDR(address);
}

/**
 * @brief Check if a register address is read-only
 * @param address Register address to check
 * @return true if register is read-only, false if read/write
 */
static inline bool is_readonly_register_address(uint16_t address) {
    return IS_READONLY_REGISTER(address);
}

/**
 * @brief Get sonicator ID from a sonicator register address
 * @param address Register address within sonicator range
 * @return Sonicator ID (0-3) or 0xFF if invalid
 */
static inline uint8_t get_sonicator_id_from_address(uint16_t address) {
    if (address >= 0x0100 && address <= 0x041F) {
        return (uint8_t)((address - 0x0100) / MODBUS_REG_SONICATOR_STRIDE);
    }
    return 0xFF; // Invalid
}

/**
 * @brief Get register offset within sonicator block from address
 * @param address Register address within sonicator range
 * @return Register offset (0x00-0x1F) or 0xFF if invalid
 */
static inline uint8_t get_sonicator_register_offset(uint16_t address) {
    if (address >= 0x0100 && address <= 0x041F) {
        return (uint8_t)((address - 0x0100) % MODBUS_REG_SONICATOR_STRIDE);
    }
    return 0xFF; // Invalid
}

#ifdef __cplusplus
}
#endif

#endif // REGISTER_MAP_H
