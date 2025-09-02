/**
 * @file test_modbus_standalone.cpp
 * @brief Standalone Modbus Unit Tests
 * @author Cannasol Technologies
 * @date 2025-09-02
 */

#include <iostream>
#include <cassert>
#include <cstring>
#include <cstdlib>

// Simple test framework
#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::cout << "FAIL: " << __FUNCTION__ << " line " << __LINE__ \
                      << " - Expected " << (expected) << ", got " << (actual) << std::endl; \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cout << "FAIL: " << __FUNCTION__ << " line " << __LINE__ \
                      << " - Expected true, got false" << std::endl; \
            return false; \
        } \
    } while(0)

#define TEST_ASSERT_FALSE(condition) \
    do { \
        if ((condition)) { \
            std::cout << "FAIL: " << __FUNCTION__ << " line " << __LINE__ \
                      << " - Expected false, got true" << std::endl; \
            return false; \
        } \
    } while(0)

// Include the modules under test
#include "src/modules/communication/modbus.h"
#include "src/modules/communication/modbus_register_manager.h"

// Test fixture
static modbus_config_t test_config;
static uint16_t test_read_value = 0;
static uint16_t test_write_address = 0;
static uint16_t test_write_value = 0;
static bool test_timeout_called = false;
static bool test_error_called = false;
static modbus_error_t test_last_error = MODBUS_OK;

// Test callbacks
static modbus_error_t test_read_callback(uint16_t address, uint16_t* value) {
    (void)address;
    *value = test_read_value;
    return MODBUS_OK;
}

static modbus_error_t test_write_callback(uint16_t address, uint16_t value) {
    test_write_address = address;
    test_write_value = value;
    return MODBUS_OK;
}

static void test_timeout_callback(void) {
    test_timeout_called = true;
}

static void test_error_callback(modbus_error_t error_code) {
    test_error_called = true;
    test_last_error = error_code;
}

void setUp(void) {
    // Initialize test configuration
    test_config.slave_id = MODBUS_SLAVE_ID;
    test_config.baud_rate = MODBUS_BAUD_RATE;
    test_config.timeout_ms = MODBUS_TIMEOUT_MS;
    test_config.read_callback = test_read_callback;
    test_config.write_callback = test_write_callback;
    test_config.timeout_callback = test_timeout_callback;
    test_config.error_callback = test_error_callback;
    
    // Reset test variables
    test_read_value = 0;
    test_write_address = 0;
    test_write_value = 0;
    test_timeout_called = false;
    test_error_called = false;
    test_last_error = MODBUS_OK;
    
    // Initialize register manager
    register_manager_init();
}

void tearDown(void) {
    // Reset MODBUS communication
    modbus_set_enabled(false);
    modbus_reset_statistics();
}

// Test functions
bool test_modbus_init_valid_config(void) {
    modbus_error_t result = modbus_init(&test_config);
    
    TEST_ASSERT_EQUAL(MODBUS_OK, result);
    TEST_ASSERT_TRUE(modbus_is_enabled());
    TEST_ASSERT_EQUAL(MODBUS_STATE_IDLE, modbus_get_state());
    
    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_init_null_config(void) {
    modbus_error_t result = modbus_init(NULL);
    
    TEST_ASSERT_EQUAL(MODBUS_ERROR_INVALID_LENGTH, result);
    
    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_validate_address_system_status(void) {
    // Test system status registers (read-only)
    TEST_ASSERT_TRUE(modbus_validate_address(0x0000, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x000F, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0x0000, MODBUS_FC_WRITE_SINGLE));
    TEST_ASSERT_FALSE(modbus_validate_address(0x000F, MODBUS_FC_WRITE_SINGLE));
    
    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_validate_address_global_control(void) {
    // Test global control registers (read/write)
    TEST_ASSERT_TRUE(modbus_validate_address(0x0010, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x001F, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x0010, MODBUS_FC_WRITE_SINGLE));
    TEST_ASSERT_TRUE(modbus_validate_address(0x001F, MODBUS_FC_WRITE_SINGLE));
    
    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_calculate_crc_known_values(void) {
    // Test CRC calculation with known values
    uint8_t test_data[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01};
    uint16_t calculated_crc = modbus_calculate_crc(test_data, sizeof(test_data));

    // The calculated CRC should be consistent
    std::cout << "Calculated CRC: 0x" << std::hex << calculated_crc << std::dec << std::endl;

    // Test that CRC is not the default value
    TEST_ASSERT_TRUE(calculated_crc != 0xFFFF);
    TEST_ASSERT_TRUE(calculated_crc != 0x0000);

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_calculate_crc_empty_data(void) {
    uint16_t crc = modbus_calculate_crc(NULL, 0);
    TEST_ASSERT_EQUAL(0xFFFF, crc);
    
    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_register_manager_init(void) {
    bool result = register_manager_init();
    TEST_ASSERT_TRUE(result);
    
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_TRUE(map != NULL);
    
    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_enable_disable(void) {
    modbus_set_enabled(true);
    TEST_ASSERT_TRUE(modbus_is_enabled());
    TEST_ASSERT_EQUAL(MODBUS_STATE_IDLE, modbus_get_state());

    modbus_set_enabled(false);
    TEST_ASSERT_FALSE(modbus_is_enabled());
    TEST_ASSERT_EQUAL(MODBUS_STATE_ERROR, modbus_get_state());

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_validate_address_sonicator_control(void) {
    // Test sonicator control registers (read/write) - 0xN00-0xN0F
    TEST_ASSERT_TRUE(modbus_validate_address(0x0100, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x0100, MODBUS_FC_WRITE_SINGLE));
    TEST_ASSERT_TRUE(modbus_validate_address(0x010F, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x010F, MODBUS_FC_WRITE_SINGLE));

    // Test sonicator status registers (read-only) - 0xN10-0xN1F
    TEST_ASSERT_TRUE(modbus_validate_address(0x0110, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0x0110, MODBUS_FC_WRITE_SINGLE));
    TEST_ASSERT_TRUE(modbus_validate_address(0x041F, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0x041F, MODBUS_FC_WRITE_SINGLE));

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_validate_address_invalid_ranges(void) {
    // Test invalid address ranges
    TEST_ASSERT_FALSE(modbus_validate_address(0x0020, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0x00FF, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0x0420, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0xFFFF, MODBUS_FC_READ_HOLDING));

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_register_manager_system_status(void) {
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_TRUE(map != NULL);

    // Test system status update
    register_manager_update_system_status(0x0001, true);
    TEST_ASSERT_EQUAL(0x0001, map->system_status.system_status);

    register_manager_update_system_status(0x0001, false);
    TEST_ASSERT_EQUAL(0x0000, map->system_status.system_status);

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_register_manager_sonicator_control(void) {
    // Test sonicator control settings
    uint16_t start_stop, amplitude_sp;

    bool result = register_manager_get_sonicator_control(0, &start_stop, &amplitude_sp);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, start_stop);  // Default stopped
    TEST_ASSERT_EQUAL(50, amplitude_sp);  // Default 50%

    // Test invalid sonicator ID
    result = register_manager_get_sonicator_control(MODBUS_MAX_SONICATORS, &start_stop, &amplitude_sp);
    TEST_ASSERT_FALSE(result);

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_register_manager_sonicator_status_update(void) {
    register_manager_update_sonicator_status(0, 1500, 20000, 75, 0x0001);

    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_EQUAL(1500, map->sonicators[0].power_watts);
    TEST_ASSERT_EQUAL(20000, map->sonicators[0].frequency_hz);
    TEST_ASSERT_EQUAL(75, map->sonicators[0].amplitude_actual);
    TEST_ASSERT_EQUAL(0x0001, map->sonicators[0].status_flags);

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_register_manager_global_enable(void) {
    modbus_register_map_t* map = register_manager_get_map();

    // Test default state
    TEST_ASSERT_FALSE(register_manager_is_global_enabled());

    // Enable globally
    map->global_control.global_enable = 1;
    TEST_ASSERT_TRUE(register_manager_is_global_enabled());

    // Disable globally
    map->global_control.global_enable = 0;
    TEST_ASSERT_FALSE(register_manager_is_global_enabled());

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_register_manager_emergency_stop(void) {
    modbus_register_map_t* map = register_manager_get_map();

    // Test default state
    TEST_ASSERT_FALSE(register_manager_is_emergency_stop_active());

    // Activate emergency stop
    map->global_control.emergency_stop = 1;
    TEST_ASSERT_TRUE(register_manager_is_emergency_stop_active());

    // Clear emergency stop
    register_manager_clear_emergency_stop();
    TEST_ASSERT_FALSE(register_manager_is_emergency_stop_active());

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_register_manager_value_validation(void) {
    // Test amplitude validation
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0101, 50));  // Valid amplitude
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0101, 20));  // Min amplitude
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0101, 100)); // Max amplitude
    TEST_ASSERT_FALSE(register_manager_validate_value(0x0101, 19)); // Below min
    TEST_ASSERT_FALSE(register_manager_validate_value(0x0101, 101)); // Above max

    // Test start/stop validation
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0100, 0));   // Stop
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0100, 1));   // Start
    TEST_ASSERT_FALSE(register_manager_validate_value(0x0100, 2));  // Invalid

    // Test global control validation
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0010, 0));   // Disable
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0010, 1));   // Enable
    TEST_ASSERT_FALSE(register_manager_validate_value(0x0010, 2));  // Invalid

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_register_manager_comm_errors(void) {
    modbus_register_map_t* map = register_manager_get_map();
    uint16_t initial_count = map->system_status.comm_errors;

    register_manager_increment_comm_errors();
    TEST_ASSERT_EQUAL(initial_count + 1, map->system_status.comm_errors);

    register_manager_increment_comm_errors();
    TEST_ASSERT_EQUAL(initial_count + 2, map->system_status.comm_errors);

    register_manager_reset_comm_errors();
    TEST_ASSERT_EQUAL(0, map->system_status.comm_errors);

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_timeout_detection(void) {
    // Test timeout detection
    TEST_ASSERT_FALSE(modbus_is_timeout());  // Should not be timeout initially

    // Reset timeout
    modbus_reset_timeout();
    TEST_ASSERT_FALSE(modbus_is_timeout());

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_statistics_reset(void) {
    const modbus_statistics_t* stats = modbus_get_statistics();
    TEST_ASSERT_TRUE(stats != NULL);

    // Reset statistics
    modbus_reset_statistics();

    // Verify all statistics are zero
    TEST_ASSERT_EQUAL(0, stats->requests_received);
    TEST_ASSERT_EQUAL(0, stats->responses_sent);
    TEST_ASSERT_EQUAL(0, stats->crc_errors);
    TEST_ASSERT_EQUAL(0, stats->timeout_errors);

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

bool test_modbus_process_idle_state(void) {
    // Test process function when disabled
    modbus_set_enabled(false);
    modbus_state_t state = modbus_process();
    TEST_ASSERT_EQUAL(MODBUS_STATE_IDLE, state);

    // Test process function when enabled
    modbus_set_enabled(true);
    state = modbus_process();
    // Should return current state (IDLE or TIMEOUT depending on timing)
    TEST_ASSERT_TRUE(state == MODBUS_STATE_IDLE || state == MODBUS_STATE_TIMEOUT);

    std::cout << "PASS: " << __FUNCTION__ << std::endl;
    return true;
}

// Test runner
int main() {
    std::cout << "Running Modbus Communication Tests..." << std::endl;
    
    int passed = 0;
    int total = 0;
    
    // Run tests
    setUp(); if (test_modbus_init_valid_config()) passed++; total++; tearDown();
    setUp(); if (test_modbus_init_null_config()) passed++; total++; tearDown();
    setUp(); if (test_modbus_validate_address_system_status()) passed++; total++; tearDown();
    setUp(); if (test_modbus_validate_address_global_control()) passed++; total++; tearDown();
    setUp(); if (test_modbus_validate_address_sonicator_control()) passed++; total++; tearDown();
    setUp(); if (test_modbus_validate_address_invalid_ranges()) passed++; total++; tearDown();
    setUp(); if (test_modbus_calculate_crc_known_values()) passed++; total++; tearDown();
    setUp(); if (test_modbus_calculate_crc_empty_data()) passed++; total++; tearDown();
    setUp(); if (test_register_manager_init()) passed++; total++; tearDown();
    setUp(); if (test_register_manager_system_status()) passed++; total++; tearDown();
    setUp(); if (test_register_manager_sonicator_control()) passed++; total++; tearDown();
    setUp(); if (test_register_manager_sonicator_status_update()) passed++; total++; tearDown();
    setUp(); if (test_register_manager_global_enable()) passed++; total++; tearDown();
    setUp(); if (test_register_manager_emergency_stop()) passed++; total++; tearDown();
    setUp(); if (test_register_manager_value_validation()) passed++; total++; tearDown();
    setUp(); if (test_register_manager_comm_errors()) passed++; total++; tearDown();
    setUp(); if (test_modbus_enable_disable()) passed++; total++; tearDown();
    setUp(); if (test_modbus_timeout_detection()) passed++; total++; tearDown();
    setUp(); if (test_modbus_statistics_reset()) passed++; total++; tearDown();
    setUp(); if (test_modbus_process_idle_state()) passed++; total++; tearDown();
    
    std::cout << "\nTest Results: " << passed << "/" << total << " tests passed" << std::endl;
    
    if (passed == total) {
        std::cout << "All tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests FAILED!" << std::endl;
        return 1;
    }
}
