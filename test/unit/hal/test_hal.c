#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_hal_basic(void) {
    TEST_ASSERT_EQUAL_INT(1, 1); // Example HAL test
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_hal_basic);
    return UNITY_END();
}
