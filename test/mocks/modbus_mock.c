/**
 * @file modbus_mock.c
 * @brief Mock implementation of MODBUS functions for unit testing
 * @author Cannasol Technologies
 * @date 2025-09-04
 * @version 1.0.0
 */

#ifdef UNIT_TEST

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Mock MODBUS types and constants
#define MODBUS_OK 0
#define MODBUS_ERROR_INVALID_LENGTH 12
#define MODBUS_STATE_IDLE 0
#define MODBUS_STATE_ERROR 1
#define MODBUS_FC_READ_HOLDING 0x03
#define MODBUS_FC_WRITE_SINGLE 0x06
#define MODBUS_SLAVE_ID 2
#define MODBUS_BAUD_RATE 115200
#define MODBUS_TIMEOUT_MS 1000

typedef int modbus_error_t;
typedef int modbus_state_t;

typedef struct {
    uint8_t slave_id;
    uint32_t baud_rate;
    uint32_t timeout_ms;
    void* read_callback;
    void* write_callback;
    void* timeout_callback;
    void* error_callback;
} modbus_config_t;

typedef struct {
    uint32_t requests_received;
    uint32_t responses_sent;
    uint32_t crc_errors;
    uint32_t timeout_errors;
} modbus_statistics_t;

// Mock state variables
static bool mock_enabled = false;
static modbus_state_t mock_state = MODBUS_STATE_IDLE;
static modbus_statistics_t mock_stats = {0};

// Mock function implementations
modbus_error_t modbus_init(const modbus_config_t* config) {
    if (config == NULL) {
        return MODBUS_ERROR_INVALID_LENGTH;
    }
    if (config->slave_id == 0) {
        return MODBUS_ERROR_INVALID_LENGTH;
    }
    mock_enabled = true;
    mock_state = MODBUS_STATE_IDLE;
    return MODBUS_OK;
}

bool modbus_is_enabled(void) {
    return mock_enabled;
}

modbus_state_t modbus_get_state(void) {
    return mock_state;
}

void modbus_set_enabled(bool enabled) {
    mock_enabled = enabled;
    if (!enabled) {
        mock_state = MODBUS_STATE_ERROR;
    } else {
        mock_state = MODBUS_STATE_IDLE;
    }
}

bool modbus_validate_address(uint16_t address, uint8_t function_code) {
    // System status registers (read-only)
    if (address >= 0x0000 && address <= 0x000F) {
        return (function_code == MODBUS_FC_READ_HOLDING);
    }
    // Global control registers (read/write)
    if (address >= 0x0010 && address <= 0x001F) {
        return true;
    }
    // Per-sonicator registers
    if (address >= 0x0100 && address <= 0x041F) {
        return true;
    }
    return false;
}

uint16_t modbus_calculate_crc(const uint8_t* data, uint16_t length) {
    if (data == NULL || length == 0) {
        return 0xFFFF;
    }
    
    // Simple mock CRC calculation
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

const modbus_statistics_t* modbus_get_statistics(void) {
    return &mock_stats;
}

void modbus_reset_statistics(void) {
    memset(&mock_stats, 0, sizeof(mock_stats));
}

// Mock register manager functions
bool register_manager_init(void) {
    return true;
}

void* register_manager_get_map(void) {
    static int dummy_map = 0;
    return &dummy_map;
}

void register_manager_update_system_status(uint16_t status, bool set) {
    (void)status;
    (void)set;
}

bool register_manager_is_global_enabled(void) {
    return true;
}

bool register_manager_is_emergency_stop_active(void) {
    return false;
}

void register_manager_clear_emergency_stop(void) {
    // Mock implementation
}

bool register_manager_validate_value(uint16_t address, uint16_t value) {
    // Mock validation logic
    if (address == 0x0101) { // Amplitude register
        return (value >= 20 && value <= 100);
    }
    if (address == 0x0100) { // Start/stop register
        return (value <= 1);
    }
    return true;
}

#endif // UNIT_TEST
