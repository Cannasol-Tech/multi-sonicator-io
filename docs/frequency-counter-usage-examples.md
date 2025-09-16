# Frequency Counter Usage Examples

## Basic Usage

```cpp
#include "frequency_counter.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize frequency counting system
    if (!frequency_counter_init()) {
        Serial.println("Error: Failed to initialize frequency counter");
        return;
    }
    
    Serial.println("Frequency counter initialized successfully");
}

void loop() {
    // Read frequency from all 4 sonicators
    for (uint8_t i = 0; i < 4; i++) {
        uint16_t frequency = frequency_calculate(i);
        
        if (frequency > 0) {
            Serial.print("Sonicator ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(frequency);
            Serial.println(" Hz");
        }
    }
    
    delay(1000); // Update every second
}
```

## Advanced Usage with Statistics

```cpp
void displayFrequencyStats() {
    Serial.println("=== Frequency Statistics ===");
    
    for (uint8_t i = 0; i < 4; i++) {
        frequency_stats_t stats;
        
        if (frequency_get_stats(i, &stats)) {
            Serial.print("Channel ");
            Serial.print(i);
            Serial.println(":");
            
            Serial.print("  Current: ");
            Serial.print(stats.current_frequency);
            Serial.println(" Hz");
            
            Serial.print("  Measurements: ");
            Serial.println(stats.total_measurements);
            
            Serial.print("  Errors: ");
            Serial.println(stats.error_count);
            
            Serial.print("  Min/Max: ");
            Serial.print(stats.min_frequency);
            Serial.print("/");
            Serial.print(stats.max_frequency);
            Serial.println(" Hz");
            
            // Check for errors
            frequency_error_t error = frequency_get_error(i);
            if (error != FREQ_ERROR_NONE) {
                Serial.print("  Error: ");
                switch (error) {
                    case FREQ_ERROR_NO_SIGNAL:
                        Serial.println("No signal detected");
                        break;
                    case FREQ_ERROR_OUT_OF_RANGE:
                        Serial.println("Frequency out of range");
                        break;
                    case FREQ_ERROR_NOISE:
                        Serial.println("Excessive noise");
                        break;
                    default:
                        Serial.println("Unknown error");
                }
            }
        }
    }
    Serial.println();
}
```

## Error Handling Example

```cpp
void checkFrequencyHealth() {
    // Check ISR health
    if (!frequency_isr_health_check()) {
        Serial.println("WARNING: ISR not functioning properly");
    }
    
    // Get ISR statistics
    uint32_t total_interrupts, last_isr_time;
    if (frequency_get_isr_stats(&total_interrupts, &last_isr_time)) {
        Serial.print("Total ISR executions: ");
        Serial.println(total_interrupts);
        
        uint32_t time_since_isr = micros() - last_isr_time;
        Serial.print("Time since last ISR: ");
        Serial.print(time_since_isr);
        Serial.println(" us");
    }
}
```

## Configuration Example

```cpp
void configureFrequencyMeasurement() {
    // Set custom measurement windows
    frequency_set_window(0, 500);  // 500ms for fast response
    frequency_set_window(1, 2000); // 2s for high accuracy
    
    // Disable noise filtering for clean signals
    frequency_set_noise_filter(2, false);
    
    // Enable noise filtering for noisy environment
    frequency_set_noise_filter(3, true);
    
    Serial.println("Frequency measurement configured");
}
```

## Integration with Sonicator Control

```cpp
void updateSonicatorFromFrequency(uint8_t sonicator_id) {
    uint16_t frequency = frequency_calculate(sonicator_id);
    
    if (frequency >= 19000 && frequency <= 21000) {
        // Frequency within normal CT2000 range
        Serial.println("Sonicator frequency normal");
        
    } else if (frequency > 0) {
        // Frequency detected but out of range
        Serial.print("WARNING: Frequency out of range: ");
        Serial.print(frequency);
        Serial.println(" Hz");
        
        // Could trigger safety shutdown here
        
    } else {
        // No frequency detected
        frequency_error_t error = frequency_get_error(sonicator_id);
        
        if (error == FREQ_ERROR_NO_SIGNAL) {
            Serial.println("No frequency signal - sonicator may be off");
        } else {
            Serial.println("Frequency measurement error");
        }
    }
}
```

## Performance Monitoring

```cpp
void monitorPerformance() {
    static uint32_t last_check = 0;
    uint32_t now = millis();
    
    if (now - last_check >= 5000) { // Every 5 seconds
        Serial.println("=== Performance Monitor ===");
        
        for (uint8_t i = 0; i < 4; i++) {
            frequency_stats_t stats;
            if (frequency_get_stats(i, &stats)) {
                float error_rate = 0;
                if (stats.total_measurements > 0) {
                    error_rate = (float)stats.error_count / stats.total_measurements * 100;
                }
                
                Serial.print("Ch");
                Serial.print(i);
                Serial.print(" Error Rate: ");
                Serial.print(error_rate);
                Serial.print("% Noise: ");
                Serial.println(stats.noise_filter_count);
            }
        }
        
        last_check = now;
    }
}
```

## Reset and Maintenance

```cpp
void resetFrequencyCounters() {
    // Reset all channels
    if (frequency_reset_stats(0xFF)) {
        Serial.println("All frequency statistics reset");
    }
    
    // Or reset individual channels
    frequency_reset_stats(0); // Reset channel 0 only
}

void maintenanceMode() {
    Serial.println("Entering maintenance mode...");
    
    // Reset all statistics
    resetFrequencyCounters();
    
    // Reconfigure for maintenance
    for (uint8_t i = 0; i < 4; i++) {
        frequency_set_window(i, 100); // Fast updates for testing
        frequency_set_noise_filter(i, false); // Disable for clean test signals
    }
    
    Serial.println("Maintenance configuration applied");
}
```

## Complete Example Program

```cpp
#include "frequency_counter.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Multi-Sonicator Frequency Monitor");
    
    if (!frequency_counter_init()) {
        Serial.println("FATAL: Frequency counter initialization failed");
        while(1); // Stop execution
    }
    
    Serial.println("System ready - monitoring 4 frequency channels");
}

void loop() {
    static unsigned long last_display = 0;
    static unsigned long last_health_check = 0;
    
    // Update frequency measurements
    for (uint8_t i = 0; i < 4; i++) {
        frequency_calculate(i); // Trigger calculations
    }
    
    // Display frequencies every second
    if (millis() - last_display >= 1000) {
        Serial.print("Frequencies: ");
        for (uint8_t i = 0; i < 4; i++) {
            uint16_t freq = frequency_get_current(i);
            Serial.print(freq);
            Serial.print("Hz ");
        }
        Serial.println();
        last_display = millis();
    }
    
    // Health check every 10 seconds
    if (millis() - last_health_check >= 10000) {
        checkFrequencyHealth();
        last_health_check = millis();
    }
    
    delay(10); // 100Hz main loop
}
```