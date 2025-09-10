/**
 * @file gpio.h
 * @brief GPIO Hardware Abstraction Layer for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Provides hardware-agnostic GPIO control interface for sonicator interfaces,
 * status indicators, and digital I/O operations. Abstracts ATmega32A port
 * manipulation for consistent cross-platform development.
 * 
 * Pin assignments per docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH).
 * Supports all sonicator control signals, status LED, and system I/O.
 */

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @brief GPIO pin modes
 */
typedef enum {
    GPIO_MODE_INPUT = 0,        //< Input mode (high impedance)
    GPIO_MODE_INPUT_PULLUP,     //< Input with internal pull-up
    GPIO_MODE_OUTPUT,           //< Output mode (push-pull)
    GPIO_MODE_OUTPUT_OPEN_DRAIN //< Output mode (open-drain)
} gpio_mode_t;

/**
 * @brief GPIO pin states
 */
typedef enum {
    GPIO_LOW = 0,               //< Logic low (0V)
    GPIO_HIGH = 1               //< Logic high (5V)
} gpio_state_t;

/**
 * @brief HAL function return codes
 */
typedef enum {
    GPIO_OK = 0,                //< Operation successful
    GPIO_ERROR_INVALID_PIN,     //< Invalid pin number
    GPIO_ERROR_INVALID_MODE,    //< Invalid pin mode
    GPIO_ERROR_NOT_INITIALIZED  //< GPIO not initialized
} gpio_result_t;

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

/**
 * @brief Initialize GPIO hardware abstraction layer
 * 
 * Configures all GPIO pins according to pin matrix specifications.
 * Must be called before any other GPIO operations.
 * 
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_init(void);

/**
 * @brief Configure pin mode for specified GPIO pin
 * 
 * @param pin Arduino pin number (0-31 for ATmega32A)
 * @param mode Desired pin mode configuration
 * @return GPIO_OK on success, error code on failure
 * 
 * @note Pin numbers follow Arduino framework convention
 */
gpio_result_t gpio_set_pin_mode(uint8_t pin, gpio_mode_t mode);

/**
 * @brief Write digital state to output pin
 * 
 * @param pin Arduino pin number
 * @param state Desired output state (HIGH/LOW)
 * @return GPIO_OK on success, error code on failure
 * 
 * @note Pin must be configured as output before writing
 */
gpio_result_t gpio_write_pin(uint8_t pin, gpio_state_t state);

/**
 * @brief Read digital state from input pin
 * 
 * @param pin Arduino pin number
 * @param state Pointer to store read state
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_read_pin(uint8_t pin, gpio_state_t* state);

/**
 * @brief Toggle output pin state
 * 
 * @param pin Arduino pin number
 * @return GPIO_OK on success, error code on failure
 * 
 * @note Pin must be configured as output
 */
gpio_result_t gpio_toggle_pin(uint8_t pin);

/**
 * @brief Get current pin mode configuration
 * 
 * @param pin Arduino pin number
 * @param mode Pointer to store current mode
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_get_pin_mode(uint8_t pin, gpio_mode_t* mode);

// ============================================================================
// SONICATOR INTERFACE CONVENIENCE FUNCTIONS
// ============================================================================

/**
 * @brief Configure all sonicator interface pins
 * 
 * Sets up all 4 sonicator interfaces according to pin matrix:
 * - Input pins: OVERLOAD, FREQ_DIV10, FREQ_LOCK
 * - Output pins: START, RESET
 * 
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_init_sonicator_pins(void);

/**
 * @brief Start sonicator operation
 * 
 * @param sonicator_id Sonicator number (1-4)
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_sonicator_start(uint8_t sonicator_id);

/**
 * @brief Stop sonicator operation
 * 
 * @param sonicator_id Sonicator number (1-4)
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_sonicator_stop(uint8_t sonicator_id);

/**
 * @brief Reset sonicator overload condition
 * 
 * @param sonicator_id Sonicator number (1-4)
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_sonicator_reset(uint8_t sonicator_id);

/**
 * @brief Read sonicator overload status
 * 
 * @param sonicator_id Sonicator number (1-4)
 * @param overload Pointer to store overload state
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_sonicator_read_overload(uint8_t sonicator_id, bool* overload);

/**
 * @brief Read sonicator frequency lock status
 * 
 * @param sonicator_id Sonicator number (1-4)
 * @param locked Pointer to store lock state
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_sonicator_read_freq_lock(uint8_t sonicator_id, bool* locked);

/**
 * @brief Control status LED
 * 
 * @param state LED state (on/off)
 * @return GPIO_OK on success, error code on failure
 */
gpio_result_t gpio_status_led(gpio_state_t state);

#ifdef __cplusplus
}
#endif

#endif // HAL_GPIO_H
