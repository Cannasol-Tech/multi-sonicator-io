/**
 * @file frequency_counter.cpp
 * @brief Hardware frequency counter implementation using Pin Change Interrupts
 * @author Cannasol Technologies
 * @date 2025-01-10
 * @version 1.0.0
 * 
 * @details
 * Implements real-time frequency measurement for FREQ_DIV10_# pins using
 * Pin Change Interrupts on Port B. Provides high-precision edge counting
 * with minimal CPU overhead and comprehensive error handling.
 */

#include "frequency_counter.h"
#include <avr/interrupt.h>
#include <Arduino.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

/** @brief Frequency counter states for all 4 channels */
volatile frequency_counter_t freq_counters[FREQ_CHANNELS];

/** @brief Previous Port B state for edge detection */
static volatile uint8_t prev_portb_state = 0;

/** @brief ISR execution statistics */
static volatile uint32_t total_isr_executions = 0;
static volatile uint32_t last_isr_timestamp = 0;

/** @brief Noise filtering configuration */
static bool noise_filter_enabled[FREQ_CHANNELS] = {true, true, true, true};

/** @brief Custom measurement windows (0 = use default) */
static uint16_t custom_windows[FREQ_CHANNELS] = {0, 0, 0, 0};

// ============================================================================
// INTERRUPT SERVICE ROUTINE
// ============================================================================

/**
 * @brief Pin Change Interrupt for Port B (FREQ_DIV10_# pins)
 * @note This ISR handles all 4 frequency input pins simultaneously
 * 
 * @details
 * Optimized for minimal execution time while maintaining accuracy.
 * - Captures timestamp immediately for precision
 * - Detects edges by comparing current vs previous port state
 * - Applies noise filtering to reject spurious edges
 * - Updates measurement window completion flags
 */
ISR(PCINT1_vect) {
    // Capture timestamp as early as possible for precision
    uint32_t current_time = micros();
    uint8_t current_portb = PINB;
    uint8_t changed_bits = current_portb ^ prev_portb_state;
    
    // Update ISR statistics
    total_isr_executions++;
    last_isr_timestamp = current_time;
    
    // Check each frequency pin for rising edge
    for (uint8_t i = 0; i < FREQ_CHANNELS; i++) {
        uint8_t pin_mask = (1 << i);  // PB0, PB1, PB2, PB3
        
        // Rising edge detected on this pin?
        if ((changed_bits & pin_mask) && (current_portb & pin_mask)) {
            volatile frequency_counter_t* counter = &freq_counters[i];
            
            // Apply noise filtering if enabled
            if (noise_filter_enabled[i]) {
                if (current_time - counter->last_edge_timestamp < FREQ_NOISE_THRESHOLD_US) {
                    counter->noise_filter_count++;
                    continue; // Skip this edge - too soon after previous
                }
            }
            
            // Valid edge detected
            counter->edge_count++;
            counter->total_edges++;
            counter->last_edge_timestamp = current_time;
            
            // Check if measurement window completed
            uint16_t window_ms = custom_windows[i] ? custom_windows[i] : FREQ_SAMPLE_WINDOW_MS;
            if (current_time - counter->window_start_time >= (window_ms * 1000UL)) {
                counter->measurement_ready = true;
            }
        }
    }
    
    prev_portb_state = current_portb;
}

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

bool frequency_counter_init(void) {
    // Disable interrupts during initialization
    cli();
    
    // Configure pins as inputs with pull-ups
    DDRB &= ~0x0F;   // PB0-PB3 as inputs
    PORTB |= 0x0F;   // Enable pull-ups
    
    // Initialize counter structures
    uint32_t now = micros();
    for (uint8_t i = 0; i < FREQ_CHANNELS; i++) {
        volatile frequency_counter_t* counter = &freq_counters[i];
        
        // Reset all counters and flags
        counter->edge_count = 0;
        counter->last_edge_timestamp = now;
        counter->window_start_time = now;
        counter->measurement_ready = false;
        
        // Reset statistics
        counter->calculated_frequency = 0;
        counter->total_measurements = 0;
        counter->error_count = 0;
        counter->noise_filter_count = 0;
        counter->min_frequency = UINT16_MAX;
        counter->max_frequency = 0;
        counter->total_edges = 0;
        counter->last_calculation_time = now;
    }
    
    // Reset ISR statistics
    total_isr_executions = 0;
    last_isr_timestamp = now;
    
    // Enable Pin Change Interrupt for Port B
    PCICR |= (1 << PCIE1);    // Enable PCINT1 group
    PCMSK1 |= 0x0F;          // Enable PCINT8-PCINT11 (PB0-PB3)
    
    // Read initial port state
    prev_portb_state = PINB;
    
    // Enable global interrupts
    sei();
    
    return true;
}

uint16_t frequency_calculate(uint8_t channel) {
    if (channel >= FREQ_CHANNELS) {
        return 0;
    }
    
    volatile frequency_counter_t* counter = &freq_counters[channel];
    
    // Return last frequency if no new measurement ready
    if (!counter->measurement_ready) {
        return counter->calculated_frequency;
    }
    
    // Disable interrupts briefly to get consistent snapshot
    cli();
    uint32_t current_time = micros();
    uint32_t window_duration_us = current_time - counter->window_start_time;
    uint32_t edge_count = counter->edge_count;
    
    // Reset for next measurement window
    counter->edge_count = 0;
    counter->window_start_time = current_time;
    counter->measurement_ready = false;
    sei();
    
    // Validate measurement has sufficient edges
    if (edge_count < FREQ_MIN_EDGES) {
        counter->error_count++;
        counter->last_calculation_time = current_time;
        return 0; // Not enough edges for reliable measurement
    }
    
    // Calculate frequency: (edges / window_seconds) * multiplier
    // Avoid floating point: (edges * 1000000 / window_us) * multiplier
    uint32_t frequency_raw = (edge_count * 1000000UL) / window_duration_us;
    uint32_t frequency_with_multiplier = frequency_raw * FREQ_MULTIPLIER;
    
    // Validate result is within reasonable range
    if (frequency_with_multiplier > FREQ_OUTPUT_MAX_HZ || 
        frequency_with_multiplier < FREQ_OUTPUT_MIN_HZ) {
        counter->error_count++;
        counter->last_calculation_time = current_time;
        return counter->calculated_frequency; // Keep last good value
    }
    
    uint16_t frequency_result = (uint16_t)frequency_with_multiplier;
    
    // Update statistics
    counter->calculated_frequency = frequency_result;
    counter->total_measurements++;
    counter->last_calculation_time = current_time;
    
    // Update min/max tracking
    if (frequency_result < counter->min_frequency) {
        counter->min_frequency = frequency_result;
    }
    if (frequency_result > counter->max_frequency) {
        counter->max_frequency = frequency_result;
    }
    
    return frequency_result;
}

uint16_t frequency_get_current(uint8_t channel) {
    if (channel >= FREQ_CHANNELS) {
        return 0;
    }
    
    return freq_counters[channel].calculated_frequency;
}

bool frequency_get_stats(uint8_t channel, frequency_stats_t* stats) {
    if (channel >= FREQ_CHANNELS || !stats) {
        return false;
    }
    
    volatile frequency_counter_t* counter = &freq_counters[channel];
    
    // Populate statistics structure
    stats->current_frequency = counter->calculated_frequency;
    stats->total_measurements = counter->total_measurements;
    stats->error_count = counter->error_count;
    stats->noise_filter_count = counter->noise_filter_count;
    stats->last_edge_timestamp = counter->last_edge_timestamp;
    stats->min_frequency = (counter->min_frequency == UINT16_MAX) ? 0 : counter->min_frequency;
    stats->max_frequency = counter->max_frequency;
    stats->total_edges = counter->total_edges;
    stats->measurement_active = counter->measurement_ready;
    
    return true;
}

frequency_error_t frequency_get_error(uint8_t channel) {
    if (channel >= FREQ_CHANNELS) {
        return FREQ_ERROR_INVALID_CHANNEL;
    }
    
    volatile frequency_counter_t* counter = &freq_counters[channel];
    uint32_t current_time = micros();
    
    // Check for no signal (no edges for > 2 seconds)
    if (current_time - counter->last_edge_timestamp > 2000000UL) {
        return FREQ_ERROR_NO_SIGNAL;
    }
    
    // Check for excessive noise
    if (counter->noise_filter_count > counter->total_edges / 2) {
        return FREQ_ERROR_NOISE;
    }
    
    // Check if frequency is out of expected range
    uint16_t freq = counter->calculated_frequency;
    if (freq > 0 && (freq < FREQ_OUTPUT_MIN_HZ || freq > FREQ_OUTPUT_MAX_HZ)) {
        return FREQ_ERROR_OUT_OF_RANGE;
    }
    
    return FREQ_ERROR_NONE;
}

bool frequency_reset_stats(uint8_t channel) {
    if (channel >= FREQ_CHANNELS && channel != 0xFF) {
        return false;
    }
    
    uint8_t start_ch = (channel == 0xFF) ? 0 : channel;
    uint8_t end_ch = (channel == 0xFF) ? FREQ_CHANNELS - 1 : channel;
    
    uint32_t now = micros();
    
    for (uint8_t i = start_ch; i <= end_ch; i++) {
        volatile frequency_counter_t* counter = &freq_counters[i];
        
        // Disable interrupts to ensure atomic reset
        cli();
        counter->total_measurements = 0;
        counter->error_count = 0;
        counter->noise_filter_count = 0;
        counter->min_frequency = UINT16_MAX;
        counter->max_frequency = 0;
        counter->total_edges = 0;
        counter->last_calculation_time = now;
        sei();
    }
    
    return true;
}

bool frequency_isr_health_check(void) {
    uint32_t current_time = micros();
    
    // Check if ISR has executed recently (within last 5 seconds)
    if (current_time - last_isr_timestamp > 5000000UL) {
        return false; // ISR not executing
    }
    
    // Check if any channel has received edges recently
    for (uint8_t i = 0; i < FREQ_CHANNELS; i++) {
        if (current_time - freq_counters[i].last_edge_timestamp < 5000000UL) {
            return true; // At least one channel active
        }
    }
    
    return false; // No recent activity on any channel
}

bool frequency_get_isr_stats(uint32_t* total_interrupts, uint32_t* last_isr_time) {
    if (!total_interrupts || !last_isr_time) {
        return false;
    }
    
    *total_interrupts = total_isr_executions;
    *last_isr_time = last_isr_timestamp;
    
    return true;
}

bool frequency_set_window(uint8_t channel, uint16_t window_ms) {
    if (channel >= FREQ_CHANNELS || window_ms < 100 || window_ms > 5000) {
        return false;
    }
    
    custom_windows[channel] = window_ms;
    return true;
}

bool frequency_set_noise_filter(uint8_t channel, bool enable) {
    if (channel >= FREQ_CHANNELS && channel != 0xFF) {
        return false;
    }
    
    if (channel == 0xFF) {
        // Apply to all channels
        for (uint8_t i = 0; i < FREQ_CHANNELS; i++) {
            noise_filter_enabled[i] = enable;
        }
    } else {
        noise_filter_enabled[channel] = enable;
    }
    
    return true;
}

// ============================================================================
// ADVANCED FEATURES IMPLEMENTATION
// ============================================================================

uint16_t frequency_get_instantaneous(uint8_t channel) {
    if (channel >= FREQ_CHANNELS) {
        return 0;
    }
    
    volatile frequency_counter_t* counter = &freq_counters[channel];
    
    // Need at least 2 edges for period calculation
    if (counter->total_edges < 2) {
        return 0;
    }
    
    // This is a simplified implementation - in practice would need
    // to store timestamps of last few edges for accurate calculation
    uint32_t current_time = micros();
    uint32_t time_since_last_edge = current_time - counter->last_edge_timestamp;
    
    // If too much time has passed, no instantaneous measurement available
    if (time_since_last_edge > 10000) { // 10ms timeout
        return 0;
    }
    
    // Return current calculated frequency as approximation
    return counter->calculated_frequency;
}

// Callback functionality would require additional storage and complexity
// Omitted from this implementation for simplicity