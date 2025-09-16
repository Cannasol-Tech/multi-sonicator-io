/**
 * @file adc.cpp
 * @brief ADC Hardware Abstraction Layer Implementation for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

#include "adc.h"
#include <system_config.h>
#include <Arduino.h>

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

/**
 * @brief Tracks whether the ADC subsystem has been properly initialized
 * @details This flag prevents ADC operations from being performed before
 *          the hardware has been configured. Set to true after successful
 *          completion of adc_init().
 */
static bool adc_initialized = false;

/**
 * @brief Current ADC voltage reference selection
 * @details Stores the currently configured voltage reference source:
 *          - ADC_REF_VCC: Uses VCC (typically 5V) as reference
 *          - ADC_REF_INTERNAL: Uses internal 2.56V reference
 *          - ADC_REF_EXTERNAL: Uses external AREF pin voltage
 */
static adc_reference_t current_reference = ADC_REF_VCC;

/**
 * @brief ADC calibration factor for voltage accuracy correction
 * @details Multiplicative factor applied to raw voltage calculations to
 *          compensate for reference voltage variations and component
 *          tolerances. Default value of 1.0 assumes perfect accuracy.
 */
static float calibration_factor = 1.0f;

// ============================================================================
// PRIVATE FUNCTION DECLARATIONS
// ============================================================================

/**
 * @brief Validates that an ADC channel number is within acceptable range
 * @param channel ADC channel to validate
 * @return true if channel is valid, false otherwise
 */
static bool is_valid_channel(adc_channel_t channel);

/**
 * @brief Maps sonicator ID to corresponding ADC channel
 * @param sonicator_id Sonicator identifier (1-4)
 * @return ADC channel for power sensing, or ADC_CHANNEL_MAX if invalid
 */
static adc_channel_t sonicator_to_adc_channel(uint8_t sonicator_id);

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Initializes the ADC subsystem with default configuration
 * @details Configures ADC with:
 *          - VCC reference voltage (5V)
 *          - 128 prescaler (16MHz/128 = 125kHz ADC clock)
 *          - Enables ADC hardware
 *          - Performs stabilization dummy conversion
 * @return ADC_OK on success, error code on failure
 */
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

/**
 * @brief Configures ADC voltage reference source
 * @details Sets ADMUX register REFS1:0 bits to select reference:
 *          - External: REFS1=0, REFS0=0 (uses AREF pin)
 *          - VCC: REFS1=0, REFS0=1 (uses VCC as reference)
 *          - Internal: REFS1=1, REFS0=1 (uses internal 2.56V)
 * @param reference Desired reference voltage source
 * @return ADC_OK on success, ADC_ERROR_INVALID_REF for invalid reference
 */
inline constexpr adc_result_t adc_set_reference(adc_reference_t reference) {
    switch (reference) {
        case ADC_REF_EXTERNAL:
            // REFS1=0, REFS0=0: External AREF
            /**
             * @brief ADMUX register manipulation for external reference
             * @details Clears both REFS1 (bit 7) and REFS0 (bit 6) in ADMUX
             *          to select external AREF pin as voltage reference
             */
            ADMUX &= ~((1 << REFS1) | (1 << REFS0));
            break;
        case ADC_REF_VCC:
            // REFS1=0, REFS0=1: VCC as reference
            /**
             * @brief ADMUX register manipulation for VCC reference
             * @details Clears REFS1 (bit 7) and sets REFS0 (bit 6) in ADMUX
             *          to select VCC as voltage reference
             */
            ADMUX &= ~(1 << REFS1);
            ADMUX |= (1 << REFS0);
            break;
        case ADC_REF_INTERNAL:
            // REFS1=1, REFS0=1: Internal 2.56V reference
            /**
             * @brief ADMUX register manipulation for internal reference
             * @details Sets both REFS1 (bit 7) and REFS0 (bit 6) in ADMUX
             *          to select internal 2.56V reference
             */
            ADMUX |= (1 << REFS1) | (1 << REFS0);
            break;
        default:
            return ADC_ERROR_INVALID_REF;
    }
    
    /**
     * @brief Updates current reference tracking variable
     * @details Stores the newly configured reference for use in voltage
     *          conversion calculations
     */
    current_reference = reference;
    
    // Allow reference to settle
    /**
     * @brief Reference settling delay
     * @details 2ms delay allows internal reference circuits to stabilize
     *          after configuration change
     */
    delay(2);
    
    return ADC_OK;
}

/**
 * @brief Configures ADC clock prescaler for optimal conversion timing
 * @details Sets ADCSRA register ADPS2:0 bits to control ADC clock frequency.
 *          ADC clock should be between 50kHz-200kHz for maximum resolution.
 * @param prescaler Desired clock division factor
 * @return ADC_OK on success, ADC_ERROR_INVALID_REF for invalid prescaler
 */
inline constexpr adc_result_t adc_set_prescaler(adc_prescaler_t prescaler) {
    if (prescaler > ADC_PRESCALER_128) {
        return ADC_ERROR_INVALID_REF;
    }
    
    // Clear prescaler bits and set new value
    /**
     * @brief ADCSRA prescaler bits manipulation
     * @details Clears ADPS2 (bit 2), ADPS1 (bit 1), and ADPS0 (bit 0)
     *          then sets the new prescaler value in these bits
     */
    ADCSRA &= ~((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
    ADCSRA |= prescaler;
    
    return ADC_OK;
}

/**
 * @brief Performs complete ADC conversion on specified channel
 * @details Selects channel, starts conversion, waits for completion with
 *          timeout protection, and returns 10-bit result
 * @param channel ADC input channel (0-7 for ATmega32A)
 * @param value Pointer to store 10-bit conversion result (0-1023)
 * @return ADC_OK on success, error code on failure
 */
inline constexpr adc_result_t adc_read_channel(adc_channel_t channel, uint16_t* value) {
    if (!adc_initialized) {
        return ADC_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_channel(channel) || value == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    // Select ADC channel (MUX4:0 bits)
    /**
     * @brief ADMUX channel selection
     * @details Preserves upper 3 bits (reference and result adjustment)
     *          while setting lower 5 bits (MUX4:0) to select input channel
     */
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);
    
    // Start conversion
    /**
     * @brief Conversion start trigger
     * @details Sets ADSC (ADC Start Conversion) bit in ADCSRA register
     *          to begin analog-to-digital conversion
     */
    ADCSRA |= (1 << ADSC);
    
    // Wait for conversion to complete (timeout after 1ms)
    /**
     * @brief Conversion timeout counter
     * @details Prevents infinite loop if ADC hardware fails to complete
     *          conversion within reasonable time (1000 microseconds)
     */
    uint16_t timeout = 1000;
    while ((ADCSRA & (1 << ADSC)) && timeout > 0) {
        /**
         * @brief Microsecond delay during polling
         * @details 1μs delay between ADSC bit checks to prevent excessive
         *          CPU usage while waiting for conversion completion
         */
        delayMicroseconds(1);
        timeout--;
    }
    
    if (timeout == 0) {
        return ADC_ERROR_TIMEOUT;
    }
    
    // Read result (ADCL must be read first)
    /**
     * @brief ADC result register reading sequence
     * @details ADCL (low byte) must be read first to trigger register
     *          locking, preventing corruption during 16-bit read operation
     */
    uint8_t low = ADCL;   ///< ADC Data Register Low byte (bits 7:0)
    uint8_t high = ADCH;  ///< ADC Data Register High byte (bits 9:8)
    
    /**
     * @brief 10-bit result reconstruction
     * @details Combines high and low bytes into complete 10-bit value
     *          with proper bit positioning
     */
    *value = (high << 8) | low;
    
    return ADC_OK;
}

/**
 * @brief Initiates ADC conversion without waiting for completion
 * @details Non-blocking function that selects channel and starts conversion.
 *          Use adc_conversion_complete() and adc_get_result() to retrieve result.
 * @param channel ADC input channel to convert
 * @return ADC_OK on success, error code on failure
 */
inline constexpr adc_result_t adc_start_conversion(adc_channel_t channel) {
    if (!adc_initialized) {
        return ADC_ERROR_NOT_INITIALIZED;
    }
    
    if (!is_valid_channel(channel)) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    // Select ADC channel
    /**
     * @brief Non-blocking channel selection
     * @details Same ADMUX manipulation as blocking version but without
     *          waiting for conversion completion
     */
    ADMUX = (ADMUX & 0xE0) | (channel & 0x1F);
    
    // Start conversion
    /**
     * @brief Non-blocking conversion start
     * @details Triggers conversion start but returns immediately,
     *          allowing other code to execute during conversion time
     */
    ADCSRA |= (1 << ADSC);
    
    return ADC_OK;
}

/**
 * @brief Checks if ADC conversion has completed
 * @details Non-blocking function to poll conversion status without
 *          consuming CPU cycles in busy-wait loop
 * @param complete Pointer to store completion status (true if done)
 * @return ADC_OK on success, error code on failure
 */
inline constexpr adc_result_t adc_conversion_complete(bool* complete) {
    if (!adc_initialized) {
        return ADC_ERROR_NOT_INITIALIZED;
    }
    
    if (complete == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    /**
     * @brief ADSC bit status check
     * @details ADSC (ADC Start Conversion) bit is automatically cleared
     *          by hardware when conversion completes. Invert logic since
     *          cleared bit indicates completion.
     */
    *complete = !(ADCSRA & (1 << ADSC));
    return ADC_OK;
}

/**
 * @brief Retrieves completed ADC conversion result
 * @details Reads ADC data registers after conversion completion.
 *          Should only be called after adc_conversion_complete() returns true.
 * @param value Pointer to store 10-bit conversion result
 * @return ADC_OK on success, error code on failure
 */
inline constexpr adc_result_t adc_get_result(uint16_t* value) {
    if (!adc_initialized) {
        return ADC_ERROR_NOT_INITIALIZED;
    }
    
    if (value == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    // Check if conversion is still running
    /**
     * @brief Conversion status verification
     * @details Ensures conversion has completed before attempting to
     *          read result registers to prevent invalid data
     */
    if (ADCSRA & (1 << ADSC)) {
        return ADC_ERROR_TIMEOUT;
    }
    
    // Read result (ADCL must be read first)
    /**
     * @brief Atomic result register reading
     * @details Same sequence as blocking read - ADCL first to lock
     *          registers and prevent corruption
     */
    uint8_t low = ADCL;   ///< ADC Data Register Low byte
    uint8_t high = ADCH;  ///< ADC Data Register High byte
    
    /**
     * @brief Result value reconstruction
     * @details Combines register bytes into complete 10-bit ADC result
     */
    *value = (high << 8) | low;
    
    return ADC_OK;
}   

/**
 * @brief Converts raw ADC value to actual voltage measurement
 * @details Applies reference voltage scaling and calibration factor to
 *          convert 10-bit ADC reading to real-world voltage value
 * @param raw_value Raw ADC reading (0-1023)
 * @param voltage Pointer to store calculated voltage in volts
 * @return ADC_OK on success, error code on failure
 */
inline adc_result_t adc_to_voltage(uint16_t raw_value, float* voltage) {
    if (voltage == nullptr) {
        return ADC_ERROR_INVALID_CHANNEL;
    }
    
    /**
     * @brief Reference voltage determination
     * @details Selects appropriate reference voltage based on current
     *          ADC configuration for accurate voltage calculation
     */
    float reference_voltage;
    switch (current_reference) {
        case ADC_REF_VCC:
            /**
             * @brief VCC reference voltage
             * @details Uses system-defined VCC voltage (typically 5.0V)
             *          from ADC_REFERENCE_VOLTAGE constant
             */
            reference_voltage = ADC_REFERENCE_VOLTAGE;
            break;
        case ADC_REF_INTERNAL:
            /**
             * @brief Internal reference voltage
             * @details ATmega32A internal reference is precisely 2.56V
             *          as specified in datasheet
             */
            reference_voltage = 2.56f;
            break;
        case ADC_REF_EXTERNAL:
            /**
             * @brief External reference voltage
             * @details Assumes external AREF matches system VCC
