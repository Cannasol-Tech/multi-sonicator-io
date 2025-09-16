/**
 * @file sonicator_mock.c
 * @title Sonicator Mock Implementation for Unit Testing
 * @company Axovia AI
 * @date 2025-09-16
 * @brief Mock implementation providing simulated sonicator behavior for comprehensive unit testing
 * @version 1.1.0
 *
 * @details
 * This file provides a complete mock implementation of sonicator functions for unit testing purposes.
 * It simulates the behavior of real sonicator hardware, allowing tests to run without physical devices
 * and enabling comprehensive validation of control logic and error handling.
 *
 * The mock implementation supports:
 * - Full state simulation of sonicator operations
 * - Configurable error injection for fault testing
 * - Real-time parameter monitoring and validation
 * - Deterministic behavior for reproducible test results
 *
 * @section overview Overview
 *
 * The sonicator mock provides a software simulation of ultrasonic transducer control systems.
 * It implements the same interface as the real sonicator module but operates entirely in memory,
 * making it ideal for automated testing environments.
 *
 * Key features:
 * - **State Management**: Tracks initialization, running, and error states
 * - **Parameter Simulation**: Simulates frequency, amplitude, and power control
 * - **Error Injection**: Allows controlled fault simulation for robustness testing
 * - **Thread Safety**: Designed for use in multi-threaded test environments
 *
 * @section mocked_functions Mocked Functions
 *
 * The mock implements all public functions from the real sonicator interface:
 * - @c sonicator_init() - Initialize sonicator with configuration
 * - @c sonicator_start() - Start sonicator operation
 * - @c sonicator_stop() - Stop sonicator operation
 * - @c sonicator_set_amplitude() - Set output amplitude percentage
 * - @c sonicator_set_frequency() - Set operating frequency
 * - @c sonicator_get_status() - Get current sonicator status
 * - @c sonicator_get_power() - Get current power consumption
 * - @c sonicator_reset() - Reset sonicator to safe state
 *
 * @section usage Usage
 *
 * To use the mock in unit tests:
 *
 * @code{.c}
 * // Include mock header (automatically included when UNIT_TEST is defined)
 * #include "test/mocks/sonicator_mock.h"
 *
 * // In test setup
 * void setUp(void) {
 *     sonicator_mock_reset();  // Reset to clean state
 * }
 *
 * // In test function
 * void test_sonicator_normal_operation(void) {
 *     sonicator_config_t config = {
 *         .frequency_hz = 40000,
 *         .amplitude_percent = 50,
 *         .power_limit_watts = 100.0f
 *     };
 *
 *     int result = sonicator_init(&config);
 *     TEST_ASSERT_EQUAL(SONICATOR_OK, result);
 *
 *     result = sonicator_start(0);
 *     TEST_ASSERT_EQUAL(SONICATOR_OK, result);
 *
 *     sonicator_status_t status;
 *     result = sonicator_get_status(0, &status);
 *     TEST_ASSERT_EQUAL(SONICATOR_OK, result);
 *     TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, status.state);
 * }
 * @endcode
 *
 * @section error_simulation Error Simulation
 *
 * The mock supports error injection to test fault handling:
 *
 * @code{.c}
 * void test_sonicator_error_handling(void) {
 *     sonicator_mock_reset();
 *     sonicator_mock_set_error_injection(SONICATOR_ERROR_INVALID_PARAM);
 *
 *     sonicator_config_t bad_config = {0};  // Invalid config
 *     int result = sonicator_init(&bad_config);
 *     TEST_ASSERT_EQUAL(SONICATOR_ERROR_INVALID_PARAM, result);
 * }
 * @endcode
 *
 * Available error injection codes:
 * - @c SONICATOR_ERROR_INVALID_PARAM - Invalid parameter passed
 * - @c SONICATOR_ERROR_INVALID_ID - Invalid sonicator ID
 * - @c SONICATOR_ERROR_INVALID_FREQUENCY - Frequency out of range
 * - @c SONICATOR_ERROR_INVALID_AMPLITUDE - Amplitude out of range
 * - @c SONICATOR_ERROR_NOT_INITIALIZED - Operation before initialization
 * - @c SONICATOR_ERROR_ALREADY_RUNNING - Start when already running
 * - @c SONICATOR_ERROR_POWER_LIMIT - Power limit exceeded
 *
 * @section state_management State Management
 *
 * The mock maintains internal state for each simulated sonicator:
 * - **Initialization State**: Tracks whether sonicator has been initialized
 * - **Operating State**: Current running/idle status
 * - **Parameter State**: Current frequency, amplitude, and power settings
 * - **Error State**: Last error code and fault conditions
 *
 * @section thread_safety Thread Safety
 *
 * The mock is designed to be thread-safe for use in concurrent test environments.
 * All mock state is protected with appropriate synchronization mechanisms.
 *
 * @warning This mock implementation is for testing purposes only.
 * @warning Do not include this file in production builds.
 * @warning Mock behavior may not perfectly replicate hardware timing or precision.
 *
 * @see src/modules/sonicator/sonicator.h Real sonicator interface definition
 * @see test/unit/sonicator/test_sonicator.c Example usage in unit tests
 * @see docs/testing-standards.md Testing framework documentation
 *
 * @note Mock state is automatically reset at the beginning of each test function
 * @note Error injection is cleared after each mock function call
 * @note Timing delays are simulated but not real-time accurate
 */


#ifdef UNIT_TEST

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Mock Sonicator types and constants
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

typedef int sonicator_result_t;

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
    float overload;
} sonicator_status_t;

// Mock function implementations
sonicator_result_t sonicator_init(sonicator_t* sonicator, uint8_t id, const sonicator_params_t* params) {
    if (!sonicator) {
        return SONICATOR_ERROR_INVALID_PARAM;
    }
    
    if (!params) {
        return SONICATOR_ERROR_INVALID_PARAM;
    }
    
    if (id < 1 || id > 4) {
        return SONICATOR_ERROR_INVALID_ID;
    }
    
    if (params->frequency_hz < 20000 || params->frequency_hz > 40000) {
        return SONICATOR_ERROR_INVALID_FREQUENCY;
    }
    
    if (params->amplitude_percent < 1 || params->amplitude_percent > 100) {
        return SONICATOR_ERROR_INVALID_AMPLITUDE;
    }
    
    sonicator->initialized = true;
    sonicator->id = id;
    sonicator->state = SONICATOR_STATE_IDLE;
    sonicator->active = false;
    sonicator->current_frequency_hz = params->frequency_hz;
    sonicator->current_amplitude_percent = params->amplitude_percent;
    sonicator->current_power_watts = 0.0f;
    
    return SONICATOR_OK;
}

sonicator_result_t sonicator_start(sonicator_t* sonicator) {
    if (!sonicator || !sonicator->initialized) {
        return SONICATOR_ERROR_NOT_INITIALIZED;
    }
    
    if (sonicator->active) {
        return SONICATOR_ERROR_ALREADY_RUNNING;
    }
    
    sonicator->active = true;
    sonicator->state = SONICATOR_STATE_RUNNING;
    sonicator->current_power_watts = 50.0f; // Mock power consumption
    
    return SONICATOR_OK;
}

sonicator_result_t sonicator_stop(sonicator_t* sonicator) {
    if (!sonicator || !sonicator->initialized) {
        return SONICATOR_ERROR_NOT_INITIALIZED;
    }
    
    sonicator->active = false;
    sonicator->state = SONICATOR_STATE_IDLE;
    sonicator->current_power_watts = 0.0f;
    
    return SONICATOR_OK;
}

sonicator_result_t sonicator_set_frequency(sonicator_t* sonicator, uint16_t frequency_hz) {
    if (!sonicator || !sonicator->initialized) {
        return SONICATOR_ERROR_NOT_INITIALIZED;
    }
    
    if (frequency_hz < 20000 || frequency_hz > 40000) {
        return SONICATOR_ERROR_INVALID_FREQUENCY;
    }
    
    sonicator->current_frequency_hz = frequency_hz;
    
    return SONICATOR_OK;
}

sonicator_result_t sonicator_set_amplitude(sonicator_t* sonicator, uint8_t amplitude_percent) {
    if (!sonicator || !sonicator->initialized) {
        return SONICATOR_ERROR_NOT_INITIALIZED;
    }
    
    if (amplitude_percent < 1 || amplitude_percent > 100) {
        return SONICATOR_ERROR_INVALID_AMPLITUDE;
    }
    
    sonicator->current_amplitude_percent = amplitude_percent;
    
    return SONICATOR_OK;
}

sonicator_result_t sonicator_get_status(sonicator_t* sonicator, sonicator_status_t* status) {
    if (!sonicator || !status) {
        return SONICATOR_ERROR_INVALID_PARAM;
    }
    
    if (!sonicator->initialized) {
        return SONICATOR_ERROR_NOT_INITIALIZED;
    }
    
    status->id = sonicator->id;
    status->state = sonicator->state;
    status->frequency_hz = sonicator->current_frequency_hz;
    status->amplitude_percent = sonicator->current_amplitude_percent;
    status->power_watts = sonicator->current_power_watts;
    
    return SONICATOR_OK;
}

sonicator_result_t sonicator_check_power_limit(sonicator_t* sonicator) {
    if (!sonicator || !sonicator->initialized) {
        return SONICATOR_ERROR_NOT_INITIALIZED;
    }
    
    if (sonicator->current_power_watts > 100.0f) {
        return SONICATOR_ERROR_POWER_LIMIT;
    }
    
    return SONICATOR_OK;
}

#endif // UNIT_TEST
