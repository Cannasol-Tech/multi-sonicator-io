/**
 * @file sonicator/interface.h
 * @brief Sonicator IO Control and Monitoring API for Epic 1 Story 1.4
 * @note Developed by Cannasol Technologies
 * @project Multi Sonicator IO Controller
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

 #ifndef SONICATOR_INTERFACE_H
#define SONICATOR_INTERFACE_H
 
#include <stdint.h>
#include <stdbool.h>
#include "modules/hal/hal.h"
#include "sonicator/types/state.h"
#include "sonicator/types/errors.h"
#include "sonicator/types/control.h"
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 // ============================================================================
 // >•Cannasol•Technologies•< STATE MACHINE DEFINITIONS
 // ============================================================================
 
 
 // ============================================================================
 // TIMING CONSTANTS (Story 1.4 Safety Requirements)
 // ============================================================================

 #define SONICATOR_STOP_DELAY_MS         100     //< Stop sequence delay
 #define SONICATOR_START_DELAY_MS        50      //< Start sequence delay
 #define SONICATOR_RESET_PULSE_MS        20      //< Reset pulse duration
 #define SONICATOR_COMM_TIMEOUT_MS       2000    //< Communication timeout
 #define SONICATOR_FAULT_DEBOUNCE_MS     10      //< Fault signal debounce
 #define SONICATOR_WATCHDOG_TIMEOUT_MS   1000    //< Watchdog timeout
 
 // Amplitude limits (CT2000 specifications)
 #define SONICATOR_MIN_AMPLITUDE_PERCENT 20      //< Minimum amplitude (20%)
 #define SONICATOR_MAX_AMPLITUDE_PERCENT 100     //< Maximum amplitude (100%)
 
 // ============================================================================
 // SONICATOR CONTROL STRUCTURE
 // ============================================================================
 
 #ifdef __cplusplus
 }
 
 /**
  * @brief Sonicator Controller Class
  * 
  * Encapsulates single sonicator control with proper state machine,
  * safety requirements, and HAL integration.
  */
 class CT2000Sonicator {
 public:
     /**
      * @brief Initialize CT2000 sonicator control and monitoring system
      * 
      * Sets up state machine, HAL interfaces, and safety systems.
      * Must be called before any other sonicator control functions.
      * 
      * @return true if initialization successful, false on error
      */
     bool inline begin(void) { return sonicator_begin(); }
 
     /**
      * @brief Start sonicator operation
      * 
      * Commands sonicator to start with current amplitude setting.
      * State transition: IDLE → STARTING → RUNNING
      * 
      * @return true if start command accepted, false if unable to start
      */
     bool start(void);
 
     /**
      * @brief Stop sonicator operation
      * 
      * Commands sonicator to stop gracefully.
      * State transition: RUNNING → STOPPING → IDLE
      * 
      * @return true if stop command accepted, false if already stopped
      */
     bool stop(void);
 
     /**
      * @brief Set sonicator amplitude
      * 
      * Sets amplitude setpoint for sonicator operation.
      * Valid range: 20-100% (CT2000 specification)
      * 
      * @param amplitude_percent Amplitude setting (20-100%)
      * @return true if amplitude set successfully, false if out of range
      */
     bool setAmplitude(uint8_t amplitude_percent);
 
     /**
      * @brief Reset overload condition
      * 
      * Sends reset pulse to clear overload fault condition.
      * Only effective when sonicator is in FAULT state due to overload.
      * 
      * @return true if reset command sent, false if not applicable
      */
     bool resetOverload(void);
 
     /**
      * @brief Emergency stop (immediate)
      * 
      * Immediately stops sonicator regardless of current state.
      * Used for safety-critical situations.
      * 
      * @return true always (cannot fail)
      */
     bool emergencyStop(void);
 
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
     sonicator_state_t update(void);
 
     /**
      * @brief Get current sonicator status
      * 
      * Returns complete sonicator status structure for MODBUS or diagnostics.
      * Safe to call from any context.
      * 
      * @return Pointer to current status structure (read-only)
      */
     const sonicator_state_t* getStatus(void) const;
 
     /**
      * @brief Get current state as string (for debugging)
      * 
      * @param state State to convert
      * @return String representation of state
      */
     static const char* stateToString(sonicator_state_t state);
 
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
     bool isSafe(void) const;
 
     /**
      * @brief Reset all statistics and counters
      * 
      * Clears runtime counters, fault counts, and statistics.
      * Does not affect current operational state.
      */
     void resetStatistics(void);
 
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
    inline constexpr bool forceState(sonicator_state_t* new_state) { state_control_ = new_state; return true; }
 
     /**
      * @brief Inject fault condition (testing only)
      * 
      * **WARNING**: For testing only. Injects artificial fault condition.
      * 
      * @param fault_mask Fault condition to inject
      * @return true if fault injected
      */
     bool injectFault(sonicator_fault_t* fault_mask) { state_control_->active_faults = fault_mask; return true; }
 
     /**
      * @brief Enable/disable simulation mode
      * 
      * When enabled, bypasses HAL calls for unit testing.
      * 
      * @param enable true to enable simulation, false for normal operation
      */
     void setSimulationMode(bool enable);
 
 private:
     sonicator_state_t state_control_;
     bool simulation_mode_;
 };
 
 #endif // SONICATOR_CONTROL_H
 