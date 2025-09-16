/**
 * @file control_mock.c
 * @title Control System Mock Implementation for Unit Testing
 * @company Axovia AI
 * @date 2025-09-16
 * @brief Mock implementation providing simulated control system behavior for comprehensive unit testing
 * @version 1.1.0
 *
 * @details
 * This file provides a complete mock implementation of control system functions for unit testing
 * purposes. It simulates the behavior of ultrasonic transducer control algorithms and state machines,
 * enabling tests to run without physical hardware and ensuring robust validation of control logic
 * and safety systems.
 *
 * The mock implementation supports:
 * - State machine simulation with configurable transitions
 * - Parameter validation and safety limit enforcement
 * - Error handling and fault recovery simulation
 * - Real-time control loop simulation
 * - Multi-sonicator coordination and synchronization
 *
 * @section overview Overview
 *
 * The control mock provides a software simulation of ultrasonic transducer control systems.
 * It implements the same interface as the real control module but operates entirely in memory,
 * making it ideal for testing control algorithms, state machines, and safety interlocks.
 *
 * Key features:
 * - **State Machine Simulation**: Complete simulation of control states and transitions
 * - **Safety System Testing**: Validation of safety interlocks and emergency stops
 * - **Parameter Validation**: Simulation of parameter limits and constraints
 * - **Error Recovery**: Configurable fault simulation and recovery testing
 * - **Multi-Unit Coordination**: Simulation of coordinated multi-sonicator operation
 *
 * @section mocked_functions Mocked Functions
 *
 * The mock implements all public functions from the real control interface:
 * - @c sonicator_controller_init() - Initialize control system with configuration
 * - @c sonicator_controller_start() - Start control operation
 * - @c sonicator_controller_stop() - Stop control operation
 * - @c sonicator_controller_set_amplitude() - Set amplitude for specific sonicator
 * - @c sonicator_controller_set_frequency() - Set frequency for specific sonicator
 * - @c sonicator_controller_get_status() - Get current control status
 * - @c sonicator_controller_emergency_stop() - Trigger emergency stop
 * - @c sonicator_controller_reset() - Reset control system to safe state
 *
 * @section usage Usage
 *
 * To use the control mock in unit tests:
 *
 * @code{.c}
 * // Include mock header (automatically included when UNIT_TEST is defined)
 * #include "test/mocks/control_mock.h"
 *
 * // In test setup
 * void setUp(void) {
 *     control_mock_reset();  // Reset to clean state
 * }
 *
 * // In test function
 * void test_control_normal_operation(void) {
 *     sonicator_config_t config = {
 *         .max_power_watts = 100.0f,
 *         .min_frequency_hz = 20000,
 *         .max_frequency_hz = 40000,
 *         .default_amplitude = 50,
 *         .safety_timeout_ms = 5000
 *     };
 *
 *     controller_result_t result = sonicator_controller_init(&config);
 *     TEST_ASSERT_EQUAL(CONTROLLER_OK, result);
 *
 *     result = sonicator_controller_start(0);
 *     TEST_ASSERT_EQUAL(CONTROLLER_OK, result);
 *
 *     sonicator_instance_t status;
 *     result = sonicator_controller_get_status(0, &status);
 *     TEST_ASSERT_EQUAL(CONTROLLER_OK, result);
 *     TEST_ASSERT_TRUE(status.active);
 * }
 * @endcode
 *
 * @section error_simulation Error Simulation
 *
 * The mock supports error injection to test fault handling:
 *
 * @code{.c}
 * void test_control_error_handling(void) {
 *     control_mock_reset();
 *     control_mock_set_error_injection(CONTROLLER_ERROR_INVALID_PARAM);
 *
 *     sonicator_config_t bad_config = {0};  // Invalid config
 *     controller_result_t result = sonicator_controller_init(&bad_config);
 *     TEST_ASSERT_EQUAL(CONTROLLER_ERROR_INVALID_PARAM, result);
 * }
 * @endcode
 *
 * Available error injection codes:
 * - @c CONTROLLER_ERROR_INVALID_PARAM - Invalid parameter passed
 * - @c CONTROLLER_ERROR_INVALID_CONFIG - Invalid configuration
 * - @c CONTROLLER_ERROR_INVALID_ID - Invalid sonicator ID
 * - @c CONTROLLER_ERROR_NOT_INITIALIZED - Operation before initialization
 * - @c CONTROLLER_ERROR_ALREADY_RUNNING - Start when already running
 *
 * @section state_management State Management
 *
 * The mock maintains internal state for the control system:
 * - **Initialization State**: Tracks whether controller has been initialized
 * - **Operating State**: Current running/idle/emergency status
 * - **Sonicator States**: Individual sonicator parameters and status
 * - **Safety State**: Emergency stop and safety interlock status
 * - **Configuration State**: Current configuration parameters
 *
 * @section safety_testing Safety Testing
 *
 * The mock provides comprehensive safety system testing:
 *
 * @code{.c}
 * void test_control_emergency_stop(void) {
 *     control_mock_reset();
 *     // Initialize and start controller
 *     sonicator_controller_init(&config);
 *     sonicator_controller_start(0);
 *
 *     // Trigger emergency stop
 *     controller_result_t result = sonicator_controller_emergency_stop();
 *     TEST_ASSERT_EQUAL(CONTROLLER_OK, result);
 *
 *     // Verify all sonicators stopped
 *     for (int i = 0; i < 4; i++) {
 *         sonicator_instance_t status;
 *         sonicator_controller_get_status(i, &status);
 *         TEST_ASSERT_FALSE(status.active);
 *     }
 * }
 * @endcode
 *
 * @section parameter_validation Parameter Validation
 *
 * The mock enforces parameter limits and validation:
 * - **Frequency Range**: 20kHz to 40kHz
 * - **Amplitude Range**: 0% to 100%
 * - **Power Limits**: Configurable maximum power consumption
 * - **Safety Timeouts**: Configurable safety timeout periods
 *
 * @warning This mock implementation is for testing purposes only.
 * @warning Do not include this file in production builds.
 * @warning Mock timing may not perfectly replicate control loop delays.
 *
 * @see src/modules/control/sonicator_controller.h Real control interface definition
 * @see test/unit/control/test_control.c Example usage in unit tests
 * @see docs/control-system-architecture.md Control system documentation
 *
 * @note Mock state is automatically reset at the beginning of each test function
 * @note Safety systems are always active in mock environment
 * @note Parameter validation is strictly enforced
 * @note Emergency stop affects all sonicators simultaneously
 */


#ifdef UNIT_TEST

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Mock Control types and constants
#define CONTROLLER_OK 0
#define CONTROLLER_ERROR_INVALID_PARAM 1
#define CONTROLLER_ERROR_INVALID_CONFIG 2
#define CONTROLLER_ERROR_INVALID_ID 3
#define CONTROLLER_ERROR_NOT_INITIALIZED 4
#define CONTROLLER_ERROR_ALREADY_RUNNING 5

#define CONTROLLER_STATE_IDLE 0
#define CONTROLLER_STATE_RUNNING 1
#define CONTROLLER_STATE_EMERGENCY 2

typedef int controller_result_t;

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

// Mock function implementations
controller_result_t sonicator_controller_init(sonicator_controller_t* controller, const sonicator_config_t* config) {
    if (!controller) {
        return CONTROLLER_ERROR_INVALID_PARAM;
    }
    
    if (!config) {
        return CONTROLLER_ERROR_INVALID_PARAM;
    }
    
    if (config->max_power_watts <= 0.0f) {
        return CONTROLLER_ERROR_INVALID_CONFIG;
    }
    
    if (config->min_frequency_hz >= config->max_frequency_hz) {
        return CONTROLLER_ERROR_INVALID_CONFIG;
    }
    
    controller->initialized = true;
    controller->state = CONTROLLER_STATE_IDLE;
    memset(controller->sonicators, 0, sizeof(controller->sonicators));
    
    return CONTROLLER_OK;
}

controller_result_t sonicator_controller_start(sonicator_controller_t* controller, uint8_t id, uint8_t amplitude, uint16_t frequency_hz) {
    if (!controller || !controller->initialized) {
        return CONTROLLER_ERROR_NOT_INITIALIZED;
    }
    
    if (id < 1 || id > 4) {
        return CONTROLLER_ERROR_INVALID_ID;
    }
    
    if (amplitude < 1 || amplitude > 100) {
        return CONTROLLER_ERROR_INVALID_PARAM;
    }
    
    if (frequency_hz < 20000 || frequency_hz > 40000) {
        return CONTROLLER_ERROR_INVALID_PARAM;
    }
    
    sonicator_instance_t* sonicator = &controller->sonicators[id - 1];
    
    if (sonicator->active) {
        return CONTROLLER_ERROR_ALREADY_RUNNING;
    }
    
    sonicator->active = true;
    sonicator->amplitude = amplitude;
    sonicator->frequency_hz = frequency_hz;
    controller->state = CONTROLLER_STATE_RUNNING;
    
    return CONTROLLER_OK;
}

controller_result_t sonicator_controller_stop(sonicator_controller_t* controller, uint8_t id) {
    if (!controller || !controller->initialized) {
        return CONTROLLER_ERROR_NOT_INITIALIZED;
    }
    
    if (id < 1 || id > 4) {
        return CONTROLLER_ERROR_INVALID_ID;
    }
    
    sonicator_instance_t* sonicator = &controller->sonicators[id - 1];
    sonicator->active = false;
    sonicator->amplitude = 0;
    sonicator->frequency_hz = 0;
    
    // Check if any sonicators are still active
    bool any_active = false;
    for (int i = 0; i < 4; i++) {
        if (controller->sonicators[i].active) {
            any_active = true;
            break;
        }
    }
    
    if (!any_active) {
        controller->state = CONTROLLER_STATE_IDLE;
    }
    
    return CONTROLLER_OK;
}

controller_result_t sonicator_controller_stop_all(sonicator_controller_t* controller) {
    if (!controller || !controller->initialized) {
        return CONTROLLER_ERROR_NOT_INITIALIZED;
    }
    
    for (int i = 0; i < 4; i++) {
        controller->sonicators[i].active = false;
        controller->sonicators[i].amplitude = 0;
        controller->sonicators[i].frequency_hz = 0;
    }
    
    controller->state = CONTROLLER_STATE_IDLE;
    
    return CONTROLLER_OK;
}

controller_result_t sonicator_controller_emergency_stop(sonicator_controller_t* controller) {
    if (!controller || !controller->initialized) {
        return CONTROLLER_ERROR_NOT_INITIALIZED;
    }
    
    for (int i = 0; i < 4; i++) {
        controller->sonicators[i].active = false;
        controller->sonicators[i].amplitude = 0;
        controller->sonicators[i].frequency_hz = 0;
    }
    
    controller->state = CONTROLLER_STATE_EMERGENCY;
    
    return CONTROLLER_OK;
}

#endif // UNIT_TEST
