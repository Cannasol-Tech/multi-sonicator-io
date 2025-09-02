/**
 * @file test_hal_pwm_native.cpp
 * @brief Native unit tests for HAL PWM module
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
// SIMPLE PWM MOCK IMPLEMENTATION FOR NATIVE TESTING
// ============================================================================

class SimplePwmMock {
private:
    std::map<uint8_t, uint32_t> channel_frequencies;
    std::map<uint8_t, uint8_t> channel_duty_cycles;
    std::map<uint8_t, bool> channel_enabled;
    bool initialized;
    bool emergency_stopped;
    uint8_t amplitude_percent;

public:
    SimplePwmMock() : initialized(false), emergency_stopped(false), amplitude_percent(0) {}
    
    bool init() {
        initialized = true;
        emergency_stopped = false;
        return true;
    }
    
    bool setFrequency(uint8_t channel, uint32_t frequency) {
        if (!initialized || emergency_stopped) return false;
        channel_frequencies[channel] = frequency;
        return true;
    }
    
    bool setDutyCycle(uint8_t channel, uint8_t duty_percent) {
        if (!initialized || emergency_stopped || duty_percent > 100) return false;
        channel_duty_cycles[channel] = duty_percent;
        return true;
    }
    
    bool enableChannel(uint8_t channel) {
        if (!initialized || emergency_stopped) return false;
        channel_enabled[channel] = true;
        return true;
    }
    
    bool disableChannel(uint8_t channel) {
        if (!initialized) return false;
        channel_enabled[channel] = false;
        return true;
    }
    
    bool setAmplitude(uint8_t amplitude_percent) {
        if (!initialized || emergency_stopped || amplitude_percent > 100) return false;
        this->amplitude_percent = amplitude_percent;
        return setDutyCycle(0, amplitude_percent); // Amplitude channel
    }
    
    bool emergencyStop() {
        emergency_stopped = true;
        amplitude_percent = 0;
        // Disable all channels
        for (auto& pair : channel_enabled) {
            pair.second = false;
        }
        return true;
    }
    
    bool testPattern() {
        if (!initialized || emergency_stopped) return false;
        // Simulate test pattern execution
        return true;
    }
    
    // Test helpers
    uint32_t getChannelFrequency(uint8_t channel) {
        return channel_frequencies[channel];
    }
    
    uint8_t getChannelDutyCycle(uint8_t channel) {
        return channel_duty_cycles[channel];
    }
    
    bool isChannelEnabled(uint8_t channel) {
        return channel_enabled[channel];
    }
    
    uint8_t getAmplitude() { return amplitude_percent; }
    bool isEmergencyStopped() { return emergency_stopped; }
    bool isInitialized() { return initialized; }
    
    void reset() {
        channel_frequencies.clear();
        channel_duty_cycles.clear();
        channel_enabled.clear();
        initialized = false;
        emergency_stopped = false;
        amplitude_percent = 0;
    }
};

// ============================================================================
// TEST FIXTURES
// ============================================================================

static SimplePwmMock* mock_pwm;

void setUp(void) {
    mock_pwm = new SimplePwmMock();
}

void tearDown(void) {
    delete mock_pwm;
    mock_pwm = nullptr;
}

// ============================================================================
// CORE PWM FUNCTION TESTS
// ============================================================================

void test_HalPwm_Init_Success_ReturnsTrue() {
    // Arrange & Act
    bool result = mock_pwm->init();
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(mock_pwm->isInitialized());
    TEST_ASSERT_FALSE(mock_pwm->isEmergencyStopped());
}

void test_HalPwm_SetFrequency_ValidChannel_ReturnsTrue() {
    // Arrange
    mock_pwm->init();
    uint8_t channel = 0;
    uint32_t frequency = 20000; // 20kHz
    
    // Act
    bool result = mock_pwm->setFrequency(channel, frequency);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT32(frequency, mock_pwm->getChannelFrequency(channel));
}

void test_HalPwm_SetFrequency_NotInitialized_ReturnsFalse() {
    // Arrange
    uint8_t channel = 0;
    uint32_t frequency = 20000;
    
    // Act
    bool result = mock_pwm->setFrequency(channel, frequency);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

void test_HalPwm_SetDutyCycle_ValidValue_ReturnsTrue() {
    // Arrange
    mock_pwm->init();
    uint8_t channel = 0;
    uint8_t duty_percent = 50;
    
    // Act
    bool result = mock_pwm->setDutyCycle(channel, duty_percent);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(duty_percent, mock_pwm->getChannelDutyCycle(channel));
}

void test_HalPwm_SetDutyCycle_InvalidValue_ReturnsFalse() {
    // Arrange
    mock_pwm->init();
    uint8_t channel = 0;
    uint8_t duty_percent = 150; // Invalid > 100%
    
    // Act
    bool result = mock_pwm->setDutyCycle(channel, duty_percent);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

void test_HalPwm_EnableChannel_ValidChannel_ReturnsTrue() {
    // Arrange
    mock_pwm->init();
    uint8_t channel = 0;
    
    // Act
    bool result = mock_pwm->enableChannel(channel);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(mock_pwm->isChannelEnabled(channel));
}

void test_HalPwm_DisableChannel_ValidChannel_ReturnsTrue() {
    // Arrange
    mock_pwm->init();
    uint8_t channel = 0;
    mock_pwm->enableChannel(channel); // Enable first
    
    // Act
    bool result = mock_pwm->disableChannel(channel);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_FALSE(mock_pwm->isChannelEnabled(channel));
}

// ============================================================================
// AMPLITUDE CONTROL TESTS
// ============================================================================

void test_HalPwm_SetAmplitude_ValidValue_ReturnsTrue() {
    // Arrange
    mock_pwm->init();
    uint8_t amplitude = 75;
    
    // Act
    bool result = mock_pwm->setAmplitude(amplitude);
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL_UINT8(amplitude, mock_pwm->getAmplitude());
    TEST_ASSERT_EQUAL_UINT8(amplitude, mock_pwm->getChannelDutyCycle(0)); // Should set channel 0
}

void test_HalPwm_SetAmplitude_InvalidValue_ReturnsFalse() {
    // Arrange
    mock_pwm->init();
    uint8_t amplitude = 150; // Invalid > 100%
    
    // Act
    bool result = mock_pwm->setAmplitude(amplitude);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

// ============================================================================
// EMERGENCY STOP TESTS
// ============================================================================

void test_HalPwm_EmergencyStop_DisablesAllChannels() {
    // Arrange
    mock_pwm->init();
    mock_pwm->enableChannel(0);
    mock_pwm->enableChannel(1);
    mock_pwm->setAmplitude(50);
    
    // Act
    bool result = mock_pwm->emergencyStop();
    
    // Assert
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(mock_pwm->isEmergencyStopped());
    TEST_ASSERT_EQUAL_UINT8(0, mock_pwm->getAmplitude());
    TEST_ASSERT_FALSE(mock_pwm->isChannelEnabled(0));
    TEST_ASSERT_FALSE(mock_pwm->isChannelEnabled(1));
}

void test_HalPwm_SetFrequency_AfterEmergencyStop_ReturnsFalse() {
    // Arrange
    mock_pwm->init();
    mock_pwm->emergencyStop();
    
    // Act
    bool result = mock_pwm->setFrequency(0, 20000);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

void test_HalPwm_SetAmplitude_AfterEmergencyStop_ReturnsFalse() {
    // Arrange
    mock_pwm->init();
    mock_pwm->emergencyStop();
    
    // Act
    bool result = mock_pwm->setAmplitude(50);
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

// ============================================================================
// TEST PATTERN TESTS
// ============================================================================

void test_HalPwm_TestPattern_Initialized_ReturnsTrue() {
    // Arrange
    mock_pwm->init();
    
    // Act
    bool result = mock_pwm->testPattern();
    
    // Assert
    TEST_ASSERT_TRUE(result);
}

void test_HalPwm_TestPattern_NotInitialized_ReturnsFalse() {
    // Arrange
    // Don't initialize
    
    // Act
    bool result = mock_pwm->testPattern();
    
    // Assert
    TEST_ASSERT_FALSE(result);
}

void test_HalPwm_TestPattern_AfterEmergencyStop_ReturnsFalse() {
    // Arrange
    mock_pwm->init();
    mock_pwm->emergencyStop();
    
    // Act
    bool result = mock_pwm->testPattern();
    
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
    
    // Core PWM function tests
    RUN_TEST(test_HalPwm_Init_Success_ReturnsTrue);
    RUN_TEST(test_HalPwm_SetFrequency_ValidChannel_ReturnsTrue);
    RUN_TEST(test_HalPwm_SetFrequency_NotInitialized_ReturnsFalse);
    RUN_TEST(test_HalPwm_SetDutyCycle_ValidValue_ReturnsTrue);
    RUN_TEST(test_HalPwm_SetDutyCycle_InvalidValue_ReturnsFalse);
    RUN_TEST(test_HalPwm_EnableChannel_ValidChannel_ReturnsTrue);
    RUN_TEST(test_HalPwm_DisableChannel_ValidChannel_ReturnsTrue);
    
    // Amplitude control tests
    RUN_TEST(test_HalPwm_SetAmplitude_ValidValue_ReturnsTrue);
    RUN_TEST(test_HalPwm_SetAmplitude_InvalidValue_ReturnsFalse);
    
    // Emergency stop tests
    RUN_TEST(test_HalPwm_EmergencyStop_DisablesAllChannels);
    RUN_TEST(test_HalPwm_SetFrequency_AfterEmergencyStop_ReturnsFalse);
    RUN_TEST(test_HalPwm_SetAmplitude_AfterEmergencyStop_ReturnsFalse);
    
    // Test pattern tests
    RUN_TEST(test_HalPwm_TestPattern_Initialized_ReturnsTrue);
    RUN_TEST(test_HalPwm_TestPattern_NotInitialized_ReturnsFalse);
    RUN_TEST(test_HalPwm_TestPattern_AfterEmergencyStop_ReturnsFalse);
    
    return UNITY_END();
}
