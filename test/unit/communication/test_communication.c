#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_communication_basic(void) {
    TEST_ASSERT_EQUAL_INT(1, 1); // Example communication test
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_communication_basic);
    return UNITY_END();
}
