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

/** Lifecycle */
bool multi_sonicator_begin(void);

/**
 * Request a coordinated start for a set of units.
 *
 * Mask semantics: bit 0→unit 1, bit 1→unit 2, bit 2→unit 3, bit 3→unit 4.
 * Units in the mask transition to STARTING and then RUNNING during update().
 * This coexists with per‑unit commands and represents the intended active set.
 *
 * Returns false on invalid mask or when in EMERGENCY_STOP.
 */
bool multi_sonicator_request_coordinated_start(uint8_t unit_mask);
/**
 * Request a coordinated stop for a set of units.
 *
 * Units in the intersection of (unit_mask ∧ active_mask) transition to
 * STOPPING and then STOPPED during update().
 */
bool multi_sonicator_request_coordinated_stop(uint8_t unit_mask);

/**
 * Request START for a single unit (0..3 index, unit 1 = index 0).
 *
 * Behavior:
 * - If unit is STOPPING, this aborts STOP and moves unit to STARTING.
 * - If unit is STOPPED, moves to STARTING and sets active_mask bit.
 * - If unit is RUNNING/STARTING, no change (idempotent).
 * - If in FAULT/OVERLOAD/UNKNOWN, returns false (safety gate).
 */
bool multi_sonicator_request_unit_start(uint8_t unit_index);

/**
 * Request STOP for a single unit (0..3 index, unit 1 = index 0).
 *
 * Behavior:
 * - If unit is STARTING, this aborts START and moves unit to STOPPING.
 * - If unit is RUNNING, moves to STOPPING.
 * - If unit is STOPPED/STOPPING, no change (idempotent).
 */
bool multi_sonicator_request_unit_stop(uint8_t unit_index);
/**
 * Report an error condition for a unit and force it to an error state.
 *
 * This is used to reflect per‑unit failure paths (e.g., overload) so that
 * partial success semantics are maintained: other units keep running while
 * the failed unit enters an error state and is removed from the active set.
 *
 * When overload=true, the unit enters SONICATOR_STATE_OVERLOAD; otherwise it
 * enters SONICATOR_STATE_FAULT. The unit is cleared from the active mask.
 */
bool multi_sonicator_report_unit_fault(uint8_t unit_index, bool overload);
bool multi_sonicator_emergency_stop(void);

/** Update and status */
master_state_t multi_sonicator_update(void);
const multi_status_t* multi_sonicator_get_status(void);

/** Optional coordination aids (start timeout path) */
/**
 * Inhibit automatic promotion from STARTING→RUNNING for a unit. When inhibited,
 * a unit that remains in STARTING beyond the configured timeout will transition
 * to an error state automatically.
 */
void multi_sonicator_set_start_inhibit(uint8_t unit_index, bool inhibit);

/**
 * Explicitly confirm that a unit has successfully started. This immediately
 * promotes STARTING→RUNNING and clears any pending start timeout.
 */
bool multi_sonicator_confirm_unit_started(uint8_t unit_index);

#ifdef __cplusplus
}
#endif

#endif // MULTI_SONICATOR_H
