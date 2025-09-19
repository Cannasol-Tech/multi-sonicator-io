/**
 * @file test_modbus_communication.cpp
 * @brief Unit Tests for MODBUS Communication Framework
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

#ifdef UNIT_TEST

#include <unity.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Include the modules under test
#include "../../src/modules/communication/modbus.h"
#include "../../src/modules/communication/modbus_register_manager.h"

// ============================================================================
// TEST FIXTURE SETUP
// ============================================================================

static modbus_config_t test_config;
static uint16_t test_read_value;
static uint16_t test_write_address;
static uint16_t test_write_value;
static bool test_timeout_called;
static bool test_error_called;
static modbus_error_t test_last_error;

// Test callback functions
static modbus_error_t test_read_callback(uint16_t address, uint16_t* value) {
    (void)address; // Suppress unused parameter warning
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

// ============================================================================
// MODBUS INITIALIZATION TESTS
// ============================================================================

void test_modbus_init_valid_config(void) {
    modbus_error_t result = modbus_init(&test_config);
    
    TEST_ASSERT_EQUAL(MODBUS_OK, result);
    TEST_ASSERT_TRUE(modbus_is_enabled());
    TEST_ASSERT_EQUAL(MODBUS_STATE_IDLE, modbus_get_state());
}

void test_modbus_init_null_config(void) {
    modbus_error_t result = modbus_init(NULL);
    
    TEST_ASSERT_EQUAL(MODBUS_ERROR_INVALID_LENGTH, result);
}

// ============================================================================
// REGISTER VALIDATION TESTS
// ============================================================================

void test_modbus_validate_address_system_status(void) {
    // Test system status registers (read-only)
    TEST_ASSERT_TRUE(modbus_validate_address(0x0000, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x000F, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0x0000, MODBUS_FC_WRITE_SINGLE));
    TEST_ASSERT_FALSE(modbus_validate_address(0x000F, MODBUS_FC_WRITE_SINGLE));
}

void test_modbus_validate_address_global_control(void) {
    // Test global control registers (read/write)
    TEST_ASSERT_TRUE(modbus_validate_address(0x0010, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x001F, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x0010, MODBUS_FC_WRITE_SINGLE));
    TEST_ASSERT_TRUE(modbus_validate_address(0x001F, MODBUS_FC_WRITE_SINGLE));
}

void test_modbus_validate_address_sonicator_control(void) {
    // Test sonicator control registers (read/write)
    TEST_ASSERT_TRUE(modbus_validate_address(0x0100, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x010F, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x0100, MODBUS_FC_WRITE_SINGLE));
    TEST_ASSERT_TRUE(modbus_validate_address(0x010F, MODBUS_FC_WRITE_SINGLE));
}

void test_modbus_validate_address_sonicator_status(void) {
    // Test sonicator status registers (read-only)
    TEST_ASSERT_TRUE(modbus_validate_address(0x0110, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_TRUE(modbus_validate_address(0x011F, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0x0110, MODBUS_FC_WRITE_SINGLE));
    TEST_ASSERT_FALSE(modbus_validate_address(0x011F, MODBUS_FC_WRITE_SINGLE));
}

void test_modbus_validate_address_invalid_ranges(void) {
    // Test invalid address ranges
    TEST_ASSERT_FALSE(modbus_validate_address(0x0020, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0x00FF, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0x0420, MODBUS_FC_READ_HOLDING));
    TEST_ASSERT_FALSE(modbus_validate_address(0xFFFF, MODBUS_FC_READ_HOLDING));
}

// ============================================================================
// CRC CALCULATION TESTS
// ============================================================================

void test_modbus_calculate_crc_known_values(void) {
    // Test with known MODBUS frame: Slave ID 2, Function 03, Address 0x0000, Count 1
    uint8_t test_frame[] = {0x02, 0x03, 0x00, 0x00, 0x00, 0x01};
    uint16_t expected_crc = 0x4479; // Known CRC for this frame
    
    uint16_t calculated_crc = modbus_calculate_crc(test_frame, sizeof(test_frame));
    
    TEST_ASSERT_EQUAL_HEX16(expected_crc, calculated_crc);
}

void test_modbus_calculate_crc_empty_data(void) {
    uint16_t crc = modbus_calculate_crc(NULL, 0);
    TEST_ASSERT_EQUAL_HEX16(0xFFFF, crc); // Initial CRC value
}

// ============================================================================
// REGISTER MANAGER TESTS
// ============================================================================

void test_register_manager_init(void) {
    bool result = register_manager_init();
    
    TEST_ASSERT_TRUE(result);
    
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_NOT_NULL(map);
    TEST_ASSERT_EQUAL(SYSTEM_STATUS_OK, map->system_status.system_status);
    TEST_ASSERT_EQUAL(0, map->system_status.active_count);
    TEST_ASSERT_EQUAL(0, map->system_status.active_mask);
}

void test_register_manager_system_status(void) {
    register_manager_update_system_status(SYSTEM_STATUS_FAULT, true);
    
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_TRUE(map->system_status.system_status & SYSTEM_STATUS_FAULT);
    
    register_manager_update_system_status(SYSTEM_STATUS_FAULT, false);
    TEST_ASSERT_FALSE(map->system_status.system_status & SYSTEM_STATUS_FAULT);
}

void test_register_manager_sonicator_control(void) {
    uint16_t start_stop, amplitude_sp;
    
    // Test valid sonicator ID
    bool result = register_manager_get_sonicator_control(0, &start_stop, &amplitude_sp);
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(0, start_stop);  // Default stopped
    TEST_ASSERT_EQUAL(50, amplitude_sp); // Default 50%
    
    // Test invalid sonicator ID
    result = register_manager_get_sonicator_control(MODBUS_MAX_SONICATORS, &start_stop, &amplitude_sp);
    TEST_ASSERT_FALSE(result);
}

void test_register_manager_sonicator_status_update(void) {
    sonicator_status_t status = {
        .sonicator_id = 0,
        .power_watts = 100,
        .frequency_hz = 20000,
        .amplitude_actual = 45,
        .status_flags = SON_STATUS_RUNNING
    };
    register_manager_update_sonicator_status(&status);
    
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_EQUAL(100, map->sonicators[0].power_watts);
    TEST_ASSERT_EQUAL(20000, map->sonicators[0].frequency_hz);
    TEST_ASSERT_EQUAL(45, map->sonicators[0].amplitude_actual);
    TEST_ASSERT_TRUE(map->sonicators[0].status_flags & SON_STATUS_RUNNING);
    TEST_ASSERT_EQUAL(1, map->system_status.active_count);
    TEST_ASSERT_EQUAL(0x0001, map->system_status.active_mask);
}

void test_register_manager_global_enable(void) {
    TEST_ASSERT_FALSE(register_manager_is_global_enabled());
    
    modbus_register_map_t* map = register_manager_get_map();
    map->global_control.global_enable = 1;
    
    TEST_ASSERT_TRUE(register_manager_is_global_enabled());
}

void test_register_manager_emergency_stop(void) {
    TEST_ASSERT_FALSE(register_manager_is_emergency_stop_active());
    
    modbus_register_map_t* map = register_manager_get_map();
    map->global_control.emergency_stop = 1;
    
    TEST_ASSERT_TRUE(register_manager_is_emergency_stop_active());
    
    register_manager_clear_emergency_stop();
    TEST_ASSERT_FALSE(register_manager_is_emergency_stop_active());
}

void test_register_manager_value_validation(void) {
    // Test amplitude setpoint validation
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0101, 50));  // Valid 50%
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0101, 20));  // Valid minimum
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0101, 100)); // Valid maximum
    TEST_ASSERT_FALSE(register_manager_validate_value(0x0101, 19)); // Below minimum
    TEST_ASSERT_FALSE(register_manager_validate_value(0x0101, 101)); // Above maximum
    
    // Test start/stop validation
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0100, 0));   // Valid stop
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0100, 1));   // Valid start
    TEST_ASSERT_FALSE(register_manager_validate_value(0x0100, 2));  // Invalid value
}

// ============================================================================
// COMMUNICATION ERROR TESTS
// ============================================================================

void test_register_manager_comm_errors(void) {
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_EQUAL(0, map->system_status.comm_errors);
    
    register_manager_increment_comm_errors();
    TEST_ASSERT_EQUAL(1, map->system_status.comm_errors);
    
    register_manager_increment_comm_errors();
    TEST_ASSERT_EQUAL(2, map->system_status.comm_errors);
    
    register_manager_reset_comm_errors();
    TEST_ASSERT_EQUAL(0, map->system_status.comm_errors);
}

// ============================================================================
// MODBUS STATE MANAGEMENT TESTS
// ============================================================================

void test_modbus_enable_disable(void) {
    modbus_init(&test_config);
    
    TEST_ASSERT_TRUE(modbus_is_enabled());
    TEST_ASSERT_EQUAL(MODBUS_STATE_IDLE, modbus_get_state());
    
    modbus_set_enabled(false);
    TEST_ASSERT_FALSE(modbus_is_enabled());
    TEST_ASSERT_EQUAL(MODBUS_STATE_ERROR, modbus_get_state());
    
    modbus_set_enabled(true);
    TEST_ASSERT_TRUE(modbus_is_enabled());
    TEST_ASSERT_EQUAL(MODBUS_STATE_IDLE, modbus_get_state());
}

void test_modbus_timeout_detection(void) {
    modbus_init(&test_config);
    
    // Initially no timeout
    TEST_ASSERT_FALSE(modbus_is_timeout());
    
    // Reset timeout timer
    modbus_reset_timeout();
    TEST_ASSERT_FALSE(modbus_is_timeout());
}

void test_modbus_statistics_reset(void) {
    modbus_init(&test_config);
    
    const modbus_statistics_t* stats = modbus_get_statistics();
    TEST_ASSERT_NOT_NULL(stats);
    TEST_ASSERT_EQUAL(0, stats->requests_received);
    TEST_ASSERT_EQUAL(0, stats->responses_sent);
    TEST_ASSERT_EQUAL(0, stats->crc_errors);
    
    modbus_reset_statistics();
    stats = modbus_get_statistics();
    TEST_ASSERT_EQUAL(0, stats->requests_received);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

// Additional comprehensive tests for 90%+ coverage
void test_modbus_force_error(void) {
    modbus_init(&test_config);

    // Test error forcing functionality
    modbus_force_error(MODBUS_ERROR_CRC);
    TEST_ASSERT_TRUE(test_error_called);
    TEST_ASSERT_EQUAL(MODBUS_ERROR_CRC, test_last_error);
}

void test_modbus_get_state(void) {
    modbus_init(&test_config);

    // Test state getter
    modbus_state_t state = modbus_get_state();
    TEST_ASSERT_EQUAL(MODBUS_STATE_IDLE, state);

    // Test state after disabling
    modbus_set_enabled(false);
    state = modbus_get_state();
    TEST_ASSERT_EQUAL(MODBUS_STATE_ERROR, state);
}

void test_register_manager_sonicator_boundary_conditions(void) {
    // Test boundary conditions for sonicator IDs
    uint16_t start_stop, amplitude_sp;

    // Test valid boundary
    bool result = register_manager_get_sonicator_control(MODBUS_MAX_SONICATORS - 1, &start_stop, &amplitude_sp);
    TEST_ASSERT_TRUE(result);

    // Test invalid boundary
    result = register_manager_get_sonicator_control(MODBUS_MAX_SONICATORS, &start_stop, &amplitude_sp);
    TEST_ASSERT_FALSE(result);

    // Test null pointers
    result = register_manager_get_sonicator_control(0, NULL, &amplitude_sp);
    TEST_ASSERT_FALSE(result);

    result = register_manager_get_sonicator_control(0, &start_stop, NULL);
    TEST_ASSERT_FALSE(result);
}

void test_register_manager_comprehensive_validation(void) {
    // Test edge cases for value validation

    // Test overload reset validation
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0102, 0));   // Valid reset off
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0102, 1));   // Valid reset on
    TEST_ASSERT_FALSE(register_manager_validate_value(0x0102, 2));  // Invalid reset value

    // Test system reset validation
    TEST_ASSERT_TRUE(register_manager_validate_value(MODBUS_REG_SYSTEM_RESET, 0));
    TEST_ASSERT_TRUE(register_manager_validate_value(MODBUS_REG_SYSTEM_RESET, 1));
    TEST_ASSERT_FALSE(register_manager_validate_value(MODBUS_REG_SYSTEM_RESET, 2));

    // Test status register validation (should always pass)
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0000, 0xFFFF));
    TEST_ASSERT_TRUE(register_manager_validate_value(0x0001, 0x1234));
}

void test_modbus_statistics_comprehensive(void) {
    modbus_init(&test_config);

    const modbus_statistics_t* stats = modbus_get_statistics();
    TEST_ASSERT_NOT_NULL(stats);

    // Verify all statistics fields are accessible
    TEST_ASSERT_EQUAL(0, stats->requests_received);
    TEST_ASSERT_EQUAL(0, stats->responses_sent);
    TEST_ASSERT_EQUAL(0, stats->crc_errors);
    TEST_ASSERT_EQUAL(0, stats->timeout_errors);
    TEST_ASSERT_EQUAL(0, stats->illegal_function_errors);
    TEST_ASSERT_EQUAL(0, stats->illegal_address_errors);
    TEST_ASSERT_EQUAL(0, stats->slave_failure_errors);
    TEST_ASSERT_EQUAL(0, stats->last_request_time);
    TEST_ASSERT_EQUAL(0, stats->max_response_time);
}

int main(void) {
    UNITY_BEGIN();

    // Initialization tests
    RUN_TEST(test_modbus_init_valid_config);
    RUN_TEST(test_modbus_init_null_config);

    // Address validation tests
    RUN_TEST(test_modbus_validate_address_system_status);
    RUN_TEST(test_modbus_validate_address_global_control);
    RUN_TEST(test_modbus_validate_address_sonicator_control);
    RUN_TEST(test_modbus_validate_address_sonicator_status);
    RUN_TEST(test_modbus_validate_address_invalid_ranges);

    // CRC calculation tests
    RUN_TEST(test_modbus_calculate_crc_known_values);
    RUN_TEST(test_modbus_calculate_crc_empty_data);

    // Register manager tests
    RUN_TEST(test_register_manager_init);
    RUN_TEST(test_register_manager_system_status);
    RUN_TEST(test_register_manager_sonicator_control);
    RUN_TEST(test_register_manager_sonicator_status_update);
    RUN_TEST(test_register_manager_global_enable);
    RUN_TEST(test_register_manager_emergency_stop);
    RUN_TEST(test_register_manager_value_validation);
    RUN_TEST(test_register_manager_comm_errors);

    // State management tests
    RUN_TEST(test_modbus_enable_disable);
    RUN_TEST(test_modbus_timeout_detection);
    RUN_TEST(test_modbus_statistics_reset);

    // Additional comprehensive tests for 90%+ coverage
    RUN_TEST(test_modbus_force_error);
    RUN_TEST(test_modbus_get_state);
    RUN_TEST(test_register_manager_sonicator_boundary_conditions);
    RUN_TEST(test_register_manager_comprehensive_validation);
    RUN_TEST(test_modbus_statistics_comprehensive);

    return UNITY_END();
}

#endif // UNIT_TEST
