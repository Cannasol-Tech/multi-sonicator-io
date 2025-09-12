/**
 * @file sonicator_modbus_bridge.h
 * @brief Bridge between sonicator control and MODBUS registers for Story 1.4
 * @author Cannasol Technologies
 * @date 2025-09-12
 * @version 1.0.0
 * 
 * @details
 * Provides the interface for the bridge between sonicator control functions 
 * and MODBUS register management. Enables integration of sonicator control
 * with industrial automation systems via MODBUS RTU.
 * 
 * Key features:
 * - Bidirectional MODBUS register synchronization
 * - <100ms response time compliance
 * - Status flag mapping and translation
 * - Write-and-clear command handling
 * - Real-time telemetry updates
 */

#ifndef SONICATOR_MODBUS_BRIDGE_H
#define SONICATOR_MODBUS_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize sonicator MODBUS bridge
 * 
 * Sets up the bridge between sonicator control functions and MODBUS
 * register management. Must be called after both sonicator control
 * and MODBUS systems are initialized.
 * 
 * @return true on success, false on failure
 */
bool sonicator_modbus_bridge_init(void);

/**
 * @brief Update sonicator-MODBUS bridge (call from main loop)
 * 
 * Handles bidirectional communication between sonicator control
 * and MODBUS registers. Must be called regularly to maintain
 * <100ms response time requirement.
 * 
 * This function:
 * - Reads MODBUS control registers and executes sonicator commands
 * - Updates MODBUS status registers with current sonicator state
 * - Maintains response time tracking for compliance monitoring
 * 
 * @return true if any control action was taken, false otherwise
 */
bool sonicator_modbus_bridge_update(void);

/**
 * @brief Get bridge status and timing information
 * 
 * Provides timing information for monitoring bridge performance
 * and compliance with response time requirements.
 * 
 * @param last_register_update_time Pointer to store last register update time
 * @param last_control_update_time Pointer to store last control update time  
 * @param response_time_ms Pointer to store last response time in milliseconds
 */
void sonicator_modbus_bridge_get_status(uint32_t* last_register_update_time,
                                       uint32_t* last_control_update_time,
                                       uint32_t* response_time_ms);

/**
 * @brief Check if bridge is meeting response time requirements
 * 
 * Verifies that the bridge is responding to MODBUS control commands
 * within the required <100ms response time per Story 1.4 acceptance criteria.
 * 
 * @return true if response time <100ms, false otherwise
 */
bool sonicator_modbus_bridge_is_responsive(void);

#ifdef __cplusplus
}
#endif

#endif // SONICATOR_MODBUS_BRIDGE_H
