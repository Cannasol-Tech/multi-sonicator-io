/**
 * @file control_mock.c
 * @brief Mock implementation of Control functions for unit testing
 * @author Cannasol Technologies
 * @date 2025-09-04
 * @version 1.0.0
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
