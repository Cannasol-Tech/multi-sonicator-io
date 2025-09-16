
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
* @brief Sonicator control structure
* @details This structure contains the control inputs for the sonicator.  These are 
* the values that are written to the sonicator by the controller.
* @note The control values are not the same as the status values.  The control values are 
* the commands that are sent to the sonicator to change it's state.  The status values are 
* the actual state of the sonicator.'
* @param start_control Sonicator Start Control (true/false) Read from REG_SON#_CTRL_START
* @param reset_control Sonicator Reset Control (true/false) Read from REG_SON#_CTRL_RESET
* @param safety_override Sonicator Safety Override (true/false) Read from REG_SON#_CTRL_SAFETY_OVERRIDE
* @param amplitude_control Sonicator Amplitude Control (0-100) Read from REG_SON#_CTRL_AMPLITUDE
* @note The amplitude_control value is not the same as the amplitude_actual value. 
*/
typedef struct sonicator_control_s {
    bool start_control;
    bool reset_control;
    uint8_t amplitude_control;
} sonicator_control_t;

/**
* @brief Sonicator state machine structure
* @details This structure contains the state machine for the sonicator.  This is used to 
* track the state of the sonicator and the time it spent in each state.
* @param state Current state of the sonicator
* @param previous_state Previous state of the sonicator
* @param state_entry_time Time the sonicator entered the current state
*/
typedef struct sonicator_state_ {
    sonicator_state_t state;
    sonicator_state_t previous_state;
    uint32_t state_entry_time;
} sonicator_state_machine_t;

/**
* @brief Sonicator historical status structure
* @details This structure contains the historical status of the sonicator.  This is used to 
* track the history of the sonicator state changes.
* @param start_count Start count
* @param last_start_time Last start time
* @param total_runtime_ms Total runtime in milliseconds
* @note THIS IS A BACKLOG ITEM TO BE IMPLEMENTED AFTER MVP
* @see sonicator_state_machine_t
*/
typedef struct sonicator_history_s {
    uint32_t start_count;
    uint32_t last_start_time;
    uint32_t total_runtime_ms;
} sonicator_history_t;

/**
* @brief Sonicator status structure
* @details This structure contains the status of the sonicator.  These are the values that 
* are read from the sonicator by the controller.
* @param state_machine Sonicator state machine structure
* @param power_raw_adc Raw power reading from the sonicator
* @param overload_active Overload active flag
* @param frequency_locked Frequency locked flag

*/
typedef struct sonicator_status_s {
    bool is_running;
    uint16_t frequency_hz;
    bool overload_active;
    bool frequency_locked;
    uint32_t fault_count;
    uint16_t power_raw_adc;
    uint32_t last_fault_time;
    uint8_t amplitude_actual;
    sonicator_state_machine_t state_machine;
} sonicator_status_t;


/**
 * @brief Complete sonicator control and status structure
 * @details This structure contains the control and status of the sonicator.  These are the values that 
 * are read from the sonicator by the controller.
 * @param status Sonicator status structure
 * @param control Sonicator control structure
 * @param history Sonicator historical status structure
 * @param active_faults Active faults
 * @param safety_override Safety override flag
 * @param last_start_time Last start time
 * @param last_update_time Last update time
 * @param watchdog_last_reset Last watchdog reset time
 */
typedef struct sonicator_s {
    sonicator_status_t status;
    sonicator_control_t control;
    sonicator_history_t history;
    sonicator_fault_t active_faults;
   
    bool safety_override;

    uint32_t last_start_time;
    uint32_t last_update_time;
    uint32_t watchdog_last_reset;
} sonicator_t;

#endif // SONICATOR_TYPES_STATE_H
