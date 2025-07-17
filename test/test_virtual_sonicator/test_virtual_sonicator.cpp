/**
 * @file test_virtual_sonicator.cpp
 * @brief Tests for Virtual Sonicator Simulation
 * @author Cannasol Technologies
 * @date 2025-07-17
 */

#include <unity.h>
#include <Arduino.h>
#include "../simulation/virtual_sonicator.h"

VirtualSonicator* test_sonicator;

void setUp() {
    test_sonicator = new VirtualSonicator(1);
}

void tearDown() {
    delete test_sonicator;
    test_sonicator = nullptr;
}

/**
 * @brief Test virtual sonicator initialization
 */
void test_virtual_sonicator_init() {
    TEST_ASSERT_FALSE(test_sonicator->getIsRunning());
    TEST_ASSERT_FALSE(test_sonicator->getIsOverloaded());
    TEST_ASSERT_FALSE(test_sonicator->getIsFrequencyLocked());
    TEST_ASSERT_EQUAL(50, test_sonicator->getAmplitude());
    TEST_ASSERT_EQUAL(SONICATOR_FREQ_NOMINAL, test_sonicator->getFrequency());
    TEST_ASSERT_EQUAL(0, test_sonicator->getPower());
}

/**
 * @brief Test virtual sonicator start/stop
 */
void test_virtual_sonicator_start_stop() {
    // Test start
    TEST_ASSERT_TRUE(test_sonicator->startSonication());
    TEST_ASSERT_TRUE(test_sonicator->getIsRunning());
    
    // Test stop
    TEST_ASSERT_TRUE(test_sonicator->stopSonication());
    TEST_ASSERT_FALSE(test_sonicator->getIsRunning());
}

/**
 * @brief Test virtual sonicator amplitude control
 */
void test_virtual_sonicator_amplitude() {
    // Test valid amplitude
    TEST_ASSERT_TRUE(test_sonicator->setAmplitude(75));
    TEST_ASSERT_EQUAL(75, test_sonicator->getAmplitude());
    
    // Test invalid amplitude (too low)
    TEST_ASSERT_FALSE(test_sonicator->setAmplitude(10));
    TEST_ASSERT_EQUAL(75, test_sonicator->getAmplitude()); // Should remain unchanged
    
    // Test invalid amplitude (too high)
    TEST_ASSERT_FALSE(test_sonicator->setAmplitude(150));
    TEST_ASSERT_EQUAL(75, test_sonicator->getAmplitude()); // Should remain unchanged
}

/**
 * @brief Test virtual sonicator simulation behavior
 */
void test_virtual_sonicator_simulation() {
    // Start sonication
    test_sonicator->startSonication();
    
    // Run simulation for a few cycles
    for (int i = 0; i < 10; i++) {
        test_sonicator->update();
        delay(10); // Simulate 10ms between updates
    }
    
    // Check that simulation is running
    TEST_ASSERT_TRUE(test_sonicator->getIsRunning());
    
    // Power should be greater than 0 when running
    TEST_ASSERT_GREATER_THAN(0, test_sonicator->getPower());
    
    // Frequency should be near nominal
    uint16_t freq = test_sonicator->getFrequency();
    TEST_ASSERT_GREATER_THAN(MIN_VALID_FREQUENCY_HZ, freq);
    TEST_ASSERT_LESS_THAN(MAX_VALID_FREQUENCY_HZ, freq);
}

/**
 * @brief Test virtual sonicator fault injection
 */
void test_virtual_sonicator_fault_injection() {
    // Start sonication
    test_sonicator->startSonication();
    
    // Inject overload fault
    test_sonicator->injectFault(FAULT_OVERLOAD);
    
    // Update simulation
    test_sonicator->update();
    
    // Should trigger overload
    TEST_ASSERT_TRUE(test_sonicator->getIsOverloaded());
    TEST_ASSERT_FALSE(test_sonicator->getIsRunning());
    TEST_ASSERT_GREATER_THAN(0, test_sonicator->getOverloadCount());
    
    // Clear fault injection
    test_sonicator->clearFaultInjection();
    
    // Reset overload
    TEST_ASSERT_TRUE(test_sonicator->resetOverload());
    TEST_ASSERT_FALSE(test_sonicator->getIsOverloaded());
}

/**
 * @brief Test virtual sonicator reset functionality
 */
void test_virtual_sonicator_reset() {
    // Start sonication and modify state
    test_sonicator->startSonication();
    test_sonicator->setAmplitude(80);
    
    // Reset
    test_sonicator->reset();
    
    // Check that state is reset
    TEST_ASSERT_FALSE(test_sonicator->getIsRunning());
    TEST_ASSERT_FALSE(test_sonicator->getIsOverloaded());
    TEST_ASSERT_FALSE(test_sonicator->getIsFrequencyLocked());
    TEST_ASSERT_EQUAL(50, test_sonicator->getAmplitude());
    TEST_ASSERT_EQUAL(0, test_sonicator->getPower());
}

// Test runner
void setup() {
    delay(2000); // Wait for serial connection
    UNITY_BEGIN();
    
    RUN_TEST(test_virtual_sonicator_init);
    RUN_TEST(test_virtual_sonicator_start_stop);
    RUN_TEST(test_virtual_sonicator_amplitude);
    RUN_TEST(test_virtual_sonicator_simulation);
    RUN_TEST(test_virtual_sonicator_fault_injection);
    RUN_TEST(test_virtual_sonicator_reset);
    
    UNITY_END();
}

void loop() {
    // Empty loop
}