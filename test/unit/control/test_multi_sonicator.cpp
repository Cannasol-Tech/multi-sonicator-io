/**
 * @file test_multi_sonicator.cpp
 * @brief Unit tests for Multi-Unit State Management (Story 4.1)
 */

#include <unity.h>
#include "multi_sonicator.h"

void setUp(void) { multi_sonicator_begin(); }
void tearDown(void) {}

// Forward declarations for additional tests
void test_invalid_requests_and_bounds(void);
void test_partial_coordinated_stop_targets_only_active_units(void);
void test_requests_blocked_during_emergency(void);
// New coverage-focused tests forward declarations
void test_unit_start_idempotent_when_running_and_starting(void);
void test_unit_start_rejected_when_fault_or_overload(void);
void test_unit_stop_idempotent_and_noop_on_fault_states(void);
void test_confirm_unit_started_promotes_and_clears_inhibit(void);
void test_confirm_unit_started_return_values(void);
void test_master_state_coordinated_start_when_any_starting(void);
void test_coordinated_stop_no_target_mask_noop(void);
void test_coordinated_stop_rejected_during_emergency(void);

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

void test_per_unit_abort_start_and_stop(void) {
    // Start all units via coordinated start
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x0F));
    // Before update() promotes to RUNNING, abort unit 1 (index 0) start
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_stop(0));
    // Progress state
    (void)multi_sonicator_update();

    const multi_status_t* st1 = multi_sonicator_get_status();
    // Unit 1 should be STOPPED; others RUNNING
    TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st1->unit_state[0]);
    for (int i = 1; i < 4; ++i) {
        TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st1->unit_state[i]);
    }
    TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, st1->master_state);

    // Now request STOP for unit 3 (index 2) while running
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_stop(2));
    (void)multi_sonicator_update();
    const multi_status_t* st2 = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st2->unit_state[2]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st2->unit_state[1]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st2->unit_state[3]);
    TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, st2->master_state);

    // Abort STOP for unit 3 by issuing START
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_start(2));
    (void)multi_sonicator_update();
    const multi_status_t* st3 = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st3->unit_state[2]);
}

void test_partial_success_with_unit_failure(void) {
    // Request start for all units
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x0F));

    // Before promotion to RUNNING, inject failure for unit 4 (index 3)
    TEST_ASSERT_TRUE(multi_sonicator_report_unit_fault(3, /*overload=*/true));

    // Progress update
    (void)multi_sonicator_update();

    const multi_status_t* st = multi_sonicator_get_status();
    // Units 1..3 should be RUNNING; unit 4 should be in OVERLOAD
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[0]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[1]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[2]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_OVERLOAD, st->unit_state[3]);

    // Active mask should have cleared unit 4 bit
    TEST_ASSERT_EQUAL_UINT8(0x07, st->active_mask);
    // Master remains RUNNING due to active units
    TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, st->master_state);
}

void test_start_timeout_path_for_inhibited_unit(void) {
    // Inhibit unit 2 start so it cannot auto-promote to RUNNING
    multi_sonicator_set_start_inhibit(1, true); // index 1 -> unit 2
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x03)); // units 1 and 2

    // Tick START_TIMEOUT_TICKS updates without confirming start
    for (int t = 0; t < 6; ++t) { // 6 > default 5
        (void)multi_sonicator_update();
    }

    const multi_status_t* st = multi_sonicator_get_status();
    // Unit 1 should be RUNNING, unit 2 should have faulted out
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[0]);
    TEST_ASSERT_TRUE(st->unit_state[1] == SONICATOR_STATE_FAULT || st->unit_state[1] == SONICATOR_STATE_OVERLOAD);
    // Active mask should only include unit 1
    TEST_ASSERT_EQUAL_UINT8(0x01, st->active_mask);
    TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, st->master_state);
}
void test_coordinated_stop_all_units(void) {
    // Start all units
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x0F));
    (void)multi_sonicator_update();
    const multi_status_t* st_run = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, st_run->master_state);

    // Request coordinated stop for all units
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_stop(0x0F));
    master_state_t ms = multi_sonicator_update();
    TEST_ASSERT_EQUAL(MASTER_STATE_IDLE, ms);

    const multi_status_t* st = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL_UINT8(0x00, st->active_mask);
    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st->unit_state[i]);
    }
}

void test_abort_start_per_unit_all(void) {
    for (int u = 0; u < 4; ++u) {
        // Fresh init per iteration
        multi_sonicator_begin();
        // Request start for all units
        TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x0F));
        // Abort start for unit u before update promotes states
        TEST_ASSERT_TRUE(multi_sonicator_request_unit_stop((uint8_t)u));
        (void)multi_sonicator_update();

        const multi_status_t* st = multi_sonicator_get_status();
        for (int i = 0; i < 4; ++i) {
            if (i == u) {
                TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st->unit_state[i]);
            } else {
                TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[i]);
            }
        }
        TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, st->master_state);
    }
}

void test_abort_stop_per_unit_all(void) {
    for (int u = 0; u < 4; ++u) {
        multi_sonicator_begin();
        // Bring all to RUNNING
        TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x0F));
        (void)multi_sonicator_update();

        // Stop unit u only
        TEST_ASSERT_TRUE(multi_sonicator_request_unit_stop((uint8_t)u));
        (void)multi_sonicator_update();
        const multi_status_t* st1 = multi_sonicator_get_status();
        TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st1->unit_state[u]);
        // Other units remain RUNNING
        for (int i = 0; i < 4; ++i) {
            if (i == u) continue;
            TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st1->unit_state[i]);
        }
        TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, st1->master_state);

        // Abort the stop for unit u by starting it again
        TEST_ASSERT_TRUE(multi_sonicator_request_unit_start((uint8_t)u));
        (void)multi_sonicator_update();
        const multi_status_t* st2 = multi_sonicator_get_status();
        TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st2->unit_state[u]);
    }
}

void test_partial_success_with_unit_failure_all(void) {
    for (int u = 0; u < 4; ++u) {
        multi_sonicator_begin();
        // Start all units, then inject failure for unit u before promotion
        TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x0F));
        TEST_ASSERT_TRUE(multi_sonicator_report_unit_fault((uint8_t)u, /*overload=*/true));
        (void)multi_sonicator_update();

        const multi_status_t* st = multi_sonicator_get_status();
        for (int i = 0; i < 4; ++i) {
            if (i == u) {
                TEST_ASSERT_EQUAL(SONICATOR_STATE_OVERLOAD, st->unit_state[i]);
            } else {
                TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[i]);
            }
        }
        // Mask bit for u should be cleared
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(0x0F & ~(1u << u)), st->active_mask);
        TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, st->master_state);
    }
}

void test_start_timeout_path_for_inhibited_unit_all(void) {
    for (int u = 0; u < 4; ++u) {
        multi_sonicator_begin();
        // Inhibit unit u so it cannot auto-promote; start all
        multi_sonicator_set_start_inhibit((uint8_t)u, true);
        TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x0F));
        // Tick beyond timeout window
        for (int t = 0; t < 6; ++t) {
            (void)multi_sonicator_update();
        }
        const multi_status_t* st = multi_sonicator_get_status();
        for (int i = 0; i < 4; ++i) {
            if (i == u) {
                TEST_ASSERT_TRUE(st->unit_state[i] == SONICATOR_STATE_FAULT || st->unit_state[i] == SONICATOR_STATE_OVERLOAD);
            } else {
                TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[i]);
            }
        }
        TEST_ASSERT_EQUAL(MASTER_STATE_RUNNING, st->master_state);
        // Mask should exclude inhibited/failed unit
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(0x0F & ~(1u << u)), st->active_mask);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_initial_state_idle);
    RUN_TEST(test_coordinated_start_all_units);
    RUN_TEST(test_emergency_stop_transitions);
    // Coordinated stop scenario
    RUN_TEST(test_coordinated_stop_all_units);
    RUN_TEST(test_per_unit_abort_start_and_stop);
    RUN_TEST(test_abort_start_per_unit_all);
    RUN_TEST(test_abort_stop_per_unit_all);
    RUN_TEST(test_partial_success_with_unit_failure);
    RUN_TEST(test_partial_success_with_unit_failure_all);
    RUN_TEST(test_start_timeout_path_for_inhibited_unit);
    RUN_TEST(test_start_timeout_path_for_inhibited_unit_all);
    // Additional edge cases for tighter coverage
    RUN_TEST(test_invalid_requests_and_bounds);
    RUN_TEST(test_partial_coordinated_stop_targets_only_active_units);
    RUN_TEST(test_requests_blocked_during_emergency);
    // New tests to increase branch coverage
    RUN_TEST(test_unit_start_idempotent_when_running_and_starting);
    RUN_TEST(test_unit_start_rejected_when_fault_or_overload);
    RUN_TEST(test_unit_stop_idempotent_and_noop_on_fault_states);
    RUN_TEST(test_confirm_unit_started_promotes_and_clears_inhibit);
    RUN_TEST(test_confirm_unit_started_return_values);
    RUN_TEST(test_master_state_coordinated_start_when_any_starting);
    RUN_TEST(test_coordinated_stop_no_target_mask_noop);
    RUN_TEST(test_coordinated_stop_rejected_during_emergency);
    return UNITY_END();
}

// ---- Additional tests implementation ----

void test_invalid_requests_and_bounds(void) {
    // Zero mask should be rejected for start/stop
    TEST_ASSERT_FALSE(multi_sonicator_request_coordinated_start(0x00));
    TEST_ASSERT_FALSE(multi_sonicator_request_coordinated_stop(0x00));

    // Invalid unit indices should be rejected
    TEST_ASSERT_FALSE(multi_sonicator_request_unit_start(4));
    TEST_ASSERT_FALSE(multi_sonicator_request_unit_stop(4));
}

void test_partial_coordinated_stop_targets_only_active_units(void) {
    // Start units 0,1,2
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x07));
    (void)multi_sonicator_update();

    // Request stop for units 0 and 3 (3 is not active)
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_stop(0x09));
    (void)multi_sonicator_update();

    const multi_status_t* st = multi_sonicator_get_status();
    // Unit 0 stopped, unit 1 and 2 still running, unit 3 was never running
    TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st->unit_state[0]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[1]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[2]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st->unit_state[3]);
}

void test_requests_blocked_during_emergency(void) {
    // Enter emergency stop
    TEST_ASSERT_TRUE(multi_sonicator_emergency_stop());
    (void)multi_sonicator_update();

    // Further requests should be rejected
    TEST_ASSERT_FALSE(multi_sonicator_request_coordinated_start(0x0F));
    TEST_ASSERT_FALSE(multi_sonicator_request_unit_start(1));

    const multi_status_t* st = multi_sonicator_get_status();
    // All units are stopped under emergency
    for (int i = 0; i < 4; ++i) {
        TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st->unit_state[i]);
    }
}

// ---- New coverage-focused tests ----

void test_unit_start_idempotent_when_running_and_starting(void) {
    // From STOPPED, request per‑unit start (no update yet) → STARTING
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_start(0));
    // Idempotent: request start again while STARTING
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_start(0));

    // Promote to RUNNING via update()
    (void)multi_sonicator_update();
    const multi_status_t* st1 = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st1->unit_state[0]);

    // Idempotent: request start again while RUNNING
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_start(0));
    (void)multi_sonicator_update();
    const multi_status_t* st2 = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st2->unit_state[0]);
}

void test_unit_start_rejected_when_fault_or_overload(void) {
    // Request coordinated start; then fault unit 2 before RUNNING
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x0F));
    TEST_ASSERT_TRUE(multi_sonicator_report_unit_fault(1, /*overload=*/false)); // FAULT
    // Starting unit 2 should be rejected due to FAULT
    TEST_ASSERT_FALSE(multi_sonicator_request_unit_start(1));

    // Also set overload on unit 3 and verify start rejected
    TEST_ASSERT_TRUE(multi_sonicator_report_unit_fault(2, /*overload=*/true));
    TEST_ASSERT_FALSE(multi_sonicator_request_unit_start(2));
}

void test_unit_stop_idempotent_and_noop_on_fault_states(void) {
    // Stop is idempotent when already STOPPED
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_stop(0));

    // Bring unit 1 to RUNNING, then fault it and verify STOP is a no‑op but returns true
    multi_sonicator_begin();
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_start(0));
    (void)multi_sonicator_update();
    TEST_ASSERT_TRUE(multi_sonicator_report_unit_fault(0, /*overload=*/false));
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_stop(0)); // no‑op allowed on FAULT
}

void test_confirm_unit_started_promotes_and_clears_inhibit(void) {
    multi_sonicator_begin();
    // Inhibit unit 1 start, request start, then confirm explicitly
    multi_sonicator_set_start_inhibit(0, true);
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_start(0));
    // While inhibited and in STARTING, confirm should promote to RUNNING and clear inhibit
    TEST_ASSERT_TRUE(multi_sonicator_confirm_unit_started(0));
    const multi_status_t* st = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st->unit_state[0]);
    // Additional update should keep it RUNNING
    (void)multi_sonicator_update();
    const multi_status_t* st2 = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st2->unit_state[0]);
}

void test_confirm_unit_started_return_values(void) {
    multi_sonicator_begin();
    // STOPPED → confirm returns false
    TEST_ASSERT_FALSE(multi_sonicator_confirm_unit_started(0));

    // STARTING → confirm returns true and promotes
    TEST_ASSERT_TRUE(multi_sonicator_request_unit_start(1));
    TEST_ASSERT_TRUE(multi_sonicator_confirm_unit_started(1));
    // RUNNING → confirm returns true (already running)
    TEST_ASSERT_TRUE(multi_sonicator_confirm_unit_started(1));
}

void test_master_state_coordinated_start_when_any_starting(void) {
    multi_sonicator_begin();
    // Inhibit unit 2 so it remains STARTING
    multi_sonicator_set_start_inhibit(1, true);
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x03)); // units 1 and 2
    // First update: unit 1 will RUNNING, unit 2 remains STARTING → master should be COORDINATED_START
    master_state_t ms = multi_sonicator_update();
    TEST_ASSERT_EQUAL(MASTER_STATE_COORDINATED_START, ms);
}

void test_coordinated_stop_no_target_mask_noop(void) {
    // Start only units 0..2
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_start(0x07));
    (void)multi_sonicator_update();
    const multi_status_t* st1 = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL_UINT8(0x07, st1->active_mask);

    // Request stop for unit 3 only (not active) → no changes
    TEST_ASSERT_TRUE(multi_sonicator_request_coordinated_stop(0x08));
    (void)multi_sonicator_update();
    const multi_status_t* st2 = multi_sonicator_get_status();
    TEST_ASSERT_EQUAL_UINT8(0x07, st2->active_mask);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st2->unit_state[0]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st2->unit_state[1]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_RUNNING, st2->unit_state[2]);
    TEST_ASSERT_EQUAL(SONICATOR_STATE_STOPPED, st2->unit_state[3]);
}

void test_coordinated_stop_rejected_during_emergency(void) {
    // Enter emergency stop
    TEST_ASSERT_TRUE(multi_sonicator_emergency_stop());
    (void)multi_sonicator_update();
    // Coordinated stop should be rejected while in emergency
    TEST_ASSERT_FALSE(multi_sonicator_request_coordinated_stop(0x0F));
}
