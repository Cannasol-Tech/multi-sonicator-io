/**
 * @file hal.h
 * @brief Hardware Abstraction Layer Master Header for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Master header file that includes all HAL modules for the Multi-Sonicator
 * I/O Controller. Provides unified interface to hardware peripherals.
 * 
 * Includes:
 * - GPIO: Digital I/O control for sonicator interfaces
 * - ADC: Analog-to-digital conversion for power monitoring
 * - PWM: Pulse-width modulation for amplitude control
 * - UART: Serial communication for MODBUS RTU
 * - Timer: System timing and scheduler support
 */

#ifndef HAL_H
#define HAL_H

// Include all HAL modules
#include "gpio.h"
#include "adc.h"
#include "pwm.h"
#include "uart.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// MASTER HAL INTERFACE
// ============================================================================

/**
 * @brief HAL system return codes
 */
typedef enum {
    HAL_OK = 0,                 ///< All operations successful
    HAL_ERROR_GPIO,             ///< GPIO subsystem error
    HAL_ERROR_ADC,              ///< ADC subsystem error
    HAL_ERROR_PWM,              ///< PWM subsystem error
    HAL_ERROR_UART,             ///< UART subsystem error
    HAL_ERROR_TIMER,            ///< Timer subsystem error
    HAL_ERROR_NOT_INITIALIZED   ///< HAL not initialized
} hal_result_t;

/**
 * @brief Initialize all HAL subsystems
 * 
 * Configures all hardware peripherals according to pin matrix specifications:
 * - GPIO pins for sonicator interfaces and status LED
 * - ADC channels for power monitoring (5.44mV/W scaling)
 * - PWM channel for amplitude control (0-10V output)
 * - UART interface for MODBUS RTU at 115200 baud
 * - Timer subsystems for scheduler and watchdog
 * 
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_init(void);

/**
 * @brief Perform HAL system self-test
 * 
 * Tests all HAL subsystems for proper operation:
 * - GPIO pin control and reading
 * - ADC conversion accuracy
 * - PWM frequency and duty cycle
 * - UART loopback communication
 * - Timer accuracy and scheduling
 * 
 * @param gpio_ok Pointer to store GPIO test result
 * @param adc_ok Pointer to store ADC test result
 * @param pwm_ok Pointer to store PWM test result
 * @param uart_ok Pointer to store UART test result
 * @param timer_ok Pointer to store Timer test result
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_self_test(bool* gpio_ok, bool* adc_ok, bool* pwm_ok, 
                          bool* uart_ok, bool* timer_ok);

/**
 * @brief Get HAL system status
 * 
 * @param initialized Pointer to store initialization status
 * @param uptime_ms Pointer to store system uptime
 * @param errors Pointer to store error count
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_get_status(bool* initialized, uint32_t* uptime_ms, uint16_t* errors);

/**
 * @brief Reset HAL error counters
 * 
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_clear_errors(void);

/**
 * @brief Emergency shutdown of all HAL subsystems
 * 
 * Safely disables all outputs and sets hardware to safe state:
 * - Stop all sonicators (GPIO outputs LOW)
 * - Set amplitude to minimum (20%)
 * - Disable PWM output
 * - Flush UART buffers
 * - Reset watchdog timer
 * 
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_emergency_shutdown(void);

// ============================================================================
// SONICATOR CONTROL CONVENIENCE FUNCTIONS
// ============================================================================

/**
 * @brief Complete sonicator control structure
 */
typedef struct {
    bool start;                 ///< Start/stop control
    uint8_t amplitude_percent;  ///< Amplitude setting (20-100%)
    bool reset_overload;        ///< Reset overload condition
} sonicator_control_t;

/**
 * @brief Complete sonicator status structure
 */
typedef struct {
    bool overload;              ///< Overload condition active
    bool frequency_locked;      ///< Frequency lock status
    float power_watts;          ///< Current power output
    uint16_t frequency_hz;      ///< Operating frequency
} sonicator_status_t;

/**
 * @brief Control single sonicator
 * 
 * @param sonicator_id Sonicator number (1-4)
 * @param control Pointer to control structure
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_control_sonicator(uint8_t sonicator_id, const sonicator_control_t* control);

/**
 * @brief Read single sonicator status
 * 
 * @param sonicator_id Sonicator number (1-4)
 * @param status Pointer to status structure
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_read_sonicator_status(uint8_t sonicator_id, sonicator_status_t* status);

/**
 * @brief Control all sonicators simultaneously
 * 
 * @param control_array Array of 4 control structures
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_control_all_sonicators(const sonicator_control_t control_array[4]);

/**
 * @brief Read all sonicator statuses
 * 
 * @param status_array Array to store 4 status structures
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_read_all_sonicator_status(sonicator_status_t status_array[4]);

/**
 * @brief Emergency stop all sonicators
 * 
 * @return HAL_OK on success, error code on failure
 */
hal_result_t hal_emergency_stop_all(void);

#ifdef __cplusplus
}
#endif

#endif // HAL_H
