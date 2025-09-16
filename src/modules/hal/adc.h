/**
 * @file adc.h
 * @brief ADC Hardware Abstraction Layer for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Provides hardware-agnostic ADC interface for sonicator power monitoring
 * and frequency measurement. Supports 10-bit resolution with 5V reference.
 * 
 * Power monitoring uses 5.44mV/W scaling factor per pin matrix specifications.
 * Pin assignments per config/hardware-config.yaml (SOLE SOURCE OF TRUTH).
 */

#ifndef HAL_ADC_H
#define HAL_ADC_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @brief ADC channel identifiers
 */
typedef enum {
    ADC_CHANNEL_0 = 0,          //< ADC0 - LM2907 frequency converter
    ADC_CHANNEL_1 = 1,          //< ADC1 - Reserved
    ADC_CHANNEL_2 = 2,          //< ADC2 - Reserved  
    ADC_CHANNEL_3 = 3,          //< ADC3 - Reserved
    ADC_CHANNEL_4 = 4,          //< ADC4 - Sonicator 1 power (PA4)
    ADC_CHANNEL_5 = 5,          //< ADC5 - Sonicator 2 power (PA5)
    ADC_CHANNEL_6 = 6,          //< ADC6 - Sonicator 3 power (PA6)
    ADC_CHANNEL_7 = 7,          //< ADC7 - Sonicator 4 power (PA7)
    ADC_CHANNEL_MAX = 8
} adc_channel_t;

/**
 * @brief ADC reference voltage selection
 */
typedef enum {
    ADC_REF_EXTERNAL = 0,       //< External AREF pin
    ADC_REF_VCC = 1,            //< VCC (5V) reference
    ADC_REF_INTERNAL = 2        //< Internal 2.56V reference
} adc_reference_t;

/**
 * @brief ADC prescaler values for clock division
 */
typedef enum {
    ADC_PRESCALER_2 = 1,        //< Divide by 2
    ADC_PRESCALER_4 = 2,        //< Divide by 4
    ADC_PRESCALER_8 = 3,        //< Divide by 8
    ADC_PRESCALER_16 = 4,       //< Divide by 16
    ADC_PRESCALER_32 = 5,       //< Divide by 32
    ADC_PRESCALER_64 = 6,       //< Divide by 64
    ADC_PRESCALER_128 = 7       //< Divide by 128
} adc_prescaler_t;

/**
 * @brief HAL function return codes
 */
typedef enum {
    ADC_OK = 0,                 //< Operation successful
    ADC_ERROR_INVALID_CHANNEL,  //< Invalid channel number
    ADC_ERROR_INVALID_REF,      //< Invalid reference selection
    ADC_ERROR_TIMEOUT,          //< Conversion timeout
    ADC_ERROR_NOT_INITIALIZED   //< ADC not initialized
} adc_result_t;

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define ADC_RESOLUTION_BITS     10          //< 10-bit ADC resolution
#define ADC_MAX_VALUE          1023         //< Maximum ADC reading (2^10 - 1)
#define ADC_REFERENCE_VOLTAGE   5.0f        //< Reference voltage (5V)
#define ADC_CONVERSION_TIME_US  104         //< Typical conversion time at 125kHz

// Power monitoring constants (per pin matrix)
#define ADC_POWER_SCALING_MV_PER_W  5.44f   //< 5.44mV per watt scaling
#define ADC_POWER_MAX_WATTS        2000     //< Maximum power (2000W)
#define ADC_POWER_VOLTAGE_DIVIDER   2.0f    //< 10kΩ voltage divider ratio

// Frequency monitoring constants (LM2907)
#define ADC_FREQ_SCALE_HZ_PER_V    500      //< 500 Hz per volt
#define ADC_FREQ_MIN_HZ            1900     //< Minimum frequency (1.9kHz)
#define ADC_FREQ_MAX_HZ            2100     //< Maximum frequency (2.1kHz)

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

/**
 * @brief Initialize ADC hardware abstraction layer
 * 
 * Configures ADC with VCC reference, 125kHz clock, and enables ADC subsystem.
 * Sets up all power monitoring channels for sonicator interfaces.
 * 
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_init(void);

/**
 * @brief Configure ADC reference voltage
 * 
 * @param reference Reference voltage selection
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_set_reference(adc_reference_t reference);

/**
 * @brief Configure ADC clock prescaler
 * 
 * @param prescaler Clock division factor
 * @return ADC_OK on success, error code on failure
 * 
 * @note ADC clock should be 50-200kHz for optimal accuracy
 */
adc_result_t adc_set_prescaler(adc_prescaler_t prescaler);

/**
 * @brief Perform single ADC conversion
 * 
 * @param channel ADC channel to read (0-7)
 * @param value Pointer to store 10-bit result
 * @return ADC_OK on success, error code on failure
 * 
 * @note Blocks until conversion complete (~104μs)
 */
adc_result_t adc_read_channel(adc_channel_t channel, uint16_t* value);

/**
 * @brief Start ADC conversion (non-blocking)
 * 
 * @param channel ADC channel to convert
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_start_conversion(adc_channel_t channel);

/**
 * @brief Check if ADC conversion is complete
 * 
 * @param complete Pointer to store completion status
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_conversion_complete(bool* complete);

/**
 * @brief Get result of completed conversion
 * 
 * @param value Pointer to store 10-bit result
 * @return ADC_OK on success, error code on failure
 * 
 * @note Only call after adc_conversion_complete() returns true
 */
adc_result_t adc_get_result(uint16_t* value);

/**
 * @brief Convert raw ADC value to voltage
 * 
 * @param raw_value Raw ADC reading (0-1023)
 * @param voltage Pointer to store voltage result
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_to_voltage(uint16_t raw_value, float* voltage);

/**
 * @brief Convert raw ADC value to power (watts)
 * 
 * Applies sonicator power scaling: 5.44mV/W with voltage divider
 * 
 * @param raw_value Raw ADC reading from power monitoring channel
 * @param power_watts Pointer to store power result in watts
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_to_power(uint16_t raw_value, float* power_watts);

/**
 * @brief Convert raw ADC value to frequency (Hz)
 * 
 * Applies LM2907 frequency converter scaling for channel 0
 * 
 * @param raw_value Raw ADC reading from frequency channel
 * @param frequency_hz Pointer to store frequency result in Hz
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_to_frequency(uint16_t raw_value, float* frequency_hz);

// ============================================================================
// SONICATOR POWER MONITORING CONVENIENCE FUNCTIONS
// ============================================================================

/**
 * @brief Read sonicator power level (raw ADC value)
 *
 * @param sonicator_id Sonicator number (1-4)
 * @param raw_adc Pointer to store raw ADC reading (0-1023)
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_read_sonicator_power_raw(uint8_t sonicator_id, uint16_t* raw_adc);

/**
 * @brief Read sonicator power level (converted to watts)
 * @deprecated Use adc_read_sonicator_power_raw() for efficiency
 *
 * @param sonicator_id Sonicator number (1-4)
 * @param power_watts Pointer to store power in watts
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_read_sonicator_power(uint8_t sonicator_id, float* power_watts);

/**
 * @brief Read all sonicator power levels
 * 
 * @param power_array Array to store 4 power readings [watts]
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_read_all_power(float power_array[4]);

/**
 * @brief Read frequency from LM2907 converter
 * 
 * @param frequency_hz Pointer to store frequency in Hz
 * @return ADC_OK on success, error code on failure
 */
adc_result_t adc_read_frequency(float* frequency_hz);

/**
 * @brief Calibrate ADC with known reference voltage
 * 
 * @param reference_voltage Known reference voltage for calibration
 * @return ADC_OK on success, error code on failure
 * 
 * @note Used for HIL testing with precision voltage sources
 */
adc_result_t adc_calibrate(float reference_voltage);

#ifdef __cplusplus
}
#endif

#endif // HAL_ADC_H
