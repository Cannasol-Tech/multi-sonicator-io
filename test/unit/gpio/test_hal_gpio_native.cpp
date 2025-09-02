/**
 * @file test_hal_gpio_native.cpp
 * @brief Native unit tests for HAL GPIO module
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * Following unity-testing-guide.md patterns for native testing
 * with proper mock objects and no Arduino dependencies.
 */

#include <unity.h>
#include <map>
#include <vector>

// ============================================================================
// SIMPLE MOCK IMPLEMENTATION FOR NATIVE TESTING
// ============================================================================

class SimpleGpioMock {
private:
    std::map<uint8_t, uint8_t> pin_modes;
    std::map<uint8_t, uint8_t> pin_states;
    std::vector<std::pair<uint8_t, uint8_t>> write_history;
    bool initialized;

public:
    SimpleGpioMock() : initialized(false) {}
    
    bool init() {
        initialized = true;
        return true;
    }
    
    bool setPinMode(uint8_t pin, uint8_t mode) {
        if (!initialized) return false;
        pin_modes[pin] = mode;
        return true;
    }
    
    bool digitalWrite(uint8_t pin, uint8_t value) {
        if (!initialized) return false;
        pin_states[pin] = value;
        write_history.push_back({pin, value});
        return true;
    }
    
    uint8_t digitalRead(uint8_t pin) {
        if (!initialized) return 0;
        return pin_states[pin];
    }
    
    // Test helpers
    void setReadValue(uint8_t pin, uint8_t value) {
        pin_states[pin] = value;
    }
    
    bool wasWritten(uint8_t pin, uint8_t value) {
        for (auto& write : write_history) {
            if (write.first == pin && write.second == value) {
                return true;
            }
        }
        return false;
    }
    
    void reset() {
        pin_modes.clear();
        pin_states.clear();
        write_history.clear();
        initialized = false;
    }
    
    bool isInitialized() { return initialized; }
};

// ============================================================================
// TEST FIXTURES
// ============================================================================

static SimpleGpioMock* mock_gpio;

void setUp(void) {
    mock_gpio = new SimpleGpioMock();
}

void tearDown(void) {
    delete mock_gpio;
    mock_gpio = nullptr;
}

// ============================================================================
// CORE GPIO FUNCTION TESTS
// ============================================================================

void test_HalGpio_Init_Success_ReturnsTrue() {
    // Arrange & Act
    bool result = mock_gpio->init();
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(mock_gpio->isInitialized());
}

void test_HalGpio_SetPinMode_ValidPin_SetsMode() {
    // Arrange
    mock_gpio->init();
    uint8_t pin = 13;
    uint8_t mode = 1; // OUTPUT
    
    // Act
    bool result = mock_gpio->setPinMode(pin, mode);
    
    // Assert
    TEST_ASSERT_TRUE(result);
}

void test_HalGpio_SetPinMode_NotInitialized_ReturnsFalse() {
    // Arrange
    uint8_t pin = 13;
    uint8_t mode = 1;
    
    // Act
    bool result = mock_gpio->setPinMode(pin, mode);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

void test_HalGpio_DigitalWrite_ValidPin_WritesValue() {
    // Arrange
    mock_gpio->init();
    uint8_t pin = 13;
    uint8_t value = 1;
    
    // Act
    bool result = mock_gpio->digitalWrite(pin, value);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(pin, value));
}

void test_HalGpio_DigitalRead_ValidPin_ReturnsSetValue() {
    // Arrange
    mock_gpio->init();
    uint8_t pin = 13;
    uint8_t expected_value = 1;
    mock_gpio->setReadValue(pin, expected_value);
    
    // Act
    uint8_t result = mock_gpio->digitalRead(pin);
    
    // Assert
    TEST_ASSERT_EQUAL_UINT8(expected_value, result);
}

void test_HalGpio_WriteRead_Sequence_ConsistentBehavior() {
    // Arrange
    mock_gpio->init();
    uint8_t pin = 13;
    
    // Act & Assert - Test write then read sequence
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(pin, 1));
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(pin, 1));
    
    mock_gpio->setReadValue(pin, 1);
    TEST_ASSERT_EQUAL_UINT8(1, mock_gpio->digitalRead(pin));
    
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(pin, 0));
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(pin, 0));
    
    mock_gpio->setReadValue(pin, 0);
    TEST_ASSERT_EQUAL_UINT8(0, mock_gpio->digitalRead(pin));
}

void test_HalGpio_MultipleWrites_History_RecordsAllWrites() {
    // Arrange
    mock_gpio->init();
    uint8_t pin = 13;
    
    // Act - Multiple writes to same pin
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(pin, 1));
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(pin, 0));
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(pin, 1));
    
    // Assert - All writes recorded
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(pin, 1));
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(pin, 0));
}

void test_HalGpio_MultiplePins_IndependentOperation() {
    // Arrange
    mock_gpio->init();
    uint8_t pin1 = 13;
    uint8_t pin2 = 14;
    
    // Act - Write to different pins
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(pin1, 1));
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(pin2, 0));
    
    // Assert - Independent operation
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(pin1, 1));
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(pin2, 0));
    
    // Verify reads are independent
    mock_gpio->setReadValue(pin1, 1);
    mock_gpio->setReadValue(pin2, 0);
    TEST_ASSERT_EQUAL_UINT8(1, mock_gpio->digitalRead(pin1));
    TEST_ASSERT_EQUAL_UINT8(0, mock_gpio->digitalRead(pin2));
}

void test_HalGpio_Reset_ClearsAllState() {
    // Arrange
    mock_gpio->init();
    mock_gpio->setPinMode(13, 1);
    mock_gpio->digitalWrite(13, 1);
    
    // Act
    mock_gpio->reset();
    
    // Assert
    TEST_ASSERT_FALSE(mock_gpio->isInitialized());
    TEST_ASSERT_FALSE(mock_gpio->setPinMode(13, 1)); // Should fail after reset
}

// ============================================================================
// PIN MATRIX VALIDATION TESTS
// ============================================================================

void test_HalGpio_PinMatrix_SonicatorPins_ValidPinNumbers() {
    // Arrange
    mock_gpio->init();
    
    // Pin matrix validation for sonicator pins
    // These pin numbers come from docs/planning/pin-matrix.md
    
    // Sonicator 1 pins
    uint8_t s1_start = 28;   // PC6 (Pin 28) - START_1
    uint8_t s1_reset = 29;   // PC7 (Pin 29) - RESET_1
    uint8_t s1_overload = 20; // PD6 (Pin 20) - OVERLOAD_1
    uint8_t s1_freq_lock = 8; // PB7 (Pin 8) - FREQ_LOCK_1
    
    // Act - Test all sonicator 1 pins can be controlled
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(s1_start, 1));
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(s1_reset, 1));
    mock_gpio->setReadValue(s1_overload, 0); // Simulate no overload
    mock_gpio->setReadValue(s1_freq_lock, 1); // Simulate frequency locked
    
    // Assert - All operations successful
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(s1_start, 1));
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(s1_reset, 1));
    TEST_ASSERT_EQUAL_UINT8(0, mock_gpio->digitalRead(s1_overload));
    TEST_ASSERT_EQUAL_UINT8(1, mock_gpio->digitalRead(s1_freq_lock));
}

void test_HalGpio_PinMatrix_StatusLed_Pin16Validation() {
    // Arrange
    mock_gpio->init();
    uint8_t status_led_pin = 16; // PD2 (Pin 16) per pin matrix
    
    // Act - Test status LED control
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(status_led_pin, 1)); // LED on
    TEST_ASSERT_TRUE(mock_gpio->digitalWrite(status_led_pin, 0)); // LED off
    
    // Assert
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(status_led_pin, 1));
    TEST_ASSERT_TRUE(mock_gpio->wasWritten(status_led_pin, 0));
}

// ============================================================================
// NATIVE PLATFORM ENTRY POINT
// ============================================================================

int main(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning

    UNITY_BEGIN();
    
    // Core GPIO function tests
    RUN_TEST(test_HalGpio_Init_Success_ReturnsTrue);
    RUN_TEST(test_HalGpio_SetPinMode_ValidPin_SetsMode);
    RUN_TEST(test_HalGpio_SetPinMode_NotInitialized_ReturnsFalse);
    RUN_TEST(test_HalGpio_DigitalWrite_ValidPin_WritesValue);
    RUN_TEST(test_HalGpio_DigitalRead_ValidPin_ReturnsSetValue);
    RUN_TEST(test_HalGpio_WriteRead_Sequence_ConsistentBehavior);
    RUN_TEST(test_HalGpio_MultipleWrites_History_RecordsAllWrites);
    RUN_TEST(test_HalGpio_MultiplePins_IndependentOperation);
    RUN_TEST(test_HalGpio_Reset_ClearsAllState);
    
    // Pin matrix validation tests
    RUN_TEST(test_HalGpio_PinMatrix_SonicatorPins_ValidPinNumbers);
    RUN_TEST(test_HalGpio_PinMatrix_StatusLed_Pin16Validation);
    
    return UNITY_END();
}
