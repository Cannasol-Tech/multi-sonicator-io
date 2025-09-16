/**
 * @file sonicator_modbus_bridge.cpp
 * @brief Bridge between sonicator control and MODBUS registers (generalized for 4 units)
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 1.1.0
 *
 * @details
 * Generalized bridge that synchronizes MODBUS registers with the hardware for all
 * four sonicators. It:
 * - Reads per-unit control registers and drives HAL outputs deterministically
 * - Applies write-and-clear overload reset semantics
 * - Publishes per-unit telemetry to the register map at a rate-limited cadence
 * - Maintains a <100ms responsiveness target for control path
 */

#include "modules/communication/modbus_register_manager.h"
#include "modules/communication/modbus_registers.h"
#include "sonicator_modbus_bridge.h"
#include "modules/hal/hal.h"
#include "register_map.h"
#include <Arduino.h>

// ============================================================================
// CONSTANTS
// ============================================================================

#define MODBUS_RESPONSE_TIME_TARGET_MS   100u
#define REGISTER_PUBLISH_INTERVAL_MS     100u
#define CONTROL_TIMEOUT_MS               1000u

// ============================================================================
// PRIVATE STATE
// ============================================================================

static uint32_t last_register_update = 0;
static uint32_t last_control_update = 0;
static uint32_t last_control_update_per_unit[MODBUS_MAX_SONICATORS] = {0, 0, 0, 0};

// ============================================================================
// UTILITIES
// ============================================================================

static inline uint16_t clamp_amplitude_sp(uint16_t sp) {
    if (sp < 20u)  return 20u;
    if (sp > 100u) return 100u;
    return sp;
}

// ============================================================================
// PER-UNIT OPERATIONS
// ============================================================================

static bool apply_control_from_registers(uint8_t unit_id) {
    uint16_t start_stop = 0;
    uint16_t amplitude_sp = 50;
    if (!register_manager_get_sonicator_control(unit_id, &start_stop, &amplitude_sp)) {
        return false;
    }

    bool reset_req = false;
    (void)register_manager_consume_overload_reset(unit_id, &reset_req);

    sonicator_control_t ctrl;
    ctrl.start = (start_stop != 0);
    ctrl.amplitude_percent = (uint8_t)clamp_amplitude_sp(amplitude_sp);
    ctrl.reset_overload = reset_req;

    const uint8_t hal_id = (uint8_t)(unit_id + 1); // HAL is 1-based
    bool action_taken = false;
    if (hal_control_sonicator(hal_id, &ctrl) == HAL_OK) {
        uint32_t now = millis();
        last_control_update_per_unit[unit_id] = now;
        last_control_update = now;
        action_taken = true;
    }
    return action_taken;
}

static void publish_status_to_registers(uint8_t unit_id) {
    const uint8_t hal_id = (uint8_t)(unit_id + 1);
    sonicator_status_t st = {0};
    if (hal_read_sonicator_status(hal_id, &st) != HAL_OK) {
        return;
    }

    modbus_register_map_t* map = register_manager_get_map();
    uint16_t start_stop_cmd = 0;
    uint16_t amplitude_sp = 0;
    if (map) {
        start_stop_cmd = map->sonicators[unit_id].start_stop;
        amplitude_sp   = map->sonicators[unit_id].amplitude_setpoint;
    }

    uint16_t flags = 0;
    if (start_stop_cmd)      flags |= SON_STATUS_RUNNING;
    if (st.overload)         flags |= (uint16_t)(SON_STATUS_OVERLOAD | SON_STATUS_FAULT);
    if (st.frequency_locked) flags |= SON_STATUS_FREQ_LOCK;
    // Optional comm fault synthesis (disabled unless required)
    // if ((millis() - last_control_update_per_unit[unit_id]) > CONTROL_TIMEOUT_MS) {
    //     flags |= SON_STATUS_COMM_FAULT;
    // }

    const uint16_t power_w = (st.power_watts < 0.0f) ? 0u : (uint16_t)st.power_watts;
    const uint16_t freq_hz = st.frequency_hz;
    const uint16_t amp_act = (amplitude_sp >= 20u && amplitude_sp <= 100u) ? amplitude_sp : 0u;

    register_manager_update_sonicator_status(
        unit_id,
        power_w,
        freq_hz,
        amp_act,
        flags
    );
}

// ============================================================================
// PUBLIC API
// ============================================================================

bool sonicator_modbus_bridge_init(void) {
    last_register_update = millis();
    last_control_update = last_register_update;
    for (uint8_t i = 0; i < MODBUS_MAX_SONICATORS; ++i) {
        last_control_update_per_unit[i] = last_control_update;
    }

    modbus_register_map_t* map = register_manager_get_map();
    if (map) {
        for (uint8_t i = 0; i < MODBUS_MAX_SONICATORS; ++i) {
            map->sonicators[i].start_stop = 0;
            map->sonicators[i].amplitude_setpoint = 50;
            map->sonicators[i].overload_reset = 0;
            map->sonicators[i].power_watts = 0;
            map->sonicators[i].frequency_hz = 0;
            map->sonicators[i].status_flags = 0;
            map->sonicators[i].amplitude_actual = 0;
        }
    }
    return true;
}

bool sonicator_modbus_bridge_update(void) {
    uint32_t now = millis();
    bool any_action = false;
    for (uint8_t i = 0; i < MODBUS_MAX_SONICATORS; ++i) {
        if (apply_control_from_registers(i)) {
            any_action = true;
        }
    }
    if ((now - last_register_update) >= REGISTER_PUBLISH_INTERVAL_MS) {
        for (uint8_t i = 0; i < MODBUS_MAX_SONICATORS; ++i) {
            publish_status_to_registers(i);
        }
        last_register_update = now;
    }
    return any_action;
}

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
        *response_time_ms = (now >= last_control_update) ? (now - last_control_update) : 0u;
    }
}

bool sonicator_modbus_bridge_is_responsive(void) {
    uint32_t now = millis();
    uint32_t response_time = (now >= last_control_update) ? (now - last_control_update) : 0u;
    return (response_time < MODBUS_RESPONSE_TIME_TARGET_MS);
}
