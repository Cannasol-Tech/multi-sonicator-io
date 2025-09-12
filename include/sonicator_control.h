/**
 * @file sonicator_control.h
 * @brief Single Sonicator Control API for Epic 1 Story 1.4
 * @author Cannasol Technologies
 * @date 2025-09-12
 * @version 1.0.0
 * 
 * @details
 * Provides clean API for single sonicator control with proper state machine,
 * safety requirements, and HAL integration. Designed for Story 1.4:
 * Single Sonicator Control Implementation.
 * 
 * Features:
 * - State machine with proper safety transitions
 * - Non-blocking update() for 1kHz main loop
 * - HAL abstraction for testability
 * - MODBUS register integration ready
 * - Comprehensive error handling
 */

#ifndef SONICATOR_CONTROL_H
#define SONICATOR_CONTROL_H

#include <stdint.h>
#include <stdbool.h>
#include "modules/hal/hal.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// SONICATOR STATE MACHINE DEFINITIONS
// ============================================================================

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
    SONICATOR_STATE_FAULT,          //< Fault condition detected
    SONICATOR_STATE_UNKNOWN         //< State unknown (initialization)
} sonicator_state_t;

/**
 * @brief Sonicator control commands
 */
typedef enum {
    SONICATOR_CMD_NONE = 0,         //< No command pending
    SONICATOR_CMD_START,            //< Start sonicator
    SONICATOR_CMD_STOP,             //< Stop sonicator  
    SONICATOR_CMD_RESET_OVERLOAD,   //< Reset overload condition
    SONICATOR_CMD_EMERGENCY_STOP    //< Emergency stop (immediate)
} sonicator_command_t;

/**
 * @brief Sonicator fault conditions
 */
typedef enum {
    SONICATOR_FAULT_NONE = 0,       //< No fault
    SONICATOR_FAULT_OVERLOAD = 0x01,    //< Overload detected
    SONICATOR_FAULT_FREQ_UNLOCK = 0x02, //< Frequency lock lost
    SONICATOR_FAULT_COMM_TIMEOUT = 0x04, //< Communication timeout
    SONICATOR_FAULT_HARDWARE = 0x08,     //< Hardware fault
    SONICATOR_FAULT_WATCHDOG = 0x10      //< Watchdog timeout
} sonicator_fault_t;

// ============================================================================
// TIMING CONSTANTS (Story 1.4 Safety Requirements)
// ============================================================================

#define SONICATOR_START_DELAY_MS        50      //< Start sequence delay
#define SONICATOR_STOP_DELAY_MS         100     //< Stop sequence delay
#define SONICATOR_RESET_PULSE_MS        20      //< Reset pulse duration
#define SONICATOR_FAULT_DEBOUNCE_MS     10      //< Fault signal debounce
#define SONICATOR_WATCHDOG_TIMEOUT_MS   1000    //< Watchdog timeout
#define SONICATOR_COMM_TIMEOUT_MS       2000    //< Communication timeout

// Amplitude limits (CT2000 specifications)
#define SONICATOR_MIN_AMPLITUDE_PERCENT 20      //< Minimum amplitude (20%)
#define SONICATOR_MAX_AMPLITUDE_PERCENT 100     //< Maximum amplitude (100%)

// ============================================================================
// SONICATOR CONTROL STRUCTURE
// ============================================================================

/**
 * @brief Complete sonicator control and status structure
 */
typedef struct {
    // Current state
    sonicator_state_t state;            //< Current state
    sonicator_state_t previous_state;   //< Previous state (for transitions)
    uint32_t state_entry_time;          //< When current state was entered
    
    // Control settings
    uint8_t amplitude_percent;          //< Amplitude setpoint (20-100%)
    bool start_requested;               //< Start command requested
    bool stop_requested;                //< Stop command requested
    bool reset_requested;               //< Reset overload requested
    
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
} sonicator_state_control_t;

// ============================================================================
// PUBLIC API FUNCTIONS
// ============================================================================

/**
 * @brief Initialize sonicator control system
 * 
 * Sets up state machine, HAL interfaces, and safety systems.
 * Must be called before any other sonicator control functions.
 * 
 * @return true if initialization successful, false on error
 */
bool sonicator_begin(void);

/**
 * @brief Start sonicator operation
 * 
 * Commands sonicator to start with current amplitude setting.
 * State transition: IDLE → STARTING → RUNNING
 * 
 * @return true if start command accepted, false if unable to start
 */
bool sonicator_start(void);

/**
 * @brief Stop sonicator operation
 * 
 * Commands sonicator to stop gracefully.
 * State transition: RUNNING → STOPPING → IDLE
 * 
 * @return true if stop command accepted, false if already stopped
 */
bool sonicator_stop(void);

/**
 * @brief Set sonicator amplitude
 * 
 * Sets amplitude setpoint for sonicator operation.
 * Valid range: 20-100% (CT2000 specification)
 * 
 * @param amplitude_percent Amplitude setting (20-100%)
 * @return true if amplitude set successfully, false if out of range
 */
bool sonicator_set_amplitude(uint8_t amplitude_percent);

/**
 * @brief Reset overload condition
 * 
 * Sends reset pulse to clear overload fault condition.
 * Only effective when sonicator is in FAULT state due to overload.
 * 
 * @return true if reset command sent, false if not applicable
 */
bool sonicator_reset_overload(void);

/**
 * @brief Emergency stop (immediate)
 * 
 * Immediately stops sonicator regardless of current state.
 * Used for safety-critical situations.
 * 
 * @return true always (cannot fail)
 */
bool sonicator_emergency_stop(void);

/**
 * @brief Main control update function
 * 
 * **CRITICAL**: Must be called at 1kHz (every 1ms) from main loop.
 * Handles state machine transitions, safety monitoring, and telemetry updates.
 * 
 * Non-blocking execution guaranteed <100μs typical.
 * 
 * @return Current sonicator state after update
 */
sonicator_state_t sonicator_update(void);

/**
 * @brief Get current sonicator status
 * 
 * Returns complete sonicator status structure for MODBUS or diagnostics.
 * Safe to call from any context.
 * 
 * @return Pointer to current status structure (read-only)
 */
const sonicator_state_control_t* sonicator_get_status(void);

/**
 * @brief Get current state as string (for debugging)
 * 
 * @param state State to convert
 * @return String representation of state
 */
const char* sonicator_state_to_string(sonicator_state_t state);

/**
 * @brief Check if sonicator is in safe state
 * 
 * Verifies all safety conditions are met:
 * - No active faults
 * - Watchdog not expired
 * - Communication active
 * - Hardware responding
 * 
 * @return true if safe, false if safety concern exists
 */
bool sonicator_is_safe(void);

/**
 * @brief Reset all statistics and counters
 * 
 * Clears runtime counters, fault counts, and statistics.
 * Does not affect current operational state.
 */
void sonicator_reset_statistics(void);

// ============================================================================
// ADVANCED FUNCTIONS (for testing and diagnostics)
// ============================================================================

/**
 * @brief Force state transition (testing only)
 * 
 * **WARNING**: Bypasses safety checks. Use only for unit testing.
 * 
 * @param new_state Target state
 * @return true if transition completed
 */
bool sonicator_force_state(sonicator_state_t new_state);

/**
 * @brief Inject fault condition (testing only)
 * 
 * **WARNING**: For testing only. Injects artificial fault condition.
 * 
 * @param fault_mask Fault condition to inject
 * @return true if fault injected
 */
bool sonicator_inject_fault(sonicator_fault_t fault_mask);

/**
 * @brief Enable/disable simulation mode
 * 
 * When enabled, bypasses HAL calls for unit testing.
 * 
 * @param enable true to enable simulation, false for normal operation
 */
void sonicator_set_simulation_mode(bool enable);

#ifdef __cplusplus
}
#endif

#endif // SONICATOR_CONTROL_H
