/**
 * @file modbus_register_manager.h
 * @brief MODBUS Register Map Management Interface
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Provides high-level interface for managing MODBUS register values
 * and integrating with sonicator control systems. Handles register
 * validation, value limits, and system state synchronization.
 */

#ifndef MODBUS_REGISTER_MANAGER_H
#define MODBUS_REGISTER_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "modbus_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// REGISTER MANAGER API
// ============================================================================

/**
 * @brief Initialize the register manager
 * @return true on success, false on failure
 */
bool register_manager_init(void);

/**
 * @brief Get pointer to the register map
 * @return Pointer to the complete register map
 */
modbus_register_map_t* register_manager_get_map(void);

/**
 * @brief Update system status register
 * @param status_bits Status bits to set/clear
 * @param set true to set bits, false to clear bits
 */
void register_manager_update_system_status(uint16_t status_bits, bool set);

/**
 * @brief Update sonicator status
 * @param sonicator_status Pointer to sonicator status structure
 */
void register_manager_update_sonicator_status(sonicator_status_t * sonicator_status);

/**
 * @brief Get sonicator control settings
 * @param sonicator_id Sonicator ID (0-31)
 * @param start_stop Pointer to store start/stop command
 * @param amplitude_sp Pointer to store amplitude setpoint
 * @return true if valid sonicator ID, false otherwise
 */
bool register_manager_get_sonicator_control(uint8_t sonicator_id,
                                           uint16_t* start_stop,
                                           uint16_t* amplitude_sp);

/**
 * @brief Check if global enable is active
 * @return true if globally enabled, false otherwise
 */
bool register_manager_is_global_enabled(void);

/**
 * @brief Check if emergency stop is active
 * @return true if emergency stop active, false otherwise
 */
bool register_manager_is_emergency_stop_active(void);

/**
 * @brief Clear emergency stop condition
 */
void register_manager_clear_emergency_stop(void);

/**
 * @brief Increment communication error count
 */
void register_manager_increment_comm_errors(void);

/**
 * @brief Reset communication error count
 */
void register_manager_reset_comm_errors(void);

/**
 * @brief Validate register value for specific address
 * @param address Register address
 * @param value Value to validate
 * @return true if value is valid for the register, false otherwise
 */
bool register_manager_validate_value(uint16_t address, uint16_t value);

/**
 * @brief Consume (read-and-clear) the overload reset command for a sonicator.
 * @param sonicator_id Sonicator ID (0-31)
 * @param requested Pointer to store whether a reset was requested (true/false)
 * @return true on success, false on invalid id or not initialized
 */
bool register_manager_consume_overload_reset(uint8_t sonicator_id, bool* requested);

#ifdef __cplusplus
}
#endif

#endif // MODBUS_REGISTER_MANAGER_H
