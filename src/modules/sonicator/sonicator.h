/**
 * @file sonicator.h
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
#include "../../include/config.h"
#include "../../include/types.h"

/**
 * @brief Enhanced Sonicator Interface Class
 * 
 * This class provides complete control and monitoring interface
 * for a single CT2000 sonicator unit via DB9 connector.
 * 
 * Features:
 * - Full amplitude control (20-100% via 0-10V DAC)
 * - Start/stop control via relay output
 * - Overload reset control
 * - Real-time power monitoring
 * - Frequency monitoring and lock detection
 * - Safety monitoring with <50μs response time
 * - Runtime statistics and fault logging
 */
class SonicatorInterface {
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
    
    // Statistics
    uint32_t total_start_cycles;
    uint32_t total_overload_events;
    uint32_t last_start_time;
    
    // Private methods
    void updateHardwareOutputs();
    void readHardwareInputs();
    uint16_t measureFrequency();
    uint16_t measurePower();
    void updateStateMachine();
    void handleOverloadCondition();
    void updateStatistics();
    bool validateAmplitudeRange(uint8_t amplitude);
    void logStateChange(sonicator_state_t new_state);

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
     * @brief Check if overload condition exists
     * @return true if overloaded
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
     * Must be called at least every 10ms for proper operation
     */
    void update();
    
    /**
     * @brief Fast safety check (call from safety interrupt)
     * Must be called every 1ms or faster for safety compliance
     */
    void safetyScan();
    
    /**
     * @brief Reset all statistics and counters
     */
    void resetStatistics();
    
    /**
     * @brief Update control outputs (called from update())
     */
    void updateControlOutputs();
    
    // ========================================================================
    // LEGACY COMPATIBILITY (for existing code)
    // ========================================================================
    
    /**
     * @brief Legacy start function
     */
    void start() { startSonication(); }
    
    /**
     * @brief Legacy stop function  
     */
    void stop() { stopSonication(); }
    
    /**
     * @brief Legacy frequency getter
     * @return Current frequency
     */
    int getFrequency() const { return getCurrentFrequency(); }
    
    /**
     * @brief Legacy amplitude getter
     * @return Current amplitude
     */
    int getAmplitude() const { return getCurrentAmplitude(); }
    
    /**
     * @brief Legacy amplitude setter
     * @param amplitude Amplitude percentage
     */
    void setAmplitude(int amplitude) { setAmplitude((uint8_t)amplitude); }
    
    /**
     * @brief Legacy frequency setter (read-only in CT2000)
     * @param frequency Frequency (ignored - read-only)
     */
    void setFrequency(int frequency) { /* CT2000 frequency is read-only */ }
};

#endif // SONICATOR_H