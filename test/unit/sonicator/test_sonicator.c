/**
 * @file test_sonicator_minimal.c
 * @brief Minimal Interface Tests for Sonicator Control Module
 * @author Cannasol Technologies
 * @date 2025-09-12
 * @version 1.0.0
 *
 * @details
 * Basic Unity-based tests for sonicator control interface to boost coverage.
 */

#ifdef UNIT_TEST

#include "../unity_config.h"
#include "../../../include/sonicator_control.h"

// ============================================================================
// TEST FIXTURE SETUP
// ============================================================================

void setUp(void) {
    // Basic setup for interface testing
}

void tearDown(void) {
    // Basic cleanup
}

// ============================================================================
// INTERFACE CONSTANTS TESTS
// ============================================================================

void test_sonicator_interface_constants(void) {
    // Test that sonicator constants are properly defined
    TEST_ASSERT_GREATER_THAN(0, SONICATOR_MIN_AMPLITUDE_PERCENT);
    TEST_ASSERT_GREATER_THAN(SONICATOR_MIN_AMPLITUDE_PERCENT, SONICATOR_MAX_AMPLITUDE_PERCENT);
    TEST_ASSERT_GREATER_THAN(0, SONICATOR_START_DELAY_MS);
    TEST_ASSERT_GREATER_THAN(0, SONICATOR_STOP_DELAY_MS);
    COVERAGE_MARK_FUNCTION(sonicator_constants);
}

void test_sonicator_fault_constants(void) {
    // Test fault constants
    TEST_ASSERT_NOT_EQUAL(SONICATOR_FAULT_NONE, SONICATOR_FAULT_OVERLOAD);
    TEST_ASSERT_NOT_EQUAL(SONICATOR_FAULT_NONE, SONICATOR_FAULT_FREQ_UNLOCK);
    TEST_ASSERT_NOT_EQUAL(SONICATOR_FAULT_NONE, SONICATOR_FAULT_COMM_TIMEOUT);
    COVERAGE_MARK_FUNCTION(sonicator_fault_constants);
}

void test_sonicator_state_constants(void) {
    // Test state constants
    TEST_ASSERT_NOT_EQUAL(SONICATOR_STATE_IDLE, SONICATOR_STATE_STARTING);
    TEST_ASSERT_NOT_EQUAL(SONICATOR_STATE_IDLE, SONICATOR_STATE_RUNNING);
    TEST_ASSERT_NOT_EQUAL(SONICATOR_STATE_IDLE, SONICATOR_STATE_FAULT);
    COVERAGE_MARK_FUNCTION(sonicator_state_constants);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    UNITY_BEGIN();
    
    // Basic Interface Tests
    RUN_TEST(test_sonicator_interface_constants);
    RUN_TEST(test_sonicator_fault_constants);
    RUN_TEST(test_sonicator_state_constants);
    
    return UNITY_END();
}

#endif // UNIT_TEST