/**
 * @file test_pwm_amplitude_control.cpp
 * @brief Unit tests for PWM amplitude control verification
 * @author Cannasol Technologies
 * @date 2025-09-05
 * @version 1.0.0
 * 
 * @details
 * Tests for PWM amplitude control functionality to verify MODBUS integration
 * and proper output voltage generation for sonicator amplitude control.
 */

#include <unity.h>
#include "../../../src/modules/hal/pwm.h"
#include "../../../include/system_config.h"

// Mock Arduino functions for testing
extern "C" {
    void pinMode(int pin, int mode) { /* mock */ }
    void digitalWrite(int pin, int value) { /* mock */ }
    unsigned long millis(void) { return 1000; }
    void delay(unsigned long ms) { /* mock */ }
}

// Mock timer registers for ATmega32A
extern "C" {
    volatile uint8_t TCCR2 = 0;
    volatile uint8_t OCR2 = 0;
    const int WGM21 = 3;
    const int WGM20 = 6;
    const int COM21 = 5;
    const int COM20 = 4;
    const int CS22 = 2;
    const int CS21 = 1;
    const int CS20 = 0;
    const unsigned long F_CPU = 16000000UL;
    const int OUTPUT = 1;
    const int LOW = 0;
}

void setUp(void) {
    // Reset mock state before each test
    TCCR2 = 0;
    OCR2 = 0;
}

void tearDown(void) {
    // Clean up after each test
}

void test_pwm_init(void) {
    // Test PWM initialization
    pwm_result_t result = pwm_init();
    TEST_ASSERT_EQUAL(PWM_OK, result);
    
    // Verify Timer2 is configured for Fast PWM
    TEST_ASSERT_TRUE(TCCR2 & (1 << WGM21));
    TEST_ASSERT_TRUE(TCCR2 & (1 << WGM20));
}

void test_pwm_set_amplitude_valid_range(void) {
    // Initialize PWM first
    pwm_init();
    
    // Test valid amplitude values
    TEST_ASSERT_EQUAL(PWM_OK, pwm_set_amplitude(20));  // Minimum
    TEST_ASSERT_EQUAL(PWM_OK, pwm_set_amplitude(50));  // Mid-range
    TEST_ASSERT_EQUAL(PWM_OK, pwm_set_amplitude(100)); // Maximum
}

void test_pwm_set_amplitude_invalid_range(void) {
    // Initialize PWM first
    pwm_init();
    
    // Test invalid amplitude values (should be rejected)
    TEST_ASSERT_EQUAL(PWM_ERROR_INVALID_DUTY, pwm_set_amplitude(0));   // Too low
    TEST_ASSERT_EQUAL(PWM_ERROR_INVALID_DUTY, pwm_set_amplitude(10));  // Too low
    TEST_ASSERT_EQUAL(PWM_ERROR_INVALID_DUTY, pwm_set_amplitude(101)); // Too high
    TEST_ASSERT_EQUAL(PWM_ERROR_INVALID_DUTY, pwm_set_amplitude(255)); // Too high
}

void test_pwm_amplitude_to_voltage_conversion(void) {
    // Test amplitude to voltage conversion
    float voltage = 0.0f;
    
    // Test minimum amplitude (20% -> 2V)
    TEST_ASSERT_EQUAL(PWM_OK, pwm_amplitude_to_voltage(20, &voltage));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 2.0f, voltage);
    
    // Test mid-range amplitude (60% -> 6V)
    TEST_ASSERT_EQUAL(PWM_OK, pwm_amplitude_to_voltage(60, &voltage));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 6.0f, voltage);
    
    // Test maximum amplitude (100% -> 10V)
    TEST_ASSERT_EQUAL(PWM_OK, pwm_amplitude_to_voltage(100, &voltage));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, voltage);
}

void test_pwm_get_amplitude(void) {
    // Initialize and set amplitude
    pwm_init();
    pwm_set_amplitude(75);
    
    // Verify we can read back the amplitude
    uint8_t amplitude = 0;
    TEST_ASSERT_EQUAL(PWM_OK, pwm_get_amplitude(&amplitude));
    TEST_ASSERT_EQUAL(75, amplitude);
}

void test_pwm_emergency_stop(void) {
    // Initialize PWM and set to high amplitude
    pwm_init();
    pwm_set_amplitude(80);
    
    // Trigger emergency stop
    TEST_ASSERT_EQUAL(PWM_OK, pwm_emergency_stop());
    
    // Verify amplitude is set to minimum
    uint8_t amplitude = 0;
    pwm_get_amplitude(&amplitude);
    TEST_ASSERT_EQUAL(PWM_AMPLITUDE_MIN, amplitude);
}

void test_pwm_enable_disable_channel(void) {
    // Initialize PWM
    pwm_init();
    
    // Test channel enable
    TEST_ASSERT_EQUAL(PWM_OK, pwm_enable_channel(PWM_CHANNEL_AMPLITUDE));
    
    // Verify PWM output is enabled (COM21 set, COM20 clear)
    TEST_ASSERT_TRUE(TCCR2 & (1 << COM21));
    TEST_ASSERT_FALSE(TCCR2 & (1 << COM20));
    
    // Test channel disable
    TEST_ASSERT_EQUAL(PWM_OK, pwm_disable_channel(PWM_CHANNEL_AMPLITUDE));
    
    // Verify PWM output is disabled
    TEST_ASSERT_FALSE(TCCR2 & (1 << COM21));
    TEST_ASSERT_FALSE(TCCR2 & (1 << COM20));
}

void test_pwm_amplitude_limits_constants(void) {
    // Verify amplitude limits match system configuration
    TEST_ASSERT_EQUAL(20, PWM_AMPLITUDE_MIN);
    TEST_ASSERT_EQUAL(100, PWM_AMPLITUDE_MAX);
    
    // Verify these match the system config constants
    TEST_ASSERT_EQUAL(SONICATOR_MIN_AMPLITUDE, PWM_AMPLITUDE_MIN);
    TEST_ASSERT_EQUAL(SONICATOR_MAX_AMPLITUDE, PWM_AMPLITUDE_MAX);
}

void test_pwm_frequency_configuration(void) {
    // Initialize PWM
    pwm_init();
    
    // Verify default frequency
    uint32_t frequency = 0;
    TEST_ASSERT_EQUAL(PWM_OK, pwm_get_frequency(PWM_CHANNEL_AMPLITUDE, &frequency));
    TEST_ASSERT_EQUAL(PWM_DEFAULT_FREQUENCY, frequency);
}

int main(void) {
    UNITY_BEGIN();
    
    // Basic PWM functionality tests
    RUN_TEST(test_pwm_init);
    RUN_TEST(test_pwm_set_amplitude_valid_range);
    RUN_TEST(test_pwm_set_amplitude_invalid_range);
    RUN_TEST(test_pwm_amplitude_to_voltage_conversion);
    RUN_TEST(test_pwm_get_amplitude);
    RUN_TEST(test_pwm_emergency_stop);
    RUN_TEST(test_pwm_enable_disable_channel);
    
    // Configuration and integration tests
    RUN_TEST(test_pwm_amplitude_limits_constants);
    RUN_TEST(test_pwm_frequency_configuration);
    
    return UNITY_END();
}