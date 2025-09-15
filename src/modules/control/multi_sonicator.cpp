/**
 * @file multi_sonicator.cpp
 * @brief Minimal master coordination state machine for 4 units (Story 4.1)
 */

#include "multi_sonicator.h"
#include <string.h>

/**
 * @brief Global multi-sonicator status structure
 */
static multi_status_t g_multi = {
    MASTER_STATE_IDLE,
    0u,
    { SONICATOR_STATE_STOPPED, SONICATOR_STATE_STOPPED, SONICATOR_STATE_STOPPED, SONICATOR_STATE_STOPPED },
    0u,
    0u
};

// Start timeout configuration (ticks advance on each update call)
#ifndef START_TIMEOUT_TICKS
#define START_TIMEOUT_TICKS 5
#endif

static uint8_t g_start_inhibit_mask = 0u;      // per-unit inhibit for auto-promotion
static uint8_t g_start_timeout_ticks[4] = {0, 0, 0, 0};

/**
 * @brief Initialize the multi-sonicator coordination system
 * @return true on successful initialization
 */
bool multi_sonicator_begin(void) {
    g_multi.master_state = MASTER_STATE_IDLE;
    g_multi.active_mask = 0u;
    g_multi.fault_code = 0u;
    for (int i = 0; i < 4; ++i) {
        g_multi.unit_state[i] = SONICATOR_STATE_STOPPED;
    }
    g_multi.last_transition_time_ms = 0u;
    g_start_inhibit_mask = 0u;
    for (int i = 0; i < 4; ++i) {
        g_start_timeout_ticks[i] = 0u;
    }
    return true;
}

/**
 * @brief Request coordinated start of specified sonicator units
 * @param unit_mask Bitmask of units to start (bits 0-3 for units 0-3)
 * @return true if request was accepted, false otherwise
 */
bool multi_sonicator_request_coordinated_start(uint8_t unit_mask) {
    if (unit_mask == 0u) return false;
    if (g_multi.master_state == MASTER_STATE_EMERGENCY_STOP) return false;

    g_multi.active_mask = (uint8_t)(unit_mask & 0x0Fu);
    g_multi.master_state = MASTER_STATE_COORDINATED_START;
    g_multi.last_transition_time_ms++;

    for (int i = 0; i < 4; ++i) {
        if (g_multi.active_mask & (1u << i)) {
            g_multi.unit_state[i] = SONICATOR_STATE_STARTING;
            g_start_timeout_ticks[i] = START_TIMEOUT_TICKS;
        }
    }
    return true;
}

/**
 * @brief Initiate emergency stop of all sonicator units
 * @return true on successful emergency stop initiation
 */
bool multi_sonicator_emergency_stop(void) {
    g_multi.master_state = MASTER_STATE_EMERGENCY_STOP;
    g_multi.last_transition_time_ms++;
    for (int i = 0; i < 4; ++i) {
        g_multi.unit_state[i] = SONICATOR_STATE_STOPPING;
    }
    return true;
}

/**
 * @brief Request coordinated stop of specified sonicator units
 * @param unit_mask Bitmask of units to stop (bits 0-3 for units 0-3)
 * @return true if request was accepted, false otherwise
 */
bool multi_sonicator_request_coordinated_stop(uint8_t unit_mask) {
    if (unit_mask == 0u) return false;
    if (g_multi.master_state == MASTER_STATE_EMERGENCY_STOP) return false;

    // Only apply stop to currently active units in the requested mask
    uint8_t target = (uint8_t)(unit_mask & g_multi.active_mask & 0x0Fu);
    if (target == 0u) return true; // nothing to do

    for (int i = 0; i < 4; ++i) {
        if (target & (1u << i)) {
            // Initiate coordinated stop for this unit
            g_multi.unit_state[i] = SONICATOR_STATE_STOPPING;
            g_start_timeout_ticks[i] = 0u; // clear any pending start timeout
        }
    }
    g_multi.last_transition_time_ms++;
    return true;
}

/**
 * @brief Request START for a single unit (index 0..3). Aborts STOP for that unit.
 * @param unit_index Zero-based unit index (0..3)
 * @return true if accepted; false if invalid index or not allowed (e.g., fault)
 */
bool multi_sonicator_request_unit_start(uint8_t unit_index) {
    if (unit_index > 3) return false;
    if (g_multi.master_state == MASTER_STATE_EMERGENCY_STOP) return false;

    sonicator_state_t st = g_multi.unit_state[unit_index];
    switch (st) {
        case SONICATOR_STATE_STOPPING:
        case SONICATOR_STATE_STOPPED:
            g_multi.unit_state[unit_index] = SONICATOR_STATE_STARTING;
            g_multi.active_mask = (uint8_t)(g_multi.active_mask | (1u << unit_index));
            g_multi.last_transition_time_ms++;
            // Master state reflects presence of starting/running units on update()
            return true;
        case SONICATOR_STATE_RUNNING:
        case SONICATOR_STATE_STARTING:
            return true; // idempotent
        case SONICATOR_STATE_OVERLOAD:
        case SONICATOR_STATE_FAULT:
        case SONICATOR_STATE_UNKNOWN:
            return false; // require explicit clear before start
    }
    return false;
}

/**
 * @brief Request STOP for a single unit (index 0..3). Aborts START for that unit.
 * @param unit_index Zero-based unit index (0..3)
 * @return true if accepted; false if invalid index
 */
bool multi_sonicator_request_unit_stop(uint8_t unit_index) {
    if (unit_index > 3) return false;

    sonicator_state_t st = g_multi.unit_state[unit_index];
    switch (st) {
        case SONICATOR_STATE_RUNNING:
        case SONICATOR_STATE_STARTING:
            g_multi.unit_state[unit_index] = SONICATOR_STATE_STOPPING;
            g_multi.last_transition_time_ms++;
            return true;
        case SONICATOR_STATE_STOPPING:
        case SONICATOR_STATE_STOPPED:
            return true; // idempotent
        case SONICATOR_STATE_OVERLOAD:
        case SONICATOR_STATE_FAULT:
        case SONICATOR_STATE_UNKNOWN:
            return true; // already not running; treat as no-op
    }
    return false;
}

/**
 * @brief Report a unit fault/overload and remove it from the active set.
 */
bool multi_sonicator_report_unit_fault(uint8_t unit_index, bool overload) {
    if (unit_index > 3) return false;
    // Transition unit to error state and clear from active set
    g_multi.unit_state[unit_index] = overload ? SONICATOR_STATE_OVERLOAD : SONICATOR_STATE_FAULT;
    g_multi.active_mask = (uint8_t)(g_multi.active_mask & (uint8_t)~(1u << unit_index));
    g_multi.last_transition_time_ms++;
    return true;
}

void multi_sonicator_set_start_inhibit(uint8_t unit_index, bool inhibit) {
    if (unit_index > 3) return;
    if (inhibit) {
        g_start_inhibit_mask = (uint8_t)(g_start_inhibit_mask | (1u << unit_index));
        // If entering inhibit while already STARTING, (re)start timeout window
        if (g_multi.unit_state[unit_index] == SONICATOR_STATE_STARTING) {
            g_start_timeout_ticks[unit_index] = START_TIMEOUT_TICKS;
        }
    } else {
        g_start_inhibit_mask = (uint8_t)(g_start_inhibit_mask & (uint8_t)~(1u << unit_index));
    }
}

bool multi_sonicator_confirm_unit_started(uint8_t unit_index) {
    if (unit_index > 3) return false;
    if (g_multi.unit_state[unit_index] == SONICATOR_STATE_STARTING) {
        g_multi.unit_state[unit_index] = SONICATOR_STATE_RUNNING;
        g_start_timeout_ticks[unit_index] = 0u;
        g_start_inhibit_mask = (uint8_t)(g_start_inhibit_mask & (uint8_t)~(1u << unit_index));
        return true;
    }
    return (g_multi.unit_state[unit_index] == SONICATOR_STATE_RUNNING);
}

/**
 * @brief Update the multi-sonicator state machine
 * @return Current master state after update
 */
master_state_t multi_sonicator_update(void) {
    // Emergency stop dominates normal progress
    if (g_multi.master_state == MASTER_STATE_EMERGENCY_STOP) {
        for (int i = 0; i < 4; ++i) {
            g_multi.unit_state[i] = SONICATOR_STATE_STOPPED;
        }
        return g_multi.master_state;
    }

    // Progress per‑unit transitions with start-timeout path
    uint8_t new_mask = g_multi.active_mask;
    bool any_starting = false;
    bool any_running = false;
    for (int i = 0; i < 4; ++i) {
        if (g_multi.unit_state[i] == SONICATOR_STATE_STARTING) {
            any_starting = true;
            // If not inhibited, auto-promote to RUNNING
            if ((g_start_inhibit_mask & (1u << i)) == 0u) {
                g_multi.unit_state[i] = SONICATOR_STATE_RUNNING;
                any_running = true;
                g_start_timeout_ticks[i] = 0u;
            } else {
                // Inhibited: count down timeout, then fault
                if (g_start_timeout_ticks[i] > 0u) {
                    g_start_timeout_ticks[i]--;
                    if (g_start_timeout_ticks[i] == 0u) {
                        g_multi.unit_state[i] = SONICATOR_STATE_FAULT;
                        new_mask = (uint8_t)(new_mask & (uint8_t)~(1u << i));
                    }
                }
            }
        } else if (g_multi.unit_state[i] == SONICATOR_STATE_STOPPING) {
            g_multi.unit_state[i] = SONICATOR_STATE_STOPPED;
            new_mask = (uint8_t)(new_mask & (uint8_t)~(1u << i));
        } else if (g_multi.unit_state[i] == SONICATOR_STATE_RUNNING) {
            any_running = true;
        }
    }

    if (new_mask != g_multi.active_mask) {
        g_multi.active_mask = new_mask;
        g_multi.last_transition_time_ms++;
    }

    // Aggregate master state based on unit activity
    if (any_starting) {
        g_multi.master_state = MASTER_STATE_COORDINATED_START;
    } else if (any_running) {
        g_multi.master_state = MASTER_STATE_RUNNING;
    } else {
        g_multi.master_state = MASTER_STATE_IDLE;
    }
    return g_multi.master_state;
}

/**
 * @brief Get current multi-sonicator status
 * @return Pointer to current status structure (read-only)
 */
const multi_status_t* multi_sonicator_get_status(void) {
    return &g_multi;
}
