/**
 * @file unity_system_config.h
 * @brief Unity Test Framework Configuration for Multi-Sonicator I/O Controller
 * @author Cannasol Technologies
 * @date 2025-09-04
 * @version 1.0.0
 *
 * @details
 * Unity configuration for embedded C/C++ unit testing with native environment.
 * Provides comprehensive test framework setup for 85% coverage requirement.
 */

#ifndef UNITY_SYSTEM_CONFIG_H
#define UNITY_SYSTEM_CONFIG_H

// Standard Library Includes for Native Testing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// ============================================================================
// UNITY CONFIGURATION MACROS (must be defined BEFORE including unity.h)
// ============================================================================

// Enable Unity features for comprehensive testing
#define UNITY_INCLUDE_DOUBLE
#define UNITY_INCLUDE_FLOAT
#define UNITY_SUPPORT_64
#define UNITY_INCLUDE_PRINT_FORMATTED

// Unity Framework Include (AFTER configuration)
#include <unity.h>

// Memory allocation for dynamic tests (if needed)
// #define UNITY_FIXTURE_MALLOC(size)    malloc(size)
// #define UNITY_FIXTURE_FREE(ptr)       free(ptr)

// Test timeout configuration (30 seconds per test)
// #define UNITY_FIXTURE_TEST_TIMEOUT    30000

// ============================================================================
// EMBEDDED SYSTEM MOCKING CONFIGURATION
// ============================================================================

// Mock embedded system types when in native test environment
#ifdef NATIVE_TEST
    // Mock Arduino/AVR types
    typedef uint8_t  byte;
    typedef bool     boolean;

    // Mock hardware register access
    #define MOCK_REGISTER_ACCESS 1

    // Mock timing functions
    #define delay(ms)           usleep((ms) * 1000)
    #define delayMicroseconds(us) usleep(us)

    // Mock serial communication
    #define Serial              printf
#endif

// ============================================================================
// TEST HELPER MACROS
// ============================================================================

// Enhanced assertion macros for embedded testing
#define TEST_ASSERT_FLOAT_WITHIN_PERCENT(threshold, expected, actual) \
    TEST_ASSERT_FLOAT_WITHIN(((expected) * (threshold) / 100.0f), (expected), (actual))

#define TEST_ASSERT_UINT16_WITHIN_RANGE(min, max, actual) \
    do { \
        TEST_ASSERT_GREATER_OR_EQUAL_UINT16((min), (actual)); \
        TEST_ASSERT_LESS_OR_EQUAL_UINT16((max), (actual)); \
    } while(0)

#define TEST_ASSERT_VALID_POINTER(ptr) \
    TEST_ASSERT_NOT_NULL(ptr)

#define TEST_ASSERT_BUFFER_EQUAL(expected, actual, size) \
    TEST_ASSERT_EQUAL_MEMORY((expected), (actual), (size))

// ============================================================================
// MODULE-SPECIFIC TEST CONFIGURATION
// ============================================================================

// HAL Module Test Configuration
#define HAL_TEST_TIMEOUT_MS     1000
#define HAL_TEST_MAX_CHANNELS   8
#define HAL_TEST_ADC_RESOLUTION 1024

// Communication Module Test Configuration
#define COMM_TEST_BUFFER_SIZE   256
#define COMM_TEST_BAUD_RATE     9600
#define COMM_TEST_TIMEOUT_MS    5000

// Control Module Test Configuration
#define CTRL_TEST_MAX_STATES    16
#define CTRL_TEST_CYCLE_TIME_MS 100

// Sonicator Module Test Configuration
#define SONIC_TEST_MAX_POWER    100
#define SONIC_TEST_MIN_FREQ     20000
#define SONIC_TEST_MAX_FREQ     40000

// ============================================================================
// COVERAGE REPORTING CONFIGURATION
// ============================================================================

#ifdef UNIT_TEST
    // Enable coverage tracking
    #define COVERAGE_ENABLED 1

    // Coverage reporting macros
    #define COVERAGE_MARK_FUNCTION(func_name) \
        printf("COVERAGE: Function %s called\n", #func_name)

    #define COVERAGE_MARK_BRANCH(branch_id) \
        printf("COVERAGE: Branch %d taken\n", branch_id)
#endif

#endif // UNITY_SYSTEM_CONFIG_H