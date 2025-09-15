/**
 * @file test_prev_state_registers.cpp
 * @brief Unit tests for read-only previous-state MODBUS registers (Story 4.1 AC-11)
 */

#include <unity.h>
#include "modules/communication/modbus_registers.h"
#include "modules/communication/modbus_register_manager.h"
#include "register_map.h"

static void assert_readonly_addr(uint16_t addr) {
    TEST_ASSERT_TRUE_MESSAGE(IS_READONLY_REGISTER(addr), "Address should be read-only");
}

void setUp(void) {
    register_manager_init();
}

void tearDown(void) {}

void test_system_prev_state_registers_exist_and_readonly(void) {
    // Verify addresses
    TEST_ASSERT_EQUAL_UINT16(0x0005, REG_PREV_ACTIVE_MASK);
    TEST_ASSERT_EQUAL_UINT16(0x0006, REG_LAST_SHUTDOWN_REASON);

    // Verify read-only classification
    assert_readonly_addr(REG_PREV_ACTIVE_MASK);
    assert_readonly_addr(REG_LAST_SHUTDOWN_REASON);

    // Verify fields accessible in register map
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_NOT_NULL(map);
    (void)map->system_status.prev_active_mask;
    (void)map->system_status.last_shutdown_reason;
}

void test_per_unit_prev_state_register_addresses_and_readonly(void) {
    // Unit index: 0..3 maps to units 1..4
    const uint16_t base[4] = {0x0100, 0x0120, 0x0140, 0x0160};
    for (int i = 0; i < 4; ++i) {
        uint16_t addr_prev_state = SONICATOR_REG_ADDR(i, 0x14);
        uint16_t addr_persisted_amp = SONICATOR_REG_ADDR(i, 0x15);
        uint16_t addr_last_fault = SONICATOR_REG_ADDR(i, 0x16);
        uint16_t addr_ts_lo = SONICATOR_REG_ADDR(i, 0x17);

        TEST_ASSERT_EQUAL_UINT16(base[i] + 0x14, addr_prev_state);
        TEST_ASSERT_EQUAL_UINT16(base[i] + 0x15, addr_persisted_amp);
        TEST_ASSERT_EQUAL_UINT16(base[i] + 0x16, addr_last_fault);
        TEST_ASSERT_EQUAL_UINT16(base[i] + 0x17, addr_ts_lo);

        assert_readonly_addr(addr_prev_state);
        assert_readonly_addr(addr_persisted_amp);
        assert_readonly_addr(addr_last_fault);
        assert_readonly_addr(addr_ts_lo);
    }
}

void test_register_map_fields_present_for_units(void) {
    modbus_register_map_t* map = register_manager_get_map();
    TEST_ASSERT_NOT_NULL(map);
    for (int i = 0; i < MODBUS_MAX_SONICATORS; ++i) {
        (void)map->sonicators[i].prev_state;
        (void)map->sonicators[i].persisted_amplitude;
        (void)map->sonicators[i].last_fault_code;
        (void)map->sonicators[i].last_state_timestamp_lo;
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_system_prev_state_registers_exist_and_readonly);
    RUN_TEST(test_per_unit_prev_state_register_addresses_and_readonly);
    RUN_TEST(test_register_map_fields_present_for_units);
    return UNITY_END();
}

