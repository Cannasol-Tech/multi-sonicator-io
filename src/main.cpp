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
    register_manager_init();
    setup_modbus();

    auto* map = register_manager_get_map();
    map->global_control.global_enable = 1;
    map->sonicators[3].amplitude_setpoint = 50;
}

void loop() {
    (void)modbus_process();
    son4_service();
    delay(10);
}
