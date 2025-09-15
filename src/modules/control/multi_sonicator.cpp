/**
 * @file multi_sonicator.cpp
 * @brief Minimal master coordination state machine for 4 units (Story 4.1)
 */

#include "multi_sonicator.h"
#include <string.h>

static multi_status_t g_multi = {
    MASTER_STATE_IDLE,
    0u,
    { SONICATOR_STATE_STOPPED, SONICATOR_STATE_STOPPED, SONICATOR_STATE_STOPPED, SONICATOR_STATE_STOPPED },
    0u,
    0u
};

bool multi_sonicator_begin(void) {
    g_multi.master_state = MASTER_STATE_IDLE;
    g_multi.active_mask = 0u;
    g_multi.fault_code = 0u;
    for (int i = 0; i < 4; ++i) {
        g_multi.unit_state[i] = SONICATOR_STATE_STOPPED;
    }
    g_multi.last_transition_time_ms = 0u;
    return true;
}

bool multi_sonicator_request_coordinated_start(uint8_t unit_mask) {
    if (unit_mask == 0u) return false;
    if (g_multi.master_state == MASTER_STATE_EMERGENCY_STOP) return false;

    g_multi.active_mask = (uint8_t)(unit_mask & 0x0Fu);
    g_multi.master_state = MASTER_STATE_COORDINATED_START;
    g_multi.last_transition_time_ms++;

    for (int i = 0; i < 4; ++i) {
        if (g_multi.active_mask & (1u << i)) {
            g_multi.unit_state[i] = SONICATOR_STATE_STARTING;
        }
    }
    return true;
}

bool multi_sonicator_emergency_stop(void) {
    g_multi.master_state = MASTER_STATE_EMERGENCY_STOP;
    g_multi.last_transition_time_ms++;
    for (int i = 0; i < 4; ++i) {
        g_multi.unit_state[i] = SONICATOR_STATE_STOPPING;
    }
    return true;
}

master_state_t multi_sonicator_update(void) {
    switch (g_multi.master_state) {
        case MASTER_STATE_COORDINATED_START:
            for (int i = 0; i < 4; ++i) {
                if (g_multi.active_mask & (1u << i)) {
                    g_multi.unit_state[i] = SONICATOR_STATE_RUNNING;
                }
            }
            g_multi.master_state = MASTER_STATE_RUNNING;
            g_multi.last_transition_time_ms++;
            break;
        case MASTER_STATE_EMERGENCY_STOP:
            for (int i = 0; i < 4; ++i) {
                g_multi.unit_state[i] = SONICATOR_STATE_STOPPED;
            }
            break;
        default:
            break;
    }
    return g_multi.master_state;
}

const multi_status_t* multi_sonicator_get_status(void) {
    return &g_multi;
}

