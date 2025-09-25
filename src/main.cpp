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
#include "modbus.h"
#include "modbus_register_manager.h"
#include "register_map.h"
#include "multiplexer/Multiplexer.h"
#include "frequency_counter.h"
#include "constants.h"


// Global Multiplexer instance
SonicMultiplexer multiplexer(4);

static void setup_modbus(void) {

    register_manager_init();

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

    // Initialize MODBUS interface
    setup_modbus();
    
    // The Multiplexer constructor handles sonicator initialization.
    // We call begin() here for consistency, though it's a no-op.
    multiplexer.begin();
    
    // Set default values from register map
    auto* map = register_manager_get_map();

    // Enable global control
   map->global_control.global_enable = 1;

   // Set initial amplitude
   multiplexer.setAmplitude(map->global_control.global_amplitude_sp);
}

void loop() {
    static unsigned long last_modbus_process_time = 0;
    static unsigned long last_multiplexer_update_time = 0;

    unsigned long current_time = millis();
    
    if (current_time - last_modbus_process_time >= MODBUS_PROCESS_INTERVAL_MS) {
        (void)modbus_process();
        last_modbus_process_time = current_time;
    }
    
    if (current_time - last_multiplexer_update_time >= MULTIPLEXER_UPDATE_INTERVAL_MS) {
        multiplexer.update();
        last_multiplexer_update_time = current_time;
    }
    
    delay(1);
}

