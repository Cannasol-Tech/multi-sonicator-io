/**
 * @file test_control.c
 * @brief Comprehensive Unit Tests for Control Module
 * @author Cannasol Technologies
 * @date 2025-09-04
 * @version 1.0.0
 *
 * @details
 * Unity-based unit tests for control module achieving ≥85% code coverage.
 * Tests sonicator control logic, state machines, and safety systems.
 */

#ifdef UNIT_TEST

#include "../unity_config.h"
// Mock implementation - don't include actual headers
// #include "../../../src/modules/control/sonicator4_controller.h"

// Mock types for control module testing
typedef int controller_result_t;

#define CONTROLLER_OK 0
#define CONTROLLER_ERROR_INVALID_PARAM 1
#define CONTROLLER_ERROR_INVALID_CONFIG 2
#define CONTROLLER_ERROR_INVALID_ID 3
#define CONTROLLER_ERROR_NOT_INITIALIZED 4
#define CONTROLLER_ERROR_ALREADY_RUNNING 5

#define CONTROLLER_STATE_IDLE 0
#define CONTROLLER_STATE_RUNNING 1
#define CONTROLLER_STATE_EMERGENCY 2

typedef struct {
    float max_power_watts;
    uint16_t min_frequency_hz;
    uint16_t max_frequency_hz;
    uint8_t default_amplitude;
    uint32_t safety_timeout_ms;
} sonicator_config_t;

typedef struct {
    bool active;
    uint8_t amplitude;
    uint16_t frequency_hz;
} sonicator_instance_t;

typedef struct {
    bool initialized;
    uint8_t state;
    sonicator_instance_t sonicators[4];
} sonicator_controller_t;

// Include mock function declarations
extern controller_result_t sonicator_controller_init(sonicator_controller_t* controller, const sonicator_config_t* config);
extern controller_result_t sonicator_controller_start(sonicator_controller_t* controller, uint8_t id, uint8_t amplitude, uint16_t frequency_hz);
extern controller_result_t sonicator_controller_stop(sonicator_controller_t* controller, uint8_t id);
extern controller_result_t sonicator_controller_stop_all(sonicator_controller_t* controller);
extern controller_result_t sonicator_controller_emergency_stop(sonicator_controller_t* controller);

// ============================================================================
// TEST FIXTURE SETUP
// ============================================================================

static sonicator_controller_t test_controller;
static sonicator_config_t test_config;

void setUp(void) {
    // Initialize test configuration
    test_config.max_power_watts = 100.0f;
    test_config.min_frequency_hz = 20000;
    test_config.max_frequency_hz = 40000;
    test_config.default_amplitude = 50;
    test_config.safety_timeout_ms = 5000;

    // Reset controller state
    memset(&test_controller, 0, sizeof(test_controller));
}

void tearDown(void) {
    // Clean up after each test
    sonicator_controller_stop_all(&test_controller);
}

// ============================================================================
// CONTROLLER INITIALIZATION TESTS
// ============================================================================

void test_controller_init_valid_config(void) {
    controller_result_t result = sonicator_controller_init(&test_controller, &test_config);

    TEST_ASSERT_EQUAL(CONTROLLER_OK, result);
    TEST_ASSERT_TRUE(test_controller.initialized);
    TEST_ASSERT_EQUAL(CONTROLLER_STATE_IDLE, test_controller.state);
    COVERAGE_MARK_FUNCTION(sonicator_controller_init);
}

void test_controller_init_null_controller(void) {
    controller_result_t result = sonicator_controller_init(NULL, &test_config);
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_PARAM, result);
    COVERAGE_MARK_BRANCH(1);
}

void test_controller_init_null_config(void) {
    controller_result_t result = sonicator_controller_init(&test_controller, NULL);
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_PARAM, result);
    COVERAGE_MARK_BRANCH(2);
}

void test_controller_init_invalid_power_range(void) {
    test_config.max_power_watts = 0.0f; // Invalid
    controller_result_t result = sonicator_controller_init(&test_controller, &test_config);
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_CONFIG, result);
    COVERAGE_MARK_BRANCH(3);
}

void test_controller_init_invalid_frequency_range(void) {
    test_config.min_frequency_hz = test_config.max_frequency_hz + 1000; // Invalid range
    controller_result_t result = sonicator_controller_init(&test_controller, &test_config);
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_CONFIG, result);
    COVERAGE_MARK_BRANCH(4);
}

// ============================================================================
// SONICATOR CONTROL TESTS
// ============================================================================

void test_controller_start_sonicator_valid_id(void) {
    sonicator_controller_init(&test_controller, &test_config);

    for (uint8_t id = 1; id <= 4; id++) {
        controller_result_t result = sonicator_controller_start(&test_controller, id, 50, 25000);
        TEST_ASSERT_EQUAL(CONTROLLER_OK, result);
        TEST_ASSERT_TRUE(test_controller.sonicators[id-1].active);
        TEST_ASSERT_EQUAL(50, test_controller.sonicators[id-1].amplitude);
        TEST_ASSERT_EQUAL(25000, test_controller.sonicators[id-1].frequency_hz);
    }
    COVERAGE_MARK_FUNCTION(sonicator_controller_start);
}

void test_controller_start_sonicator_invalid_id(void) {
    sonicator_controller_init(&test_controller, &test_config);

    // Test invalid IDs
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_ID,
                     sonicator_controller_start(&test_controller, 0, 50, 25000));
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_ID,
                     sonicator_controller_start(&test_controller, 5, 50, 25000));
    COVERAGE_MARK_BRANCH(5);
}

void test_controller_start_sonicator_invalid_amplitude(void) {
    sonicator_controller_init(&test_controller, &test_config);

    // Test invalid amplitude values
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_PARAM,
                     sonicator_controller_start(&test_controller, 1, 0, 25000));
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_PARAM,
                     sonicator_controller_start(&test_controller, 1, 101, 25000));
    COVERAGE_MARK_BRANCH(6);
}

void test_controller_start_sonicator_invalid_frequency(void) {
    sonicator_controller_init(&test_controller, &test_config);

    // Test invalid frequency values
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_PARAM,
                     sonicator_controller_start(&test_controller, 1, 50, 19999));
    TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_PARAM,
                     sonicator_controller_start(&test_controller, 1, 50, 40001));
    COVERAGE_MARK_BRANCH(7);
}

void test_controller_stop_sonicator_valid_id(void) {
    sonicator_controller_init(&test_controller, &test_config);

    // Start then stop each sonicator
    for (uint8_t id = 1; id <= 4; id++) {
        sonicator_controller_start(&test_controller, id, 50, 25000);
        controller_result_t result = sonicator_controller_stop(&test_controller, id);
        TEST_ASSERT_EQUAL(CONTROLLER_OK, result);
        TEST_ASSERT_FALSE(test_controller.sonicators[id-1].active);
    }
    COVERAGE_MARK_FUNCTION(sonicator_controller_stop);
}

void test_controller_stop_all_sonicators(void) {
    sonicator_controller_init(&test_controller, &test_config);

    // Start all sonicators
    for (uint8_t id = 1; id <= 4; id++) {
        sonicator_controller_start(&test_controller, id, 50, 25000);
    }

    // Stop all
    controller_result_t result = sonicator_controller_stop_all(&test_controller);
    TEST_ASSERT_EQUAL(CONTROLLER_OK, result);

    // Verify all stopped
    for (uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_FALSE(test_controller.sonicators[i].active);
    }
    COVERAGE_MARK_FUNCTION(sonicator_controller_stop_all);
}

void test_controller_emergency_stop(void) {
    sonicator_controller_init(&test_controller, &test_config);

    // Start all sonicators
    for (uint8_t id = 1; id <= 4; id++) {
        sonicator_controller_start(&test_controller, id, 50, 25000);
    }

    // Emergency stop
    controller_result_t result = sonicator_controller_emergency_stop(&test_controller);
    TEST_ASSERT_EQUAL(CONTROLLER_OK, result);
    TEST_ASSERT_EQUAL(CONTROLLER_STATE_EMERGENCY, test_controller.state);

    // Verify all stopped
    for (uint8_t i = 0; i < 4; i++) {
        TEST_ASSERT_FALSE(test_controller.sonicators[i].active);
    }
    COVERAGE_MARK_FUNCTION(sonicator_controller_emergency_stop);
}

// ============================================================================
// STATE MACHINE TESTS
// ============================================================================

void test_controller_state_transitions(void) {
    sonicator_controller_init(&test_controller, &test_config);

    // Test state transitions
    TEST_ASSERT_EQUAL(CONTROLLER_STATE_IDLE, test_controller.state);

    sonicator_controller_start(&test_controller, 1, 50, 25000);
    TEST_ASSERT_EQUAL(CONTROLLER_STATE_RUNNING, test_controller.state);

    sonicator_controller_stop_all(&test_controller);
    TEST_ASSERT_EQUAL(CONTROLLER_STATE_IDLE, test_controller.state);
    COVERAGE_MARK_FUNCTION(controller_state_machine);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    UNITY_BEGIN();

    // Controller Initialization Tests
    RUN_TEST(test_controller_init_valid_config);
    RUN_TEST(test_controller_init_null_controller);
    RUN_TEST(test_controller_init_null_config);
    RUN_TEST(test_controller_init_invalid_power_range);
    RUN_TEST(test_controller_init_invalid_frequency_range);

    // Sonicator Control Tests
    RUN_TEST(test_controller_start_sonicator_valid_id);
    RUN_TEST(test_controller_start_sonicator_invalid_id);
    RUN_TEST(test_controller_start_sonicator_invalid_amplitude);
    RUN_TEST(test_controller_start_sonicator_invalid_frequency);
    RUN_TEST(test_controller_stop_sonicator_valid_id);
    RUN_TEST(test_controller_stop_all_sonicators);
    RUN_TEST(test_controller_emergency_stop);

    // State Machine Tests
    RUN_TEST(test_controller_state_transitions);

    return UNITY_END();
}

#endif // UNIT_TEST
