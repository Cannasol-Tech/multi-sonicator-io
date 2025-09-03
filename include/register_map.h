/**
 * @file register_map.h
 * @brief Single source-of-truth register address enumeration for MODBUS map
 * @author Cannasol Technologies
 * @date 2025-09-03
 *
 * @details
 * Defines canonical register address names that align with modbus_registers.h
 * macros. Prefer using these names across firmware and tests so addressing stays
 * consistent. Unused addresses may be reserved as REG_UNUSED_XXXX enumerators
 * to avoid collisions and keep layout explicit.
 */

#ifndef REGISTER_MAP_H
#define REGISTER_MAP_H

#include <stdint.h>
#include "modules/communication/modbus_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Canonical register addresses.
 *
 * Notes:
 * - Sonicator IDs in SONICATOR_REG_ADDR() are zero-based (0..31).
 * - Unit 4 (per pin-matrix) corresponds to sonicator_id = 3.
 */
typedef enum {
    // System Status (0x0000-0x000F)
    REG_SYSTEM_STATUS        = MODBUS_REG_SYSTEM_STATUS,       ///< 0x0000
    REG_ACTIVE_COUNT         = MODBUS_REG_ACTIVE_COUNT,        ///< 0x0001
    REG_ACTIVE_MASK          = MODBUS_REG_ACTIVE_MASK,         ///< 0x0002
    REG_WATCHDOG_STATUS      = MODBUS_REG_WATCHDOG_STATUS,     ///< 0x0003
    REG_COMM_ERRORS          = MODBUS_REG_COMM_ERRORS,         ///< 0x0004

    // Global Control (0x0010-0x001F)
    REG_GLOBAL_ENABLE        = MODBUS_REG_GLOBAL_ENABLE,       ///< 0x0010
    REG_EMERGENCY_STOP       = MODBUS_REG_EMERGENCY_STOP,      ///< 0x0011
    REG_SYSTEM_RESET         = MODBUS_REG_SYSTEM_RESET,        ///< 0x0012

    // Sonicator 4 (id=3) Control (0x0160-0x016F)
    REG_SON4_START_STOP      = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_START_STOP),     ///< 0x0160
    REG_SON4_AMPLITUDE_SP    = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_AMPLITUDE_SP),   ///< 0x0161
    REG_SON4_OVERLOAD_RESET  = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_OVERLOAD_RESET), ///< 0x0162

    // Sonicator 4 (id=3) Status (0x0170-0x017F)
    REG_SON4_POWER_WATTS     = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_POWER_WATTS),    ///< 0x0170
    REG_SON4_FREQUENCY_HZ    = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_FREQUENCY_HZ),   ///< 0x0171
    REG_SON4_STATUS_FLAGS    = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_STATUS_FLAGS),   ///< 0x0172
    REG_SON4_AMPLITUDE_ACT   = SONICATOR_REG_ADDR(3, MODBUS_REG_SON_AMPLITUDE_ACT),  ///< 0x0173

    // Example placeholders for gaps (reserve explicitly as needed)
    // REG_UNUSED_0005 = 0x0005,
    // REG_UNUSED_0006 = 0x0006,
} register_address_t;

#ifdef __cplusplus
}
#endif

#endif // REGISTER_MAP_H

