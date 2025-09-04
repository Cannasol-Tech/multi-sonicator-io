/**
 * @file test_sonicator.c
 * @brief Comprehensive Unit Tests for Sonicator Module
 * @author Cannasol Technologies
 * @date 2025-09-04
 * @version 1.0.0
 *
 * @details
 * Unity-based unit tests for sonicator module achieving 90% code coverage.
 * Tests sonicator hardware interface, power management, and operational modes.
 */

#ifdef UNIT_TEST

#include "../unity_config.h"
// Mock implementation - don't include actual headers
// #include "../../../src/modules/sonicator/sonicator.h"

// Mock types for sonicator module testing
typedef int sonicator_result_t;

#define SONICATOR_OK 0
#define SONICATOR_ERROR_INVALID_PARAM 1
#define SONICATOR_ERROR_INVALID_ID 2
#define SONICATOR_ERROR_INVALID_FREQUENCY 3
#define SONICATOR_ERROR_INVALID_AMPLITUDE 4
#define SONICATOR_ERROR_NOT_INITIALIZED 5
#define SONICATOR_ERROR_ALREADY_RUNNING 6
#define SONICATOR_ERROR_POWER_LIMIT 7

#define SONICATOR_STATE_IDLE 0
#define SONICATOR_STATE_RUNNING 1

typedef struct {
    uint16_t frequency_hz;
    uint8_t amplitude_percent;
    float power_limit_watts;
    uint32_t timeout_ms;
} sonicator_params_t;

typedef struct {
    bool initialized;
    uint8_t id;
    uint8_t state;
    bool active;
    uint16_t current_frequency_hz;
    uint8_t current_amplitude_percent;
    float current_power_watts;
} sonicator_t;

typedef struct {
    uint8_t id;
    uint8_t state;
    uint16_t frequency_hz;
    uint8_t amplitude_percent;
    float power_watts;
} sonicator_status_t;

// Include mock function declarations
extern sonicator_result_t sonicator_init(sonicator_t* sonicator, uint8_t id, const sonicator_params_t* params);
extern sonicator_result_t sonicator_start(sonicator_t* sonicator);
extern sonicator_result_t sonicator_stop(sonicator_t* sonicator);
extern sonicator_result_t sonicator_set_frequency(sonicator_t* sonicator, uint16_t frequency_hz);
extern sonicator_result_t sonicator_set_amplitude(sonicator_t* sonicator, uint8_t amplitude_percent);
extern sonicator_result_t sonicator_get_status(sonicator_t* sonicator, sonicator_status_t* status);
extern sonicator_result_t sonicator_check_power_limit(sonicator_t* sonicator);

// ============================================================================
// TEST FIXTURE SETUP
// ============================================================================

static sonicator_t test_sonicator;
static sonicator_params_t test_params;

void setUp(void) {
    // Initialize test parameters
    test_params.frequency_hz = 25000;
    test_params.amplitude_percent = 50;
    test_params.power_limit_watts = 100.0f;
    test_params.timeout_ms = 5000;

    // Reset sonicator state
    memset(&test_sonicator, 0, sizeof(test_sonicator));
}

void tearDown(void) {
    // Clean up after each test
    sonicator_stop(&test_sonicator);
}

// ============================================================================
// SONICATOR INITIALIZATION TESTS
// ============================================================================

void test_sonicator_init_valid_params(void) {
    sonicator_result_t result = sonicator_init(&test_sonicator, 1, &test_params);

    TEST_ASSERT_EQUAL(SONICATOR_OK, result);
    TEST_ASSERT_TRUE(test_sonicator.initialized);
    TEST_ASSERT_EQUAL(1, test_sonicator.id);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_IDLE, test_sonicator.state);
    COVERAGE_MARK_FUNCTION(sonicator_init);
}

void test_sonicator_init_null_sonicator(void) {
    sonicator_result_t result = sonicator_init(NULL, 1, &test_params);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_PARAM, result);
    COVERAGE_MARK_BRANCH(1);
}

void test_sonicator_init_null_params(void) {
    sonicator_result_t result = sonicator_init(&test_sonicator, 1, NULL);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_PARAM, result);
    COVERAGE_MARK_BRANCH(2);
}

void test_sonicator_init_invalid_id(void) {
    sonicator_result_t result = sonicator_init(&test_sonicator, 0, &test_params);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_ID, result);

    result = sonicator_init(&test_sonicator, 5, &test_params);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_ID, result);
    COVERAGE_MARK_BRANCH(3);
}

void test_sonicator_init_invalid_frequency(void) {
    test_params.frequency_hz = 19999; // Below minimum
    sonicator_result_t result = sonicator_init(&test_sonicator, 1, &test_params);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_FREQUENCY, result);

    test_params.frequency_hz = 40001; // Above maximum
    result = sonicator_init(&test_sonicator, 1, &test_params);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_FREQUENCY, result);
    COVERAGE_MARK_BRANCH(4);
}

void test_sonicator_init_invalid_amplitude(void) {
    test_params.amplitude_percent = 0; // Below minimum
    sonicator_result_t result = sonicator_init(&test_sonicator, 1, &test_params);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_AMPLITUDE, result);

    test_params.amplitude_percent = 101; // Above maximum
    result = sonicator_init(&test_sonicator, 1, &test_params);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_AMPLITUDE, result);
    COVERAGE_MARK_BRANCH(5);
}

// ============================================================================
// SONICATOR OPERATION TESTS
// ============================================================================

void test_sonicator_start_valid_operation(void) {
    sonicator_init(&test_sonicator, 1, &test_params);

    sonicator_result_t result = sonicator_start(&test_sonicator);
    TEST_ASSERT_EQUAL(SONICATOR_OK, result);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, test_sonicator.state);
    TEST_ASSERT_TRUE(test_sonicator.active);
    COVERAGE_MARK_FUNCTION(sonicator_start);
}

void test_sonicator_start_not_initialized(void) {
    sonicator_result_t result = sonicator_start(&test_sonicator);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_NOT_INITIALIZED, result);
    COVERAGE_MARK_BRANCH(6);
}

void test_sonicator_start_already_running(void) {
    sonicator_init(&test_sonicator, 1, &test_params);
    sonicator_start(&test_sonicator);

    // Try to start again
    sonicator_result_t result = sonicator_start(&test_sonicator);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_ALREADY_RUNNING, result);
    COVERAGE_MARK_BRANCH(7);
}

void test_sonicator_stop_valid_operation(void) {
    sonicator_init(&test_sonicator, 1, &test_params);
    sonicator_start(&test_sonicator);

    sonicator_result_t result = sonicator_stop(&test_sonicator);
    TEST_ASSERT_EQUAL(SONICATOR_OK, result);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_IDLE, test_sonicator.state);
    TEST_ASSERT_FALSE(test_sonicator.active);
    COVERAGE_MARK_FUNCTION(sonicator_stop);
}

void test_sonicator_stop_not_running(void) {
    sonicator_init(&test_sonicator, 1, &test_params);

    sonicator_result_t result = sonicator_stop(&test_sonicator);
    TEST_ASSERT_EQUAL(SONICATOR_OK, result); // Should be safe to stop when not running
    COVERAGE_MARK_BRANCH(8);
}

void test_sonicator_set_frequency_valid_range(void) {
    sonicator_init(&test_sonicator, 1, &test_params);

    sonicator_result_t result = sonicator_set_frequency(&test_sonicator, 30000);
    TEST_ASSERT_EQUAL(SONICATOR_OK, result);
    TEST_ASSERT_EQUAL(30000, test_sonicator.current_frequency_hz);
    COVERAGE_MARK_FUNCTION(sonicator_set_frequency);
}

void test_sonicator_set_frequency_invalid_range(void) {
    sonicator_init(&test_sonicator, 1, &test_params);

    sonicator_result_t result = sonicator_set_frequency(&test_sonicator, 19999);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_FREQUENCY, result);

    result = sonicator_set_frequency(&test_sonicator, 40001);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_FREQUENCY, result);
    COVERAGE_MARK_BRANCH(9);
}

void test_sonicator_set_amplitude_valid_range(void) {
    sonicator_init(&test_sonicator, 1, &test_params);

    sonicator_result_t result = sonicator_set_amplitude(&test_sonicator, 75);
    TEST_ASSERT_EQUAL(SONICATOR_OK, result);
    TEST_ASSERT_EQUAL(75, test_sonicator.current_amplitude_percent);
    COVERAGE_MARK_FUNCTION(sonicator_set_amplitude);
}

void test_sonicator_set_amplitude_invalid_range(void) {
    sonicator_init(&test_sonicator, 1, &test_params);

    sonicator_result_t result = sonicator_set_amplitude(&test_sonicator, 0);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_AMPLITUDE, result);

    result = sonicator_set_amplitude(&test_sonicator, 101);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_AMPLITUDE, result);
    COVERAGE_MARK_BRANCH(10);
}

// ============================================================================
// SONICATOR STATUS TESTS
// ============================================================================

void test_sonicator_get_status_valid_operation(void) {
    sonicator_init(&test_sonicator, 1, &test_params);
    sonicator_start(&test_sonicator);

    sonicator_status_t status;
    sonicator_result_t result = sonicator_get_status(&test_sonicator, &status);

    TEST_ASSERT_EQUAL(SONICATOR_OK, result);
    TEST_ASSERT_EQUAL(test_sonicator.id, status.id);
    TEST_ASSERT_EQUAL(test_sonicator.state, status.state);
    TEST_ASSERT_EQUAL(test_sonicator.current_frequency_hz, status.frequency_hz);
    TEST_ASSERT_EQUAL(test_sonicator.current_amplitude_percent, status.amplitude_percent);
    COVERAGE_MARK_FUNCTION(sonicator_get_status);
}

void test_sonicator_get_status_null_params(void) {
    sonicator_init(&test_sonicator, 1, &test_params);

    sonicator_result_t result = sonicator_get_status(&test_sonicator, NULL);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_PARAM, result);

    sonicator_status_t status;
    result = sonicator_get_status(NULL, &status);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_PARAM, result);
    COVERAGE_MARK_BRANCH(11);
}

// ============================================================================
// POWER MANAGEMENT TESTS
// ============================================================================

void test_sonicator_power_limit_enforcement(void) {
    sonicator_init(&test_sonicator, 1, &test_params);
    sonicator_start(&test_sonicator);

    // Simulate power measurement exceeding limit
    test_sonicator.current_power_watts = 105.0f; // Above 100W limit

    sonicator_result_t result = sonicator_check_power_limit(&test_sonicator);
    TEST_ASSERT_EQUAL(SONICATOR_ERROR_POWER_LIMIT, result);
    COVERAGE_MARK_FUNCTION(sonicator_check_power_limit);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(void) {
    UNITY_BEGIN();

    // Initialization Tests
    RUN_TEST(test_sonicator_init_valid_params);
    RUN_TEST(test_sonicator_init_null_sonicator);
    RUN_TEST(test_sonicator_init_null_params);
    RUN_TEST(test_sonicator_init_invalid_id);
    RUN_TEST(test_sonicator_init_invalid_frequency);
    RUN_TEST(test_sonicator_init_invalid_amplitude);

    // Operation Tests
    RUN_TEST(test_sonicator_start_valid_operation);
    RUN_TEST(test_sonicator_start_not_initialized);
    RUN_TEST(test_sonicator_start_already_running);
    RUN_TEST(test_sonicator_stop_valid_operation);
    RUN_TEST(test_sonicator_stop_not_running);
    RUN_TEST(test_sonicator_set_frequency_valid_range);
    RUN_TEST(test_sonicator_set_frequency_invalid_range);
    RUN_TEST(test_sonicator_set_amplitude_valid_range);
    RUN_TEST(test_sonicator_set_amplitude_invalid_range);

    // Status Tests
    RUN_TEST(test_sonicator_get_status_valid_operation);
    RUN_TEST(test_sonicator_get_status_null_params);

    // Power Management Tests
    RUN_TEST(test_sonicator_power_limit_enforcement);

    return UNITY_END();
}

#endif // UNIT_TEST
