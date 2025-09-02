/**
 * @file timer.h
 * @brief Timer Hardware Abstraction Layer for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Provides hardware-agnostic timer interface for cooperative scheduler,
 * watchdog timing, and precise delay functions. Uses Timer0 and Timer1
 * for multiple concurrent timing operations.
 * 
 * Supports millisecond timing resolution for scheduler intervals and
 * microsecond precision for MODBUS timing requirements.
 */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @brief Timer identifiers
 */
typedef enum {
    TIMER_ID_0 = 0,             ///< Timer 0 (8-bit)
    TIMER_ID_1 = 1,             ///< Timer 1 (16-bit)
    TIMER_ID_SCHEDULER,         ///< Dedicated scheduler timer
    TIMER_ID_WATCHDOG,          ///< Dedicated watchdog timer
    TIMER_ID_MAX = 4
} timer_id_t;

/**
 * @brief Timer modes
 */
typedef enum {
    TIMER_MODE_NORMAL = 0,      ///< Normal mode (overflow)
    TIMER_MODE_CTC,             ///< Clear Timer on Compare match
    TIMER_MODE_PWM_FAST,        ///< Fast PWM mode
    TIMER_MODE_PWM_PHASE_CORRECT ///< Phase correct PWM mode
} timer_mode_t;

/**
 * @brief Timer prescaler values
 */
typedef enum {
    TIMER_PRESCALER_1 = 1,      ///< No prescaling
    TIMER_PRESCALER_8 = 2,      ///< Divide by 8
    TIMER_PRESCALER_64 = 3,     ///< Divide by 64
    TIMER_PRESCALER_256 = 4,    ///< Divide by 256
    TIMER_PRESCALER_1024 = 5,   ///< Divide by 1024
    TIMER_PRESCALER_EXT_FALL = 6, ///< External clock on falling edge
    TIMER_PRESCALER_EXT_RISE = 7  ///< External clock on rising edge
} timer_prescaler_t;

/**
 * @brief Timer configuration structure
 */
typedef struct {
    timer_mode_t mode;          ///< Timer mode
    timer_prescaler_t prescaler;///< Clock prescaler
    uint16_t period;            ///< Timer period value
    bool enable_interrupt;      ///< Enable timer interrupt
} timer_config_t;

/**
 * @brief HAL function return codes
 */
typedef enum {
    TIMER_OK = 0,               ///< Operation successful
    TIMER_ERROR_INVALID_ID,     ///< Invalid timer ID
    TIMER_ERROR_INVALID_PERIOD, ///< Invalid period value
    TIMER_ERROR_INVALID_CONFIG, ///< Invalid configuration
    TIMER_ERROR_NOT_INITIALIZED ///< Timer not initialized
} timer_result_t;

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define TIMER_RESOLUTION_US     1           ///< Timer resolution (1 μs)
#define TIMER_MAX_PERIOD_MS     65535       ///< Maximum period (65.535 seconds)
#define TIMER_SCHEDULER_PERIOD  10          ///< Scheduler period (10ms)
#define TIMER_WATCHDOG_PERIOD   1000        ///< Watchdog period (1 second)

// Safety timing requirements
#define TIMER_EMERGENCY_RESPONSE_US 50      ///< Emergency response time
#define TIMER_MODBUS_CHAR_TIMEOUT   104     ///< MODBUS character timeout

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

/**
 * @brief Initialize timer hardware abstraction layer
 * 
 * Configures system timers for scheduler and watchdog operations.
 * Sets up Timer0 for microsecond timing and Timer1 for millisecond timing.
 * 
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_init(void);

/**
 * @brief Configure specific timer
 * 
 * @param timer_id Timer identifier
 * @param config Pointer to timer configuration
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_configure(timer_id_t timer_id, const timer_config_t* config);

/**
 * @brief Start timer with specified period
 * 
 * @param timer_id Timer identifier
 * @param period_ms Period in milliseconds
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_start(timer_id_t timer_id, uint32_t period_ms);

/**
 * @brief Stop specified timer
 * 
 * @param timer_id Timer identifier
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_stop(timer_id_t timer_id);

/**
 * @brief Check if timer has expired
 * 
 * @param timer_id Timer identifier
 * @param expired Pointer to store expiration status
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_expired(timer_id_t timer_id, bool* expired);

/**
 * @brief Reset timer to initial state
 * 
 * @param timer_id Timer identifier
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_reset(timer_id_t timer_id);

/**
 * @brief Get current timer value
 * 
 * @param timer_id Timer identifier
 * @param value Pointer to store current timer value
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_get_value(timer_id_t timer_id, uint16_t* value);

/**
 * @brief Get elapsed time since timer start
 * 
 * @param timer_id Timer identifier
 * @param elapsed_ms Pointer to store elapsed time in milliseconds
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_get_elapsed(timer_id_t timer_id, uint32_t* elapsed_ms);

// ============================================================================
// SYSTEM TIMING FUNCTIONS
// ============================================================================

/**
 * @brief Get system uptime in milliseconds
 * 
 * @param uptime_ms Pointer to store uptime value
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_get_millis(uint32_t* uptime_ms);

/**
 * @brief Get system uptime in microseconds
 * 
 * @param uptime_us Pointer to store uptime value
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_get_micros(uint32_t* uptime_us);

/**
 * @brief Blocking delay in milliseconds
 * 
 * @param delay_ms Delay duration in milliseconds
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_delay_ms(uint32_t delay_ms);

/**
 * @brief Blocking delay in microseconds
 * 
 * @param delay_us Delay duration in microseconds
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_delay_us(uint32_t delay_us);

/**
 * @brief Non-blocking delay check
 * 
 * @param start_time_ms Start time reference
 * @param delay_ms Desired delay duration
 * @param complete Pointer to store completion status
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_delay_complete(uint32_t start_time_ms, uint32_t delay_ms, bool* complete);

// ============================================================================
// SCHEDULER SUPPORT FUNCTIONS
// ============================================================================

/**
 * @brief Initialize scheduler timer (10ms intervals)
 * 
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_scheduler_init(void);

/**
 * @brief Check if scheduler tick is available
 * 
 * @param tick_available Pointer to store tick status
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_scheduler_tick(bool* tick_available);

/**
 * @brief Get scheduler tick count
 * 
 * @param tick_count Pointer to store tick count
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_scheduler_get_ticks(uint32_t* tick_count);

/**
 * @brief Reset scheduler tick counter
 * 
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_scheduler_reset(void);

// ============================================================================
// WATCHDOG SUPPORT FUNCTIONS
// ============================================================================

/**
 * @brief Initialize watchdog timer (1 second timeout)
 * 
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_watchdog_init(void);

/**
 * @brief Reset watchdog timer (feed the dog)
 * 
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_watchdog_reset(void);

/**
 * @brief Check if watchdog has expired
 * 
 * @param expired Pointer to store expiration status
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_watchdog_expired(bool* expired);

/**
 * @brief Get time until watchdog expiration
 * 
 * @param time_remaining_ms Pointer to store remaining time
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_watchdog_time_remaining(uint32_t* time_remaining_ms);

// ============================================================================
// MODBUS TIMING SUPPORT
// ============================================================================

/**
 * @brief Start MODBUS character timeout timer
 * 
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_modbus_char_start(void);

/**
 * @brief Check if MODBUS character timeout expired
 * 
 * @param expired Pointer to store expiration status
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_modbus_char_expired(bool* expired);

/**
 * @brief Start MODBUS frame gap timer (3.5 character times)
 * 
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_modbus_frame_gap_start(void);

/**
 * @brief Check if MODBUS frame gap completed
 * 
 * @param complete Pointer to store completion status
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_modbus_frame_gap_complete(bool* complete);

// ============================================================================
// DIAGNOSTIC AND TESTING FUNCTIONS
// ============================================================================

/**
 * @brief Measure timer accuracy
 * 
 * @param timer_id Timer identifier
 * @param expected_ms Expected period in milliseconds
 * @param actual_ms Pointer to store actual measured period
 * @param accuracy_percent Pointer to store accuracy percentage
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_measure_accuracy(timer_id_t timer_id, uint32_t expected_ms, 
                                     uint32_t* actual_ms, float* accuracy_percent);

/**
 * @brief Test all timer functions
 * 
 * @param success Pointer to store test result
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_test_all(bool* success);

/**
 * @brief Get timer statistics
 * 
 * @param timer_id Timer identifier
 * @param start_count Pointer to store start count
 * @param overflow_count Pointer to store overflow count
 * @param interrupt_count Pointer to store interrupt count
 * @return TIMER_OK on success, error code on failure
 */
timer_result_t timer_get_stats(timer_id_t timer_id, uint32_t* start_count,
                              uint32_t* overflow_count, uint32_t* interrupt_count);

#ifdef __cplusplus
}
#endif

#endif // HAL_TIMER_H
