/**
 * @file timer.cpp
 * @brief Timer Hardware Abstraction Layer Implementation for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

#include "timer.h"
#include "config.h"
#include <Arduino.h>
#include <avr/interrupt.h>

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static bool timer_initialized = false;

// Timer state tracking
typedef struct {
    bool active;
    uint32_t start_time;
    uint32_t period_ms;
    uint32_t start_count;
    uint32_t overflow_count;
    uint32_t interrupt_count;
} timer_state_t;

static timer_state_t timer_states[TIMER_ID_MAX];

// Scheduler variables
static volatile uint32_t scheduler_ticks = 0;
static volatile bool scheduler_tick_available = false;

// Watchdog variables
static uint32_t watchdog_last_reset = 0;

// MODBUS timing variables
static uint32_t modbus_char_start_time = 0;
static uint32_t modbus_frame_gap_start_time = 0;

// ============================================================================
// INTERRUPT SERVICE ROUTINES
// ============================================================================

// NOTE: Do not define Timer0 ISR when using Arduino core; use millis() based scheduler instead.

// Timer1 overflow interrupt
ISR(TIMER1_OVF_vect) {
    timer_states[TIMER_ID_1].interrupt_count++;
    timer_states[TIMER_ID_1].overflow_count++;
}

// ============================================================================
// PRIVATE FUNCTION DECLARATIONS
// ============================================================================

static bool is_valid_timer_id(timer_id_t timer_id);
static timer_result_t configure_timer0(const timer_config_t* config);
static timer_result_t configure_timer1(const timer_config_t* config);
static void reset_timer_state(timer_id_t timer_id);

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

timer_result_t timer_init(void) {
    // Initialize all timer states
    for (uint8_t i = 0; i < TIMER_ID_MAX; i++) {
        reset_timer_state((timer_id_t)i);
    }

    // Initialize scheduler timer (Timer0)
    timer_result_t result = timer_scheduler_init();
    if (result != TIMER_OK) {
        return result;
    }

    // Initialize watchdog timer
    result = timer_watchdog_init();
    if (result != TIMER_OK) {
        return result;
    }

    timer_initialized = true;
    return TIMER_OK;
}

timer_result_t timer_configure(timer_id_t timer_id, const timer_config_t* config) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_timer_id(timer_id) || config == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    switch (timer_id) {
        case TIMER_ID_0:
            return configure_timer0(config);
        case TIMER_ID_1:
            return configure_timer1(config);
        default:
            return TIMER_ERROR_INVALID_ID;
    }
}

timer_result_t timer_start(timer_id_t timer_id, uint32_t period_ms) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_timer_id(timer_id)) {
        return TIMER_ERROR_INVALID_ID;
    }

    if (period_ms == 0 || period_ms > TIMER_MAX_PERIOD_MS) {
        return TIMER_ERROR_INVALID_PERIOD;
    }

    // Store timer parameters
    timer_states[timer_id].active = true;
    timer_states[timer_id].start_time = millis();
    timer_states[timer_id].period_ms = period_ms;
    timer_states[timer_id].start_count++;

    return TIMER_OK;
}

timer_result_t timer_stop(timer_id_t timer_id) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_timer_id(timer_id)) {
        return TIMER_ERROR_INVALID_ID;
    }

    timer_states[timer_id].active = false;
    return TIMER_OK;
}

timer_result_t timer_expired(timer_id_t timer_id, bool* expired) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_timer_id(timer_id) || expired == nullptr) {
        return TIMER_ERROR_INVALID_ID;
    }

    if (!timer_states[timer_id].active) {
        *expired = false;
        return TIMER_OK;
    }

    uint32_t current_time = millis();
    uint32_t elapsed = current_time - timer_states[timer_id].start_time;
    *expired = (elapsed >= timer_states[timer_id].period_ms);

    return TIMER_OK;
}

timer_result_t timer_reset(timer_id_t timer_id) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_timer_id(timer_id)) {
        return TIMER_ERROR_INVALID_ID;
    }

    timer_states[timer_id].start_time = millis();
    return TIMER_OK;
}

timer_result_t timer_get_value(timer_id_t timer_id, uint16_t* value) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_timer_id(timer_id) || value == nullptr) {
        return TIMER_ERROR_INVALID_ID;
    }

    switch (timer_id) {
        case TIMER_ID_0:
            *value = TCNT0;
            break;
        case TIMER_ID_1:
            *value = TCNT1;
            break;
        default:
            *value = 0;
            break;
    }

    return TIMER_OK;
}

timer_result_t timer_get_elapsed(timer_id_t timer_id, uint32_t* elapsed_ms) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_timer_id(timer_id) || elapsed_ms == nullptr) {
        return TIMER_ERROR_INVALID_ID;
    }

    if (!timer_states[timer_id].active) {
        *elapsed_ms = 0;
        return TIMER_OK;
    }

    uint32_t current_time = millis();
    *elapsed_ms = current_time - timer_states[timer_id].start_time;

    return TIMER_OK;
}

// ============================================================================
// SYSTEM TIMING FUNCTIONS
// ============================================================================

timer_result_t timer_get_millis(uint32_t* uptime_ms) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (uptime_ms == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    *uptime_ms = millis();
    return TIMER_OK;
}

timer_result_t timer_get_micros(uint32_t* uptime_us) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (uptime_us == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    *uptime_us = micros();
    return TIMER_OK;
}

timer_result_t timer_delay_ms(uint32_t delay_ms) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    delay(delay_ms);
    return TIMER_OK;
}

timer_result_t timer_delay_us(uint32_t delay_us) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    delayMicroseconds(delay_us);
    return TIMER_OK;
}

timer_result_t timer_delay_complete(uint32_t start_time_ms, uint32_t delay_ms, bool* complete) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (complete == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    uint32_t current_time = millis();
    uint32_t elapsed = current_time - start_time_ms;
    *complete = (elapsed >= delay_ms);

    return TIMER_OK;
}

// ============================================================================
// SCHEDULER FUNCTIONS
// ============================================================================

timer_result_t timer_scheduler_init(void) {
    // Configure Timer0 for 1ms interrupts
    // CTC mode with OCR0 = 249 (250 counts for 1ms at 16MHz/64)
    TCCR0 = (1 << WGM01) | (1 << CS01) | (1 << CS00); // CTC mode, prescaler 64
    OCR0 = 249; // 1ms period: (16MHz / 64) / 1000 - 1
    TIMSK |= (1 << OCIE0); // Enable compare match interrupt

    scheduler_ticks = 0;
    scheduler_tick_available = false;

    sei(); // Enable global interrupts

    return TIMER_OK;
}

timer_result_t timer_scheduler_tick(bool* tick_available) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (tick_available == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }
    // millis()-based 10ms tick
    static uint32_t last_ms = 0;
    uint32_t now = millis();
    if (now - last_ms >= 10) {
        uint32_t delta = now - last_ms;
        last_ms = now;
        scheduler_ticks += (delta / 10);
        scheduler_tick_available = true;
    }


    cli();
    *tick_available = scheduler_tick_available;
    scheduler_tick_available = false;
    sei();

    return TIMER_OK;
}

timer_result_t timer_scheduler_get_ticks(uint32_t* tick_count) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (tick_count == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    cli();
    *tick_count = scheduler_ticks;
    sei();

    return TIMER_OK;
}

timer_result_t timer_scheduler_reset(void) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    cli();
    scheduler_ticks = 0;
    scheduler_tick_available = false;
    sei();

    return TIMER_OK;
}

// ============================================================================
// WATCHDOG FUNCTIONS
// ============================================================================

timer_result_t timer_watchdog_init(void) {
    watchdog_last_reset = millis();
    return TIMER_OK;
}

timer_result_t timer_watchdog_reset(void) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    watchdog_last_reset = millis();
    return TIMER_OK;
}

timer_result_t timer_watchdog_expired(bool* expired) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (expired == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    uint32_t current_time = millis();
    uint32_t elapsed = current_time - watchdog_last_reset;
    *expired = (elapsed > TIMER_WATCHDOG_PERIOD);

    return TIMER_OK;
}

timer_result_t timer_watchdog_time_remaining(uint32_t* time_remaining_ms) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (time_remaining_ms == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    uint32_t current_time = millis();
    uint32_t elapsed = current_time - watchdog_last_reset;

    if (elapsed >= TIMER_WATCHDOG_PERIOD) {
        *time_remaining_ms = 0;
    } else {
        *time_remaining_ms = TIMER_WATCHDOG_PERIOD - elapsed;
    }

    return TIMER_OK;
}

// ============================================================================
// MODBUS TIMING FUNCTIONS
// ============================================================================

timer_result_t timer_modbus_char_start(void) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    modbus_char_start_time = micros();
    return TIMER_OK;
}

timer_result_t timer_modbus_char_expired(bool* expired) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (expired == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    uint32_t current_time = micros();
    uint32_t elapsed = current_time - modbus_char_start_time;
    *expired = (elapsed > TIMER_MODBUS_CHAR_TIMEOUT);

    return TIMER_OK;
}

timer_result_t timer_modbus_frame_gap_start(void) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    modbus_frame_gap_start_time = micros();
    return TIMER_OK;
}

timer_result_t timer_modbus_frame_gap_complete(bool* complete) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (complete == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    uint32_t current_time = micros();
    uint32_t elapsed = current_time - modbus_frame_gap_start_time;
    *complete = (elapsed >= 350); // 3.5 character times at 115200 baud

    return TIMER_OK;
}

// ============================================================================
// DIAGNOSTIC FUNCTIONS
// ============================================================================

timer_result_t timer_measure_accuracy(timer_id_t timer_id, uint32_t expected_ms,
                                     uint32_t* actual_ms, float* accuracy_percent) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_timer_id(timer_id) || actual_ms == nullptr || accuracy_percent == nullptr) {
        return TIMER_ERROR_INVALID_ID;
    }

    // Start measurement
    uint32_t start_time = millis();
    timer_start(timer_id, expected_ms);

    // Wait for timer expiration
    bool expired = false;
    while (!expired) {
        timer_expired(timer_id, &expired);
        delay(1);
    }

    uint32_t end_time = millis();
    *actual_ms = end_time - start_time;

    // Calculate accuracy
    if (expected_ms > 0) {
        *accuracy_percent = (float)(*actual_ms) / expected_ms * 100.0f;
    } else {
        *accuracy_percent = 0.0f;
    }

    return TIMER_OK;
}

timer_result_t timer_test_all(bool* success) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (success == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    *success = true;

    // Test scheduler timer
    uint32_t initial_ticks;
    timer_scheduler_get_ticks(&initial_ticks);
    delay(50); // Wait 50ms
    uint32_t final_ticks;
    timer_scheduler_get_ticks(&final_ticks);

    // Should have approximately 5 ticks (50ms / 10ms)
    uint32_t tick_diff = final_ticks - initial_ticks;
    if (tick_diff < 4 || tick_diff > 6) {
        *success = false;
    }

    // Test basic timer functionality
    timer_start(TIMER_ID_1, 100);
    delay(50);
    bool expired;
    timer_expired(TIMER_ID_1, &expired);
    if (expired) {
        *success = false; // Should not be expired yet
    }

    delay(60);
    timer_expired(TIMER_ID_1, &expired);
    if (!expired) {
        *success = false; // Should be expired now
    }

    return TIMER_OK;
}

timer_result_t timer_get_stats(timer_id_t timer_id, uint32_t* start_count,
                              uint32_t* overflow_count, uint32_t* interrupt_count) {
    if (!timer_initialized) {
        return TIMER_ERROR_NOT_INITIALIZED;
    }

    if (!is_valid_timer_id(timer_id)) {
        return TIMER_ERROR_INVALID_ID;
    }

    if (start_count) *start_count = timer_states[timer_id].start_count;
    if (overflow_count) *overflow_count = timer_states[timer_id].overflow_count;
    if (interrupt_count) *interrupt_count = timer_states[timer_id].interrupt_count;

    return TIMER_OK;
}

// ============================================================================
// PRIVATE FUNCTION IMPLEMENTATIONS
// ============================================================================

static bool is_valid_timer_id(timer_id_t timer_id) {
    return (timer_id < TIMER_ID_MAX);
}

static timer_result_t configure_timer0(const timer_config_t* config) {
    if (config == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    // Configure Timer0 based on mode
    switch (config->mode) {
        case TIMER_MODE_NORMAL:
            TCCR0 &= ~((1 << WGM01) | (1 << WGM00));
            break;
        case TIMER_MODE_CTC:
            TCCR0 = (TCCR0 & ~(1 << WGM00)) | (1 << WGM01);
            OCR0 = (uint8_t)config->period;
            break;
        default:
            return TIMER_ERROR_INVALID_CONFIG;
    }

    // Set prescaler
    TCCR0 = (TCCR0 & 0xF8) | (config->prescaler & 0x07);

    // Enable interrupt if requested
    if (config->enable_interrupt) {
        if (config->mode == TIMER_MODE_CTC) {
            TIMSK |= (1 << OCIE0);
        } else {
            TIMSK |= (1 << TOIE0);
        }
    }

    return TIMER_OK;
}

static timer_result_t configure_timer1(const timer_config_t* config) {
    if (config == nullptr) {
        return TIMER_ERROR_INVALID_CONFIG;
    }

    // Configure Timer1 based on mode
    switch (config->mode) {
        case TIMER_MODE_NORMAL:
            TCCR1A = 0;
            TCCR1B &= ~((1 << WGM13) | (1 << WGM12));
            break;
        case TIMER_MODE_CTC:
            TCCR1A = 0;
            TCCR1B |= (1 << WGM12);
            OCR1A = config->period;
            break;
        default:
            return TIMER_ERROR_INVALID_CONFIG;
    }

    // Set prescaler
    TCCR1B = (TCCR1B & 0xF8) | (config->prescaler & 0x07);

    // Enable interrupt if requested
    if (config->enable_interrupt) {
        if (config->mode == TIMER_MODE_CTC) {
            TIMSK |= (1 << OCIE1A);
        } else {
            TIMSK |= (1 << TOIE1);
        }
    }

    return TIMER_OK;
}

static void reset_timer_state(timer_id_t timer_id) {
    if (timer_id < TIMER_ID_MAX) {
        timer_states[timer_id].active = false;
        timer_states[timer_id].start_time = 0;
        timer_states[timer_id].period_ms = 0;
        timer_states[timer_id].start_count = 0;
        timer_states[timer_id].overflow_count = 0;
        timer_states[timer_id].interrupt_count = 0;
    }
}
