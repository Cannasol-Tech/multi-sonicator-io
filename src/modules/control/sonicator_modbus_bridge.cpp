/**
 * @file sonicator_modbus_bridge.cpp
 * @brief Bridge between sonicator control and MODBUS registers for Story 1.4
 * @author Cannasol Technologies
 * @date 2025-09-12
 * @version 1.0.0
 * 
 * @details
 * Provides the bridge between sonicator control functions and MODBUS register 
 * management. Handles register read/write operations and translates between
 * MODBUS register format and sonicator control API.
 * > > > > > > > > > > > > > > > > > > > > > > > > > > >
 * Implements the MODBUS register integration for Sonicator 4 (zero-based ID 3):
 * - 0x0160: Start/Stop control (RW)
 * - 0x0161: Amplitude setpoint (RW, 20-100%)  
 * - 0x0162: Overload reset (RW, write-and-clear)
 * - 0x0170: Power output (R, watts)
 * - 0x0171: Frequency (R, Hz)
 * - 0x0172: Status flags (R, bit-mapped)
 * - 0x0173: Actual amplitude (R, %)
 * > > > > > > > > > > > > > > > > > > > > > > > > > > >
 * @note > > > > > > > > > > > > > > > > > > > > > > > > 
 * - Sonicator 4 is zero-based ID 3 in the register map 
 * - MODBUS response time target is 100ms per Story 1.4
 */

#include "modules/communication/modbus_register_manager.h"
#include "modules/communication/modbus_registers.h"
#include "sonicator_modbus_bridge.h"
#include "sonicator_control.h"
#include "register_map.h"
#include <Arduino.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define SONICATOR_4_ID 3  // Zero-based sonicator ID for register indexing
#define MODBUS_RESPONSE_TIME_TARGET_MS 100  // Target response time per Story 1.4

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static uint32_t last_register_update = 0;
static uint32_t last_control_update = 0;

// ============================================================================
// PRIVATE FUNCTIONS
// ============================================================================

/**
 * @brief Convert sonicator status to MODBUS status flags
 * @param status Sonicator status structure
 * @return MODBUS status flags (bit-mapped)
 */
static uint16_t build_status_flags(const sonicator_state_control_t* status) {
    uint16_t flags = 0;
    
    if (status->is_running) {
        flags |= SON_STATUS_RUNNING;
    }
    
    if (status->overload_active) {
        flags |= SON_STATUS_OVERLOAD;
    }
    
    if (status->frequency_locked) {
        flags |= SON_STATUS_FREQ_LOCK;
    }
    
    if (status->active_faults != SONICATOR_FAULT_NONE) {
        flags |= SON_STATUS_FAULT;
    }
    
    // Check for communication fault based on last update time
    uint32_t now = millis();
    if (now - status->last_update_time > SONICATOR_COMM_TIMEOUT_MS) {
        flags |= SON_STATUS_COMM_FAULT;
    }
    
    return flags;
}

/**
 * @brief Update MODBUS registers from sonicator status
 */
static void update_modbus_registers_from_sonicator(void) {
    const sonicator_state_control_t* status = sonicator_get_status();
    if (!status) {
        return;
    }
    
    // Build status flags
    uint16_t status_flags = build_status_flags(status);
    
    // Update sonicator status registers via register manager
    register_manager_update_sonicator_status(
        SONICATOR_4_ID,                    // Sonicator ID (zero-based)
        (uint16_t)status->power_watts,     // Power in watts
        status->frequency_hz,              // Frequency in Hz
        status->amplitude_percent,         // Actual amplitude %  
        status_flags                       // Status flags
    );
    
    last_register_update = millis();
}

/**
 * @brief Update sonicator control from MODBUS registers
 * @return true if any control action was taken
 */
static bool update_sonicator_from_modbus_registers(void) {
    // Get current register values for Sonicator 4
    uint16_t start_stop = 0;
    uint16_t amplitude_sp = 50;
    
    if (!register_manager_get_sonicator_control(SONICATOR_4_ID, &start_stop, &amplitude_sp)) {
        return false;
    }
    
    bool action_taken = false;
    
    // Handle start/stop command
    const sonicator_state_control_t* status = sonicator_get_status();
    if (status) {
        // Start command
        if (start_stop && !status->is_running && 
            status->state == SONICATOR_STATE_IDLE) {
            if (sonicator_start()) {
                action_taken = true;
            }
        }
        
        // Stop command  
        if (!start_stop && (status->is_running || 
                           status->state == SONICATOR_STATE_RUNNING ||
                           status->state == SONICATOR_STATE_STARTING)) {
            if (sonicator_stop()) {
                action_taken = true;
            }
        }
    }
    
    // Handle amplitude setpoint (clamp to valid range)
    if (amplitude_sp >= 20 && amplitude_sp <= 100) {
        if (sonicator_set_amplitude((uint8_t)amplitude_sp)) {
            action_taken = true;
        }
    }
    
    // Handle overload reset (write-and-clear command)
    bool reset_requested = false;
    if (register_manager_consume_overload_reset(SONICATOR_4_ID, &reset_requested)) {
        if (reset_requested) {
            if (sonicator_reset_overload()) {
                action_taken = true;
            }
        }
    }
    
    if (action_taken) {
        last_control_update = millis();
    }
    
    return action_taken;
}

// ============================================================================
// PUBLIC FUNCTIONS
// ============================================================================

/**
 * @brief Initialize sonicator MODBUS bridge
 * @return true on success, false on failure
 */
bool sonicator_modbus_bridge_init(void) {
    // Initialize timestamps
    last_register_update = millis();
    last_control_update = last_register_update;
    
    // Set initial default values in registers
    modbus_register_map_t* map = register_manager_get_map();
    if (map) {
        // Set safe defaults for Sonicator 4
        map->sonicators[SONICATOR_4_ID].start_stop = 0;          // Stopped
        map->sonicators[SONICATOR_4_ID].amplitude_setpoint = 50; // 50% amplitude
        map->sonicators[SONICATOR_4_ID].overload_reset = 0;      // No reset
        
        // Initialize status registers  
        map->sonicators[SONICATOR_4_ID].power_watts = 0;
        map->sonicators[SONICATOR_4_ID].frequency_hz = 0;
        map->sonicators[SONICATOR_4_ID].status_flags = 0;
        map->sonicators[SONICATOR_4_ID].amplitude_actual = 0;
    }
    
    return true;
}

/**
 * @brief Update sonicator-MODBUS bridge (call from main loop)
 * 
 * Handles bidirectional communication between sonicator control
 * and MODBUS registers. Must be called regularly to maintain
 * <100ms response time requirement.
 * 
 * @return true if any control action was taken
 */
bool sonicator_modbus_bridge_update(void) {
    uint32_t now = millis();
    bool action_taken = false;
    
    // Update sonicator control from MODBUS registers (higher priority)
    // Check every loop iteration for responsive control
    if (update_sonicator_from_modbus_registers()) {
        action_taken = true;
    }
    
    // Update MODBUS registers from sonicator status (lower frequency)
    // Update at 10Hz to avoid overwhelming the system
    if (now - last_register_update >= 100) {
        update_modbus_registers_from_sonicator();
    }
    
    return action_taken;
}

/**
 * @brief Get bridge status and timing information
 * @param last_register_update_time Pointer to store last register update time
 * @param last_control_update_time Pointer to store last control update time
 * @param response_time_ms Pointer to store last response time
 */
void sonicator_modbus_bridge_get_status(uint32_t* last_register_update_time,
                                       uint32_t* last_control_update_time,
                                       uint32_t* response_time_ms) {
    if (last_register_update_time) {
        *last_register_update_time = last_register_update;
    }
    
    if (last_control_update_time) {
        *last_control_update_time = last_control_update;
    }
    
    if (response_time_ms) {
        uint32_t now = millis();
        *response_time_ms = (now > last_control_update) ? 
                           (now - last_control_update) : 0;
    }
}

/**
 * @brief Check if bridge is meeting response time requirements
 * @return true if response time <100ms, false otherwise
 */
bool sonicator_modbus_bridge_is_responsive(void) {
    uint32_t now = millis();
    uint32_t response_time = (now > last_control_update) ? 
                            (now - last_control_update) : 0;
    return (response_time < MODBUS_RESPONSE_TIME_TARGET_MS);
}
