/**
 * @file pwm.h
 * @brief PWM Hardware Abstraction Layer for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Provides hardware-agnostic PWM interface for sonicator amplitude control.
 * Uses Timer2 to generate PWM signal on PD7 (pin 21) for shared amplitude
 * control of all 4 sonicators.
 * 
 * PWM signal feeds 1kΩ + 10µF RC filter and LM358N op-amp for 0-10V output.
 * Pin assignments per docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH).
 */

#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @brief PWM channel identifiers
 */
typedef enum {
    PWM_CHANNEL_AMPLITUDE = 0,  //< Amplitude control channel (PD7)
    PWM_CHANNEL_MAX = 1
} pwm_channel_t;

/**
 * @brief PWM timer prescaler values
 */
typedef enum {
    PWM_PRESCALER_1 = 1,        //< No prescaling
    PWM_PRESCALER_8 = 2,        //< Divide by 8
    PWM_PRESCALER_32 = 3,       //< Divide by 32
    PWM_PRESCALER_64 = 4,       //< Divide by 64
    PWM_PRESCALER_128 = 5,      //< Divide by 128
    PWM_PRESCALER_256 = 6,      //< Divide by 256
    PWM_PRESCALER_1024 = 7      //< Divide by 1024
} pwm_prescaler_t;

/**
 * @brief PWM waveform generation modes
 */
typedef enum {
    PWM_MODE_FAST_8BIT = 0,     //< Fast PWM, 8-bit (0-255)
    PWM_MODE_FAST_9BIT,         //< Fast PWM, 9-bit (0-511)
    PWM_MODE_FAST_10BIT,        //< Fast PWM, 10-bit (0-1023)
    PWM_MODE_PHASE_CORRECT_8BIT,//< Phase correct PWM, 8-bit
    PWM_MODE_PHASE_CORRECT_9BIT,//< Phase correct PWM, 9-bit
    PWM_MODE_PHASE_CORRECT_10BIT//< Phase correct PWM, 10-bit
} pwm_mode_t;

/**
 * @brief HAL function return codes
 */
typedef enum {
    PWM_OK = 0,                 //< Operation successful
    PWM_ERROR_INVALID_CHANNEL,  //< Invalid channel number
    PWM_ERROR_INVALID_DUTY,     //< Invalid duty cycle value
    PWM_ERROR_INVALID_FREQ,     //< Invalid frequency value
    PWM_ERROR_NOT_INITIALIZED   //< PWM not initialized
} pwm_result_t;

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define PWM_DEFAULT_FREQUENCY   1000        //< Default PWM frequency (1kHz)
#define PWM_MIN_FREQUENCY       100         //< Minimum PWM frequency (100Hz)
#define PWM_MAX_FREQUENCY       50000       //< Maximum PWM frequency (50kHz)

#define PWM_DUTY_MIN           0            //< Minimum duty cycle (0%)
#define PWM_DUTY_MAX           100          //< Maximum duty cycle (100%)

#define PWM_AMPLITUDE_MIN      20           //< Minimum amplitude (20%)
#define PWM_AMPLITUDE_MAX      100          //< Maximum amplitude (100%)

// Filter characteristics (per schematic)
#define PWM_FILTER_CUTOFF_HZ   16           //< RC filter cutoff frequency
#define PWM_OPAMP_GAIN         2.0f         //< LM358N gain (0-5V → 0-10V)

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

/**
 * @brief Initialize PWM hardware abstraction layer
 * 
 * Configures Timer2 for Fast PWM mode at 1kHz frequency.
 * Sets up amplitude control channel on PD7 with safe initial state (20%).
 * 
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_init(void);

/**
 * @brief Configure PWM frequency for specified channel
 * 
 * @param channel PWM channel identifier
 * @param frequency Desired frequency in Hz (100-50000)
 * @return PWM_OK on success, error code on failure
 * 
 * @note Frequency affects all channels using the same timer
 */
pwm_result_t pwm_set_frequency(pwm_channel_t channel, uint32_t frequency);

/**
 * @brief Set duty cycle for specified PWM channel
 * 
 * @param channel PWM channel identifier
 * @param duty_percent Duty cycle percentage (0-100)
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_set_duty_cycle(pwm_channel_t channel, uint8_t duty_percent);

/**
 * @brief Enable PWM output on specified channel
 * 
 * @param channel PWM channel identifier
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_enable_channel(pwm_channel_t channel);

/**
 * @brief Disable PWM output on specified channel
 * 
 * @param channel PWM channel identifier
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_disable_channel(pwm_channel_t channel);

/**
 * @brief Get current duty cycle for specified channel
 * 
 * @param channel PWM channel identifier
 * @param duty_percent Pointer to store current duty cycle
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_get_duty_cycle(pwm_channel_t channel, uint8_t* duty_percent);

/**
 * @brief Get current frequency for specified channel
 * 
 * @param channel PWM channel identifier
 * @param frequency Pointer to store current frequency
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_get_frequency(pwm_channel_t channel, uint32_t* frequency);

/**
 * @brief Check if PWM channel is enabled
 * 
 * @param channel PWM channel identifier
 * @param enabled Pointer to store enable status
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_is_enabled(pwm_channel_t channel, bool* enabled);

// ============================================================================
// AMPLITUDE CONTROL CONVENIENCE FUNCTIONS
// ============================================================================

/**
 * @brief Set sonicator amplitude percentage
 * 
 * Converts amplitude percentage to appropriate duty cycle for 0-10V output.
 * Enforces minimum 20% amplitude for stable operation.
 * 
 * @param amplitude_percent Amplitude percentage (20-100)
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_set_amplitude(uint8_t amplitude_percent);

/**
 * @brief Get current amplitude setting
 * 
 * @param amplitude_percent Pointer to store current amplitude
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_get_amplitude(uint8_t* amplitude_percent);

/**
 * @brief Set amplitude to safe minimum (20%)
 * 
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_amplitude_minimum(void);

/**
 * @brief Set amplitude to maximum (100%)
 * 
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_amplitude_maximum(void);

/**
 * @brief Ramp amplitude from current to target over time
 * 
 * @param target_percent Target amplitude percentage
 * @param ramp_time_ms Time to reach target in milliseconds
 * @return PWM_OK on success, error code on failure
 * 
 * @note Non-blocking function, use pwm_ramp_complete() to check status
 */
pwm_result_t pwm_amplitude_ramp(uint8_t target_percent, uint16_t ramp_time_ms);

/**
 * @brief Check if amplitude ramp is complete
 * 
 * @param complete Pointer to store completion status
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_ramp_complete(bool* complete);

/**
 * @brief Emergency stop - set amplitude to minimum immediately
 * 
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_emergency_stop(void);

// ============================================================================
// DIAGNOSTIC AND TESTING FUNCTIONS
// ============================================================================

/**
 * @brief Convert duty cycle to output voltage (theoretical)
 * 
 * @param duty_percent Duty cycle percentage
 * @param output_voltage Pointer to store calculated voltage
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_duty_to_voltage(uint8_t duty_percent, float* output_voltage);

/**
 * @brief Convert amplitude to output voltage
 * 
 * @param amplitude_percent Amplitude percentage (20-100)
 * @param output_voltage Pointer to store calculated voltage (2-10V)
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_amplitude_to_voltage(uint8_t amplitude_percent, float* output_voltage);

/**
 * @brief Test PWM output with known pattern
 * 
 * Generates test pattern for HIL verification:
 * 25% → 50% → 75% → 100% → 25% (1 second each)
 * 
 * @return PWM_OK on success, error code on failure
 */
pwm_result_t pwm_test_pattern(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_PWM_H
