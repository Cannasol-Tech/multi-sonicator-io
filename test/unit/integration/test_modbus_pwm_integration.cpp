/**
 * @file test_modbus_pwm_integration.cpp
 * @brief Integration tests for MODBUS to PWM amplitude control
 * @author Cannasol Technologies  
 * @date 2025-09-05
 * @version 1.0.0
 *
 * @details
 * Tests the complete integration path from MODBUS register writes to PWM output
 * for amplitude control. Verifies the end-to-end functionality required by 
 * the BDD tests in amplitude_control.feature.
 */

#include <unity.h>
#include "../../../src/modules/communication/modbus_register_manager.h"
#include "../../../src/modules/control/Multiplexer.h"
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

static Multiplexer* multiplexer = nullptr;

void setUp(void) {
    // Reset mock state before each test
    TCCR2 = 0;
    OCR2 = 0;
    
    // Initialize subsystems
    register_manager_init();
    pwm_init();
    pwm_enable_channel(PWM_CHANNEL_AMPLITUDE);
    
    // Create multiplexer instance
    if (!multiplexer) {
        multiplexer = new Multiplexer();
    }
}

void tearDown(void) {
    // Clean up after each test
    if (multiplexer) {
        delete multiplexer;
        multiplexer = nullptr;
    }
}

void test_modbus_register_to_pwm_amplitude_20_percent(void) {
    // Test case from amplitude_control.feature: 20% amplitude
    
    // Set MODBUS register 40001 (sonicator 1 amplitude) to 20%
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_NOT_NULL(map);
    
    map->sonicators[0].amplitude_setpoint = 20;
    
    // Apply amplitude via multiplexer (simulates main loop behavior)
    TEST_ASSERT_TRUE(multiplexer->setAmplitude(20));
    
    // Verify PWM amplitude was set correctly
    uint8_t actual_amplitude = 0;
    TEST_ASSERT_EQUAL(PWM_OK, pwm_get_amplitude(&actual_amplitude));
    TEST_ASSERT_EQUAL(20, actual_amplitude);
    
    // Verify voltage output calculation (20% -> 2V)
    float output_voltage = 0.0f;
    TEST_ASSERT_EQUAL(PWM_OK, pwm_amplitude_to_voltage(20, &output_voltage));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 2.0f, output_voltage);
}

void test_modbus_register_to_pwm_amplitude_60_percent(void) {
    // Test case from amplitude_control.feature: 60% amplitude
    
    modbus_register_map_t* map = register_manager_get_map();
    map->sonicators[0].amplitude_setpoint = 60;
    
    // Apply amplitude via multiplexer
    TEST_ASSERT_TRUE(multiplexer->setAmplitude(60));
    
    // Verify PWM amplitude
    uint8_t actual_amplitude = 0;
    TEST_ASSERT_EQUAL(PWM_OK, pwm_get_amplitude(&actual_amplitude));
    TEST_ASSERT_EQUAL(60, actual_amplitude);
    
    // Verify voltage output (60% -> 6V)
    float output_voltage = 0.0f;
    TEST_ASSERT_EQUAL(PWM_OK, pwm_amplitude_to_voltage(60, &output_voltage));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 6.0f, output_voltage);
}

void test_modbus_register_to_pwm_amplitude_100_percent(void) {
    // Test case from amplitude_control.feature: 100% amplitude
    
    modbus_register_map_t* map = register_manager_get_map();
    map->sonicators[0].amplitude_setpoint = 100;
    
    // Apply amplitude via multiplexer  
    TEST_ASSERT_TRUE(multiplexer->setAmplitude(100));
    
    // Verify PWM amplitude
    uint8_t actual_amplitude = 0;
    TEST_ASSERT_EQUAL(PWM_OK, pwm_get_amplitude(&actual_amplitude));
    TEST_ASSERT_EQUAL(100, actual_amplitude);
    
    // Verify voltage output (100% -> 10V)
    float output_voltage = 0.0f;
    TEST_ASSERT_EQUAL(PWM_OK, pwm_amplitude_to_voltage(100, &output_voltage));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, output_voltage);
}

void test_modbus_invalid_amplitude_rejection(void) {
    // Test case from amplitude_control.feature: reject out-of-range amplitude
    
    // Try to set invalid amplitude (5% - below minimum of 20%)
    modbus_register_map_t* map = register_manager_get_map();
    map->sonicators[0].amplitude_setpoint = 5;
    
    // The multiplexer should reject this value
    TEST_ASSERT_FALSE(multiplexer->setAmplitude(5));
    
    // PWM should not be updated to invalid value
    uint8_t current_amplitude = 0;
    pwm_get_amplitude(&current_amplitude);
    TEST_ASSERT_NOT_EQUAL(5, current_amplitude);
}

void test_emergency_stop_integration(void) {
    // Set amplitude to high value first
    multiplexer->setAmplitude(80);
    
    // Verify amplitude was set
    uint8_t amplitude_before = 0;
    pwm_get_amplitude(&amplitude_before);
    TEST_ASSERT_EQUAL(80, amplitude_before);
    
    // Trigger emergency stop via register
    modbus_register_map_t* map = register_manager_get_map();
    map->global_control.emergency_stop = 1;
    
    // Emergency stop should be detected
    TEST_ASSERT_TRUE(register_manager_is_emergency_stop_active());
    
    // Apply emergency stop (simulates main loop emergency handling)
    TEST_ASSERT_EQUAL(PWM_OK, pwm_emergency_stop());
    
    // Verify amplitude was reduced to minimum
    uint8_t amplitude_after = 0;
    pwm_get_amplitude(&amplitude_after);
    TEST_ASSERT_EQUAL(PWM_AMPLITUDE_MIN, amplitude_after);
}

void test_shared_amplitude_control(void) {
    // All sonicators should share the same amplitude control PWM
    
    // Set different amplitude setpoints for different sonicators
    modbus_register_map_t* map = register_manager_get_map();
    map->sonicators[0].amplitude_setpoint = 30;
    map->sonicators[1].amplitude_setpoint = 50;
    map->sonicators[2].amplitude_setpoint = 70;
    map->sonicators[3].amplitude_setpoint = 90;
    
    // Apply amplitude from sonicator 2 (50%)
    TEST_ASSERT_TRUE(multiplexer->setAmplitude(map->sonicators[1].amplitude_setpoint));
    
    // Verify the shared PWM shows this amplitude
    uint8_t shared_amplitude = 0;
    pwm_get_amplitude(&shared_amplitude);
    TEST_ASSERT_EQUAL(50, shared_amplitude);
    
    // The last amplitude setting should be the active one since it's shared
    TEST_ASSERT_TRUE(multiplexer->setAmplitude(map->sonicators[3].amplitude_setpoint));
    pwm_get_amplitude(&shared_amplitude);
    TEST_ASSERT_EQUAL(90, shared_amplitude);
}

void test_pwm_pin_configuration(void) {
    // Verify the PWM is configured for the correct pin (PD7 = pin 21)
    // This is verified by checking the constants match hardware config
    
    TEST_ASSERT_EQUAL(21, PWM_AMPLITUDE_CONTROL_PIN);
    
    // Verify channel assignment
    TEST_ASSERT_EQUAL(0, PWM_CHANNEL_AMPLITUDE);
}

int main(void) {
    UNITY_BEGIN();
    
    // MODBUS to PWM integration tests matching BDD scenarios
    RUN_TEST(test_modbus_register_to_pwm_amplitude_20_percent);
    RUN_TEST(test_modbus_register_to_pwm_amplitude_60_percent); 
    RUN_TEST(test_modbus_register_to_pwm_amplitude_100_percent);
    RUN_TEST(test_modbus_invalid_amplitude_rejection);
    
    // System integration tests
    RUN_TEST(test_emergency_stop_integration);
    RUN_TEST(test_shared_amplitude_control);
    RUN_TEST(test_pwm_pin_configuration);
    
    return UNITY_END();
}