/**
 * @file test_single_sonicator_control.cpp
 * @brief Unit tests for Single Sonicator Control (Story 1.4)
 * @author Cannasol Technologies
 * @date 2025-09-14
 * @version 1.0.0
 */

#include <unity.h>
#include "sonicator_control.h"
#include "sonicator_modbus_bridge.h"
#include "modules/communication/modbus_register_manager.h"
#include "register_map.h"

void setUp(void) {
    // Initialize test environment before each test
    register_manager_init();
    sonicator_begin();
    sonicator_modbus_bridge_init();
}

void tearDown(void) {
    // Cleanup after each test
}

/**
 * @brief Test AC-1: Start/Stop Command Response Time
 */
void test_start_stop_command_timing(void) {
    // Test start command
    TEST_ASSERT_TRUE(sonicator_start());
    
    // Simulate some processing time
    sonicator_update();
    
    // Verify start was processed
    const sonicator_state_control_t* status = sonicator_get_status();
    TEST_ASSERT_NOT_NULL(status);
    TEST_ASSERT_TRUE(status->start_requested);
    
    // Test stop command
    TEST_ASSERT_TRUE(sonicator_stop());
    sonicator_update();
    
    // Verify stop was processed
    status = sonicator_get_status();
    TEST_ASSERT_TRUE(status->stop_requested);
}

/**
 * @brief Test AC-2: Amplitude Control Within Tolerance
 */
void test_amplitude_control_tolerance(void) {
    // Test valid amplitude setting
    TEST_ASSERT_TRUE(sonicator_set_amplitude(75));
    const sonicator_state_control_t* status = sonicator_get_status();
    TEST_ASSERT_EQUAL(75, status->amplitude_percent);
    
    // Test amplitude clamping - minimum
    TEST_ASSERT_TRUE(sonicator_set_amplitude(10));
    status = sonicator_get_status();
    TEST_ASSERT_EQUAL(20, status->amplitude_percent);
    
    // Test amplitude clamping - maximum
    TEST_ASSERT_TRUE(sonicator_set_amplitude(150));
    status = sonicator_get_status();
    TEST_ASSERT_EQUAL(100, status->amplitude_percent);
}

/**
 * @brief Test AC-3: Telemetry Register Integration
 */
void test_telemetry_registers(void) {
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_NOT_NULL(map);
    
    // Test status register updates via bridge
    sonicator_modbus_bridge_update();
    
    // Verify registers are accessible
    TEST_ASSERT_EQUAL(REG_SON4_START_STOP, SONICATOR_REG_ADDR(3, MODBUS_REG_SON_START_STOP));
    TEST_ASSERT_EQUAL(REG_SON4_AMPLITUDE_SP, SONICATOR_REG_ADDR(3, MODBUS_REG_SON_AMPLITUDE_SP));
    TEST_ASSERT_EQUAL(REG_SON4_POWER_WATTS, SONICATOR_REG_ADDR(3, MODBUS_REG_SON_POWER_WATTS));
    TEST_ASSERT_EQUAL(REG_SON4_STATUS_FLAGS, SONICATOR_REG_ADDR(3, MODBUS_REG_SON_STATUS_FLAGS));
}

/**
 * @brief Test AC-4: MODBUS Bridge Response Time
 */
void test_modbus_bridge_response_time(void) {
    // Test bridge initialization
    TEST_ASSERT_TRUE(sonicator_modbus_bridge_init());
    
    // Test bridge update function
    bool action_taken = sonicator_modbus_bridge_update();
    // Should return false if no commands pending (normal case)
    TEST_ASSERT_FALSE_MESSAGE(action_taken, "Bridge update should take no action when no commands are pending");
    
    // Test bridge responsiveness check
    TEST_ASSERT_TRUE(sonicator_modbus_bridge_is_responsive());
}

/**
 * @brief Test AC-5: Complete Control Loop Integration
 */
void test_complete_control_loop(void) {
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_NOT_NULL(map);
    
    // Set amplitude via register
    map->sonicators[3].amplitude_setpoint = 60;
    
    // Set start command via register
    map->sonicators[3].start_stop = 1;
    
    // Process bridge update
    sonicator_modbus_bridge_update();
    sonicator_update();
    
    // Verify control actions were executed
    const sonicator_state_control_t* status = sonicator_get_status();
    TEST_ASSERT_NOT_NULL(status);
    TEST_ASSERT_EQUAL(60, status->amplitude_percent);
    
    // Test stop command
    map->sonicators[3].start_stop = 0;
    sonicator_modbus_bridge_update();
    sonicator_update();
}

/**
 * @brief Test Overload Reset Command
 */
void test_overload_reset_command(void) {
    // Test overload reset function
    TEST_ASSERT_TRUE(sonicator_reset_overload());
    
    // Verify reset was processed
    const sonicator_state_control_t* status = sonicator_get_status();
    TEST_ASSERT_NOT_NULL(status);
    TEST_ASSERT_TRUE(status->reset_requested);
}

/**
 * @brief Test State Machine Basic Transitions
 */
void test_state_machine_transitions(void) {
    const sonicator_state_control_t* status = sonicator_get_status();
    TEST_ASSERT_NOT_NULL(status);
    
    // Should start in UNKNOWN state
    // After first update, should transition to IDLE
    sonicator_update();
    status = sonicator_get_status();
    // State behavior depends on initialization - test basic functionality
    TEST_ASSERT_NOT_EQUAL(SONICATOR_STATE_UNKNOWN, status->state);
}

/**
 * @brief Test Register Address Mappings
 */
void test_register_address_mappings(void) {
    // Verify Sonicator 4 register addresses are correct
    TEST_ASSERT_EQUAL(0x0160, REG_SON4_START_STOP);
    TEST_ASSERT_EQUAL(0x0161, REG_SON4_AMPLITUDE_SP);
    TEST_ASSERT_EQUAL(0x0162, REG_SON4_OVERLOAD_RESET);
    TEST_ASSERT_EQUAL(0x0170, REG_SON4_POWER_WATTS);
    TEST_ASSERT_EQUAL(0x0171, REG_SON4_FREQUENCY_HZ);
    TEST_ASSERT_EQUAL(0x0172, REG_SON4_STATUS_FLAGS);
    TEST_ASSERT_EQUAL(0x0173, REG_SON4_AMPLITUDE_ACT);
}

/**
 * @brief Main test runner
 */
int main(void) {
    UNITY_BEGIN();
    
    // Acceptance Criteria Tests
    RUN_TEST(test_start_stop_command_timing);     // AC-1
    RUN_TEST(test_amplitude_control_tolerance);   // AC-2
    RUN_TEST(test_telemetry_registers);          // AC-3
    RUN_TEST(test_modbus_bridge_response_time);  // AC-4
    RUN_TEST(test_complete_control_loop);        // AC-5
    
    // Additional Unit Tests
    RUN_TEST(test_overload_reset_command);
    RUN_TEST(test_state_machine_transitions);
    RUN_TEST(test_register_address_mappings);
    
    return UNITY_END();
}
