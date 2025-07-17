/**
 * @file test_sonicator.cpp
 * @brief Sonicator interface tests
 * @author Cannasol Technologies
 * @date 2025-07-17
 */

#include <unity.h>
#include <Arduino.h>
#include "../../include/config.h"
#include "../../include/types.h"
#include "../../src/modules/sonicator/sonicator.h"

/**
 * @brief Test sonicator pin assignments
 */
void test_sonicator_pins() {
    // Test that all sonicator pins are within valid range
    TEST_ASSERT_TRUE(SON1_OVERLOAD_PIN >= 0 && SON1_OVERLOAD_PIN <= 31);
    TEST_ASSERT_TRUE(SON1_FREQ_OUTPUT_PIN >= 0 && SON1_FREQ_OUTPUT_PIN <= 31);
    TEST_ASSERT_TRUE(SON1_FREQ_LOCK_PIN >= 0 && SON1_FREQ_LOCK_PIN <= 31);
    TEST_ASSERT_TRUE(SON1_START_PIN >= 0 && SON1_START_PIN <= 31);
    TEST_ASSERT_TRUE(SON1_RESET_PIN >= 0 && SON1_RESET_PIN <= 31);
    
    TEST_ASSERT_TRUE(SON2_OVERLOAD_PIN >= 0 && SON2_OVERLOAD_PIN <= 31);
    TEST_ASSERT_TRUE(SON2_FREQ_OUTPUT_PIN >= 0 && SON2_FREQ_OUTPUT_PIN <= 31);
    TEST_ASSERT_TRUE(SON2_FREQ_LOCK_PIN >= 0 && SON2_FREQ_LOCK_PIN <= 31);
    TEST_ASSERT_TRUE(SON2_START_PIN >= 0 && SON2_START_PIN <= 31);
    TEST_ASSERT_TRUE(SON2_RESET_PIN >= 0 && SON2_RESET_PIN <= 31);
}

/**
 * @brief Test sonicator hardware configuration structure
 */
void test_sonicator_hardware_config() {
    sonicator_hardware_config_t config;
    config.overload_input_pin = SON1_OVERLOAD_PIN;
    config.frequency_output_pin = SON1_FREQ_OUTPUT_PIN;
    config.frequency_lock_pin = SON1_FREQ_LOCK_PIN;
    config.start_output_pin = SON1_START_PIN;
    config.reset_output_pin = SON1_RESET_PIN;
    config.pwm_amplitude_pin = PWM_SONICATOR_1_PIN;
    config.adc_power_pin = ADC_SONICATOR_1_PIN;
    config.adc_channel = 4; // ADC4 for sonicator 1
    config.status_led_pin = LED_SONICATOR_1;
    
    TEST_ASSERT_EQUAL_INT(SON1_OVERLOAD_PIN, config.overload_input_pin);
    TEST_ASSERT_EQUAL_INT(SON1_FREQ_OUTPUT_PIN, config.frequency_output_pin);
    TEST_ASSERT_EQUAL_INT(SON1_FREQ_LOCK_PIN, config.frequency_lock_pin);
    TEST_ASSERT_EQUAL_INT(SON1_START_PIN, config.start_output_pin);
    TEST_ASSERT_EQUAL_INT(SON1_RESET_PIN, config.reset_output_pin);
    TEST_ASSERT_EQUAL_INT(PWM_SONICATOR_1_PIN, config.pwm_amplitude_pin);
    TEST_ASSERT_EQUAL_INT(ADC_SONICATOR_1_PIN, config.adc_power_pin);
    TEST_ASSERT_EQUAL_INT(4, config.adc_channel);
    TEST_ASSERT_EQUAL_INT(LED_SONICATOR_1, config.status_led_pin);
}

/**
 * @brief Test sonicator status structure initialization
 */
void test_sonicator_status_init() {
    sonicator_status_t status;
    memset(&status, 0, sizeof(status));
    
    status.sonicator_id = 1;
    status.amplitude_setpoint = 50;
    status.start_command = false;
    status.reset_command = false;
    status.actual_frequency_hz = 20000;
    status.actual_power_watts = 0;
    status.state = SONICATOR_STATE_STOPPED;
    status.is_running = false;
    status.is_overloaded = false;
    status.is_frequency_locked = false;
    status.communication_ok = true;
    status.interface_enabled = true;
    status.last_error = ERROR_NONE;
    
    TEST_ASSERT_EQUAL_INT(1, status.sonicator_id);
    TEST_ASSERT_EQUAL_INT(50, status.amplitude_setpoint);
    TEST_ASSERT_FALSE(status.start_command);
    TEST_ASSERT_FALSE(status.reset_command);
    TEST_ASSERT_EQUAL_INT(20000, status.actual_frequency_hz);
    TEST_ASSERT_EQUAL_INT(0, status.actual_power_watts);
    TEST_ASSERT_EQUAL_INT(SONICATOR_STATE_STOPPED, status.state);
    TEST_ASSERT_FALSE(status.is_running);
    TEST_ASSERT_FALSE(status.is_overloaded);
    TEST_ASSERT_FALSE(status.is_frequency_locked);
    TEST_ASSERT_TRUE(status.communication_ok);
    TEST_ASSERT_TRUE(status.interface_enabled);
    TEST_ASSERT_EQUAL_INT(ERROR_NONE, status.last_error);
}

/**
 * @brief Test amplitude validation
 */
void test_amplitude_validation() {
    // Test valid amplitude ranges
    TEST_ASSERT_TRUE(MIN_VALID_AMPLITUDE_PERCENT >= 20);
    TEST_ASSERT_TRUE(MAX_VALID_AMPLITUDE_PERCENT <= 100);
    TEST_ASSERT_TRUE(MIN_VALID_AMPLITUDE_PERCENT <= MAX_VALID_AMPLITUDE_PERCENT);
    
    // Test sonicator amplitude limits
    TEST_ASSERT_EQUAL_INT(20, SONICATOR_MIN_AMPLITUDE);
    TEST_ASSERT_EQUAL_INT(100, SONICATOR_MAX_AMPLITUDE);
}

/**
 * @brief Test power measurement validation
 */
void test_power_measurement_validation() {
    // Test valid power ranges
    TEST_ASSERT_TRUE(MIN_VALID_POWER_WATTS >= 0);
    TEST_ASSERT_TRUE(MAX_VALID_POWER_WATTS <= 2200);
    TEST_ASSERT_TRUE(MIN_VALID_POWER_WATTS <= MAX_VALID_POWER_WATTS);
    
    // Test sonicator power limits
    TEST_ASSERT_EQUAL_INT(2000, SONICATOR_MAX_POWER);
    
    // Test power scaling factor
    TEST_ASSERT_FLOAT_WITHIN(0.1, 5.44, POWER_SCALING_MV_PER_WATT);
}

/**
 * @brief Test frequency measurement validation
 */
void test_frequency_measurement_validation() {
    // Test valid frequency ranges
    TEST_ASSERT_TRUE(MIN_VALID_FREQUENCY_HZ >= 19900);
    TEST_ASSERT_TRUE(MAX_VALID_FREQUENCY_HZ <= 20100);
    TEST_ASSERT_TRUE(MIN_VALID_FREQUENCY_HZ <= MAX_VALID_FREQUENCY_HZ);
    
    // Test sonicator frequency specifications
    TEST_ASSERT_EQUAL_INT(20000, SONICATOR_FREQ_NOMINAL);
    TEST_ASSERT_EQUAL_INT(20, SONICATOR_FREQ_TOLERANCE);
    TEST_ASSERT_EQUAL_INT(10, FREQUENCY_DIVIDER);
}