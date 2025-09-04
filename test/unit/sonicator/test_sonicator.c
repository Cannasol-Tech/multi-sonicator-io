#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_sonicator_basic(void) {
    TEST_ASSERT_EQUAL_INT(1, 1); // Example sonicator test
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_sonicator_basic);
    return UNITY_END();
}
