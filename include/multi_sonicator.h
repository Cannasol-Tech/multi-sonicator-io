#ifndef MULTI_SONICATOR_H
#define MULTI_SONICATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "multiplexer/master_state.h"
#include "sonicator/types/state.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Aggregated status for the multi‑sonicator coordination system
 */
typedef struct multi_status_s {
    master_state_t      master_state;              /**< Master coordination state */
    uint8_t             active_mask;               /**< Bitmask of active units (bits 0..3) */
    sonicator_state_t   unit_state[4];             /**< Per‑unit states */
    uint16_t            fault_code;                /**< Optional system fault code (0=none) */
    uint32_t            last_transition_time_ms;   /**< Monotonic tick for state changes */
} multi_status_t;

// Lifecycle
bool            multi_sonicator_begin(void);
master_state_t  multi_sonicator_update(void);
const multi_status_t* multi_sonicator_get_status(void);

// Coordinated operations
bool multi_sonicator_request_coordinated_start(uint8_t unit_mask);
bool multi_sonicator_request_coordinated_stop(uint8_t unit_mask);
bool multi_sonicator_emergency_stop(void);

// Per‑unit operations
bool multi_sonicator_request_unit_start(uint8_t unit_index);
bool multi_sonicator_request_unit_stop(uint8_t unit_index);
bool multi_sonicator_report_unit_fault(uint8_t unit_index, bool overload);

// Start inhibit and confirmation helpers
void multi_sonicator_set_start_inhibit(uint8_t unit_index, bool inhibit);
bool multi_sonicator_confirm_unit_started(uint8_t unit_index);

#ifdef __cplusplus
}
#endif

#endif // MULTI_SONICATOR_H

