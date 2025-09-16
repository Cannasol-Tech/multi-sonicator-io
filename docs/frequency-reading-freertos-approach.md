# Frequency Reading: FreeRTOS Task-Based Approach

## Overview

This document provides a comprehensive FreeRTOS-based approach for reading frequency from the FREQ_DIV10_# pins on the ATmega32A microcontroller. This approach uses dedicated tasks with polling or interrupt notifications to count edges and calculate frequency.

## Hardware Configuration

Based on `config/hardware-config.yaml`, the frequency input pins are:
- **FREQ_DIV10_1**: PB3 (Physical pin 4) - Sonicator 1
- **FREQ_DIV10_2**: PB2 (Physical pin 3) - Sonicator 2
- **FREQ_DIV10_3**: PB1 (Physical pin 2) - Sonicator 3  
- **FREQ_DIV10_4**: PB0 (Physical pin 1) - Sonicator 4

**Note**: These pins receive frequency ÷10 signals from CT2000 sonicators (nominal 1.9-2.1 kHz for 19-21 kHz operation).

## FreeRTOS Implementation Approach

### Advantages

1. **Task Isolation**: Each frequency measurement runs in separate task context
2. **Flexible Scheduling**: Priority-based task scheduling with preemption
3. **Built-in Synchronization**: Queues, semaphores, and notifications
4. **Scalable Architecture**: Easy to add more measurement channels
5. **Error Isolation**: Task failure doesn't affect other measurements
6. **Resource Management**: Built-in memory and CPU time management
7. **Professional RTOS**: Proven real-time operating system

### Disadvantages

1. **Memory Overhead**: ~2-4KB RAM for FreeRTOS kernel + task stacks
2. **Code Size**: ~8-12KB flash for FreeRTOS kernel
3. **Complexity**: Higher learning curve and debugging complexity
4. **Task Switching**: Overhead from context switching
5. **ATmega32A Limitations**: Only 2KB RAM total - very tight for FreeRTOS
6. **Timing Jitter**: Task scheduling can introduce measurement delays
7. **Arduino Compatibility**: May conflict with Arduino framework assumptions

## Detailed Implementation

### 1. FreeRTOS Configuration

```cpp
// In include/FreeRTOSConfig.h
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// ATmega32A @ 16MHz configuration
#define configCPU_CLOCK_HZ              16000000UL
#define configTICK_RATE_HZ              1000        // 1ms tick
#define configMAX_PRIORITIES            4
#define configMINIMAL_STACK_SIZE        128         // Words (256 bytes)
#define configTOTAL_HEAP_SIZE           1024        // Bytes
#define configUSE_PREEMPTION            1
#define configIDLE_SHOULD_YIELD         1
#define configUSE_TIME_SLICING          1

// Memory management
#define configSUPPORT_DYNAMIC_ALLOCATION    1
#define configSUPPORT_STATIC_ALLOCATION     0

// Task features
#define configUSE_TASK_NOTIFICATIONS        1
#define configUSE_COUNTING_SEMAPHORES       1
#define configUSE_BINARY_SEMAPHORES         1
#define configUSE_QUEUE_SETS                0

// Debugging features (disable in production)
#define configUSE_TRACE_FACILITY            1
#define configUSE_STATS_FORMATTING_FUNCTIONS 1

// Hook functions
#define configUSE_IDLE_HOOK                 1
#define configUSE_TICK_HOOK                 0
#define configUSE_MALLOC_FAILED_HOOK        1
#define configUSE_STACK_OVERFLOW_HOOK       2

// Interrupt priorities (0 = highest, 7 = lowest)
#define configKERNEL_INTERRUPT_PRIORITY         7
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    3

// Include platform-specific definitions
#include "FreeRTOSVariant.h"

#endif /* FREERTOS_CONFIG_H */
```

### 2. Data Structures

```cpp
// In include/frequency_freertos.h
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

typedef struct {
    uint8_t             sonicator_id;       // Sonicator ID (0-3)
    uint8_t             pin_number;         // Physical pin number
    uint8_t             pin_mask;           // Pin bitmask for port
    TaskHandle_t        task_handle;        // FreeRTOS task handle
    QueueHandle_t       result_queue;       // Queue for frequency results
    TimerHandle_t       sample_timer;       // Timer for measurement windows
    
    // Measurement data
    volatile uint32_t   edge_count;         // Edges in current window
    volatile uint32_t   last_edge_time;     // Last edge timestamp
    volatile bool       measurement_active; // Measurement in progress
    
    // Statistics
    uint16_t           current_frequency;   // Last measured frequency (Hz)
    uint32_t           total_measurements;  // Total valid measurements
    uint32_t           error_count;         // Error counter
    uint32_t           task_cycles;         // Task execution cycles
    
    // Configuration
    uint16_t           sample_period_ms;    // Measurement window (ms)
    uint16_t           task_priority;       // FreeRTOS task priority
    uint16_t           stack_size_words;    // Task stack size
} frequency_task_context_t;

// Global context for all 4 frequency measurement tasks
extern frequency_task_context_t freq_contexts[4];
```

### 3. Task Implementation - Method 1: Polling-Based

```cpp
// In src/modules/hal/frequency_freertos_polling.cpp
#include "frequency_freertos.h"
#include <Arduino.h>

// Global contexts
frequency_task_context_t freq_contexts[4];

/**
 * @brief Frequency measurement task (polling-based)
 * @param pvParameters Pointer to frequency_task_context_t
 */
void frequency_measurement_task(void *pvParameters) {
    frequency_task_context_t *ctx = (frequency_task_context_t*)pvParameters;
    
    uint8_t prev_pin_state = digitalRead(ctx->pin_number);
    uint32_t window_start = 0;
    bool window_active = false;
    
    // Task main loop
    for (;;) {
        uint32_t current_time = micros();
        uint8_t current_pin_state = digitalRead(ctx->pin_number);
        
        // Detect rising edge
        if (current_pin_state && !prev_pin_state) {
            ctx->last_edge_time = current_time;
            
            if (window_active) {
                ctx->edge_count++;
            } else {
                // Start new measurement window
                window_start = current_time;
                ctx->edge_count = 1;
                window_active = true;
                ctx->measurement_active = true;
            }
        }
        
        // Check if measurement window completed
        if (window_active && 
            (current_time - window_start) >= (ctx->sample_period_ms * 1000UL)) {
            
            // Calculate frequency
            uint32_t window_duration_us = current_time - window_start;
            uint16_t frequency = 0;
            
            if (ctx->edge_count >= 10) { // Minimum edges for valid measurement
                uint32_t freq_raw = (ctx->edge_count * 1000000UL) / window_duration_us;
                frequency = (uint16_t)(freq_raw * 10); // Hardware divides by 10
            }
            
            // Send result to queue
            if (xQueueSend(ctx->result_queue, &frequency, 0) != pdPASS) {
                ctx->error_count++;
            }
            
            // Update statistics
            ctx->current_frequency = frequency;
            ctx->total_measurements++;
            ctx->measurement_active = false;
            window_active = false;
        }
        
        prev_pin_state = current_pin_state;
        ctx->task_cycles++;
        
        // Yield CPU - critical for preventing task starvation
        // Poll at ~10kHz to not miss edges (2.5kHz max expected)
        vTaskDelay(pdMS_TO_TICKS(0)); // Yield immediately
        delayMicroseconds(100);       // 100µs delay = 10kHz poll rate
    }
}
```

### 4. Task Implementation - Method 2: Interrupt + Notification

```cpp
// In src/modules/hal/frequency_freertos_interrupt.cpp
#include "frequency_freertos.h"

// ISR for pin changes - notifies appropriate task
ISR(PCINT1_vect) {
    uint8_t current_portb = PINB;
    static uint8_t prev_portb_state = 0;
    uint8_t changed_bits = current_portb ^ prev_portb_state;
    BaseType_t higher_priority_task_woken = pdFALSE;
    
    // Check each frequency pin
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t pin_mask = (1 << i);
        
        // Rising edge detected?
        if ((changed_bits & pin_mask) && (current_portb & pin_mask)) {
            // Notify the appropriate task
            if (freq_contexts[i].task_handle != NULL) {
                vTaskNotifyGiveFromISR(freq_contexts[i].task_handle, 
                                     &higher_priority_task_woken);
            }
        }
    }
    
    prev_portb_state = current_portb;
    
    // Context switch if higher priority task was woken
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief Frequency measurement task (interrupt-driven)
 * @param pvParameters Pointer to frequency_task_context_t
 */
void frequency_measurement_task_interrupt(void *pvParameters) {
    frequency_task_context_t *ctx = (frequency_task_context_t*)pvParameters;
    
    uint32_t window_start = 0;
    bool window_active = false;
    
    for (;;) {
        // Wait for interrupt notification (blocking)
        uint32_t notification_value = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        if (notification_value > 0) {
            uint32_t current_time = micros();
            ctx->last_edge_time = current_time;
            
            if (!window_active) {
                // Start new measurement window
                window_start = current_time;
                ctx->edge_count = 1;
                window_active = true;
                ctx->measurement_active = true;
            } else {
                ctx->edge_count++;
                
                // Check if measurement window completed
                if ((current_time - window_start) >= (ctx->sample_period_ms * 1000UL)) {
                    
                    // Calculate frequency
                    uint32_t window_duration_us = current_time - window_start;
                    uint16_t frequency = 0;
                    
                    if (ctx->edge_count >= 10) {
                        uint32_t freq_raw = (ctx->edge_count * 1000000UL) / window_duration_us;
                        frequency = (uint16_t)(freq_raw * 10);
                    }
                    
                    // Send result to queue
                    if (xQueueSend(ctx->result_queue, &frequency, 0) != pdPASS) {
                        ctx->error_count++;
                    }
                    
                    // Update statistics
                    ctx->current_frequency = frequency;
                    ctx->total_measurements++;
                    ctx->measurement_active = false;
                    window_active = false;
                }
            }
            ctx->task_cycles++;
        }
    }
}
```

### 5. Initialization and Setup

```cpp
// In src/modules/hal/frequency_freertos_init.cpp

/**
 * @brief Initialize FreeRTOS-based frequency measurement system
 * @return true if initialization successful
 */
bool frequency_freertos_init(void) {
    
    // Pin configuration - all pins as inputs with pull-ups
    DDRB &= ~0x0F;   // PB0-PB3 as inputs
    PORTB |= 0x0F;   // Enable pull-ups
    
    // Configure Pin Change Interrupts (for interrupt-driven method)
    PCICR |= (1 << PCIE1);    // Enable PCINT1
    PCMSK1 |= 0x0F;          // Enable PCINT8-PCINT11 (PB0-PB3)
    
    // Initialize contexts for all 4 sonicators
    const uint8_t pin_numbers[4] = {1, 2, 3, 4}; // PB0, PB1, PB2, PB3
    const char* task_names[4] = {"FreqS1", "FreqS2", "FreqS3", "FreqS4"};
    
    for (uint8_t i = 0; i < 4; i++) {
        frequency_task_context_t *ctx = &freq_contexts[i];
        
        // Initialize context
        ctx->sonicator_id = i;
        ctx->pin_number = pin_numbers[i];
        ctx->pin_mask = (1 << i);
        ctx->sample_period_ms = 1000;  // 1 second windows
        ctx->task_priority = tskIDLE_PRIORITY + 2;  // Medium priority
        ctx->stack_size_words = 128;   // 256 bytes stack
        
        // Create result queue
        ctx->result_queue = xQueueCreate(5, sizeof(uint16_t));
        if (ctx->result_queue == NULL) {
            return false;
        }
        
        // Create measurement task
        BaseType_t result = xTaskCreate(
            frequency_measurement_task_interrupt,  // Task function
            task_names[i],                         // Task name
            ctx->stack_size_words,                 // Stack size
            ctx,                                   // Parameters
            ctx->task_priority,                    // Priority
            &ctx->task_handle                      // Task handle
        );
        
        if (result != pdPASS) {
            return false;
        }
    }
    
    return true;
}
```

### 6. Integration Layer

```cpp
// In src/modules/hal/frequency_freertos_api.cpp

/**
 * @brief Get current frequency for specified sonicator
 * @param sonicator_id Sonicator ID (0-3)
 * @return Current frequency in Hz, 0 if no valid measurement
 */
uint16_t frequency_freertos_get(uint8_t sonicator_id) {
    if (sonicator_id >= 4) return 0;
    
    frequency_task_context_t *ctx = &freq_contexts[sonicator_id];
    uint16_t frequency = 0;
    
    // Try to get latest measurement from queue (non-blocking)
    if (xQueueReceive(ctx->result_queue, &frequency, 0) == pdPASS) {
        ctx->current_frequency = frequency;
        return frequency;
    }
    
    // Return last known frequency if no new measurement
    return ctx->current_frequency;
}

/**
 * @brief Get measurement statistics
 * @param sonicator_id Sonicator ID (0-3)
 * @param stats Output statistics structure
 * @return true if valid statistics
 */
bool frequency_freertos_get_stats(uint8_t sonicator_id, frequency_stats_t *stats) {
    if (sonicator_id >= 4 || !stats) return false;
    
    frequency_task_context_t *ctx = &freq_contexts[sonicator_id];
    
    stats->current_frequency = ctx->current_frequency;
    stats->total_measurements = ctx->total_measurements;
    stats->error_count = ctx->error_count;
    stats->task_cycles = ctx->task_cycles;
    stats->measurement_active = ctx->measurement_active;
    
    // Get queue statistics
    stats->queue_items_waiting = uxQueueMessagesWaiting(ctx->result_queue);
    stats->queue_spaces_free = uxQueueSpacesAvailable(ctx->result_queue);
    
    return true;
}

/**
 * @brief Set measurement parameters
 * @param sonicator_id Sonicator ID (0-3)  
 * @param sample_period_ms Measurement window in milliseconds
 * @return true if parameters set successfully
 */
bool frequency_freertos_set_params(uint8_t sonicator_id, uint16_t sample_period_ms) {
    if (sonicator_id >= 4 || sample_period_ms < 100 || sample_period_ms > 10000) {
        return false;
    }
    
    freq_contexts[sonicator_id].sample_period_ms = sample_period_ms;
    return true;
}
```

### 7. Integration with Sonicator Interface

```cpp
// In src/modules/control/sonicator.cpp - Update readHardwareInputs()

void SonicatorInterface::readHardwareInputs() {
    state_.overload_active = halGpioReadSafe(pins_.overload_pin);
    state_.frequency_locked = halGpioReadSafe(pins_.freq_lock_pin);

    // Read power from ADC
    uint16_t adc_value = halAdcReadSafe(pins_.power_sense_channel);
    state_.power_watts = (float)(adc_value * 2000) / 1023.0f;

    // Read frequency using FreeRTOS task-based measurement
    uint8_t freq_channel = pins_.sonicator_id - 1; // Convert to 0-3 range
    state_.frequency_hz = frequency_freertos_get(freq_channel);
    
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

### 8. Resource Requirements

#### Memory Usage (ATmega32A with 2KB RAM)
```cpp
// FreeRTOS kernel overhead
#define FREERTOS_KERNEL_RAM         400   // bytes
#define FREERTOS_HEAP_SIZE         1024   // bytes (configurable)

// Per-task overhead (4 tasks)
#define TASK_STACK_SIZE             256   // bytes per task
#define TASK_TCB_SIZE                64   // bytes per task
#define TOTAL_TASK_OVERHEAD        1280   // bytes (4 * 320)

// Application data
#define FREQ_CONTEXTS_SIZE           80   // bytes (4 * 20)
#define QUEUES_SIZE                  80   // bytes (4 * 20)

// Total RAM usage: ~2.8KB - EXCEEDS ATmega32A capacity!
```

**Critical Issue**: Standard FreeRTOS configuration requires ~2.8KB RAM, but ATmega32A only has 2KB total. This makes FreeRTOS impractical for this hardware platform.

### 9. Minimal FreeRTOS Configuration

```cpp
// Minimal FreeRTOS config for ATmega32A (experimental)
#define configTOTAL_HEAP_SIZE           512    // Reduced heap
#define configMINIMAL_STACK_SIZE         64    // Reduced stack (128 bytes)
#define configMAX_PRIORITIES             3     // Fewer priorities
#define configUSE_TRACE_FACILITY         0     // Disable debugging
#define configUSE_STATS_FORMATTING_FUNCTIONS 0

// Use only essential features
#define configUSE_COUNTING_SEMAPHORES    0
#define configUSE_BINARY_SEMAPHORES      1
#define configUSE_QUEUE_SETS            0
#define configUSE_TIME_SLICING          0

// Estimated RAM usage with minimal config: ~1.8KB
```

### 10. Testing Framework

```cpp
// In test/unit/frequency_freertos_test.cpp
#include "frequency_freertos.h"
#include "unity.h"

void setUp(void) {
    // Initialize FreeRTOS before each test
    frequency_freertos_init();
    vTaskStartScheduler(); // Start in test mode
}

void tearDown(void) {
    // Clean up after each test
    vTaskEndScheduler();
}

void test_frequency_task_creation(void) {
    // Verify all 4 tasks were created
    for (uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_NOT_NULL(freq_contexts[i].task_handle);
        TEST_ASSERT_NOT_NULL(freq_contexts[i].result_queue);
    }
}

void test_frequency_measurement(void) {
    // Simulate frequency measurement
    // Note: This requires careful mocking of FreeRTOS functions
    
    uint16_t test_freq = frequency_freertos_get(0);
    TEST_ASSERT_TRUE(test_freq <= 25000); // Reasonable max frequency
}
```

### 11. Error Handling and Diagnostics

```cpp
// In src/modules/hal/frequency_freertos_diagnostics.cpp

typedef struct {
    uint32_t stack_high_water[4];       // Minimum stack space remaining
    uint32_t task_runtime[4];           // Task execution time
    uint32_t queue_overflows[4];        // Queue full events
    uint32_t context_switches;          // Total context switches
    uint32_t heap_free_bytes;          // Free heap memory
} freertos_diagnostics_t;

/**
 * @brief Get FreeRTOS system diagnostics
 * @param diag Output diagnostics structure
 * @return true if diagnostics available
 */
bool frequency_freertos_get_diagnostics(freertos_diagnostics_t *diag) {
    if (!diag) return false;
    
    // Check stack usage for each task
    for (uint8_t i = 0; i < 4; i++) {
        if (freq_contexts[i].task_handle != NULL) {
            diag->stack_high_water[i] = uxTaskGetStackHighWaterMark(
                freq_contexts[i].task_handle);
        }
    }
    
    // Get heap status
    diag->heap_free_bytes = xPortGetFreeHeapSize();
    
    // Runtime statistics (if enabled)
    #if (configGENERATE_RUN_TIME_STATS == 1)
        // Get task runtime statistics
        vTaskGetRunTimeStats((char*)&diag->task_runtime);
    #endif
    
    return true;
}

/**
 * @brief Monitor for FreeRTOS errors
 * @return Error code, 0 if no errors
 */
uint8_t frequency_freertos_check_errors(void) {
    freertos_diagnostics_t diag;
    
    if (!frequency_freertos_get_diagnostics(&diag)) {
        return 1; // Diagnostics failure
    }
    
    // Check for stack overflow
    for (uint8_t i = 0; i < 4; i++) {
        if (diag.stack_high_water[i] < 32) { // Less than 32 words free
            return 2; // Stack overflow risk
        }
    }
    
    // Check heap memory
    if (diag.heap_free_bytes < 100) {
        return 3; // Low memory
    }
    
    return 0; // No errors
}
```

## Performance Analysis

### CPU Utilization
- **Polling Method**: ~40-60% CPU usage (10kHz polling rate)
- **Interrupt Method**: ~5-10% CPU usage (event-driven)
- **FreeRTOS Overhead**: ~10-15% CPU for task switching

### Memory Constraints
- **ATmega32A RAM**: 2048 bytes total
- **FreeRTOS Minimum**: ~1800 bytes  
- **Application Space**: ~248 bytes remaining
- **Verdict**: Very tight memory constraints, minimal functionality possible

### Real-Time Performance
- **Task Switching Latency**: 10-50 µs depending on stack size
- **Interrupt Response**: 2-5 µs + task notification overhead
- **Frequency Update Rate**: Limited by measurement window (1 Hz typical)

## Conclusion

The FreeRTOS approach provides excellent task isolation and professional RTOS features, but faces severe limitations on the ATmega32A platform:

### Major Concerns
1. **Memory Constraints**: FreeRTOS requires ~90% of available RAM
2. **Complexity**: Significant overhead for this application
3. **Arduino Compatibility**: Potential conflicts with Arduino framework
4. **Overkill**: Simple edge counting doesn't require full RTOS

### Recommended Use Cases
FreeRTOS would be appropriate for:
- More complex microcontrollers (ARM Cortex-M, etc.)
- Applications requiring multiple concurrent processes
- Systems with >8KB RAM available
- Professional embedded systems with strict timing requirements

### Alternative Recommendation
For this specific application on ATmega32A, the **ISR-based approach** is significantly more appropriate due to:
- Lower memory overhead (80 bytes vs 1.8KB)
- Better real-time performance
- Simpler implementation and debugging
- Better Arduino framework compatibility
- More efficient use of limited resources

The FreeRTOS approach should be considered for future hardware platforms with more capable microcontrollers.