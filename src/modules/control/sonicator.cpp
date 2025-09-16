/**
 * @file sonicator_interface.cpp
 * @brief Implementation of CT2000Sonicator class
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 1.0.0
 *
 * @details
 * Implementation of the CT2000Sonicator class that provides clean API
 * for single sonicator control with proper state machine, safety requirements,
 * and HAL integration.
 */

#include "sonicator/interface.h"
#include "sonicator/types/state.h"
#include "sonicator/types/errors.h"
#include "system_config.h"
#include "modules/hal/gpio.h"
#include "modules/hal/pwm.h"
#include "modules/hal/adc.h"
#include <Arduino.h>
#include <string.h>

// ============================================================================
// COMPILE-TIME CONFIGURATION
// ============================================================================

/**
 * @brief Enable simulation mode for unit testing (bypasses HAL calls)
 */
#ifndef SONICATOR_SIMULATION_MODE
#define SONICATOR_SIMULATION_MODE 0
#endif

/**
 * @brief Debug logging macro (only active in DEBUG builds)
 * @param msg Message to log
 */
#ifdef DEBUG_MODE
#define SONICATOR_LOG(msg) Serial.println(msg)
#else
#define SONICATOR_LOG(msg) ((void)0)
#endif

/**
 * @brief Default sonicator state structure initialization
 */
sonicator_state_t SONICATOR_DEFAULT_STATE = {
    .state = SONICATOR_STATE_UNKNOWN,
    .previous_state = SONICATOR_STATE_UNKNOWN,
    .state_entry_time = 0,
    .amplitude_percent = SONICATOR_MIN_AMPLITUDE_PERCENT,
    .start_requested = false,
    .stop_requested = false,
    .reset_requested = false,
    .is_running = false,
    .overload_active = false,
    .frequency_locked = false,
    .power_watts = 0.0f,
    .frequency_hz = 0,
    .active_faults = SONICATOR_FAULT_NONE,
    .fault_count = 0,
    .last_fault_time = 0,
    .last_update_time = 0,
    .watchdog_last_reset = 0,
    .safety_override = false,
    .start_count = 0,
    .total_runtime_ms = 0,
    .last_start_time = 0
};

// ============================================================================
// CLASS IMPLEMENTATION
// ============================================================================

/**
 * @brief Constructor for CT2000Sonicator
 * @details Initializes the sonicator interface with default state and simulation mode setting
 */
CT2000Sonicator::CT2000Sonicator() : simulation_mode_(SONICATOR_SIMULATION_MODE)
    {   
        // Initialize state structure
        state_ = SONICATOR_DEFAULT_STATE;
    }

/**
 * @brief Destructor for CT2000Sonicator
 * @details Performs cleanup if needed
 */
CT2000Sonicator::~CT2000Sonicator() {
    // Cleanup if needed
}

// ============================================================================
// PRIVATE UTILITY METHODS
// ============================================================================

/**
 * @brief Get current timestamp in milliseconds
 * @return Current timestamp in milliseconds
 */
uint32_t CT2000Sonicator::getTimestampMs() const {
    return millis();
}

/**
 * @brief Check if a timeout has occurred
 * @param start_time Starting timestamp in milliseconds
 * @param timeout_ms Timeout duration in milliseconds
 * @return true if timeout has occurred, false otherwise
 */
bool CT2000Sonicator::isTimeout(uint32_t start_time, uint32_t timeout_ms) const {
    return (getTimestampMs() - start_time) >= timeout_ms;
}

/**
 * @brief Clamp amplitude value to valid range
 * @param amplitude Amplitude value to clamp
 * @return Clamped amplitude value between SONICATOR_MIN_AMPLITUDE_PERCENT and SONICATOR_MAX_AMPLITUDE_PERCENT
 */
uint8_t CT2000Sonicator::clampAmplitude(uint8_t amplitude) const {
    if (amplitude < SONICATOR_MIN_AMPLITUDE_PERCENT) {
        return SONICATOR_MIN_AMPLITUDE_PERCENT;
    }
    if (amplitude > SONICATOR_MAX_AMPLITUDE_PERCENT) {
        return SONICATOR_MAX_AMPLITUDE_PERCENT;
    }
    return amplitude;
}

/**
 * @brief Convert amplitude percentage to PWM duty cycle
 * @param amplitude_percent Amplitude as percentage (20-100%)
 * @return PWM duty cycle value (0-255)
 * @details Scales 20-100% amplitude range to 0-255 PWM range
 */
uint8_t CT2000Sonicator::amplitudeToPwm(uint8_t amplitude_percent) const {
    // Scale 20-100% to 0-255 PWM range
    // PWM = (amplitude - 20) * 255 / (100 - 20)
    if (amplitude_percent < SONICATOR_MIN_AMPLITUDE_PERCENT) {
        return 0;
    }
    return ((amplitude_percent - SONICATOR_MIN_AMPLITUDE_PERCENT) * 255) /
           (SONICATOR_MAX_AMPLITUDE_PERCENT - SONICATOR_MIN_AMPLITUDE_PERCENT);
}

// ============================================================================
// PRIVATE HAL INTERFACE METHODS
// ============================================================================

/**
 * @brief Safely write GPIO pin state with simulation mode support
 * @param pin GPIO pin number
 * @param state Desired pin state (true = HIGH, false = LOW)
 */
void CT2000Sonicator::halGpioWriteSafe(uint8_t pin, bool state) {
    if (!simulation_mode_) {
        gpio_write_pin(pin, state ? GPIO_HIGH : GPIO_LOW);
    }
    SONICATOR_LOG(state ? "GPIO HIGH" : "GPIO LOW");
}

/**
 * @brief Safely read GPIO pin state with simulation mode support
 * @param pin GPIO pin number
 * @return Pin state (true = HIGH, false = LOW)
 * @details In simulation mode, returns safe defaults for specific pins
 */
bool CT2000Sonicator::halGpioReadSafe(uint8_t pin) {
    if (simulation_mode_) {
        // In simulation mode, return safe defaults
        if (pin == SON4_OVERLOAD_PIN) return false; // No overload
        if (pin == SON4_FREQ_LOCK_PIN) return true; // Frequency locked
        return false;
    }
    gpio_state_t state;
    gpio_read_pin(pin, &state);
    return (state == GPIO_HIGH);
}

/**
 * @brief Safely set PWM duty cycle with simulation mode support
 * @param pin PWM pin number (unused parameter)
 * @param duty_cycle PWM duty cycle value (0-255)
 */
void CT2000Sonicator::halPwmSetSafe(uint8_t pin, uint8_t duty_cycle) {
    (void)pin; // Suppress unused parameter warning
    if (!simulation_mode_) {
        pwm_set_duty_cycle(PWM_CHANNEL_AMPLITUDE, duty_cycle);
    }
    SONICATOR_LOG("PWM SET");
}

/**
 * @brief Safely read ADC channel with simulation mode support
 * @param channel ADC channel to read
 * @return ADC reading value
 * @details In simulation mode, returns simulated 500W power reading
 */
uint16_t CT2000Sonicator::halAdcReadSafe(adc_channel_t channel) {
    if (simulation_mode_) {
        // Return simulated power reading (500W)
        return (uint16_t)((500UL * 1023UL) / 2000UL); // Scale to ADC range
    }
    uint16_t value;
    if (adc_read_channel(channel, &value) == ADC_OK) {
        return value;
    }
    return 0;
}

// ============================================================================
// PRIVATE HARDWARE INTERFACE METHODS
// ============================================================================

/**
 * @brief Update all hardware outputs based on current state
 * @details Updates START signal, amplitude PWM, and handles reset pulse timing
 */
void CT2000Sonicator::updateHardwareOutputs() {
    // Update START signal (PC0)
    bool start_signal = (state_.state == SONICATOR_STATE_RUNNING ||
                        state_.state == SONICATOR_STATE_STARTING);
    halGpioWriteSafe(SON4_START_PIN, start_signal);

    // Update amplitude PWM (PD7) - shared for all sonicators
    uint8_t pwm_value = 0;
    if (state_.state == SONICATOR_STATE_RUNNING) {
        pwm_value = amplitudeToPwm(state_.amplitude_percent);
    }
    halPwmSetSafe(PWM_AMPLITUDE_CONTROL_PIN, pwm_value);

    // Handle reset pulse timing (PC1)
    static uint32_t reset_pulse_start = 0;
    static bool reset_pulse_active = false;

    if (state_.reset_requested && !reset_pulse_active) {
        halGpioWriteSafe(SON4_RESET_PIN, true);
        reset_pulse_start = getTimestampMs();
        reset_pulse_active = true;
        state_.reset_requested = false;
        SONICATOR_LOG("Reset pulse started");
    }

    if (reset_pulse_active && isTimeout(reset_pulse_start, SONICATOR_RESET_PULSE_MS)) {
        halGpioWriteSafe(SON4_RESET_PIN, false);
        reset_pulse_active = false;
        SONICATOR_LOG("Reset pulse ended");
    }
}

/**
 * @brief Read all hardware inputs and update state
 * @details Reads overload status, frequency lock, power measurement, and frequency
 */
void CT2000Sonicator::readHardwareInputs() {
    // Read overload status (PD3)
    state_.overload_active = halGpioReadSafe(SON4_OVERLOAD_PIN);

    // Read frequency lock status (PB4)
    state_.frequency_locked = halGpioReadSafe(SON4_FREQ_LOCK_PIN);

    // Read power measurement (PA7)
    uint16_t adc_value = halAdcReadSafe(ADC_CHANNEL_7);
    // Convert ADC to watts: ADC range 0-1023 maps to 0-2000W
    state_.power_watts = (float)(adc_value * 2000) / 1023.0f;

    // Frequency measurement (simplified - would need timer capture in full implementation)
    if (state_.frequency_locked) {
        state_.frequency_hz = 20000; // Nominal CT2000 frequency
    } else {
        state_.frequency_hz = 0;
    }
}

/**
 * @brief Check for fault conditions with debouncing
 * @return Bitmask of active fault conditions
 * @details Checks overload, frequency lock loss, communication timeout, and watchdog timeout
 */
sonicator_fault_t CT2000Sonicator::checkFaultConditions() {
    sonicator_fault_t faults = SONICATOR_FAULT_NONE;
    uint32_t now = getTimestampMs();

    // Check overload condition with debouncing
    static uint32_t overload_detected_time = 0;
    static bool overload_debounce = false;

    if (state_.overload_active && !overload_debounce) {
        overload_detected_time = now;
        overload_debounce = true;
    } else if (!state_.overload_active) {
        overload_debounce = false;
    }

    if (overload_debounce && isTimeout(overload_detected_time, SONICATOR_FAULT_DEBOUNCE_MS)) {
        faults = (sonicator_fault_t)(faults | SONICATOR_FAULT_OVERLOAD);
    }

    // Check frequency lock loss (only when running)
    if ((state_.state == SONICATOR_STATE_RUNNING) && !state_.frequency_locked) {
        faults = (sonicator_fault_t)(faults | SONICATOR_FAULT_FREQ_UNLOCK);
    }

    // Check communication timeout
    if (isTimeout(state_.last_update_time, SONICATOR_COMM_TIMEOUT_MS)) {
        faults = (sonicator_fault_t)(faults | SONICATOR_FAULT_COMM_TIMEOUT);
    }

    // Check watchdog timeout
    if (isTimeout(state_.watchdog_last_reset, SONICATOR_WATCHDOG_TIMEOUT_MS)) {
        faults = (sonicator_fault_t)(faults | SONICATOR_FAULT_WATCHDOG);
    }

    return faults;
}

/**
 * @brief Handle detected fault conditions
 * @param faults Bitmask of detected fault conditions
 * @details Performs emergency stop and transitions to fault state when faults are detected
 */
void CT2000Sonicator::handleFaultConditions(sonicator_fault_t faults) {
    if (faults != SONICATOR_FAULT_NONE) {
        // Emergency stop - immediate hardware shutdown
        halGpioWriteSafe(SON4_START_PIN, false);
        halPwmSetSafe(PWM_AMPLITUDE_CONTROL_PIN, 0);

        // Update state and fault tracking
        state_.previous_state = state_.state;
        state_.state = SONICATOR_STATE_FAULT;
        state_.state_entry_time = getTimestampMs();
        state_.active_faults = faults;
        state_.fault_count++;
        state_.last_fault_time = state_.state_entry_time;
        state_.is_running = false;

        SONICATOR_LOG("FAULT DETECTED - Emergency stop activated");
    }
}

// ============================================================================
// PRIVATE STATE MACHINE METHOD
// ============================================================================

/**
 * @brief Process the sonicator state machine
 * @details Handles state transitions and timing for IDLE, STARTING, RUNNING, STOPPING, and FAULT states
 */
void CT2000Sonicator::processStateMachine() {
    uint32_t now = getTimestampMs();
    uint32_t state_duration = now - state_.state_entry_time;

    switch (state_.state) {
        case SONICATOR_STATE_IDLE:
            state_.is_running = false;

            // Transition: IDLE → STARTING
            if (state_.start_requested && (state_.active_faults == SONICATOR_FAULT_NONE)) {
                state_.previous_state = state_.state;
                state_.state = SONICATOR_STATE_STARTING;
                state_.state_entry_time = now;
                state_.start_requested = false;
                SONICATOR_LOG("State: IDLE -> STARTING");
            }
            break;

        case SONICATOR_STATE_STARTING:
            state_.is_running = false;

            // Transition: STARTING → RUNNING (after delay)
            if (state_duration >= SONICATOR_START_DELAY_MS) {
                state_.previous_state = state_.state;
                state_.state = SONICATOR_STATE_RUNNING;
                state_.state_entry_time = now;
                state_.is_running = true;
                state_.start_count++;
                state_.last_start_time = now;
                SONICATOR_LOG("State: STARTING -> RUNNING");
            }
            break;

        case SONICATOR_STATE_RUNNING:
            state_.is_running = true;

            // Update runtime statistics
            static uint32_t last_runtime_update = 0;
            if (now - last_runtime_update >= 1000) { // Update every second
                state_.total_runtime_ms += (now - last_runtime_update);
