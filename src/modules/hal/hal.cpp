/**
 * @file hal.cpp
 * @brief Hardware Abstraction Layer Master Implementation for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

#include "hal.h"
#include <config.h>

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static bool hal_initialized = false;
static uint16_t error_count = 0;
static uint32_t init_time = 0;

// ============================================================================
// PRIVATE FUNCTION DECLARATIONS
// ============================================================================

static hal_result_t map_gpio_error(gpio_result_t result);
static hal_result_t map_adc_error(adc_result_t result);
static hal_result_t map_pwm_error(pwm_result_t result);
static hal_result_t map_uart_error(uart_result_t result);
static hal_result_t map_timer_error(timer_result_t result);

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

hal_result_t hal_init(void) {
    hal_result_t result = HAL_OK;
    
    // Initialize Timer first (needed for other subsystems)
    timer_result_t timer_result = timer_init();
    if (timer_result != TIMER_OK) {
        return map_timer_error(timer_result);
    }
    
    // Initialize GPIO subsystem
    gpio_result_t gpio_result = gpio_init();
    if (gpio_result != GPIO_OK) {
        return map_gpio_error(gpio_result);
    }
    
    // Initialize ADC subsystem
    adc_result_t adc_result = adc_init();
    if (adc_result != ADC_OK) {
        return map_adc_error(adc_result);
    }
    
    // Initialize PWM subsystem
    pwm_result_t pwm_result = pwm_init();
    if (pwm_result != PWM_OK) {
        return map_pwm_error(pwm_result);
    }
    
    // Initialize UART subsystem
    uart_result_t uart_result = uart_init();
    if (uart_result != UART_OK) {
        return map_uart_error(uart_result);
    }
    
    // Record initialization time and mark as complete
    timer_get_millis(&init_time);
    error_count = 0;
    hal_initialized = true;
    
    return HAL_OK;
}

hal_result_t hal_self_test(bool* gpio_ok, bool* adc_ok, bool* pwm_ok, 
                          bool* uart_ok, bool* timer_ok) {
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    // Test GPIO subsystem
    if (gpio_ok != nullptr) {
        // Test status LED control
        gpio_result_t gpio_result = gpio_status_led(GPIO_HIGH);
        timer_delay_ms(100);
        if (gpio_status_led(GPIO_LOW) != GPIO_OK) {
            *gpio_ok = false;
        } else {
            // Test sonicator pin reading (should be pulled up)
            bool overload_state;
            gpio_result_t r2 = gpio_sonicator_read_overload(1, &overload_state);
            *gpio_ok = (gpio_result == GPIO_OK) && (r2 == GPIO_OK);
        }
    }
    
    // Test ADC subsystem
    if (adc_ok != nullptr) {
        // Test power channel reading
        float power_reading;
        adc_result_t adc_result = adc_read_sonicator_power(1, &power_reading);
        
        // Test frequency channel reading
        float freq_reading;
        adc_result_t r2 = adc_read_frequency(&freq_reading);

        *adc_ok = (adc_result == ADC_OK) && (r2 == ADC_OK) && (power_reading >= 0.0f) && (freq_reading >= 0.0f);
    }
    
    // Test PWM subsystem
    if (pwm_ok != nullptr) {
        pwm_result_t pwm_result = pwm_test_pattern();
        *pwm_ok = (pwm_result == PWM_OK);
    }
    
    // Test UART subsystem
    if (uart_ok != nullptr) {
        uart_result_t uart_result = uart_test_loopback(uart_ok);
        if (uart_result != UART_OK) {
            *uart_ok = false;
        }
    }
    
    // Test Timer subsystem
    if (timer_ok != nullptr) {
        timer_result_t timer_result = timer_test_all(timer_ok);
        if (timer_result != TIMER_OK) {
            *timer_ok = false;
        }
    }
    
    return HAL_OK;
}

hal_result_t hal_get_status(bool* initialized, uint32_t* uptime_ms, uint16_t* errors) {
    if (initialized != nullptr) {
        *initialized = hal_initialized;
    }
    
    if (uptime_ms != nullptr) {
        if (hal_initialized) {
            uint32_t current_time;
            timer_get_millis(&current_time);
            *uptime_ms = current_time - init_time;
        } else {
            *uptime_ms = 0;
        }
    }
    
    if (errors != nullptr) {
        *errors = error_count;
    }
    
    return HAL_OK;
}

hal_result_t hal_clear_errors(void) {
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    error_count = 0;
    
    // Clear errors in all subsystems
    uart_clear_errors();
    
    return HAL_OK;
}

hal_result_t hal_emergency_shutdown(void) {
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    // Stop all sonicators immediately
    for (uint8_t i = 1; i <= MAX_SONICATORS; i++) {
        gpio_sonicator_stop(i);
    }
    
    // Set amplitude to minimum and disable PWM
    pwm_emergency_stop();
    pwm_disable_channel(PWM_CHANNEL_AMPLITUDE);
    
    // Turn off status LED
    gpio_status_led(GPIO_LOW);
    
    // Flush UART buffers
    uart_flush_tx();
    uart_flush_rx();
    
    // Reset watchdog timer
    timer_watchdog_reset();
    
    return HAL_OK;
}

// ============================================================================
// SONICATOR CONTROL FUNCTIONS
// ============================================================================

hal_result_t hal_control_sonicator(uint8_t sonicator_id, const sonicator_control_t* control) {
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    if (control == nullptr || sonicator_id < 1 || sonicator_id > MAX_SONICATORS) {
        return HAL_ERROR_GPIO;
    }
    
    // Handle start/stop control
    gpio_result_t gpio_result = GPIO_OK;
    if (control->start) {
        gpio_result = gpio_sonicator_start(sonicator_id);
    } else {
        gpio_result = gpio_sonicator_stop(sonicator_id);
    }
    
    if (gpio_result != GPIO_OK) {
        error_count++;
        return map_gpio_error(gpio_result);
    }
    
    // Handle amplitude control (shared for all sonicators)
    if (control->amplitude_percent >= PWM_AMPLITUDE_MIN && 
        control->amplitude_percent <= PWM_AMPLITUDE_MAX) {
        pwm_result_t pwm_result = pwm_set_amplitude(control->amplitude_percent);
        if (pwm_result != PWM_OK) {
            error_count++;
            return map_pwm_error(pwm_result);
        }
    }
    
    // Handle overload reset
    if (control->reset_overload) {
        gpio_result = gpio_sonicator_reset(sonicator_id);
        if (gpio_result != GPIO_OK) {
            error_count++;
            return map_gpio_error(gpio_result);
        }
    }
    
    return HAL_OK;
}

hal_result_t hal_read_sonicator_status(uint8_t sonicator_id, sonicator_status_t* status) {
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    if (status == nullptr || sonicator_id < 1 || sonicator_id > MAX_SONICATORS) {
        return HAL_ERROR_GPIO;
    }
    
    // Read overload status
    gpio_result_t gpio_result = gpio_sonicator_read_overload(sonicator_id, &status->overload);
    if (gpio_result != GPIO_OK) {
        error_count++;
        return map_gpio_error(gpio_result);
    }
    
    // Read frequency lock status
    gpio_result = gpio_sonicator_read_freq_lock(sonicator_id, &status->frequency_locked);
    if (gpio_result != GPIO_OK) {
        error_count++;
        return map_gpio_error(gpio_result);
    }
    
    // Read power level
    adc_result_t adc_result = adc_read_sonicator_power(sonicator_id, &status->power_watts);
    if (adc_result != ADC_OK) {
        error_count++;
        return map_adc_error(adc_result);
    }
    
    // Read frequency (available for Sonicator 4 via LM2907 F-V converter on ADC0)
    if (sonicator_id == 4) {
        float frequency_hz;
        adc_result = adc_read_frequency(&frequency_hz);
        if (adc_result == ADC_OK) {
            status->frequency_hz = (uint16_t)frequency_hz;
        } else {
            status->frequency_hz = 0;
        }
    } else {
        status->frequency_hz = 0; // Not measured for other sonicators in current hardware
    }

    return HAL_OK;
}

hal_result_t hal_control_all_sonicators(const sonicator_control_t control_array[4]) {
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    if (control_array == nullptr) {
        return HAL_ERROR_GPIO;
    }
    
    hal_result_t result = HAL_OK;
    
    // Control each sonicator
    for (uint8_t i = 0; i < 4; i++) {
        hal_result_t individual_result = hal_control_sonicator(i + 1, &control_array[i]);
        if (individual_result != HAL_OK) {
            result = individual_result; // Return last error
        }
    }
    
    return result;
}

hal_result_t hal_read_all_sonicator_status(sonicator_status_t status_array[4]) {
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    if (status_array == nullptr) {
        return HAL_ERROR_GPIO;
    }
    
    hal_result_t result = HAL_OK;
    
    // Read status for each sonicator
    for (uint8_t i = 0; i < 4; i++) {
        hal_result_t individual_result = hal_read_sonicator_status(i + 1, &status_array[i]);
        if (individual_result != HAL_OK) {
            result = individual_result; // Return last error
        }
    }
    
    return result;
}

hal_result_t hal_emergency_stop_all(void) {
    if (!hal_initialized) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    // Stop all sonicators
    for (uint8_t i = 1; i <= MAX_SONICATORS; i++) {
        gpio_sonicator_stop(i);
    }
    
    // Set amplitude to minimum
    pwm_emergency_stop();
    
    return HAL_OK;
}

// ============================================================================
// PRIVATE FUNCTION IMPLEMENTATIONS
// ============================================================================

static hal_result_t map_gpio_error(gpio_result_t result) {
    return (result == GPIO_OK) ? HAL_OK : HAL_ERROR_GPIO;
}

static hal_result_t map_adc_error(adc_result_t result) {
    return (result == ADC_OK) ? HAL_OK : HAL_ERROR_ADC;
}

static hal_result_t map_pwm_error(pwm_result_t result) {
    return (result == PWM_OK) ? HAL_OK : HAL_ERROR_PWM;
}

static hal_result_t map_uart_error(uart_result_t result) {
    return (result == UART_OK) ? HAL_OK : HAL_ERROR_UART;
}

static hal_result_t map_timer_error(timer_result_t result) {
    return (result == TIMER_OK) ? HAL_OK : HAL_ERROR_TIMER;
}
