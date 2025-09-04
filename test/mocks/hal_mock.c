/**
 * @file hal_mock.c
 * @brief Mock implementation of HAL functions for unit testing
 * @author Cannasol Technologies
 * @date 2025-09-04
 * @version 1.0.0
 */

#ifdef UNIT_TEST

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Mock HAL types and constants
#define HAL_OK 0
#define HAL_ERROR_GPIO 1
#define HAL_ERROR_ADC 2
#define HAL_ERROR_PWM 3
#define HAL_ERROR_UART 4
#define HAL_ERROR_TIMER 5
#define HAL_ERROR_NOT_INITIALIZED 6

typedef int hal_result_t;

typedef struct {
    uint8_t id;
    uint8_t state;
    float power_watts;
    uint16_t frequency_hz;
    uint8_t amplitude_percent;
} sonicator_status_t;

// Mock state variables
static bool mock_initialized = false;
static uint32_t mock_uptime = 0;
static uint16_t mock_errors = 0;

// Mock function implementations
hal_result_t hal_init(void) {
    mock_initialized = true;
    mock_uptime = 0;
    mock_errors = 0;
    return HAL_OK;
}

hal_result_t hal_self_test(bool* gpio_ok, bool* adc_ok, bool* pwm_ok, bool* uart_ok, bool* timer_ok) {
    if (!gpio_ok || !adc_ok || !pwm_ok || !uart_ok || !timer_ok) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    *gpio_ok = true;
    *adc_ok = true;
    *pwm_ok = true;
    *uart_ok = true;
    *timer_ok = true;
    
    return HAL_OK;
}

hal_result_t hal_emergency_shutdown(void) {
    return HAL_OK;
}

hal_result_t hal_get_status(bool* initialized, uint32_t* uptime_ms, uint16_t* errors) {
    if (!initialized || !uptime_ms || !errors) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    *initialized = mock_initialized;
    *uptime_ms = mock_uptime;
    *errors = mock_errors;
    
    return HAL_OK;
}

hal_result_t hal_read_all_sonicator_status(sonicator_status_t* status_array) {
    if (!status_array) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    for (int i = 0; i < 4; i++) {
        status_array[i].id = i + 1;
        status_array[i].state = 0; // Idle
        status_array[i].power_watts = 25.5f + i * 10.0f;
        status_array[i].frequency_hz = 25000 + i * 1000;
        status_array[i].amplitude_percent = 50 + i * 10;
    }
    
    return HAL_OK;
}

hal_result_t hal_read_sonicator_status(uint8_t id, sonicator_status_t* status) {
    if (id < 1 || id > 4) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    if (!status) {
        return HAL_ERROR_NOT_INITIALIZED;
    }
    
    status->id = id;
    status->state = 0; // Idle
    status->power_watts = 25.5f + (id - 1) * 10.0f;
    status->frequency_hz = 25000 + (id - 1) * 1000;
    status->amplitude_percent = 50 + (id - 1) * 10;
    
    return HAL_OK;
}

#endif // UNIT_TEST
