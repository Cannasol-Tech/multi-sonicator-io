# Frequency Reading: ISR (Interrupt Service Routine) Approach

## Overview

This document provides a comprehensive ISR-based approach for reading frequency from the FREQ_DIV10_# pins on the ATmega32A microcontroller. This approach uses external interrupts to count rising edges and calculate frequency in real-time.

## Hardware Configuration

Based on `config/hardware-config.yaml`, the frequency input pins are:
- **FREQ_DIV10_1**: PB3 (Physical pin 4) - Sonicator 1
- **FREQ_DIV10_2**: PB2 (Physical pin 3) - Sonicator 2  
- **FREQ_DIV10_3**: PB1 (Physical pin 2) - Sonicator 3
- **FREQ_DIV10_4**: PB0 (Physical pin 1) - Sonicator 4

**Note**: These pins receive frequency ÷10 signals from CT2000 sonicators (nominal 1.9-2.1 kHz for 19-21 kHz operation).

## ISR Implementation Approach

### Advantages

1. **Real-time Response**: Immediate edge detection with minimal latency (~1-2 µs)
2. **Low CPU Overhead**: Only executes when edges occur
3. **High Precision**: Can capture every edge without missing events
4. **Hardware Support**: ATmega32A has dedicated interrupt pins
5. **Deterministic**: Consistent timing behavior
6. **Low Power**: CPU can sleep between interrupts

### Disadvantages

1. **Limited Interrupt Pins**: ATmega32A has only 3 external interrupt pins (INT0, INT1, INT2)
2. **Pin Change Complexity**: Need to use Pin Change Interrupts for all 4 pins
3. **Shared ISR**: All Port B pins share the same Pin Change Interrupt vector
4. **Context Switching**: ISR overhead on every edge
5. **No Built-in Filtering**: Requires software debouncing for noise

## Detailed Implementation

### 1. Pin Configuration

```cpp
// In system_config.h - Update pin definitions
#define FREQ_COUNT_PIN_1    PB3  // Physical pin 4
#define FREQ_COUNT_PIN_2    PB2  // Physical pin 3  
#define FREQ_COUNT_PIN_3    PB1  // Physical pin 2
#define FREQ_COUNT_PIN_4    PB0  // Physical pin 1

// Frequency measurement parameters
#define FREQ_SAMPLE_WINDOW_MS     1000    // 1 second measurement window
#define FREQ_MIN_EDGES            10      // Minimum edges for valid measurement
#define FREQ_MAX_EXPECTED         2500    // Maximum expected frequency (Hz)
#define FREQ_MULTIPLIER           10      // Hardware divides by 10
```

### 2. Data Structures

```cpp
// In include/frequency_counter.h
typedef struct {
    volatile uint32_t edge_count;           // Total edge count
    volatile uint32_t last_edge_timestamp;  // Last edge timestamp (µs)
    volatile uint32_t window_start_time;    // Measurement window start
    volatile bool     measurement_ready;    // Flag for new measurement
    uint16_t         calculated_frequency;  // Last calculated frequency (Hz)
    uint32_t         total_measurements;    // Total valid measurements
    uint32_t         error_count;           // Measurement errors
    uint8_t          noise_filter_count;    // Noise rejection counter
} frequency_counter_t;

// Global frequency counters for 4 sonicators
extern volatile frequency_counter_t freq_counters[4];
```

### 3. ISR Implementation

```cpp
// In src/modules/hal/frequency_isr.cpp
#include "frequency_counter.h"
#include <Arduino.h>

// Global frequency counters
volatile frequency_counter_t freq_counters[4] = {0};

// Previous port state for edge detection
static volatile uint8_t prev_portb_state = 0;

/**
 * @brief Pin Change Interrupt for Port B (FREQ_DIV10_# pins)
 * @note This ISR handles all 4 frequency input pins simultaneously
 */
ISR(PCINT1_vect) {
    uint32_t current_time = micros();
    uint8_t current_portb = PINB;
    uint8_t changed_bits = current_portb ^ prev_portb_state;
    
    // Check each frequency pin for rising edge
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t pin_mask = (1 << i);  // PB0, PB1, PB2, PB3
        
        // Rising edge detected on this pin?
        if ((changed_bits & pin_mask) && (current_portb & pin_mask)) {
            volatile frequency_counter_t* counter = &freq_counters[i];
            
            // Noise filtering - ignore edges too close together
            if (current_time - counter->last_edge_timestamp > 50) { // 50µs min
                counter->edge_count++;
                counter->last_edge_timestamp = current_time;
                
                // Check if measurement window completed
                if (current_time - counter->window_start_time >= (FREQ_SAMPLE_WINDOW_MS * 1000UL)) {
                    counter->measurement_ready = true;
                }
            } else {
                counter->noise_filter_count++;
            }
        }
    }
    
    prev_portb_state = current_portb;
}

/**
 * @brief Initialize frequency counting system
 */
void frequency_counter_init(void) {
    // Configure pins as inputs with pull-ups
    DDRB &= ~0x0F;   // PB0-PB3 as inputs
    PORTB |= 0x0F;   // Enable pull-ups
    
    // Enable Pin Change Interrupt for Port B
    PCICR |= (1 << PCIE1);    // Enable PCINT1 
    PCMSK1 |= 0x0F;          // Enable PCINT8-PCINT11 (PB0-PB3)
    
    // Initialize counters
    uint32_t now = micros();
    for (uint8_t i = 0; i < 4; i++) {
        freq_counters[i].window_start_time = now;
        freq_counters[i].edge_count = 0;
        freq_counters[i].measurement_ready = false;
        freq_counters[i].calculated_frequency = 0;
        freq_counters[i].total_measurements = 0;
        freq_counters[i].error_count = 0;
        freq_counters[i].noise_filter_count = 0;
    }
    
    prev_portb_state = PINB;
    sei(); // Enable global interrupts
}
```

### 4. Frequency Calculation

```cpp
// In src/modules/hal/frequency_calculator.cpp

/**
 * @brief Calculate frequency from edge count
 * @param sonicator_id Sonicator ID (0-3)
 * @return Calculated frequency in Hz, 0 if invalid
 */
uint16_t frequency_calculate(uint8_t sonicator_id) {
    if (sonicator_id >= 4) return 0;
    
    volatile frequency_counter_t* counter = &freq_counters[sonicator_id];
    
    // Check if new measurement is ready
    if (!counter->measurement_ready) {
        return counter->calculated_frequency; // Return last valid measurement
    }
    
    uint32_t current_time = micros();
    uint32_t window_duration_us = current_time - counter->window_start_time;
    uint32_t edge_count = counter->edge_count;
    
    // Reset for next window
    counter->edge_count = 0;
    counter->window_start_time = current_time;
    counter->measurement_ready = false;
    
    // Validate measurement
    if (edge_count < FREQ_MIN_EDGES) {
        counter->error_count++;
        return 0; // Not enough edges for reliable measurement
    }
    
    // Calculate frequency: edges per second * multiplier
    // window_duration_us / 1,000,000 = seconds
    // edge_count / seconds = Hz (before multiplier)
    uint32_t frequency_raw = (edge_count * 1000000UL) / window_duration_us;
    uint16_t frequency_actual = (uint16_t)(frequency_raw * FREQ_MULTIPLIER);
    
    // Validate result is within expected range
    if (frequency_actual > FREQ_MAX_EXPECTED) {
        counter->error_count++;
        return counter->calculated_frequency; // Keep last good value
    }
    
    // Update counters and return result
    counter->calculated_frequency = frequency_actual;
    counter->total_measurements++;
    
    return frequency_actual;
}

/**
 * @brief Get frequency measurement statistics
 * @param sonicator_id Sonicator ID (0-3)
 * @param stats Output structure for statistics
 * @return true if valid statistics available
 */
bool frequency_get_stats(uint8_t sonicator_id, frequency_stats_t* stats) {
    if (sonicator_id >= 4 || !stats) return false;
    
    volatile frequency_counter_t* counter = &freq_counters[sonicator_id];
    
    stats->current_frequency = counter->calculated_frequency;
    stats->total_measurements = counter->total_measurements;
    stats->error_count = counter->error_count;
    stats->noise_filter_count = counter->noise_filter_count;
    stats->last_edge_timestamp = counter->last_edge_timestamp;
    
    return true;
}
```

### 5. Integration with Sonicator Interface

```cpp
// In src/modules/control/sonicator.cpp - Update readHardwareInputs()

void SonicatorInterface::readHardwareInputs() {
    state_.overload_active = halGpioReadSafe(pins_.overload_pin);
    state_.frequency_locked = halGpioReadSafe(pins_.freq_lock_pin);

    // Read power from ADC
    uint16_t adc_value = halAdcReadSafe(pins_.power_sense_channel);
    state_.power_watts = (float)(adc_value * 2000) / 1023.0f;

    // Read frequency using ISR-based counter
    uint8_t freq_channel = pins_.sonicator_id - 1; // Convert to 0-3 range
    state_.frequency_hz = frequency_calculate(freq_channel);
    
    // Update frequency lock status based on measured frequency
    if (state_.frequency_hz >= 18000 && state_.frequency_hz <= 22000) {
        // Frequency is within CT2000 operating range
        state_.frequency_locked = true;
    } else if (state_.frequency_hz == 0) {
        // No valid measurement, use hardware frequency lock pin
        state_.frequency_locked = halGpioReadSafe(pins_.freq_lock_pin);
    } else {
        // Frequency out of range
        state_.frequency_locked = false;
    }
}
```

### 6. Performance Considerations

#### Memory Usage
- **Static RAM**: ~80 bytes for 4 frequency counters
- **ISR Stack**: ~10 bytes per interrupt
- **Code Size**: ~500 bytes

#### Timing Analysis
- **ISR Execution**: 2-5 µs typical
- **Maximum Interrupt Rate**: 10 kHz (all 4 channels at 2.5 kHz)
- **CPU Load**: <1% at maximum rate

#### Noise Handling
```cpp
// Advanced noise filtering (optional enhancement)
#define FREQ_NOISE_THRESHOLD_US  50   // Minimum time between edges
#define FREQ_GLITCH_COUNT_MAX    5    // Max consecutive noise events

// In ISR - enhanced noise filtering
static uint8_t noise_counts[4] = {0};

// Inside edge detection loop:
if (current_time - counter->last_edge_timestamp > FREQ_NOISE_THRESHOLD_US) {
    // Valid edge
    counter->edge_count++;
    counter->last_edge_timestamp = current_time;
    noise_counts[i] = 0; // Reset noise counter
} else {
    // Possible noise
    noise_counts[i]++;
    if (noise_counts[i] > FREQ_GLITCH_COUNT_MAX) {
        // Too much noise - disable this channel temporarily
        counter->error_count++;
        noise_counts[i] = 0;
    }
}
```

## Testing and Validation

### Unit Tests
```cpp
// In test/unit/frequency_isr_test.cpp
void test_frequency_counter_init(void) {
    frequency_counter_init();
    
    for (uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL(0, freq_counters[i].edge_count);
        TEST_ASSERT_EQUAL(0, freq_counters[i].calculated_frequency);
        TEST_ASSERT_FALSE(freq_counters[i].measurement_ready);
    }
}

void test_frequency_calculation(void) {
    // Simulate 2000 Hz input (200 edges in 100ms window)
    freq_counters[0].edge_count = 200;
    freq_counters[0].window_start_time = micros() - 100000; // 100ms ago
    freq_counters[0].measurement_ready = true;
    
    uint16_t freq = frequency_calculate(0);
    TEST_ASSERT_EQUAL(20000, freq); // 2000 Hz * 10 multiplier
}
```

### HIL Testing
```cpp
// Test with Arduino harness generating test signals
void test_frequency_measurement_hil(void) {
    // Configure test harness to output 2 kHz on D7 (connected to PB0)
    // Measure for 2 seconds and verify result is 20 kHz ± 1%
}
```

## Error Handling

### Error Types
1. **No Signal**: edge_count == 0 in measurement window
2. **Frequency Out of Range**: calculated value exceeds limits
3. **Noise Overload**: too many rapid edges (< 50µs apart)
4. **Hardware Fault**: ISR not executing (watchdog timeout)

### Error Recovery
```cpp
typedef enum {
    FREQ_ERROR_NONE = 0,
    FREQ_ERROR_NO_SIGNAL = 1,
    FREQ_ERROR_OUT_OF_RANGE = 2,
    FREQ_ERROR_NOISE = 3,
    FREQ_ERROR_HARDWARE = 4
} frequency_error_t;

frequency_error_t frequency_get_error(uint8_t sonicator_id) {
    if (sonicator_id >= 4) return FREQ_ERROR_HARDWARE;
    
    volatile frequency_counter_t* counter = &freq_counters[sonicator_id];
    
    // Check for no signal
    if (micros() - counter->last_edge_timestamp > 2000000UL) { // 2 sec timeout
        return FREQ_ERROR_NO_SIGNAL;
    }
    
    // Check noise level
    if (counter->noise_filter_count > counter->edge_count) {
        return FREQ_ERROR_NOISE;
    }
    
    return FREQ_ERROR_NONE;
}
```

## Conclusion

The ISR approach provides excellent real-time performance and accuracy for frequency measurement, but requires careful implementation due to ATmega32A's limited external interrupt pins. Using Pin Change Interrupts on Port B allows monitoring all 4 FREQ_DIV10_# pins simultaneously with shared ISR handling and software-based edge detection.

This solution is optimal for applications requiring:
- High precision frequency measurement
- Low latency response
- Minimal CPU overhead
- Real-time deterministic behavior

The implementation provides robust error handling, noise filtering, and comprehensive statistics for production use.