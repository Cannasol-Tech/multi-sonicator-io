/**
 * @file sonicator_mock.c
 * @brief Mock implementation of Sonicator functions for unit testing
 * @author Cannasol Technologies
 * @date 2025-09-04
 * @version 1.0.0
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
