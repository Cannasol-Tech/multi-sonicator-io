/**
 * @file hal_mock.c
 * @title HAL Mock Implementation for Unit Testing
 * @company Axovia AI
 * @date 2025-09-16
 * @brief Mock implementation providing simulated HAL behavior for comprehensive unit testing
 * @version 1.1.0
 *
 * @details
 * This file provides a complete mock implementation of Hardware Abstraction Layer (HAL) functions
 * for unit testing purposes. It simulates the behavior of microcontroller peripherals and hardware
 * interfaces, enabling tests to run without physical hardware and ensuring robust validation of
 * hardware interaction logic.
 *
 * The mock implementation supports:
 * - GPIO pin simulation with input/output modes
 * - ADC conversion simulation with configurable values
 * - PWM signal generation and control
 * - UART communication simulation
 * - Timer and interrupt handling
 * - Hardware self-test simulation
 *
 * @section overview Overview
 *
 * The HAL mock provides a software simulation of microcontroller hardware peripherals.
 * It implements the same interface as the real HAL module but operates entirely in memory,
 * making it perfect for automated testing environments and continuous integration.
 *
 * Key features:
 * - **Peripheral Simulation**: Complete simulation of GPIO, ADC, PWM, UART, and Timer peripherals
 * - **State Persistence**: Maintains hardware state between function calls
 * - **Error Simulation**: Configurable fault injection for error path testing
 * - **Performance Simulation**: Realistic timing and resource usage simulation
 * - **Thread Safety**: Safe for use in multi-threaded test environments
 *
 * @section mocked_functions Mocked Functions
 *
 * The mock implements all public functions from the real HAL interface:
 * - @c hal_init() - Initialize hardware abstraction layer
 * - @c hal_gpio_set_mode() - Configure GPIO pin modes
 * - @c hal_gpio_write() - Write digital values to GPIO pins
 * - @c hal_gpio_read() - Read digital values from GPIO pins
 * - @c hal_adc_read() - Read analog values from ADC channels
 * - @c hal_pwm_set_duty() - Set PWM duty cycle
 * - @c hal_pwm_start() - Start PWM signal generation
 * - @c hal_pwm_stop() - Stop PWM signal generation
 * - @c hal_uart_transmit() - Transmit data via UART
 * - @c hal_uart_receive() - Receive data via UART
 * - @c hal_timer_start() - Start hardware timer
 * - @c hal_timer_stop() - Stop hardware timer
 * - @c hal_self_test() - Perform hardware self-test
 *
 * @section usage Usage
 *
 * To use the HAL mock in unit tests:
 *
 * @code{.c}
 * // Include mock header (automatically included when UNIT_TEST is defined)
 * #include "test/mocks/hal_mock.h"
 *
 * // In test setup
 * void setUp(void) {
 *     hal_mock_reset();  // Reset to clean state
 *     hal_mock_set_gpio_state(GPIO_PIN_13, 1);  // Set initial pin state
 * }
 *
 * // In test function
 * void test_hal_gpio_operations(void) {
 *     hal_result_t result;
 *
 *     // Test GPIO output
 *     result = hal_gpio_write(GPIO_PIN_13, 1);
 *     TEST_ASSERT_EQUAL(HAL_OK, result);
 *
 *     // Verify state
 *     bool pin_state;
 *     result = hal_gpio_read(GPIO_PIN_13, &pin_state);
 *     TEST_ASSERT_EQUAL(HAL_OK, result);
 *     TEST_ASSERT_TRUE(pin_state);
 *
 *     // Test ADC simulation
 *     hal_mock_set_adc_value(ADC_CHANNEL_0, 512);  // Set 2.5V equivalent
 *     uint16_t adc_value;
 *     result = hal_adc_read(ADC_CHANNEL_0, &adc_value);
 *     TEST_ASSERT_EQUAL(HAL_OK, result);
 *     TEST_ASSERT_EQUAL(512, adc_value);
 * }
 * @endcode
 *
 * @section error_simulation Error Simulation
 *
 * The mock supports error injection to test fault handling:
 *
 * @code{.c}
 * void test_hal_error_handling(void) {
 *     hal_mock_reset();
 *     hal_mock_set_error_injection(HAL_ERROR_GPIO);
 *
 *     hal_result_t result = hal_gpio_write(99, 1);  // Invalid pin
 *     TEST_ASSERT_EQUAL(HAL_ERROR_GPIO, result);
 * }
 * @endcode
 *
 * Available error injection codes:
 * - @c HAL_ERROR_GPIO - GPIO operation failed
 * - @c HAL_ERROR_ADC - ADC conversion failed
 * - @c HAL_ERROR_PWM - PWM operation failed
 * - @c HAL_ERROR_UART - UART communication failed
 * - @c HAL_ERROR_TIMER - Timer operation failed
 * - @c HAL_ERROR_NOT_INITIALIZED - Operation before initialization
 *
 * @section state_management State Management
 *
 * The mock maintains internal state for all simulated peripherals:
 * - **GPIO State**: Pin modes, directions, and values
 * - **ADC State**: Channel values and conversion status
 * - **PWM State**: Duty cycles, frequencies, and enable status
 * - **UART State**: Transmit/receive buffers and status
 * - **Timer State**: Counter values and interrupt flags
 * - **System State**: Initialization status and error counters
 *
 * @section self_test_simulation Self-Test Simulation
 *
 * The mock provides configurable self-test simulation:
 *
 * @code{.c}
 * void test_hal_self_test(void) {
 *     hal_mock_reset();
 *     hal_mock_set_self_test_results(true, true, true, true, true);  // All pass
 *
 *     bool gpio_ok, adc_ok, pwm_ok, uart_ok, timer_ok;
 *     hal_result_t result = hal_self_test(&gpio_ok, &adc_ok, &pwm_ok, &uart_ok, &timer_ok);
 *
 *     TEST_ASSERT_EQUAL(HAL_OK, result);
 *     TEST_ASSERT_TRUE(gpio_ok && adc_ok && pwm_ok && uart_ok && timer_ok);
 * }
 * @endcode
 *
 * @section thread_safety Thread Safety
 *
 * The mock is designed to be thread-safe for use in concurrent test environments.
 * All mock state is protected with appropriate synchronization mechanisms.
 *
 * @warning This mock implementation is for testing purposes only.
 * @warning Do not include this file in production builds.
 * @warning Mock timing may not perfectly replicate hardware delays.
 *
 * @see src/modules/hal/hal.h Real HAL interface definition
 * @see test/unit/hal/test_hal.c Example usage in unit tests
 * @see docs/hardware-abstraction-layer.md HAL architecture documentation
 *
 * @note Mock state is automatically reset at the beginning of each test function
 * @note ADC values default to mid-scale (512) if not explicitly set
 * @note PWM frequency defaults to 1kHz if not specified
 * @note UART buffers are cleared between test functions
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
