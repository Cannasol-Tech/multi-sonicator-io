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
#include <config.h>
#include <types.h>

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
//@brief Basie Sonicator Interface Class

private:
    // Hardware configuration
    sonicator_hardware_config_t hardware_config;
    uint8_t sonicator_id;
    bool interface_enabled;
    
    // Current status and state
    sonicator_status_t current_status;
    sonicator_state_t previous_state;
    uint32_t state_change_timestamp;
    
    // Control command tracking
    bool start_command_pending;
    bool reset_command_pending;
    uint32_t reset_pulse_start_time;
    
    // Monitoring and measurement
    uint16_t frequency_measurement_buffer[10];
    uint8_t frequency_buffer_index;
    uint32_t last_frequency_measurement;
    uint32_t last_power_measurement;
    
    // Safety monitoring
    bool overload_detected;
    uint32_t overload_detection_time;
    uint32_t last_safety_check;
    
    // Statistics and monitoring
    uint32_t total_start_cycles;
    uint32_t total_overload_events;
    uint32_t last_start_time;
    uint32_t total_runtime_ms;
    uint32_t last_update_timestamp;
    uint16_t update_call_count;
    
    // Error handling and diagnostics
    error_code_t last_error_code;
    uint32_t consecutive_error_count;
    uint32_t last_error_timestamp;
    bool critical_error_flag;
    
    // Performance monitoring
    uint32_t min_update_time_us;
    uint32_t max_update_time_us;
    uint32_t avg_update_time_us;
    bool performance_monitoring_enabled;
    
    // Private methods - Core functionality
    void updateHardwareOutputs() noexcept;
    void readHardwareInputs() noexcept;
    void updateStateMachine() noexcept;
    void handleOverloadCondition() noexcept;
    void updateStatistics() noexcept;
    void logStateChange(sonicator_state_t new_state) noexcept;
    
    // Private methods - Measurement and validation
    uint16_t measureFrequency() const noexcept;
    uint16_t measurePower() const noexcept;
    bool validateAmplitudeRange(uint8_t amplitude) const noexcept;
    bool validateSystemState() const noexcept;
    
    // Private methods - Safety and error handling
    bool checkSafetyConstraints() const noexcept;
    void handleSystemError(error_code_t error) noexcept;
    bool isWithinOperatingLimits() const noexcept;
    
    // Private methods - Timing and performance
    uint32_t getCurrentTimestamp() const noexcept;
    bool hasTimedOut(uint32_t start_time, uint32_t timeout_ms) const noexcept;

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
    
    // ========================================================================
    // LEGACY COMPATIBILITY (for existing code)
    // ========================================================================
    
    /**
     * @brief Start the sonicator (legacy compatibility)
     * @return true if start command accepted, false on error
     * @deprecated Use startSonication() instead for better error handling
     */
    [[deprecated("Use startSonication() instead")]]
    bool start() { return startSonication(); }
    
    /**
     * @brief Stop the sonicator (legacy compatibility)
     * @return true if stop command accepted, false on error
     * @deprecated Use stopSonication() instead for better error handling
     */
    [[deprecated("Use stopSonication() instead")]]
    bool stop() { return stopSonication(); }
    
    /**
     * @brief Get the current frequency of the sonicator (legacy compatibility)
     * @return Current frequency in Hz
     * @deprecated Use getCurrentFrequency() instead for consistency
     */
    [[deprecated("Use getCurrentFrequency() instead")]]
    uint16_t getFrequency() const { return getCurrentFrequency(); }
    
    /**
     * @brief Get the current amplitude of the sonicator (legacy compatibility)
     * @return Current amplitude percentage (20-100%)
     * @deprecated Use getCurrentAmplitude() instead for consistency
     */
    [[deprecated("Use getCurrentAmplitude() instead")]]
    uint8_t getAmplitude() const { return getCurrentAmplitude(); }
    
    /**
     * @brief Set the amplitude of the sonicator (legacy compatibility)
     * @param amplitude Amplitude percentage (20-100)
     * @return true if amplitude set successfully, false on error
     * @deprecated Use setAmplitude() instead for better error handling
     */
    [[deprecated("Use the non-legacy setAmplitude() instead")]]
    bool setAmplitudeLegacy(uint8_t amplitude) { return setAmplitude(amplitude); }
    
    /**
     * @brief Set the frequency of the sonicator (legacy compatibility - no-op)
     * @param frequency Frequency (ignored - CT2000 frequency is read-only)
     * @return false always (operation not supported)
     * @deprecated CT2000 frequency is read-only and cannot be set
     */
    [[deprecated("CT2000 frequency is read-only")]]
    bool setFrequency(int frequency) {
        (void)frequency; // Suppress unused parameter warning
        return false; // CT2000 frequency is read-only
    }
};

#endif // SONICATOR_H
