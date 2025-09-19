/**
 * @file pwm.cpp
 * @brief PWM Hardware Abstraction Layer Implementation for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

/**
 * @defgroup HAL_PWM PWM HAL (ATmega32A)
 * @ingroup HAL
 * @brief Implementation notes for amplitude PWM generation on PD7.
 *
 * @section hal_pwm_overview Overview
 * - Uses Timer2 Fast PWM to produce a duty cycle proportional to amplitude.
 * - Downstream RC filter (1kΩ/10µF) and LM358 gain of 2 produce 0–10 V output.
 * - Channel multiplexing: a single PWM drives all four sonicators' amplitude.
 *
 * @section hal_pwm_limits Limits
 * - Frequency defaults to 1 kHz; see PWM_MIN/MAX_FREQUENCY.
 * - Enforces amplitude 20–100% (stable op-amp region; see system_config.h).
 *
 * @section hal_pwm_concurrency Concurrency
 * - Non‑blocking API; ramping performed cooperatively in pwm_ramp_complete().
 * - No dynamic allocation; minimal static state for embedded safety.
 *
 * @section hal_pwm_errors Error Handling
 * - Returns PWM_OK or PWM_ERROR_* codes for invalid params or state.
 *
 * @section hal_pwm_related Related
 * - @see pwm.h Public API and constants
 * - @see system_config.h PWM_AMPLITUDE_CONTROL_PIN, analog front‑end notes
 */


#include "pwm.h"
#include <system_config.h>
#include <Arduino.h>

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static bool pwm_initialized = false;
static uint32_t current_frequency = PWM_DEFAULT_FREQUENCY;
static uint8_t current_duty_cycle = 0;
static bool channel_enabled = false;

// Ramping variables
static bool ramp_active = false;
static uint8_t ramp_start_amplitude = 0;
static uint8_t ramp_target_amplitude = 0;
static uint32_t ramp_start_time = 0;
static uint16_t ramp_duration_ms = 0;

// ============================================================================
// PRIVATE FUNCTION DECLARATIONS
// ============================================================================

static bool is_valid_channel(pwm_channel_t channel);
static pwm_result_t calculate_timer_settings(uint32_t frequency, uint8_t* prescaler, uint8_t* top_value);
static uint8_t amplitude_to_duty_cycle(uint8_t amplitude_percent);
static uint8_t duty_cycle_to_amplitude(uint8_t duty_percent);

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

pwm_result_t pwm_init(void) {
    // Configure PWM pin as output
    pinMode(PWM_AMPLITUDE_CONTROL_PIN, OUTPUT);

    // Initialize Timer2 for Fast PWM mode
    // WGM21:WGM20 = 11 (Fast PWM mode, TOP = 0xFF)
    TCCR2 |= (1 << WGM21) | (1 << WGM20);

    // Set initial frequency (1kHz default)
    pwm_result_t result = pwm_set_frequency(PWM_CHANNEL_AMPLITUDE, PWM_DEFAULT_FREQUENCY);
    if (result != PWM_OK) {
        return result;
    }

    // Set safe initial amplitude (20% minimum)
    result = pwm_set_amplitude(PWM_AMPLITUDE_MIN);
    if (result != PWM_OK) {
        return result;
    }

    // Disable channel initially for safety
    pwm_disable_channel(PWM_CHANNEL_AMPLITUDE);

    pwm_initialized = true;
    return PWM_OK;
}

pwm_result_t pwm_set_frequency(pwm_channel_t channel, uint32_t frequency) {
    if (!pwm_initialized) {
        return PWM_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_channel(channel)) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    if (frequency < PWM_MIN_FREQUENCY || frequency > PWM_MAX_FREQUENCY) {
        return PWM_ERROR_INVALID_FREQ;
    }

    uint8_t prescaler, top_value;
    pwm_result_t result = calculate_timer_settings(frequency, &prescaler, &top_value);
    if (result != PWM_OK) {
        return result;
    }

    // Set prescaler (CS22:CS20 bits)
    TCCR2 &= ~((1 << CS22) | (1 << CS21) | (1 << CS20));
    TCCR2 |= prescaler;

    current_frequency = frequency;
    return PWM_OK;
}

pwm_result_t pwm_set_duty_cycle(pwm_channel_t channel, uint8_t duty_percent) {
    if (!pwm_initialized) {
        return PWM_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_channel(channel)) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    if (duty_percent > PWM_DUTY_MAX) {
        return PWM_ERROR_INVALID_DUTY;
    }

    // Calculate OCR2 value for desired duty cycle
    uint16_t ocr_value = (duty_percent * 255UL) / 100;
    OCR2 = (uint8_t)ocr_value;

    current_duty_cycle = duty_percent;
    return PWM_OK;
}

pwm_result_t pwm_enable_channel(pwm_channel_t channel) {
    if (!pwm_initialized) {
        return PWM_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_channel(channel)) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    // Enable PWM output (COM21:COM20 = 10, non-inverting mode)
    TCCR2 |= (1 << COM21);
    TCCR2 &= ~(1 << COM20);

    channel_enabled = true;
    return PWM_OK;
}

pwm_result_t pwm_disable_channel(pwm_channel_t channel) {
    if (!pwm_initialized) {
        return PWM_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_channel(channel)) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    // Disable PWM output (COM21:COM20 = 00)
    TCCR2 &= ~((1 << COM21) | (1 << COM20));

    // Ensure pin is LOW when disabled
    digitalWrite(PWM_AMPLITUDE_CONTROL_PIN, LOW);

    channel_enabled = false;
    return PWM_OK;
}

pwm_result_t pwm_get_duty_cycle(pwm_channel_t channel, uint8_t* duty_percent) {
    if (!pwm_initialized) {
        return PWM_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_channel(channel) || duty_percent == nullptr) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    *duty_percent = current_duty_cycle;
    return PWM_OK;
}

pwm_result_t pwm_get_frequency(pwm_channel_t channel, uint32_t* frequency) {
    if (!pwm_initialized) {
        return PWM_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_channel(channel) || frequency == nullptr) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    *frequency = current_frequency;
    return PWM_OK;
}

pwm_result_t pwm_is_enabled(pwm_channel_t channel, bool* enabled) {
    if (!pwm_initialized) {
        return PWM_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_channel(channel) || enabled == nullptr) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    *enabled = channel_enabled;
    return PWM_OK;
}

// ============================================================================
// AMPLITUDE CONTROL FUNCTIONS
// ============================================================================

pwm_result_t pwm_set_amplitude(uint8_t amplitude_percent) {
    if (amplitude_percent < PWM_AMPLITUDE_MIN || amplitude_percent > PWM_AMPLITUDE_MAX) {
        return PWM_ERROR_INVALID_DUTY;
    }

    uint8_t duty_cycle = amplitude_to_duty_cycle(amplitude_percent);
    return pwm_set_duty_cycle(PWM_CHANNEL_AMPLITUDE, duty_cycle);
}

pwm_result_t pwm_get_amplitude(uint8_t* amplitude_percent) {
    if (amplitude_percent == nullptr) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    *amplitude_percent = duty_cycle_to_amplitude(current_duty_cycle);
    return PWM_OK;
}

pwm_result_t pwm_amplitude_minimum(void) {
    return pwm_set_amplitude(PWM_AMPLITUDE_MIN);
}

pwm_result_t pwm_amplitude_maximum(void) {
    return pwm_set_amplitude(PWM_AMPLITUDE_MAX);
}

pwm_result_t pwm_amplitude_ramp(uint8_t target_percent, uint16_t ramp_time_ms) {
    if (target_percent < PWM_AMPLITUDE_MIN || target_percent > PWM_AMPLITUDE_MAX) {
        return PWM_ERROR_INVALID_DUTY;
    }

    if (ramp_time_ms == 0) {
        return pwm_set_amplitude(target_percent);
    }

    // Initialize ramp parameters
    ramp_start_amplitude = duty_cycle_to_amplitude(current_duty_cycle);
    ramp_target_amplitude = target_percent;
    ramp_start_time = millis();
    ramp_duration_ms = ramp_time_ms;
    ramp_active = true;

    return PWM_OK;
}

pwm_result_t pwm_ramp_complete(bool* complete) {
    if (complete == nullptr) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    if (!ramp_active) {
        *complete = true;
        return PWM_OK;
    }

    uint32_t elapsed_time = millis() - ramp_start_time;

    if (elapsed_time >= ramp_duration_ms) {
        // Ramp complete
        pwm_set_amplitude(ramp_target_amplitude);
        ramp_active = false;
        *complete = true;
    } else {
        // Update current amplitude based on ramp progress
        float progress = (float)elapsed_time / ramp_duration_ms;
        uint8_t current_amplitude = ramp_start_amplitude +
            (uint8_t)((ramp_target_amplitude - ramp_start_amplitude) * progress);
        pwm_set_amplitude(current_amplitude);
        *complete = false;
    }

    return PWM_OK;
}

pwm_result_t pwm_emergency_stop(void) {
    // Stop any active ramping
    ramp_active = false;

    // Immediately set to minimum amplitude
    return pwm_amplitude_minimum();
}

// ============================================================================
// DIAGNOSTIC FUNCTIONS
// ============================================================================

pwm_result_t pwm_duty_to_voltage(uint8_t duty_percent, float* output_voltage) {
    if (output_voltage == nullptr) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    if (duty_percent > PWM_DUTY_MAX) {
        return PWM_ERROR_INVALID_DUTY;
    }

    // PWM voltage before filtering and amplification
    float pwm_voltage = (duty_percent * 5.0f) / 100.0f;

    // After RC filter and op-amp gain
    *output_voltage = pwm_voltage * PWM_OPAMP_GAIN;

    return PWM_OK;
}

pwm_result_t pwm_amplitude_to_voltage(uint8_t amplitude_percent, float* output_voltage) {
    if (output_voltage == nullptr) {
        return PWM_ERROR_INVALID_CHANNEL;
    }

    if (amplitude_percent < PWM_AMPLITUDE_MIN || amplitude_percent > PWM_AMPLITUDE_MAX) {
        return PWM_ERROR_INVALID_DUTY;
    }

    // Linear mapping: 20% → 2V, 100% → 10V
    *output_voltage = 2.0f + ((amplitude_percent - PWM_AMPLITUDE_MIN) * 8.0f) /
                     (PWM_AMPLITUDE_MAX - PWM_AMPLITUDE_MIN);

    return PWM_OK;
}

pwm_result_t pwm_test_pattern(void) {
    if (!pwm_initialized) {
        return PWM_ERROR_NOT_INITIALIZED;
    }

    uint8_t test_amplitudes[] = {25, 50, 75, 100, 25};
    uint8_t num_steps = sizeof(test_amplitudes) / sizeof(test_amplitudes[0]);

    pwm_enable_channel(PWM_CHANNEL_AMPLITUDE);

    for (uint8_t i = 0; i < num_steps; i++) {
        pwm_result_t result = pwm_set_amplitude(test_amplitudes[i]);
        if (result != PWM_OK) {
            return result;
        }
        delay(1000); // 1 second per step
    }

    return PWM_OK;
}

// ============================================================================
// PRIVATE FUNCTION IMPLEMENTATIONS
// ============================================================================

static inline bool is_valid_channel(pwm_channel_t channel) {
    return (channel < PWM_CHANNEL_MAX);
}

static pwm_result_t calculate_timer_settings(uint32_t frequency, uint8_t* prescaler, uint8_t* top_value) {
    if (prescaler == nullptr || top_value == nullptr) {
        return PWM_ERROR_INVALID_FREQ;
    }

    // For Fast PWM mode with 8-bit resolution, TOP = 255
    *top_value = 255;

    // Calculate required prescaler for desired frequency
    // f_pwm = f_cpu / (prescaler * 256)
    uint32_t required_divisor = F_CPU / (frequency * 256UL);

    if (required_divisor <= 1) {
        *prescaler = PWM_PRESCALER_1;
    } else if (required_divisor <= 8) {
        *prescaler = PWM_PRESCALER_8;
    } else if (required_divisor <= 32) {
        *prescaler = PWM_PRESCALER_32;
    } else if (required_divisor <= 64) {
        *prescaler = PWM_PRESCALER_64;
    } else if (required_divisor <= 128) {
        *prescaler = PWM_PRESCALER_128;
    } else if (required_divisor <= 256) {
        *prescaler = PWM_PRESCALER_256;
    } else if (required_divisor <= 1024) {
        *prescaler = PWM_PRESCALER_1024;
    } else {
        return PWM_ERROR_INVALID_FREQ;
    }

    return PWM_OK;
}

static inline uint8_t amplitude_to_duty_cycle(uint8_t amplitude_percent) {
    // Map amplitude (20-100%) to duty cycle (20-100%)
    // This assumes linear relationship between duty cycle and output voltage
    if (amplitude_percent < PWM_AMPLITUDE_MIN) {
        amplitude_percent = PWM_AMPLITUDE_MIN;
    }
    if (amplitude_percent > PWM_AMPLITUDE_MAX) {
        amplitude_percent = PWM_AMPLITUDE_MAX;
    }

    return amplitude_percent;
}

static inline uint8_t duty_cycle_to_amplitude(uint8_t duty_percent) {
    // Convert duty cycle back to amplitude percentage
    if (duty_percent < PWM_AMPLITUDE_MIN) {
        return PWM_AMPLITUDE_MIN;
    }
    if (duty_percent > PWM_AMPLITUDE_MAX) {
        return PWM_AMPLITUDE_MAX;
    }

    return duty_percent;
}
