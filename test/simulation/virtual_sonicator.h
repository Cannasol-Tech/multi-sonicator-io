/**
 * @file virtual_sonicator.h
 * @brief Virtual Sonicator Header for Hardware Simulation
 * @author Cannasol Technologies
 * @date 2025-07-17
 * 
 * Defines the VirtualSonicator class that simulates CT2000 sonicator
 * behavior for integration testing when physical hardware is not available.
 */

#ifndef VIRTUAL_SONICATOR_H
#define VIRTUAL_SONICATOR_H

#include <stdint.h>
#include "../../include/config.h"
#include "../../include/types.h"

/**
 * @brief Fault injection types for testing edge cases
 */
typedef enum {
    FAULT_NONE = 0,
    FAULT_OVERLOAD = 1,
    FAULT_FREQUENCY_DRIFT = 2,
    FAULT_POWER_SPIKE = 3,
    FAULT_FREQUENCY_LOCK_LOSS = 4
} FaultType;

/**
 * @brief Virtual Sonicator Class
 * 
 * Simulates the behavior of a CT2000 sonicator for testing purposes.
 * Provides realistic responses to control commands and generates
 * appropriate status updates including fault conditions.
 */
class VirtualSonicator {
private:
    // Sonicator identification
    uint8_t sonicator_id;
    
    // Current state
    bool is_running;
    bool is_overloaded;
    bool is_frequency_locked;
    uint8_t amplitude_setpoint;
    
    // Measured values
    float actual_frequency;
    uint16_t actual_power;
    
    // Statistics
    uint32_t overload_count;
    uint32_t total_runtime;
    uint32_t last_update_time;
    
    // Simulation parameters
    float frequency_drift_rate;
    float power_noise_level;
    float overload_threshold;
    
    // Fault injection
    bool fault_injection_active;
    FaultType injected_fault_type;
    
    // Private methods
    void updateFrequency(float dt);
    void updatePower(float dt);
    void checkOverloadConditions();
    void handleFaultInjection();
    void triggerOverload();
    
public:
    /**
     * @brief Constructor
     * @param id Sonicator ID (1-4)
     */
    VirtualSonicator(uint8_t id);
    
    /**
     * @brief Destructor
     */
    ~VirtualSonicator();
    
    /**
     * @brief Reset sonicator to initial state
     */
    void reset();
    
    /**
     * @brief Update simulation (call periodically)
     * Must be called at least every 100ms for realistic behavior
     */
    void update();
    
    // Control interface methods
    bool startSonication();
    bool stopSonication();
    bool setAmplitude(uint8_t percentage);
    bool resetOverload();
    
    // Status query methods
    bool getIsRunning() const;
    bool getIsOverloaded() const;
    bool getIsFrequencyLocked() const;
    uint8_t getAmplitude() const;
    uint16_t getFrequency() const;
    uint16_t getPower() const;
    uint32_t getRuntime() const;
    uint32_t getOverloadCount() const;
    
    // Fault injection methods (for testing)
    void injectFault(FaultType fault_type);
    void clearFaultInjection();
};

#endif // VIRTUAL_SONICATOR_H