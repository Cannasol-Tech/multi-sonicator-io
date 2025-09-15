/**
 * @file test_multi_sonicator.cpp
 * @brief Unit tests for Multi-Unit State Management (Story 4.1)
 */

#include <unity.h>
#include "multi_sonicator.h"

void setUp(void) { multi_sonicator_begin(); }
void tearDown(void) {}

void test_initial_state_idle(void) {
    const multi_status_t* st = multi_sonicator_get_status();
    TEST_ASSERT_NOT_NULL(st);
    TEST_ASSERT_EQUAL(MASTER_STATE_IDLE, st->master_state);
    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st->unit_state[i]);
    }
}

void test_coordinated_start_all_units(void) {
    uint8_t mask = 0x0F; // units 0..3
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(mask));
    master_state_t ms = multi_sonicator_update();
    TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, ms);

    const multi_status_t* st = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL(mask, st->active_mask);
    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[i]);
    }
}

void test_emergency_stop_transitions(void) {
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x05)); // units 0 and 2
    (void)multi_sonicator_update();
    TEST_ASSERT_TRUE(multi_sonicator_emergency_stop());
    master_state_t ms = multi_sonicator_update();
    TEST_ASSERT_EQUAL(MASTER_STATE_EMERGENCY_STOP, ms);

    const multi_status_t* st = multi_sonicator_get_status();
    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st->unit_state[i]);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_initial_state_idle);
    RUN_TEST(test_coordinated_start_all_units);
    RUN_TEST(test_emergency_stop_transitions);
    return UNITY_END();
}

