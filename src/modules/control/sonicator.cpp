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
#include "modbus_register_manager.h"
#include "modbus_registers.h"
#include "frequency_counter.h"
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

/* Default runtime state initialized in constructor (SONICATOR_DEFAULT_STATE removed) */

// ... (rest of the file is the same until getStatus)

const sonicator_status_t* SonicatorInterface::getStatus() const {
    buildStatusView_();
    return &status_view_;
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

// ... (rest of the file is the same until forceState)

bool SonicatorInterface::forceState(const sonicator_status_t& newState) {
    // Map public status view into internal runtime state
    state_.is_running        = newState.is_running;
    state_.frequency_hz      = newState.frequency_hz;
    state_.overload_active   = newState.overload_active;
    state_.frequency_locked  = newState.frequency_locked;
    state_.fault_count       = newState.fault_count;
    state_.power_watts       = static_cast<float>(newState.power_raw_adc);
    state_.last_fault_time   = newState.last_fault_time;
    state_.amplitude_percent = newState.amplitude_actual;

    state_.previous_state    = newState.state_machine.previous_state;
    state_.state             = newState.state_machine.state;
    state_.state_entry_time  = newState.state_machine.state_entry_time;

    state_.start_requested   = false;
    state_.stop_requested    = false;
    state_.reset_requested   = false;

    state_.last_update_time    = getTimestampMs();
    state_.watchdog_last_reset = state_.last_update_time;

    return true;
}

bool SonicatorInterface::injectFault(const sonicator_fault_t& faultMask) {
    handleFaultConditions(faultMask);
    return true;
}


// ============================================================================
// CLASS IMPLEMENTATION
// ============================================================================

SonicatorInterface::SonicatorInterface(const SonicatorPins& pins)
    : pins_(pins), simulation_mode_(SONICATOR_SIMULATION_MODE) {
    // Initialize runtime defaults safely
    state_.state = SONICATOR_STATE_IDLE;
    state_.previous_state = SONICATOR_STATE_UNKNOWN;
    state_.state_entry_time = 0;

    state_.amplitude_percent = SONICATOR_MIN_AMPLITUDE_PERCENT;
    state_.start_requested = false;
    state_.stop_requested = false;
    state_.reset_requested = false;

    state_.is_running = false;
    state_.overload_active = false;
    state_.frequency_locked = false;

    state_.power_watts = 0.0f;
    state_.frequency_hz = 0;

    state_.active_faults = SONICATOR_FAULT_NONE;
    state_.fault_count = 0;
    state_.last_fault_time = 0;

    state_.last_update_time = 0;
    state_.watchdog_last_reset = 0;

    state_.safety_override = false;

    state_.start_count = 0;
    state_.total_runtime_ms = 0;
    state_.last_start_time = 0;
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
    if (!simulation_mode_) {
        pwm_set_duty_cycle(PWM_CHANNEL_AMPLITUDE, duty_cycle);
    }
    SONICATOR_LOG("PWM SET");
}

uint16_t SonicatorInterface::halAdcReadSafe(adc_channel_t channel) {
    if (simulation_mode_) {
        return 272;  // Simulate ~245W power (raw ADC value for testing)
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

    uint8_t pwm_value = 0;
    if (state_.state == SONICATOR_STATE_RUNNING) {
        pwm_value = amplitudeToPwm(state_.amplitude_percent);
    }
    halPwmSetSafe(PWM_AMPLITUDE_CONTROL_PIN, pwm_value);

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

    // Read raw ADC power value (no conversion - cloud handles scaling)
    uint16_t adc_value = halAdcReadSafe(pins_.power_sense_channel);
    state_.power_watts = (float)adc_value;  // Store raw ADC value (0-1023)

    // Read frequency using ISR-based edge counting
    uint8_t freq_channel = pins_.sonicator_id - 1; // Convert to 0-3 range
    state_.frequency_hz = frequency_calculate(freq_channel);
    
    // Update frequency lock status based on measured frequency
    if (state_.frequency_hz >= 18000 && state_.frequency_hz <= 22000) {
        // Frequency is within CT2000 operating range
        state_.frequency_locked = true;
    } else if (state_.frequency_hz == 0) {
        // No valid measurement, use hardware frequency lock pin
        state_.frequency_locked = halGpioReadSafe(pins_.freq_lock_pin);
    } else {
        // Frequency out of range - indicates problem
        state_.frequency_locked = false;
    }
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
        halPwmSetSafe(PWM_AMPLITUDE_CONTROL_PIN, 0);

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
    halPwmSetSafe(PWM_AMPLITUDE_CONTROL_PIN, 0);
    
    state_.state = SONICATOR_STATE_IDLE;
    state_.state_entry_time = getTimestampMs();
    state_.is_running = false;
    state_.start_requested = false;
    state_.stop_requested = false;
    
    SONICATOR_LOG("Emergency stop activated");
    return true;
}

sonicator_state_t SonicatorInterface::update() {
    const uint8_t idx = (pins_.sonicator_id > 0) ? static_cast<uint8_t>(pins_.sonicator_id - 1) : 0;

    // 1) Read MODBUS control for THIS sonicator and apply immediately
    if (modbus_register_map_t* map = register_manager_get_map()) {
        sonicator_registers_t& reg = map->sonicators[idx];

        // Start/Stop command: 1=start, 2=stop (then clear command)
        if (reg.start_stop == 1) {
            (void)start();
            reg.start_stop = 0;
        } else if (reg.start_stop == 2) {
            (void)stop();
            reg.start_stop = 0;
        }

        // Amplitude setpoint (20-100%)
        (void)setAmplitude(static_cast<uint8_t>(reg.amplitude_setpoint));

        // Overload reset pulse (write-one-to-consume)
        if (reg.overload_reset == 1) {
            (void)resetOverload();
            reg.overload_reset = 0;
        }
    }

    // 2) Update internal timers
    uint32_t now = getTimestampMs();
    state_.last_update_time = now;

    // 3) Read hardware inputs and detect faults
    readHardwareInputs();

    sonicator_fault_t faults = checkFaultConditions();
    if (faults != SONICATOR_FAULT_NONE) {
        handleFaultConditions(faults);
    }

    // 4) State machine and outputs
    processStateMachine();
    updateHardwareOutputs();

    // 5) Publish status to MODBUS
    {
        uint16_t flags = 0;
        if (state_.state == SONICATOR_STATE_RUNNING) {
            flags |= SON_STATUS_RUNNING;
        }
        if (state_.overload_active) {
            flags |= SON_STATUS_OVERLOAD;
        }
        if (state_.frequency_locked) {
            flags |= SON_STATUS_FREQ_LOCK;
        }
        if (state_.state == SONICATOR_STATE_FAULT || state_.active_faults != SONICATOR_FAULT_NONE) {
            flags |= SON_STATUS_FAULT;
        }

        const uint16_t power_u16 = static_cast<uint16_t>((state_.power_watts > 0.f) ? state_.power_watts : 0.f);
        const uint16_t freq_hz   = static_cast<uint16_t>(state_.frequency_hz);
        const uint16_t amp_act   = static_cast<uint16_t>(state_.amplitude_percent);

        register_manager_update_sonicator_status(idx, power_u16, freq_hz, amp_act, flags);
    }

    // 6) Watchdog service
    state_.watchdog_last_reset = now;

    return state_.state;
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

void SonicatorInterface::buildStatusView_() const {
    status_view_.is_running       = state_.is_running;
    status_view_.frequency_hz     = state_.frequency_hz;
    status_view_.overload_active  = state_.overload_active;
    status_view_.frequency_locked = state_.frequency_locked;
    status_view_.fault_count      = state_.fault_count;
    status_view_.power_raw_adc    = static_cast<uint16_t>((state_.power_watts > 0.0f) ? state_.power_watts : 0.0f);
    status_view_.last_fault_time  = state_.last_fault_time;
    status_view_.amplitude_actual = state_.amplitude_percent;

    status_view_.state_machine.state            = state_.state;
    status_view_.state_machine.previous_state   = state_.previous_state;
    status_view_.state_machine.state_entry_time = state_.state_entry_time;
}

void SonicatorInterface::setSimulationMode(bool enable) {
    simulation_mode_ = enable;
    SONICATOR_LOG(enable ? "Simulation mode enabled" : "Simulation mode disabled");
}

