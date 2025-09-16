/**
 * @file main.cpp
 * @brief Unified main for MODBUS + Sonicator 4 control (single source of truth)
 * @author Cannasol Technologies
 * @date 2025-09-12
 * @version 1.0.0
 * 
 * @details
 * Main application entry point that coordinates MODBUS communication with
 * sonicator control systems. Provides unified control loop for multi-unit
 * sonicator operation with real-time register synchronization.
 * 
 * Key Features:
 * - MODBUS RTU slave communication
 * - Multi-sonicator coordination
 * - Real-time status synchronization
 * - Hardware abstraction layer integration
 * 
 * @note Main loop maintains <100ms response time for MODBUS communication
 */

#include <Arduino.h>
#include "modules/hal/hal.h"
#include "modules/communication/modbus.h"
#include "modules/communication/modbus_register_manager.h"
#include "register_map.h"
#include "modules/control/sonicator4_controller.h"
#include "sonicator_control.h"
#include "sonicator_modbus_bridge.h"
#include "multi_sonicator.h"

/**
 * @brief Configure and initialize MODBUS communication
 * 
 * Sets up MODBUS RTU slave with default configuration parameters.
 * Initializes communication callbacks and timeout handling.
 * 
 * @param cfg Configuration structure for MODBUS communication including:
 * - slave_id: Identifier for the MODBUS slave device.
 * - baud_rate: Communication speed in bits per second.
 * - timeout_ms: Timeout duration in milliseconds for communication.
 * - read_callback: Callback function for reading registers.
 * - write_callback: Callback function for writing registers.
 * - timeout_callback: Callback function for handling timeouts.
 * - error_callback: Callback function for handling communication errors.
 */
static void setup_modbus(void) {
    modbus_config_t cfg{};
    cfg.slave_id = MODBUS_SLAVE_ID;
    cfg.baud_rate = MODBUS_BAUD_RATE;
    cfg.timeout_ms = MODBUS_TIMEOUT_MS;
    cfg.read_callback = nullptr;
    cfg.write_callback = nullptr;
    cfg.timeout_callback = nullptr;
    cfg.error_callback = nullptr;
    (void)modbus_init(&cfg);
}

/**
 * @brief Arduino setup function - initialize all subsystems
 * 
 * Initializes hardware abstraction layer, MODBUS communication,
 * sonicator control systems, and sets default operating parameters.
 * Establishes the sonicator-MODBUS bridge for register synchronization.
 */
void setup() {
    // Initialize hardware abstraction layer
    (void)hal_init();
    
    // Initialize MODBUS register management
    register_manager_init();
    setup_modbus();
    
    // Initialize sonicator control system
    sonicator_begin();
    // Initialize multi-unit controller
    multi_sonicator_begin();
    
    // Initialize sonicator-MODBUS bridge
    sonicator_modbus_bridge_init();
    
    // Set default values
    auto* map = register_manager_get_map();
    map->global_control.global_enable = 1; // Enable global control
    map->sonicators[3].amplitude_setpoint = 50; // Sonicator 4 default amplitude
}

/**
 * @brief Arduino main loop - coordinate all subsystems
 * 
 * Main control loop that processes MODBUS communication, services
 * sonicator hardware, updates register bridges, and synchronizes
 * multi-unit status to MODBUS registers.
 * 
 * Loop maintains real-time operation with 10ms cycle time to ensure
 * MODBUS response time requirements are met.
 * 
 * @param current_time Current time in milliseconds since the Arduino board began running
 * @param m Pointer to multi_status_t structure containing the state of multi-sonicator system, including:
 * - active_mask: Bitmask indicating which sonicators are active.
 * - active_count: Number of active sonicators.
 * - unit_state: Array holding the current state of each sonicator.
 * @param flags Status flags for each sonicator indicating RUNNING/FAULT states.
 * @param error_state Boolean indicating if the sonicator is in an error state.
 */
void loop() {
    static unsigned long last_modbus_time = 0;
    static unsigned long last_sonicator_time = 0;
    static unsigned long last_bridge_time = 0;
    static unsigned long last_multi_update_time = 0;
    
    const unsigned long MODBUS_PROCESS_LOOP_MS = 5;
    const unsigned long SONICATOR_SERVICE_LOOP_MS = 10;
    const unsigned long BRIDGE_UPDATE_LOOP_MS = 20;
    const unsigned long MULTI_UPDATE_LOOP_MS = 50;
    
    unsigned long current_time = millis();
    
    // Process MODBUS communications
    if (current_time - last_modbus_time >= MODBUS_PROCESS_LOOP_MS) {
        (void)modbus_process();
        last_modbus_time = current_time;
    }
    
    // Service sonicator hardware
    if (current_time - last_sonicator_time >= SONICATOR_SERVICE_LOOP_MS) {
        son4_service();
        last_sonicator_time = current_time;
    }
    
    // Update sonicator-MODBUS bridge
    if (current_time - last_bridge_time >= BRIDGE_UPDATE_LOOP_MS) {
        sonicator_modbus_bridge_update();
        last_bridge_time = current_time;
    }
    
    // Update multi-unit coordination
    if (current_time - last_multi_update_time >= MULTI_UPDATE_LOOP_MS) {
        update_multi_sonicator_status();
        last_multi_update_time = current_time;
    }
    
    delay(1); // Small delay to prevent CPU hogging
}

/**
 * @brief Update multi-sonicator status and reflect to MODBUS registers
 */
static void update_multi_sonicator_status() {
    (void)multi_sonicator_update();
    const multi_status_t* m = multi_sonicator_get_status();
    if (m) {
        modbus_register_map_t* map = register_manager_get_map();
        if (map) {
            // Active mask/count
            uint16_t active_mask = m->active_mask; // Bitmask of active sonicators
            uint16_t active_count = 0; // Count of active sonicators
            for (int i = 0; i < 4; ++i) {
                if (active_mask & (1u << i)) active_count++;
                // Update per-unit RUNNING/FAULT flags minimally for acceptance visibility
                uint16_t flags = map->sonicators[i].status_flags; // Current status flags of the sonicator
                // RUNNING bit
                if (m->unit_state[i] == SONICATOR_STATE_RUNNING) {
                    flags |= SON_STATUS_RUNNING; // Set running flag
                } else {
                    flags &= (uint16_t)~SON_STATUS_RUNNING; // Clear running flag
                }
                // FAULT indication for non-running error states (FAULT/OVERLOAD)
                bool error_state = ((int)m->unit_state[i] == (int)SONICATOR_STATE_FAULT) ||
                                 ((int)m->unit_state[i] == (int)SONICATOR_STATE_OVERLOAD);
                if (error_state) {
                    flags |= SON_STATUS_FAULT; // Set fault flag
                } else {
                    // Do not clear FAULT here if higher-level control wants to retain it; keep sticky behavior optional
                    // For now, clear only when running
                    if (m->unit_state[i] == SONICATOR_STATE_RUNNING || m->unit_state[i] == SONICATOR_STATE_IDLE) {
                        flags &= (uint16_t)~SON_STATUS_FAULT; // Clear fault flag
                    }
                }
                map->sonicators[i].status_flags = flags; // Update status flags in register map
            }
            map->system_status.active_mask = active_mask; // Update system active mask
            map->system_status.active_count = active_count; // Update system active count
        }
    }
}
    delay(10); // Delay for 10ms for timing control
}
