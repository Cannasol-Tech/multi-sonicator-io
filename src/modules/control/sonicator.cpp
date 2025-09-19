/**
 * @file sonicator.cpp
 * @brief Implementation of the SonicatorInterface class
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 2.0.0
 *
 * @details
 * Implementation of the generic SonicatorInterface class. This version is
 * configured via a pin structure, allowing it to manage any of the four
 * sonicator channels.
 */

#include "sonicator/sonicator.h"
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

#ifndef SONICATOR_SIMULATION_MODE
#define SONICATOR_SIMULATION_MODE 0
#endif

#ifdef DEBUG_MODE
#define SONICATOR_LOG(msg) Serial.println(msg)
#else
#define SONICATOR_LOG(msg) ((void)0)
#endif

// ============================================================================
// DEFAULT STATE
// ============================================================================

static const sonicator_status_t SONICATOR_DEFAULT_STATE = {
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

SonicatorInterface::SonicatorInterface(const SonicatorPins& pins)
    : pins_(pins), simulation_mode_(SONICATOR_SIMULATION_MODE) {
    state_ = SONICATOR_DEFAULT_STATE;
}

SonicatorInterface::~SonicatorInterface() {
    // No-op
}

// ============================================================================
// PRIVATE UTILITY METHODS
// ============================================================================

uint32_t SonicatorInterface::getTimestampMs() const {
    return millis();
}

bool SonicatorInterface::isTimeout(uint32_t start_time, uint32_t timeout_ms) const {
    return (getTimestampMs() - start_time) >= timeout_ms;
}

uint8_t SonicatorInterface::clampAmplitude(uint8_t amplitude) const {
    if (amplitude < SONICATOR_MIN_AMPLITUDE_PERCENT) return SONICATOR_MIN_AMPLITUDE_PERCENT;
    if (amplitude > SONICATOR_MAX_AMPLITUDE_PERCENT) return SONICATOR_MAX_AMPLITUDE_PERCENT;
    return amplitude;
}

uint8_t SonicatorInterface::amplitudeToPwm(uint8_t amplitude_percent) const {
    if (amplitude_percent < SONICATOR_MIN_AMPLITUDE_PERCENT) {
        return 0;
    }
    return ((uint32_t)(amplitude_percent - SONICATOR_MIN_AMPLITUDE_PERCENT) * 255) /
           (SONICATOR_MAX_AMPLITUDE_PERCENT - SONICATOR_MIN_AMPLITUDE_PERCENT);
}

// ============================================================================
// PRIVATE HAL INTERFACE METHODS
// ============================================================================

void SonicatorInterface::halGpioWriteSafe(uint8_t pin, bool state) {
    if (!simulation_mode_) {
        gpio_write_pin(pin, state ? GPIO_HIGH : GPIO_LOW);
    }
    SONICATOR_LOG(state ? "GPIO HIGH" : "GPIO LOW");
}

bool SonicatorInterface::halGpioReadSafe(uint8_t pin) {
    if (simulation_mode_) {
        if (pin == pins_.overload_pin) return false;
        if (pin == pins_.freq_lock_pin) return true;
        return false;
    }
    gpio_state_t state;
    gpio_read_pin(pin, &state);
    return (state == GPIO_HIGH);
}

void SonicatorInterface::halPwmSetSafe(uint8_t pin, uint8_t duty_cycle) {
    (void)pin;
    if (!simulation_mode_) {
        pwm_set_duty_cycle(PWM_CHANNEL_AMPLITUDE, duty_cycle);
    }
    SONICATOR_LOG("PWM SET");
}

uint16_t SonicatorInterface::halAdcReadSafe(adc_channel_t channel) {
    if (simulation_mode_) {
        return (uint16_t)((500UL * 1023UL) / 2000UL);
    }
    uint16_t value;
    return (adc_read_channel(channel, &value) == ADC_OK) ? value : 0;
}

// ============================================================================
// PRIVATE HARDWARE INTERFACE METHODS
// ============================================================================

void SonicatorInterface::updateHardwareOutputs() {
    bool start_signal = (state_.state == SONICATOR_STATE_RUNNING || state_.state == SONICATOR_STATE_STARTING);
    halGpioWriteSafe(pins_.start_pin, start_signal);

    // Amplitude PWM is managed globally by the Multiplexer.
    // No per-channel PWM writes here.

    static uint32_t reset_pulse_start = 0;
    static bool reset_pulse_active = false;

    if (state_.reset_requested && !reset_pulse_active) {
        halGpioWriteSafe(pins_.reset_pin, true);
        reset_pulse_start = getTimestampMs();
        reset_pulse_active = true;
        state_.reset_requested = false;
        SONICATOR_LOG("Reset pulse started");
    }

    if (reset_pulse_active && isTimeout(reset_pulse_start, SONICATOR_RESET_PULSE_MS)) {
        halGpioWriteSafe(pins_.reset_pin, false);
        reset_pulse_active = false;
        SONICATOR_LOG("Reset pulse ended");
    }
}

void SonicatorInterface::readHardwareInputs() {
    state_.overload_active = halGpioReadSafe(pins_.overload_pin);
    state_.frequency_locked = halGpioReadSafe(pins_.freq_lock_pin);

    uint16_t adc_value = halAdcReadSafe(pins_.power_sense_channel);
    state_.power_watts = (float)(adc_value * 2000) / 1023.0f;

    state_.frequency_hz = state_.frequency_locked ? 20000 : 0;
}

sonicator_fault_t SonicatorInterface::checkFaultConditions() {
    sonicator_fault_t faults = SONICATOR_FAULT_NONE;
    uint32_t now = getTimestampMs();

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

    if ((state_.state == SONICATOR_STATE_RUNNING) && !state_.frequency_locked) {
        faults = (sonicator_fault_t)(faults | SONICATOR_FAULT_FREQ_UNLOCK);
    }

    if (isTimeout(state_.last_update_time, SONICATOR_COMM_TIMEOUT_MS)) {
        faults = (sonicator_fault_t)(faults | SONICATOR_FAULT_COMM_TIMEOUT);
    }

    if (isTimeout(state_.watchdog_last_reset, SONICATOR_WATCHDOG_TIMEOUT_MS)) {
        faults = (sonicator_fault_t)(faults | SONICATOR_FAULT_WATCHDOG);
    }

    return faults;
}

void SonicatorInterface::handleFaultConditions(sonicator_fault_t faults) {
    if (faults != SONICATOR_FAULT_NONE) {
        halGpioWriteSafe(pins_.start_pin, false);
        // Amplitude PWM is managed by the Multiplexer; do not write here.

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

void SonicatorInterface::processStateMachine() {
    uint32_t now = getTimestampMs();
    uint32_t state_duration = now - state_.state_entry_time;

    switch (state_.state) {
        case SONICATOR_STATE_IDLE:
            if (state_.start_requested && (state_.active_faults == SONICATOR_FAULT_NONE)) {
                state_.state = SONICATOR_STATE_STARTING;
                state_.state_entry_time = now;
                state_.start_requested = false;
                SONICATOR_LOG("State: IDLE -> STARTING");
            }
            break;

        case SONICATOR_STATE_STARTING:
            if (state_duration >= SONICATOR_START_DELAY_MS) {
                state_.state = SONICATOR_STATE_RUNNING;
                state_.state_entry_time = now;
                state_.is_running = true;
                state_.start_count++;
                state_.last_start_time = now;
                SONICATOR_LOG("State: STARTING -> RUNNING");
            }
            break;

        case SONICATOR_STATE_RUNNING:
            // Runtime stats are updated in the main update function
            break;

        case SONICATOR_STATE_STOPPING:
            if (state_duration >= SONICATOR_STOP_DELAY_MS) {
                state_.state = SONICATOR_STATE_IDLE;
                state_.state_entry_time = now;
                state_.stop_requested = false;
                SONICATOR_LOG("State: STOPPING -> IDLE");
            }
            break;

        case SONICATOR_STATE_FAULT:
            // Await reset
            break;

        default:
            state_.state = SONICATOR_STATE_IDLE;
            state_.state_entry_time = now;
            break;
    }
}

// ============================================================================
// PUBLIC API METHODS
// ============================================================================

bool SonicatorInterface::start() {
    if (state_.state == SONICATOR_STATE_IDLE && state_.active_faults == SONICATOR_FAULT_NONE) {
        state_.start_requested = true;
        SONICATOR_LOG("Start command accepted");
        return true;
    }
    return false;
}

bool SonicatorInterface::stop() {
    if (state_.state == SONICATOR_STATE_RUNNING || state_.state == SONICATOR_STATE_STARTING) {
        state_.stop_requested = true;
        SONICATOR_LOG("Stop command accepted");
        return true;
    }
    return false;
}

bool SonicatorInterface::setAmplitude(uint8_t amplitude_percent) {
    state_.amplitude_percent = clampAmplitude(amplitude_percent);
    SONICATOR_LOG("Amplitude set");
    return true;
}

bool SonicatorInterface::resetOverload() {
    if (state_.state == SONICATOR_STATE_FAULT) {
        state_.reset_requested = true;
        // Clear faults that can be reset
        state_.active_faults = (sonicator_fault_t)(state_.active_faults & ~SONICATOR_FAULT_OVERLOAD);
        if(state_.active_faults == SONICATOR_FAULT_NONE) {
            state_.state = SONICATOR_STATE_IDLE;
            state_.state_entry_time = getTimestampMs();
        }
        SONICATOR_LOG("Reset overload command sent");
        return true;
    }
    return false;
}

bool SonicatorInterface::emergencyStop() {
    halGpioWriteSafe(pins_.start_pin, false);

    state_.state = SONICATOR_STATE_IDLE;
    state_.state_entry_time = getTimestampMs();
    state_.is_running = false;
    state_.start_requested = false;
    state_.stop_requested = false;

    SONICATOR_LOG("Emergency stop activated");
    return true;
}

sonicator_state_t SonicatorInterface::update() {
    uint32_t now = getTimestampMs();
    state_.last_update_time = now;
    
    readHardwareInputs();
    
    sonicator_fault_t faults = checkFaultConditions();
    if (faults != SONICATOR_FAULT_NONE) {
        handleFaultConditions(faults);
    }
    
    processStateMachine();
    updateHardwareOutputs();
    
    state_.watchdog_last_reset = now;
    
    return state_.state;
}

const sonicator_status_t* SonicatorInterface::getStatus() const {
    return &state_;
}

const char* SonicatorInterface::stateToString(sonicator_state_t state) {
    switch (state) {
        case SONICATOR_STATE_IDLE: return "IDLE";
        case SONICATOR_STATE_STARTING: return "STARTING";
        case SONICATOR_STATE_RUNNING: return "RUNNING";
        case SONICATOR_STATE_STOPPING: return "STOPPING";
        case SONICATOR_STATE_FAULT: return "FAULT";
        default: return "UNKNOWN";
    }
}

bool SonicatorInterface::isSafe() const {
    uint32_t now = getTimestampMs();
    bool no_active_faults = (state_.active_faults == SONICATOR_FAULT_NONE);
    bool watchdog_ok = !isTimeout(state_.watchdog_last_reset, SONICATOR_WATCHDOG_TIMEOUT_MS);
    bool comm_ok = !isTimeout(state_.last_update_time, SONICATOR_COMM_TIMEOUT_MS);
    return no_active_faults && watchdog_ok && comm_ok;
}

void SonicatorInterface::resetStatistics() {
    state_.start_count = 0;
    state_.total_runtime_ms = 0;
    state_.fault_count = 0;
    state_.last_fault_time = 0;
    SONICATOR_LOG("Statistics reset");
}

void SonicatorInterface::setSimulationMode(bool enable) {
    simulation_mode_ = enable;
    SONICATOR_LOG(enable ? "Simulation mode enabled" : "Simulation mode disabled");
}

bool SonicatorInterface::forceState(const sonicator_state_t& newState) {
    state_ = newState;
    return true;
}

bool SonicatorInterface::injectFault(const sonicator_fault_t& faultMask) {
    handleFaultConditions(faultMask);
    return true;
}
