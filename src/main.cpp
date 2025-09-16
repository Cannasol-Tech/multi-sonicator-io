/**
 * @file main.cpp
 * @brief Unified main for MODBUS and Multiplexer control
 * @author James (AI Developer)
 * @date 2025-09-16
 * @version 2.0.0
 * 
 * @details
 * Main application entry point that uses the Multiplexer class to manage all
 * sonicator units and synchronizes their state with the MODBUS register map.
 */

#include <Arduino.h>
#include "modules/hal/hal.h"
#include "modules/communication/modbus.h"
#include "modules/communication/modbus_register_manager.h"
#include "register_map.h"
#include "multiplexer/Multiplexer.h"
#include "frequency_counter.h"
#include "constants.h"

// Global Multiplexer instance
Multiplexer multiplexer;

// Forward declaration
static void update_modbus_registers();

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
    (void)hal_init();
    
    // Initialize frequency counting system
    if (!frequency_counter_init()) {
        // Handle frequency counter initialization failure
        // Could set error LED or enter safe mode
    }
    
    register_manager_init();
    setup_modbus();
    
    // The Multiplexer constructor handles sonicator initialization.
    // We call begin() here for consistency, though it's a no-op.
    multiplexer.begin();
    
    // Set default values from register map
    auto* map = register_manager_get_map();
    map->global_control.global_enable = 1;
    for (int i = 0; i < 4; ++i) {
        map->sonicators[i].amplitude_setpoint = 50;
    }
    multiplexer.setAmplitude(map->sonicators[0].amplitude_setpoint);
}

void loop() {
    static unsigned long last_modbus_process_time = 0;
    static unsigned long last_multiplexer_update_time = 0;
    static unsigned long last_modbus_sync_time = 0;

    
    unsigned long current_time = millis();
    
    if (current_time - last_modbus_process_time >= MODBUS_PROCESS_INTERVAL_MS) {
        (void)modbus_process();
        last_modbus_process_time = current_time;
    }
    
    if (current_time - last_multiplexer_update_time >= MULTIPLEXER_UPDATE_INTERVAL_MS) {
        multiplexer.update();
        last_multiplexer_update_time = current_time;
    }
    
    if (current_time - last_modbus_sync_time >= MODBUS_SYNC_INTERVAL_MS) {
        update_modbus_registers();
        last_modbus_sync_time = current_time;
    }
    
    delay(1);
}

/**
 * @brief Synchronizes the Multiplexer state with the MODBUS register map.
 * 
 * Reads control values from MODBUS and applies them to the multiplexer.
 * Reads status values from the multiplexer and writes them to MODBUS.
 */
static void update_modbus_registers() {
    modbus_register_map_t* map = register_manager_get_map();
    if (!map) return;

    uint16_t active_mask = 0;
    uint16_t active_count = 0;

    // Sync each sonicator
    for (int i = 0; i < NUM_SONICATORS; ++i) {
        // From MODBUS to Multiplexer (Control)
        if (map->sonicators[i].start_stop == 1) {
            multiplexer.start(i);
            map->sonicators[i].start_stop = 0; // Clear command
        } else if (map->sonicators[i].start_stop == 2) { // Using 2 for stop
            multiplexer.stop(i);
            map->sonicators[i].start_stop = 0; // Clear command
        }

        if (map->sonicators[i].overload_reset == 1) {
            multiplexer.resetOverload(i);
            map->sonicators[i].overload_reset = 0; // Clear command
        }
        multiplexer.setAmplitude(map->sonicators[i].amplitude_setpoint);

        // From Multiplexer to MODBUS (Status)
        const sonicator_status_t* status = multiplexer.getStatus(i);
        if (status) {
            uint16_t flags = 0;
            if (status->state == SONICATOR_STATE_RUNNING) {
                flags |= SON_STATUS_RUNNING;
                active_mask |= (1 << i);
                active_count++;
            }
            if (status->state == SONICATOR_STATE_FAULT) {
                flags |= SON_STATUS_FAULT;
            }
            map->sonicators[i].status_flags = flags;
            map->sonicators[i].power_watts = (uint16_t)status->power_watts;
            map->sonicators[i].frequency_hz = status->frequency_hz;
        }
    }

    // Update global system status
    map->system_status.active_mask = active_mask;
    map->system_status.active_count = active_count;
}
