/**
 * @file sonicator.h
 * @author Stephen Boyett
 * @brief Enhanced Sonicator Interface Class for CT2000 Control
 * @author Cannasol Technologies
 * @date 2025-06-27
 * @version 1.0.0
 * 
 * Enhanced sonicator interface providing full control and monitoring
 * capabilities for CT2000 sonicators via DB9 interface.
 */

#ifndef SONICATOR_H
#define SONICATOR_H

#include <Arduino.h>
#include <system_config.h>
#include <types.h>
#include "frequency_counter.h"

/**
 * @brief Enhanced Sonicator Interface Class
 * 
 * This class provides complete control and monitoring interface
 * for a single CT2000 sonicator unit via DB9 connector.
 * 
 * Features:
 * - Control Signals: Amplitude, Start/Stop, Overload Reset
 * - Monitoring Signals: 
 * - Full amplitude control (20-100% via 0-10V DAC)
 * - Start/stop control via relay output
 * - Overload reset control
 * - Real-time power monitoring
 * - Frequency monitoring and lock detection
 * - Safety monitoring with response time faster than MODBUS protocol requirements
 * - Runtime statistics and fault logging
 */
class SonicatorInterface {
//@brief Base Sonicator Interface Class

private:
    // Hardware configuration
    uint8_t sonicator_id_;                                  //!< Unique identifier for this sonicator (1-4)

    static sonicator_hardware_config_t hardware_config_;   //!< Hardware pin configuration for all instances
    bool interface_enabled_;                                //!< Interface enable/disable flag
    
    // Current status and state
    sonicator_status_t current_status_;                     //!< Current complete status structure
    sonicator_state_t previous_state_;                      //!< Previous state for transition tracking
    uint32_t state_change_timestamp_;                       //!< Timestamp of last state change
    
    // Control command tracking
    bool start_command_pending_;                            //!< Flag indicating start command is pending
    bool reset_command_pending_;                            //!< Flag indicating reset command is pending
    uint32_t reset_pulse_start_time_;                       //!< Timestamp when reset pulse started
    
    // Monitoring and measurement
    uint16_t frequency_measurement_buffer_[10];             //!< Circular buffer for frequency measurements
    uint8_t frequency_buffer_index_;                        //!< Current index in frequency buffer
    uint32_t last_frequency_measurement_;                   //!< Timestamp of last frequency measurement
    uint32_t last_power_measurement_;                       //!< Timestamp of last power measurement
    
    // Safety monitoring
    bool overload_detected_;                                //!< Flag indicating overload condition detected
    uint32_t last_safety_check_;                            //!< Timestamp of last safety check
    uint32_t overload_detection_time_;                      //!< Timestamp when overload was first detected
    
    // Statistics and monitoring
    uint32_t last_start_time_;                              //!< Timestamp of last start command
    uint32_t total_runtime_ms_;                             //!< Total accumulated runtime in milliseconds
    uint16_t update_call_count_;                            //!< Counter for update() function calls
    uint32_t total_start_cycles_;                           //!< Total number of start cycles executed
    uint32_t total_overload_events_;                        //!< Total number of overload events detected
    uint32_t last_update_timestamp_;                        //!< Timestamp of last update() call
    
    // Error handling and diagnostics
    error_code_t last_error_code_;                          //!< Most recent error code encountered
    uint32_t consecutive_error_count_;                      //!< Number of consecutive errors
    uint32_t last_error_timestamp_;                         //!< Timestamp of last error occurrence
    bool critical_error_flag_;                              //!< Flag indicating critical error state
    
    // Performance monitoring
    uint32_t min_update_time_us_;                           //!< Minimum update() execution time in microseconds
    uint32_t max_update_time_us_;                           //!< Maximum update() execution time in microseconds
    uint32_t avg_update_time_us_;                           //!< Average update() execution time in microseconds
    bool performance_monitoring_enabled_;                   //!< Enable/disable performance monitoring
    
    // Private methods - Core functionality
    void updateHardwareOutputs_() noexcept;
    void readHardwareInputs_() noexcept;
    void updateStateMachine_() noexcept;
    void handleOverloadCondition_() noexcept;
    void updateStatistics_() noexcept;
    void logStateChange_(sonicator_state_t new_state) noexcept;
    
    // Private methods - Measurement and validation
    uint16_t measureFrequency_() const noexcept;
    uint16_t measurePower_() const noexcept;
    bool validateAmplitudeRange_(uint8_t amplitude) const noexcept;
    bool validateSystemState_() const noexcept;
    
    // Private methods - Safety and error handling
    bool checkSafetyConstraints_() const noexcept;
    void handleSystemError_(error_code_t error) noexcept;
    bool isWithinOperatingLimits_() const noexcept;
    
    // Private methods - Timing and performance
    uint32_t getCurrentTimestamp_() const noexcept;
    bool hasTimedOut_(uint32_t start_time, uint32_t timeout_ms) const noexcept;

public:
    /**
     * @brief Constructor
     * @param id Sonicator ID (1-4)
     * @param config Hardware pin configuration
     */
    SonicatorInterface(uint8_t id, const sonicator_hardware_config_t& config);
    
    /**
     * @brief Destructor
     */
    ~SonicatorInterface();
    
    // ========================================================================
    // INITIALIZATION AND CONFIGURATION
    // ========================================================================
    
    /**
     * @brief Initialize the sonicator interface
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Enable or disable the interface
     * @param enabled true to enable, false to disable
     */
    void setEnabled(bool enabled);
    
    /**
     * @brief Check if interface is enabled
     * @return true if enabled
     */
    bool isEnabled() const;
    
    /**
     * @brief Perform self-diagnostic test
     * @return true if all tests pass
     */
    bool performSelfDiagnostic();
    
    // ========================================================================
    // CONTROL FUNCTIONS (New Enhanced Capabilities)
    // ========================================================================
    
    /**
     * @brief Set sonicator amplitude
     * @param percentage Amplitude percentage (20-100%)
     * @return true if command accepted
     */
    bool setAmplitude(uint8_t percentage);
    
    /**
     * @brief Start sonication
     * @return true if command accepted
     */
    bool startSonication();
    
    /**
     * @brief Stop sonication  
     * @return true if command accepted
     */
    bool stopSonication();
    
    /**
     * @brief Reset overload condition
     * @return true if reset command issued
     */
    bool resetOverload();
    
    /**
     * @brief Emergency stop (immediate)
     * @return true if emergency stop executed
     */
    bool emergencyStop();
    
    // ========================================================================
    // MONITORING FUNCTIONS (Enhanced)
    // ========================================================================
    
    /**
     * @brief Check if sonicator is currently running
     * @return true if running
     */
    bool isRunning() const;
    
    /**
     * @brief Check if overload condition exists and throw an interrupt if so
     * @return true if overloaded
     * @note this should throw an interrupt handled by the main loop if an Overload occurs
     */
    bool isOverloaded() const;
    
    /**
     * @brief Check if frequency is locked
     * @return true if frequency locked
     */
    bool isFrequencyLocked() const;
    
    /**
     * @brief Get current operating frequency
     * @return Frequency in Hz
     */
    uint16_t getCurrentFrequency() const;
    
    /**
     * @brief Get current power output
     * @return Power in watts
     */
    uint16_t getCurrentPowerOutput() const;
    
    /**
     * @brief Get current amplitude setpoint
     * @return Amplitude percentage (20-100%)
     */
    uint8_t getCurrentAmplitude() const;
    
    /**
     * @brief Get current operating state
     * @return Current sonicator state
     */
    sonicator_state_t getCurrentState() const;
    
    // ========================================================================
    // STATUS AND DIAGNOSTICS
    // ========================================================================
    
    /**
     * @brief Get complete status structure
     * @return Complete sonicator status
     */
    sonicator_status_t getCompleteStatus() const;
    
    /**
     * @brief Get total runtime in seconds
     * @return Total runtime
     */
    uint32_t getTotalRuntime() const;
    
    /**
     * @brief Get overload event count
     * @return Number of overload events
     */
    uint32_t getOverloadCount() const;
    
    /**
     * @brief Get start cycle count
     * @return Number of start cycles
     */
    uint32_t getStartCount() const;
    
    /**
     * @brief Get last error code
     * @return Most recent error
     */
    error_code_t getLastError() const;
    
    /**
     * @brief Check interface communication health
     * @return true if communication OK
     */
    bool isCommunicationHealthy() const;
    
    // ========================================================================
    // PERIODIC UPDATE AND MAINTENANCE
    // ========================================================================
    
    /**
     * @brief Main update function (call from main loop)
     * @return Current sonicator state
     * @mandatory Must be called at least every 10ms for proper operation
     * @performance Optimized for real-time execution, typically completes in <1ms
     * @thread_safety Not thread-safe - call only from main thread
     * @error_handling Returns SONICATOR_STATE_FAULT on critical errors
     */
    sonicator_state_t update() noexcept;

    /**
     * @brief Reset all statistics and counters
     * @note This is a void operation that doesn't affect current state
     * @thread_safety Not thread-safe - call only when sonicator is stopped
     */
    void resetStatistics() noexcept;
    
    /**
     * @brief Update control outputs based on current state
     * @return true if outputs updated successfully, false on hardware error
     * @note Called internally by update() - typically not called directly
     * @performance Optimized for minimal latency (<100μs typical)
     */
    bool updateControlOutputs() noexcept;
};

#endif // SONICATOR_H
