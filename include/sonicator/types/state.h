
#ifndef SONICATOR_STATE_H
#define SONICATOR_STATE_H

/**
 * @brief Sonicator control states
 * 
 * State transitions follow safety requirements with proper debouncing:
 * IDLE → STARTING (50ms delay) → RUNNING → STOPPING (100ms) → IDLE
 * Any state → FAULT (immediate) → IDLE (after fault cleared)
 */
typedef enum {
    SONICATOR_STATE_IDLE = 0,       //< Sonicator stopped, ready for commands
    SONICATOR_STATE_STARTING,       //< Start sequence in progress (50ms)
    SONICATOR_STATE_RUNNING,        //< Sonicator operating normally
    SONICATOR_STATE_STOPPING,      //< Stop sequence in progress (100ms)
    SONICATOR_STATE_OVERLOAD,      //< Overload condition detected
    SONICATOR_STATE_FAULT,          //< Fault condition detected
    SONICATOR_STATE_UNKNOWN         //< State unknown (initialization)
} sonicator_state_t;


/**
 * @brief Complete sonicator control and status structure
 */
typedef struct {
    // Current state
    sonicator_state_t state;            //< Current state
    sonicator_state_t previous_state;   //< Previous state (for transitions)
    uint32_t state_entry_time;          //< When current state was entered
    
    // Control settings
    bool stop_requested;                //< Stop command requested
    bool start_requested;               //< Start command requested
    bool reset_requested;               //< Reset overload requested
    uint8_t amplitude_percent;          //< Amplitude setpoint (20-100%)

    
    // Status and telemetry
    bool is_running;                    //< Sonicator is running
    bool overload_active;               //< Overload condition
    bool frequency_locked;              //< Frequency lock status
    float power_watts;                  //< Current power output
    uint16_t frequency_hz;              //< Operating frequency
    
    // Fault tracking
    sonicator_fault_t active_faults;    //< Active fault conditions
    uint32_t fault_count;               //< Total fault events
    uint32_t last_fault_time;           //< Last fault timestamp
    
    // Safety and timing
    uint32_t last_update_time;          //< Last update() call time
    uint32_t watchdog_last_reset;       //< Watchdog last reset time
    bool safety_override;               //< Emergency safety override
    
    // Statistics
    uint32_t start_count;               //< Number of start cycles
    uint32_t total_runtime_ms;          //< Total running time
    uint32_t last_start_time;           //< Last start timestamp
} sonicator_state_t;

#endif // SONICATOR_STATE_H
