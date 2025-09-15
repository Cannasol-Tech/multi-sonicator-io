/**
 * @file multi_sonicator.h
 * @brief Master coordination for multi-unit sonicator control (Story 4.1)
 */

#ifndef MULTI_SONICATOR_H
#define MULTI_SONICATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MASTER_STATE_IDLE = 0,
    MASTER_STATE_COORDINATED_START = 1,
    MASTER_STATE_RUNNING = 2,
    MASTER_STATE_EMERGENCY_STOP = 3,
    MASTER_STATE_FAULT_ISOLATION = 4
} master_state_t;

typedef struct {
    master_state_t master_state;
    uint8_t active_mask;                 // bitmask of units [b0..b3]
    sonicator_state_t unit_state[4];     // per‑unit states
    uint16_t fault_code;
    uint32_t last_transition_time_ms;
} multi_status_t;

// Lifecycle
bool multi_sonicator_begin(void);

// Commands
bool multi_sonicator_request_coordinated_start(uint8_t unit_mask);
bool multi_sonicator_emergency_stop(void);

// Update and status
master_state_t multi_sonicator_update(void);
const multi_status_t* multi_sonicator_get_status(void);

#ifdef __cplusplus
}
#endif

#endif // MULTI_SONICATOR_H

