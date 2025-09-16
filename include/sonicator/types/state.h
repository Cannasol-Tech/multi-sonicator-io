
#ifndef SONICATOR_TYPES_STATE_H
#define SONICATOR_TYPES_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include "errors.h"

/**
 * @brief Sonicator control states
 */
typedef enum {
    SONICATOR_STATE_IDLE = 0,
    SONICATOR_STATE_STARTING,
    SONICATOR_STATE_RUNNING,
    SONICATOR_STATE_STOPPING,
    SONICATOR_STATE_FAULT,
    SONICATOR_STATE_UNKNOWN
} sonicator_state_t;

/**
 * @brief Complete sonicator control and status structure
 */
typedef struct {
    sonicator_state_t state;
    sonicator_state_t previous_state;
    uint32_t state_entry_time;
    
    bool start_requested;
    bool stop_requested;
    bool reset_requested;
    uint8_t amplitude_percent;

    bool is_running;
    bool overload_active;
    bool frequency_locked;
    float power_watts;
    uint16_t frequency_hz;
    
    sonicator_fault_t active_faults;
    uint32_t fault_count;
    uint32_t last_fault_time;
    
    uint32_t last_update_time;
    uint32_t watchdog_last_reset;
    bool safety_override;
    
    uint32_t start_count;
    uint32_t total_runtime_ms;
    uint32_t last_start_time;
} sonicator_status_t;

#endif // SONICATOR_TYPES_STATE_H
