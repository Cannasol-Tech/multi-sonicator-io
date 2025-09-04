#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_module_1_functionality(void) {
    TEST_ASSERT_EQUAL_INT(1, 1); // Example test
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_module_1_functionality);
    return UNITY_END();
}
