/**
 * @file frequency_counter.h
 * @brief Hardware frequency counter using Pin Change Interrupts
 * @author Cannasol Technologies  
 * @date 2025-01-10
 * @version 1.0.0
 * 
 * @details
 * Provides real-time frequency measurement for FREQ_DIV10_# pins (PB0-PB3)
 * using Pin Change Interrupts. Measures CT2000 sonicator frequencies via
 * edge counting with hardware precision and minimal CPU overhead.
 * 
 * Hardware Configuration:
 * - FREQ_DIV10_1: PB3 (pin 4) - Sonicator 1
 * - FREQ_DIV10_2: PB2 (pin 3) - Sonicator 2  
 * - FREQ_DIV10_3: PB1 (pin 2) - Sonicator 3
 * - FREQ_DIV10_4: PB0 (pin 1) - Sonicator 4
 * 
 * Expected Input: 1900-2100 Hz (CT2000 frequency ÷ 10)
 * Calculated Output: 19000-21000 Hz (actual sonicator frequency)
 */

#ifndef FREQUENCY_COUNTER_H
#define FREQUENCY_COUNTER_H

#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

/** @brief Number of frequency input channels */
#define FREQ_CHANNELS                 4

/** @brief Measurement window in milliseconds */
#define FREQ_SAMPLE_WINDOW_MS         1000

/** @brief Minimum edges required for valid measurement */
#define FREQ_MIN_EDGES                10

/** @brief Maximum expected input frequency (Hz) */
#define FREQ_MAX_EXPECTED             2500

/** @brief Hardware frequency divider (CT2000 outputs freq ÷ 10) */
#define FREQ_MULTIPLIER               10

/** @brief Minimum time between edges to filter noise (microseconds) */
#define FREQ_NOISE_THRESHOLD_US       50

/** @brief Expected frequency range (input, before multiplier) */
#define FREQ_INPUT_MIN_HZ             1800
#define FREQ_INPUT_MAX_HZ             2200

/** @brief Expected frequency range (output, after multiplier) */
#define FREQ_OUTPUT_MIN_HZ            18000
#define FREQ_OUTPUT_MAX_HZ            22000

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @brief Frequency counter state for each channel
 */
typedef struct {
    volatile uint32_t edge_count;           /**< Edges counted in current window */
    volatile uint32_t last_edge_timestamp;  /**< Timestamp of last edge (µs) */
    volatile uint32_t window_start_time;    /**< Start time of current window (µs) */
    volatile bool     measurement_ready;    /**< New measurement available flag */
    
    uint16_t         calculated_frequency;  /**< Last calculated frequency (Hz) */
    uint32_t         total_measurements;    /**< Total valid measurements taken */
    uint32_t         error_count;           /**< Number of measurement errors */
    uint32_t         noise_filter_count;    /**< Edges rejected by noise filter */
    
    // Advanced statistics
    uint16_t         min_frequency;         /**< Minimum frequency recorded */
    uint16_t         max_frequency;         /**< Maximum frequency recorded */
    uint32_t         total_edges;           /**< Total edges counted since init */
    uint32_t         last_calculation_time; /**< When frequency was last calculated */
} frequency_counter_t;

/**
 * @brief Frequency measurement statistics
 */
typedef struct {
    uint16_t current_frequency;             /**< Current frequency (Hz) */
    uint32_t total_measurements;            /**< Total measurements taken */
    uint32_t error_count;                   /**< Number of errors */
    uint32_t noise_filter_count;            /**< Noise events filtered */
    uint32_t last_edge_timestamp;           /**< Last edge time (µs) */
    uint16_t min_frequency;                 /**< Minimum recorded (Hz) */
    uint16_t max_frequency;                 /**< Maximum recorded (Hz) */
    uint32_t total_edges;                   /**< Total edge count */
    bool     measurement_active;            /**< Currently measuring */
} frequency_stats_t;

/**
 * @brief Frequency measurement error codes
 */
typedef enum {
    FREQ_ERROR_NONE = 0,                    /**< No error */
    FREQ_ERROR_NO_SIGNAL,                   /**< No edges detected */
    FREQ_ERROR_OUT_OF_RANGE,                /**< Frequency outside expected range */
    FREQ_ERROR_NOISE,                       /**< Too much noise detected */
    FREQ_ERROR_HARDWARE,                    /**< Hardware/ISR not functioning */
    FREQ_ERROR_INVALID_CHANNEL              /**< Invalid channel ID specified */
} frequency_error_t;

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

/** @brief Frequency counter states for all 4 channels */
extern volatile frequency_counter_t freq_counters[FREQ_CHANNELS];

// ============================================================================
// PUBLIC FUNCTION DECLARATIONS
// ============================================================================

/**
 * @brief Initialize frequency counting system
 * @return true if initialization successful
 * 
 * @details
 * - Configures PB0-PB3 as inputs with pull-ups
 * - Sets up Pin Change Interrupt for Port B
 * - Initializes all frequency counter structures
 * - Enables global interrupts
 */
bool frequency_counter_init(void);

/**
 * @brief Calculate frequency for specified channel
 * @param channel Channel ID (0-3 for sonicators 1-4)
 * @return Calculated frequency in Hz, 0 if invalid/no signal
 * 
 * @details
 * - Returns immediately if no new measurement ready
 * - Calculates frequency from edge count and time window
 * - Applies hardware multiplier (×10) to get actual frequency
 * - Validates result is within expected range
 * - Updates statistics and error counters
 */
uint16_t frequency_calculate(uint8_t channel);

/**
 * @brief Get current frequency without triggering new calculation
 * @param channel Channel ID (0-3)
 * @return Last calculated frequency in Hz
 */
uint16_t frequency_get_current(uint8_t channel);

/**
 * @brief Get detailed frequency measurement statistics
 * @param channel Channel ID (0-3)
 * @param stats Pointer to output statistics structure
 * @return true if statistics retrieved successfully
 */
bool frequency_get_stats(uint8_t channel, frequency_stats_t* stats);

/**
 * @brief Check for frequency measurement errors
 * @param channel Channel ID (0-3)
 * @return Error code (FREQ_ERROR_NONE if no errors)
 */
frequency_error_t frequency_get_error(uint8_t channel);

/**
 * @brief Reset statistics and counters for specified channel
 * @param channel Channel ID (0-3), or 0xFF for all channels
 * @return true if reset successful
 */
bool frequency_reset_stats(uint8_t channel);

/**
 * @brief Check if ISR is functioning correctly
 * @return true if ISR has executed recently
 * 
 * @details
 * Verifies that the Pin Change Interrupt is working by checking
 * if any edges have been detected recently across all channels.
 */
bool frequency_isr_health_check(void);

/**
 * @brief Get ISR execution statistics
 * @param total_interrupts Output: total ISR executions
 * @param last_isr_time Output: timestamp of last ISR (µs)
 * @return true if statistics available
 */
bool frequency_get_isr_stats(uint32_t* total_interrupts, uint32_t* last_isr_time);

/**
 * @brief Set measurement window for specified channel
 * @param channel Channel ID (0-3)
 * @param window_ms Measurement window in milliseconds (100-5000)
 * @return true if parameter set successfully
 */
bool frequency_set_window(uint8_t channel, uint16_t window_ms);

/**
 * @brief Enable or disable noise filtering
 * @param channel Channel ID (0-3), or 0xFF for all channels
 * @param enable true to enable noise filtering
 * @return true if setting applied successfully
 */
bool frequency_set_noise_filter(uint8_t channel, bool enable);

// ============================================================================
// ADVANCED FEATURES (OPTIONAL)
// ============================================================================

/**
 * @brief Get instantaneous frequency (last period measurement)
 * @param channel Channel ID (0-3)
 * @return Frequency based on last two edges, 0 if unavailable
 * 
 * @details
 * Provides faster frequency updates by measuring time between
 * consecutive edges rather than counting over full window.
 * Less accurate but more responsive to frequency changes.
 */
uint16_t frequency_get_instantaneous(uint8_t channel);

/**
 * @brief Set callback for frequency change notifications
 * @param channel Channel ID (0-3)
 * @param callback Function to call when frequency changes significantly
 * @param threshold Minimum frequency change to trigger callback (Hz)
 * @return true if callback registered successfully
 */
typedef void (*frequency_change_callback_t)(uint8_t channel, uint16_t old_freq, uint16_t new_freq);
bool frequency_set_change_callback(uint8_t channel, frequency_change_callback_t callback, uint16_t threshold);

#endif // FREQUENCY_COUNTER_H