/**
 * @file test_main.cpp
 * @brief Main test runner for Multi-Sonicator I/O Controller
 * @author Cannasol Technologies
 * @date 2025-07-17
 * @version 1.0.0
 */

#include <unity.h>
#include <Arduino.h>

// Test function declarations
void test_config_constants();
void test_pin_definitions();
void test_sonicator_limits();
void test_modbus_registers();
void test_error_codes();
void test_system_initialization();

/**
 * @brief Unity setup function
 */
void setUp(void) {
    // Set up code here, if needed
}

/**
 * @brief Unity teardown function
 */
void tearDown(void) {
    // Clean up code here, if needed
}

/**
 * @brief Main test runner entry point
 */
int main() {
    UNITY_BEGIN();
    
    // Configuration validation tests
    RUN_TEST(test_config_constants);
    RUN_TEST(test_pin_definitions);
    RUN_TEST(test_sonicator_limits);
    RUN_TEST(test_modbus_registers);
    RUN_TEST(test_error_codes);
    
    // System initialization tests
    RUN_TEST(test_system_initialization);
    
    return UNITY_END();
}

/**
 * @brief Arduino setup function (required by framework)
 */
void setup() {
    // Initialize serial for test output
    Serial.begin(115200);
    delay(2000); // Wait for serial connection
    
    // Run tests
    main();
}

/**
 * @brief Arduino loop function (required by framework)
 */
void loop() {
    // Tests run once in setup()
}