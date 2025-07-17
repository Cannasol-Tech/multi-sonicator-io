/**
 * @file virtual_sonicator.cpp
 * @brief Virtual Sonicator Implementation for Hardware Simulation
 * @author Cannasol Technologies
 * @date 2025-07-17
 * 
 * Simulates CT2000 sonicator behavior for integration testing
 * when physical hardware is not available.
 */

#include "virtual_sonicator.h"
#include <Arduino.h>
#include <cmath>
#include <cstdlib>
#include <ctime>

VirtualSonicator::VirtualSonicator(uint8_t id) 
    : sonicator_id(id)
    , is_running(false)
    , is_overloaded(false)
    , is_frequency_locked(false)
    , amplitude_setpoint(50)
    , actual_frequency(SONICATOR_FREQ_NOMINAL)
    , actual_power(0)
    , overload_count(0)
    , total_runtime(0)
    , last_update_time(0)
    , frequency_drift_rate(0)
    , power_noise_level(0.05f)
    , overload_threshold(SONICATOR_MAX_POWER * 0.95f)
    , fault_injection_active(false)
    , injected_fault_type(FAULT_NONE)
{
    // Initialize random seed for realistic simulation
    srand(time(nullptr) + id);
    
    // Initialize frequency drift (small random variation)
    frequency_drift_rate = (rand() % 20 - 10) * 0.01f; // ±0.1 Hz/s
    
    // Initialize simulation parameters
    reset();
}

VirtualSonicator::~VirtualSonicator() {
    // Cleanup simulation resources
}

void VirtualSonicator::reset() {
    is_running = false;
    is_overloaded = false;
    is_frequency_locked = false;
    amplitude_setpoint = 50;
    actual_frequency = SONICATOR_FREQ_NOMINAL;
    actual_power = 0;
    last_update_time = millis();
    fault_injection_active = false;
    injected_fault_type = FAULT_NONE;
}

void VirtualSonicator::update() {
    uint32_t current_time = millis();
    float dt = (current_time - last_update_time) / 1000.0f; // Delta time in seconds
    
    if (dt < 0.001f) return; // Skip if too little time has passed
    
    // Update runtime if running
    if (is_running) {
        total_runtime += (uint32_t)(dt * 1000);
    }
    
    // Simulate frequency drift
    updateFrequency(dt);
    
    // Simulate power output
    updatePower(dt);
    
    // Check for overload conditions
    checkOverloadConditions();
    
    // Handle fault injection
    handleFaultInjection();
    
    last_update_time = current_time;
}

void VirtualSonicator::updateFrequency(float dt) {
    if (!is_running) {
        actual_frequency = SONICATOR_FREQ_NOMINAL;
        is_frequency_locked = false;
        return;
    }
    
    // Simulate frequency lock behavior
    if (!is_frequency_locked) {
        // Frequency locking takes 100-500ms
        static float lock_time = 0;
        lock_time += dt;
        if (lock_time > 0.2f + (rand() % 300) / 1000.0f) {
            is_frequency_locked = true;
            lock_time = 0;
        }
    }
    
    // Apply frequency drift
    actual_frequency += frequency_drift_rate * dt;
    
    // Add small random noise
    float noise = (rand() % 20 - 10) * 0.1f; // ±1 Hz noise
    actual_frequency += noise * dt;
    
    // Clamp to valid range
    if (actual_frequency < MIN_VALID_FREQUENCY_HZ) {
        actual_frequency = MIN_VALID_FREQUENCY_HZ;
    }
    if (actual_frequency > MAX_VALID_FREQUENCY_HZ) {
        actual_frequency = MAX_VALID_FREQUENCY_HZ;
    }
    
    // Lose frequency lock if too far from nominal
    if (std::abs(actual_frequency - SONICATOR_FREQ_NOMINAL) > SONICATOR_FREQ_TOLERANCE) {
        is_frequency_locked = false;
    }
}

void VirtualSonicator::updatePower(float dt) {
    if (!is_running) {
        actual_power = 0;
        return;
    }
    
    // Calculate theoretical power based on amplitude
    float theoretical_power = (amplitude_setpoint / 100.0f) * SONICATOR_MAX_POWER;
    
    // Add power noise
    float noise = (rand() % 200 - 100) / 100.0f * power_noise_level * theoretical_power;
    actual_power = (uint16_t)(theoretical_power + noise);
    
    // Clamp to valid range
    if (actual_power > SONICATOR_MAX_POWER) {
        actual_power = SONICATOR_MAX_POWER;
    }
}

void VirtualSonicator::checkOverloadConditions() {
    // Check for overload based on power
    if (actual_power > overload_threshold) {
        triggerOverload();
    }
    
    // Check for frequency lock loss overload
    if (is_running && !is_frequency_locked && 
        (millis() - last_update_time) > 1000) { // 1 second without lock
        triggerOverload();
    }
}

void VirtualSonicator::handleFaultInjection() {
    if (!fault_injection_active) return;
    
    switch (injected_fault_type) {
        case FAULT_OVERLOAD:
            triggerOverload();
            break;
            
        case FAULT_FREQUENCY_DRIFT:
            frequency_drift_rate += 0.5f; // Accelerate drift
            break;
            
        case FAULT_POWER_SPIKE:
            actual_power = SONICATOR_MAX_POWER + 100; // Over limit
            break;
            
        case FAULT_FREQUENCY_LOCK_LOSS:
            is_frequency_locked = false;
            break;
            
        case FAULT_NONE:
        default:
            break;
    }
}

void VirtualSonicator::triggerOverload() {
    if (!is_overloaded) {
        is_overloaded = true;
        is_running = false;
        is_frequency_locked = false;
        actual_power = 0;
        overload_count++;
    }
}

// Control interface methods
bool VirtualSonicator::startSonication() {
    if (is_overloaded) {
        return false; // Cannot start if overloaded
    }
    
    is_running = true;
    return true;
}

bool VirtualSonicator::stopSonication() {
    is_running = false;
    is_frequency_locked = false;
    actual_power = 0;
    return true;
}

bool VirtualSonicator::setAmplitude(uint8_t percentage) {
    if (percentage < SONICATOR_MIN_AMPLITUDE || 
        percentage > SONICATOR_MAX_AMPLITUDE) {
        return false;
    }
    
    amplitude_setpoint = percentage;
    return true;
}

bool VirtualSonicator::resetOverload() {
    if (is_overloaded) {
        is_overloaded = false;
        return true;
    }
    return false;
}

// Status query methods
bool VirtualSonicator::getIsRunning() const {
    return is_running;
}

bool VirtualSonicator::getIsOverloaded() const {
    return is_overloaded;
}

bool VirtualSonicator::getIsFrequencyLocked() const {
    return is_frequency_locked;
}

uint8_t VirtualSonicator::getAmplitude() const {
    return amplitude_setpoint;
}

uint16_t VirtualSonicator::getFrequency() const {
    return (uint16_t)actual_frequency;
}

uint16_t VirtualSonicator::getPower() const {
    return actual_power;
}

uint32_t VirtualSonicator::getRuntime() const {
    return total_runtime;
}

uint32_t VirtualSonicator::getOverloadCount() const {
    return overload_count;
}

// Fault injection methods
void VirtualSonicator::injectFault(FaultType fault_type) {
    fault_injection_active = true;
    injected_fault_type = fault_type;
}

void VirtualSonicator::clearFaultInjection() {
    fault_injection_active = false;
    injected_fault_type = FAULT_NONE;
}