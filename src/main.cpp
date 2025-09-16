/**
 * @file main.cpp
 * @brief Unified main for MODBUS + Sonicator 4 control (single source of truth)
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
    map->global_control.global_enable = 1;
    map->sonicators[3].amplitude_setpoint = 50; // Sonicator 4 default amplitude
}

void loop() {
    (void)modbus_process();
    son4_service();
    
    // Update sonicator-MODBUS bridge
    sonicator_modbus_bridge_update();

    // Update multi-unit coordination state and reflect to MODBUS map
    (void)multi_sonicator_update();
    const multi_status_t* m = multi_sonicator_get_status();
    if (m) {
        modbus_register_map_t* map = register_manager_get_map();
        if (map) {
            // Active mask/count
            uint16_t active_mask = m->active_mask;
            uint16_t active_count = 0;
            for (int i = 0; i < 4; ++i) {
                if (active_mask & (1u << i)) active_count++;
                // Update per-unit RUNNING/FAULT flags minimally for acceptance visibility
                uint16_t flags = map->sonicators[i].status_flags;
                // RUNNING bit
                if (m->unit_state[i] == SONICATOR_STATE_RUNNING) {
                    flags |= SON_STATUS_RUNNING;
                } else {
                    flags &= (uint16_t)~SON_STATUS_RUNNING;
                }
                // FAULT indication for non-running error states (FAULT/OVERLOAD)
                bool error_state = ((int)m->unit_state[i] == (int)SONICATOR_STATE_FAULT) ||
                                 ((int)m->unit_state[i] == (int)SONICATOR_STATE_OVERLOAD);
                if (error_state) {
                    flags |= SON_STATUS_FAULT;
                } else {
                    // Do not clear FAULT here if higher-level control wants to retain it; keep sticky behavior optional
                    // For now, clear only when running
                    if (m->unit_state[i] == SONICATOR_STATE_RUNNING || m->unit_state[i] == SONICATOR_STATE_IDLE) {
                        flags &= (uint16_t)~SON_STATUS_FAULT;
                    }
                }
                map->sonicators[i].status_flags = flags;
            }
            map->system_status.active_mask = active_mask;
            map->system_status.active_count = active_count;
        }
    }
    
    delay(10);
}
