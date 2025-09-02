/**
 * @file adc.cpp
 * @brief ADC Hardware Abstraction Layer Implementation for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

#include "adc.h"
#include <config.h>
#include <Arduino.h>

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static bool adc_initialized = false;
static adc_reference_t current_reference = ADC_REF_VCC;
static float calibration_factor = 1.0f;

// ============================================================================
// PRIVATE FUNCTION DECLARATIONS
// ============================================================================

static bool is_valid_channel(adc_channel_t channel);
static adc_channel_t sonicator_to_adc_channel(uint8_t sonicator_id);

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

adc_result_t adc_init(void) {
    // Configure ADC with VCC reference (5V)
    adc_result_t result = adc_set_reference(ADC_REF_VCC);
    if (result != ADC_OK) {
        return result;
    }
    
    // Set prescaler for 125kHz ADC clock (16MHz / 128 = 125kHz)
    result = adc_set_prescaler(ADC_PRESCALER_128);
    if (result != ADC_OK) {
        return result;
    }
    
    // Enable ADC subsystem
    ADCSRA |= (1 << ADEN);
    
    // Perform dummy conversion to stabilize ADC
    uint16_t dummy_value;
    adc_read_channel(ADC_CHANNEL_0, &dummy_value);
    
    adc_initialized = true;
    return ADC_OK;
}

adc_result_t adc_set_reference(adc_reference_t reference) {
    switch (reference) {
        case ADC_REF_EXTERNAL:
            // REFS1=0, REFS0=0: External AREF
            ADMUX &= ~((1 << REFS1) | (1 << REFS0));
            break;
        case ADC_REF_VCC:
            // REFS1=0, REFS0=1: VCC as reference
            ADMUX &= ~(1 << REFS1);
            ADMUX |= (1 << REFS0);
            break;
        case ADC_REF_INTERNAL:
            // REFS1=1, REFS0=1: Internal 2.56V reference
            ADMUX |= (1 << REFS1) | (1 << REFS0);
            break;
        default:
            return ADC_ERROR_INVALID_REF;
    }
    
    current_reference = reference;
    
    // Allow reference to settle
    delay(2);
    
    return ADC_OK;
}

adc_result_t adc_set_prescaler(adc_prescaler_t prescaler) {
    if (prescaler > ADC_PRESCALER_128) {
        return ADC_ERROR_INVALID_REF;
    }
    
    // Clear prescaler bits and set new value
    ADCSRA &= ~((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
    ADCSRA |= prescaler;
    
    return ADC_OK;
}

adc_result_t adc_read_channel(adc_channel_t channel, uint16_t* value) {
    if (!adc_initialized) {
        return ADC_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_channel(channel) || value == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    // Select ADC channel (MUX4:0 bits)
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);
    
    // Start conversion
    ADCSRA |= (1 << ADSC);
    
    // Wait for conversion to complete (timeout after 1ms)
    uint16_t timeout = 1000;
    while ((ADCSRA & (1 << ADSC)) && timeout > 0) {
        delayMicroseconds(1);
        timeout--;
    }
    
    if (timeout == 0) {
        return ADC_ERROR_TIMEOUT;
    }
    
    // Read result (ADCL must be read first)
    uint8_t low = ADCL;
    uint8_t high = ADCH;
    *value = (high << 8) | low;
    
    return ADC_OK;
}

adc_result_t adc_start_conversion(adc_channel_t channel) {
    if (!adc_initialized) {
        return ADC_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_channel(channel)) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    // Select ADC channel
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);
    
    // Start conversion
    ADCSRA |= (1 << ADSC);
    
    return ADC_OK;
}

adc_result_t adc_conversion_complete(bool* complete) {
    if (!adc_initialized) {
        return ADC_ERROR_NOT_INITIALIZED;
    }
    
    if (complete == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    *complete = !(ADCSRA & (1 << ADSC));
    return ADC_OK;
}

adc_result_t adc_get_result(uint16_t* value) {
    if (!adc_initialized) {
        return ADC_ERROR_NOT_INITIALIZED;
    }
    
    if (value == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    // Check if conversion is still running
    if (ADCSRA & (1 << ADSC)) {
        return ADC_ERROR_TIMEOUT;
    }
    
    // Read result (ADCL must be read first)
    uint8_t low = ADCL;
    uint8_t high = ADCH;
    *value = (high << 8) | low;
    
    return ADC_OK;
}

adc_result_t adc_to_voltage(uint16_t raw_value, float* voltage) {
    if (voltage == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    float reference_voltage;
    switch (current_reference) {
        case ADC_REF_VCC:
            reference_voltage = ADC_REFERENCE_VOLTAGE;
            break;
        case ADC_REF_INTERNAL:
            reference_voltage = 2.56f;
            break;
        case ADC_REF_EXTERNAL:
            reference_voltage = ADC_REFERENCE_VOLTAGE; // Assume 5V external
            break;
        default:
            return ADC_ERROR_INVALID_REF;
    }
    
    *voltage = (raw_value * reference_voltage * calibration_factor) / ADC_MAX_VALUE;
    return ADC_OK;
}

adc_result_t adc_to_power(uint16_t raw_value, float* power_watts) {
    if (power_watts == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    float voltage;
    adc_result_t result = adc_to_voltage(raw_value, &voltage);
    if (result != ADC_OK) {
        return result;
    }
    
    // Account for voltage divider (factor of 2)
    float actual_voltage = voltage * ADC_POWER_VOLTAGE_DIVIDER;
    
    // Convert to power using scaling factor: 5.44mV/W
    *power_watts = actual_voltage / (ADC_POWER_SCALING_MV_PER_W / 1000.0f);
    
    return ADC_OK;
}

adc_result_t adc_to_frequency(uint16_t raw_value, float* frequency_hz) {
    if (frequency_hz == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    float voltage;
    adc_result_t result = adc_to_voltage(raw_value, &voltage);
    if (result != ADC_OK) {
        return result;
    }
    
    // Convert to frequency using LM2907 scaling: 500 Hz/V
    *frequency_hz = voltage * ADC_FREQ_SCALE_HZ_PER_V;
    
    return ADC_OK;
}

// ============================================================================
// SONICATOR CONVENIENCE FUNCTIONS
// ============================================================================

adc_result_t adc_read_sonicator_power(uint8_t sonicator_id, float* power_watts) {
    if (sonicator_id < 1 || sonicator_id > MAX_SONICATORS || power_watts == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    adc_channel_t channel = sonicator_to_adc_channel(sonicator_id);
    if (channel >= ADC_CHANNEL_MAX) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    uint16_t raw_value;
    adc_result_t result = adc_read_channel(channel, &raw_value);
    if (result != ADC_OK) {
        return result;
    }
    
    return adc_to_power(raw_value, power_watts);
}

adc_result_t adc_read_all_power(float power_array[4]) {
    if (power_array == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    for (uint8_t i = 0; i < 4; i++) {
        adc_result_t result = adc_read_sonicator_power(i + 1, &power_array[i]);
        if (result != ADC_OK) {
            return result;
        }
    }
    
    return ADC_OK;
}

adc_result_t adc_read_frequency(float* frequency_hz) {
    if (frequency_hz == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    uint16_t raw_value;
    adc_result_t result = adc_read_channel(ADC_CHANNEL_0, &raw_value);
    if (result != ADC_OK) {
        return result;
    }
    
    return adc_to_frequency(raw_value, frequency_hz);
}

adc_result_t adc_calibrate(float reference_voltage) {
    if (reference_voltage <= 0.0f) {
        return ADC_ERROR_INVALID_REF;
    }
    
    // Read a stable channel for calibration
    uint16_t raw_value;
    adc_result_t result = adc_read_channel(ADC_CHANNEL_0, &raw_value);
    if (result != ADC_OK) {
        return result;
    }
    
    float measured_voltage;
    result = adc_to_voltage(raw_value, &measured_voltage);
    if (result != ADC_OK) {
        return result;
    }
    
    // Calculate calibration factor
    if (measured_voltage > 0.0f) {
        calibration_factor = reference_voltage / measured_voltage;
    }
    
    return ADC_OK;
}

// ============================================================================
// PRIVATE FUNCTION IMPLEMENTATIONS
// ============================================================================

static bool is_valid_channel(adc_channel_t channel) {
    return (channel < ADC_CHANNEL_MAX);
}

static adc_channel_t sonicator_to_adc_channel(uint8_t sonicator_id) {
    switch (sonicator_id) {
        case 1: return ADC_CHANNEL_4; // PA4
        case 2: return ADC_CHANNEL_5; // PA5
        case 3: return ADC_CHANNEL_6; // PA6
        case 4: return ADC_CHANNEL_7; // PA7
        default: return ADC_CHANNEL_MAX; // Invalid
    }
}
