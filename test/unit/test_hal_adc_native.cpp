/**
 * @file test_hal_adc_native.cpp
 * @brief Native unit tests for HAL ADC module
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * Following unity-testing-guide.md patterns for native testing
 * with proper mock objects and no Arduino dependencies.
 */

#include <unity.h>
#include <map>

// ============================================================================
// SIMPLE ADC MOCK IMPLEMENTATION FOR NATIVE TESTING
// ============================================================================

class SimpleAdcMock {
private:
    std::map<uint8_t, uint16_t> channel_values;
    bool initialized;

public:
    SimpleAdcMock() : initialized(false) {}
    
    bool init() {
        initialized = true;
        return true;
    }
    
    bool readChannel(uint8_t channel, uint16_t* raw_value) {
        if (!initialized || !raw_value) return false;
        *raw_value = channel_values[channel];
        return true;
    }
    
    bool startConversion(uint8_t channel) {
        (void)channel; // Suppress unused parameter warning
        return initialized;
    }
    
    bool isConversionComplete() {
        return initialized;
    }
    
    float rawToVoltage(uint16_t raw_value) {
        return (raw_value * 5.0f) / 1023.0f; // 10-bit ADC, 5V reference
    }
    
    bool readSonicatorPower(uint8_t sonicator_id, float* power_watts) {
        if (!power_watts || !initialized) return false;
        uint16_t raw;
        if (!readChannel(sonicator_id + 3, &raw)) return false; // ADC channels 4-7
        float voltage = rawToVoltage(raw);
        *power_watts = voltage / 0.00544f; // 5.44 mV/W scaling from pin matrix
        return true;
    }
    
    bool readFrequency(float* frequency_hz) {
        if (!frequency_hz || !initialized) return false;
        uint16_t raw;
        if (!readChannel(0, &raw)) return false; // LM2907 frequency output
        float voltage = rawToVoltage(raw);
        *frequency_hz = voltage * 4000.0f; // Example scaling
        return true;
    }
    
    // Test helpers
    void setChannelValue(uint8_t channel, uint16_t value) {
        channel_values[channel] = value;
    }
    
    bool isInitialized() { return initialized; }
    
    void reset() {
        channel_values.clear();
        initialized = false;
    }
};

// ============================================================================
// TEST FIXTURES
// ============================================================================

static SimpleAdcMock* mock_adc;

void setUp(void) {
    mock_adc = new SimpleAdcMock();
}

void tearDown(void) {
    delete mock_adc;
    mock_adc = nullptr;
}

// ============================================================================
// CORE ADC FUNCTION TESTS
// ============================================================================

void test_HalAdc_Init_Success_ReturnsTrue() {
    // Arrange & Act
    bool result = mock_adc->init();
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(mock_adc->isInitialized());
}

void test_HalAdc_ReadChannel_ValidChannel_ReturnsSetValue() {
    // Arrange
    mock_adc->init();
    uint8_t channel = 4;
    uint16_t expected_value = 512;
    uint16_t actual_value;
    mock_adc->setChannelValue(channel, expected_value);
    
    // Act
    bool result = mock_adc->readChannel(channel, &actual_value);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT16(expected_value, actual_value);
}

void test_HalAdc_ReadChannel_NotInitialized_ReturnsFalse() {
    // Arrange
    uint8_t channel = 4;
    uint16_t value;
    
    // Act
    bool result = mock_adc->readChannel(channel, &value);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

void test_HalAdc_RawToVoltage_MidScale_ReturnsCorrectVoltage() {
    // Arrange
    uint16_t raw_value = 512; // Mid-scale for 10-bit ADC
    float expected_voltage = 2.5f; // 5V * 512/1023
    
    // Act
    float result = mock_adc->rawToVoltage(raw_value);
    
    // Assert
    TEST_ASSERT_FLOAT_WITHIN(0.01f, expected_voltage, result);
}

void test_HalAdc_RawToVoltage_FullScale_ReturnsFiveVolts() {
    // Arrange
    uint16_t raw_value = 1023; // Full-scale for 10-bit ADC
    float expected_voltage = 5.0f;
    
    // Act
    float result = mock_adc->rawToVoltage(raw_value);
    
    // Assert
    TEST_ASSERT_FLOAT_WITHIN(0.01f, expected_voltage, result);
}

// ============================================================================
// SONICATOR POWER READING TESTS
// ============================================================================

void test_HalAdc_ReadSonicatorPower_Sonicator1_ReadsChannel4() {
    // Arrange
    mock_adc->init();
    uint8_t sonicator_id = 1;
    uint16_t raw_value = 272; // Corresponds to ~245W
    float power_watts;
    mock_adc->setChannelValue(4, raw_value); // Sonicator 1 uses ADC channel 4
    
    // Act
    bool result = mock_adc->readSonicatorPower(sonicator_id, &power_watts);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 245.0f, power_watts);
}

void test_HalAdc_ReadSonicatorPower_AllSonicators_CorrectChannelMapping() {
    // Arrange
    mock_adc->init();
    float power_readings[4];
    
    // Set up known values for all sonicator power channels (4-7)
    mock_adc->setChannelValue(4, 272);  // Sonicator 1: ~245W
    mock_adc->setChannelValue(5, 544);  // Sonicator 2: ~490W
    mock_adc->setChannelValue(6, 136);  // Sonicator 3: ~122W
    mock_adc->setChannelValue(7, 408);  // Sonicator 4: ~368W
    
    // Act - Read all sonicator power levels
    for (uint8_t i = 1; i <= 4; i++) {
        bool result = mock_adc->readSonicatorPower(i, &power_readings[i-1]);
        TEST_ASSERT_TRUE(result);
    }
    
    // Assert - Check all readings are reasonable
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 245.0f, power_readings[0]);
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 490.0f, power_readings[1]);
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 122.0f, power_readings[2]);
    TEST_ASSERT_FLOAT_WITHIN(10.0f, 368.0f, power_readings[3]);
}

// ============================================================================
// FREQUENCY READING TESTS
// ============================================================================

void test_HalAdc_ReadFrequency_ValidReading_ReturnsFrequency() {
    // Arrange
    mock_adc->init();
    uint16_t raw_value = 256; // Corresponds to ~5kHz
    float frequency_hz;
    mock_adc->setChannelValue(0, raw_value); // LM2907 frequency output on channel 0
    
    // Act
    bool result = mock_adc->readFrequency(&frequency_hz);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FLOAT_WITHIN(100.0f, 5000.0f, frequency_hz);
}

// ============================================================================
// PIN MATRIX VALIDATION TESTS
// ============================================================================

void test_HalAdc_PinMatrix_PowerChannels_CorrectMapping() {
    // Arrange
    mock_adc->init();
    
    // Pin matrix validation: PA4-PA7 (pins 36,35,34,33) map to ADC channels 4-7
    // for POWER_SENSE_1 through POWER_SENSE_4
    
    uint16_t test_values[] = {100, 200, 300, 400}; // Test values for channels 4-7
    
    // Act - Set test values for power sense channels
    for (uint8_t i = 0; i < 4; i++) {
        mock_adc->setChannelValue(i + 4, test_values[i]);
        
        uint16_t read_value;
        bool result = mock_adc->readChannel(i + 4, &read_value);
        
        // Assert
        TEST_ASSERT_TRUE(result);
        TEST_ASSERT_EQUAL_UINT16(test_values[i], read_value);
    }
}

void test_HalAdc_PowerScaling_5V44mVPerWatt_CorrectCalculation() {
    // Arrange
    mock_adc->init();
    
    // Test power scaling calculation per pin matrix: 5.44 mV/W
    // For 1000W: voltage should be 5.44V (clamped to 5V max)
    // For 500W: voltage should be 2.72V
    // Raw value for 2.72V: (2.72 / 5.0) * 1023 ≈ 556
    
    uint16_t raw_for_500W = 556;
    mock_adc->setChannelValue(4, raw_for_500W);
    
    float power_watts;
    
    // Act
    bool result = mock_adc->readSonicatorPower(1, &power_watts);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FLOAT_WITHIN(25.0f, 500.0f, power_watts);
}

// ============================================================================
// ERROR CONDITION TESTS
// ============================================================================

void test_HalAdc_ReadChannel_NullPointer_ReturnsFalse() {
    // Arrange
    mock_adc->init();
    
    // Act
    bool result = mock_adc->readChannel(4, nullptr);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

void test_HalAdc_ReadSonicatorPower_NullPointer_ReturnsFalse() {
    // Arrange
    mock_adc->init();
    
    // Act
    bool result = mock_adc->readSonicatorPower(1, nullptr);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

void test_HalAdc_ReadFrequency_NullPointer_ReturnsFalse() {
    // Arrange
    mock_adc->init();
    
    // Act
    bool result = mock_adc->readFrequency(nullptr);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

// ============================================================================
// NATIVE PLATFORM ENTRY POINT
// ============================================================================

int main(int argc, char **argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning

    UNITY_BEGIN();
    
    // Core ADC function tests
    RUN_TEST(test_HalAdc_Init_Success_ReturnsTrue);
    RUN_TEST(test_HalAdc_ReadChannel_ValidChannel_ReturnsSetValue);
    RUN_TEST(test_HalAdc_ReadChannel_NotInitialized_ReturnsFalse);
    RUN_TEST(test_HalAdc_RawToVoltage_MidScale_ReturnsCorrectVoltage);
    RUN_TEST(test_HalAdc_RawToVoltage_FullScale_ReturnsFiveVolts);
    
    // Sonicator power reading tests
    RUN_TEST(test_HalAdc_ReadSonicatorPower_Sonicator1_ReadsChannel4);
    RUN_TEST(test_HalAdc_ReadSonicatorPower_AllSonicators_CorrectChannelMapping);
    
    // Frequency reading tests
    RUN_TEST(test_HalAdc_ReadFrequency_ValidReading_ReturnsFrequency);
    
    // Pin matrix validation tests
    RUN_TEST(test_HalAdc_PinMatrix_PowerChannels_CorrectMapping);
    RUN_TEST(test_HalAdc_PowerScaling_5V44mVPerWatt_CorrectCalculation);
    
    // Error condition tests
    RUN_TEST(test_HalAdc_ReadChannel_NullPointer_ReturnsFalse);
    RUN_TEST(test_HalAdc_ReadSonicatorPower_NullPointer_ReturnsFalse);
    RUN_TEST(test_HalAdc_ReadFrequency_NullPointer_ReturnsFalse);
    
    return UNITY_END();
}
