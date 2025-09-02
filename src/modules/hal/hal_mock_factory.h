/**
 * @file hal_mock_factory.h
 * @brief Mock Factory Implementation for Unit Testing
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * Mock implementations following unity-testing-guide.md patterns
 * for comprehensive unit testing of HAL modules without real hardware.
 */

#ifndef HAL_MOCK_FACTORY_H
#define HAL_MOCK_FACTORY_H

#include "hal_interfaces.h"
#include <map>
#include <vector>

// ============================================================================
// MOCK IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Mock GPIO implementation for testing
 * 
 * Following mock patterns from unity-testing-guide.md with
 * state tracking and verification helpers.
 */
class MockGpioPort : public IGpioPort {
private:
    std::map<uint8_t, uint8_t> pin_modes;
    std::map<uint8_t, uint8_t> pin_states;
    std::vector<std::pair<uint8_t, uint8_t>> write_history;
    bool initialized;
    uint8_t error_pin; // Pin that will always fail for error testing

public:
    MockGpioPort() : initialized(false), error_pin(255) {}
    
    // Interface implementation
    bool init(void) override {
        initialized = true;
        return true;
    }
    
    bool setPinMode(uint8_t pin, uint8_t mode) override {
        if (!initialized || pin == error_pin) return false;
        pin_modes[pin] = mode;
        return true;
    }
    
    bool digitalWrite(uint8_t pin, uint8_t value) override {
        if (!initialized || pin == error_pin) return false;
        pin_states[pin] = value;
        write_history.push_back({pin, value});
        return true;
    }
    
    uint8_t digitalRead(uint8_t pin) override {
        if (!initialized || pin == error_pin) return 0;
        return pin_states[pin];
    }
    
    bool togglePin(uint8_t pin) override {
        if (!initialized || pin == error_pin) return false;
        pin_states[pin] = !pin_states[pin];
        write_history.push_back({pin, pin_states[pin]});
        return true;
    }
    
    bool sonicatorStart(uint8_t sonicator_id) override {
        return digitalWrite(getSonicatorStartPin(sonicator_id), 1);
    }
    
    bool sonicatorStop(uint8_t sonicator_id) override {
        return digitalWrite(getSonicatorStartPin(sonicator_id), 0);
    }
    
    bool sonicatorReset(uint8_t sonicator_id) override {
        return digitalWrite(getSonicatorResetPin(sonicator_id), 1);
    }
    
    bool sonicatorReadOverload(uint8_t sonicator_id, bool* state) override {
        if (!state || !initialized) return false;
        *state = digitalRead(getSonicatorOverloadPin(sonicator_id));
        return true;
    }
    
    bool sonicatorReadFreqLock(uint8_t sonicator_id, bool* state) override {
        if (!state || !initialized) return false;
        *state = digitalRead(getSonicatorFreqLockPin(sonicator_id));
        return true;
    }
    
    bool statusLed(bool state) override {
        return digitalWrite(13, state ? 1 : 0); // Status LED pin
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
    
    void setErrorPin(uint8_t pin) { error_pin = pin; }
    void clearErrorPin() { error_pin = 255; }
    
    void reset() {
        pin_modes.clear();
        pin_states.clear();
        write_history.clear();
        initialized = false;
        error_pin = 255;
    }
    
private:
    uint8_t getSonicatorStartPin(uint8_t id) {
        // Map sonicator ID to start pins based on pin matrix
        switch(id) {
            case 1: return 28; // PC6
            case 2: return 26; // PC4
            case 3: return 24; // PC2
            case 4: return 22; // PC0
            default: return 255;
        }
    }
    
    uint8_t getSonicatorResetPin(uint8_t id) {
        // Map sonicator ID to reset pins based on pin matrix
        switch(id) {
            case 1: return 29; // PC7
            case 2: return 27; // PC5
            case 3: return 25; // PC3
            case 4: return 23; // PC1
            default: return 255;
        }
    }
    
    uint8_t getSonicatorOverloadPin(uint8_t id) {
        // Map sonicator ID to overload pins based on pin matrix
        switch(id) {
            case 1: return 20; // PD6
            case 2: return 19; // PD5
            case 3: return 18; // PD4
            case 4: return 17; // PD3
            default: return 255;
        }
    }
    
    uint8_t getSonicatorFreqLockPin(uint8_t id) {
        // Map sonicator ID to frequency lock pins based on pin matrix
        switch(id) {
            case 1: return 8;  // PB7
            case 2: return 7;  // PB6
            case 3: return 6;  // PB5
            case 4: return 5;  // PB4
            default: return 255;
        }
    }
};

/**
 * @brief Mock ADC implementation for testing
 */
class MockAdcReader : public IAdcReader {
private:
    std::map<uint8_t, uint16_t> channel_values;
    bool initialized;
    uint8_t error_channel; // Channel that will always fail

public:
    MockAdcReader() : initialized(false), error_channel(255) {}
    
    bool init(void) override {
        initialized = true;
        return true;
    }
    
    bool readChannel(uint8_t channel, uint16_t* raw_value) override {
        if (!initialized || !raw_value || channel == error_channel) return false;
        *raw_value = channel_values[channel];
        return true;
    }
    
    bool startConversion(uint8_t channel) override {
        return (initialized && channel != error_channel);
    }
    
    bool isConversionComplete(void) override {
        return initialized;
    }
    
    float rawToVoltage(uint16_t raw_value) override {
        return (raw_value * 5.0f) / 1023.0f; // 10-bit ADC, 5V reference
    }
    
    bool readSonicatorPower(uint8_t sonicator_id, float* power_watts) override {
        if (!power_watts || !initialized) return false;
        uint16_t raw;
        if (!readChannel(sonicator_id + 3, &raw)) return false; // ADC channels 4-7
        float voltage = rawToVoltage(raw);
        *power_watts = voltage / 0.00544f; // 5.44 mV/W scaling from pin matrix
        return true;
    }
    
    bool readFrequency(float* frequency_hz) override {
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
    
    void setErrorChannel(uint8_t channel) { error_channel = channel; }
    void clearErrorChannel() { error_channel = 255; }
    
    bool isInitialized() { return initialized; }
    
    void reset() {
        channel_values.clear();
        initialized = false;
        error_channel = 255;
    }
};

/**
 * @brief Mock PWM implementation for testing
 */
class MockPwmGenerator : public IPwmGenerator {
private:
    std::map<uint8_t, uint32_t> channel_frequencies;
    std::map<uint8_t, uint8_t> channel_duty_cycles;
    std::map<uint8_t, bool> channel_enabled;
    bool initialized;
    bool emergency_stopped;
    uint8_t amplitude_percent;

public:
    MockPwmGenerator() : initialized(false), emergency_stopped(false), amplitude_percent(0) {}
    
    bool init(void) override {
        initialized = true;
        emergency_stopped = false;
        return true;
    }
    
    bool setFrequency(uint8_t channel, uint32_t frequency) override {
        if (!initialized || emergency_stopped) return false;
        channel_frequencies[channel] = frequency;
        return true;
    }
    
    bool setDutyCycle(uint8_t channel, uint8_t duty_percent) override {
        if (!initialized || emergency_stopped || duty_percent > 100) return false;
        channel_duty_cycles[channel] = duty_percent;
        return true;
    }
    
    bool enableChannel(uint8_t channel) override {
        if (!initialized || emergency_stopped) return false;
        channel_enabled[channel] = true;
        return true;
    }
    
    bool disableChannel(uint8_t channel) override {
        if (!initialized) return false;
        channel_enabled[channel] = false;
        return true;
    }
    
    bool setAmplitude(uint8_t amplitude_percent) override {
        if (!initialized || emergency_stopped || amplitude_percent > 100) return false;
        this->amplitude_percent = amplitude_percent;
        return setDutyCycle(0, amplitude_percent); // Amplitude channel
    }
    
    bool emergencyStop(void) override {
        emergency_stopped = true;
        amplitude_percent = 0;
        // Disable all channels
        for (auto& pair : channel_enabled) {
            pair.second = false;
        }
        return true;
    }
    
    bool testPattern(void) override {
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
// MOCK FACTORY
// ============================================================================

/**
 * @brief Factory for creating mock HAL instances for testing
 */
class MockHalFactory : public IHalFactory {
private:
    MockGpioPort gpio_port;
    MockAdcReader adc_reader;
    MockPwmGenerator pwm_generator;
    // Note: UART and Timer mocks would be similar but simpler for this example

public:
    IGpioPort* createGpioPort() override {
        return &gpio_port;
    }
    
    IAdcReader* createAdcReader() override {
        return &adc_reader;
    }
    
    IPwmGenerator* createPwmGenerator() override {
        return &pwm_generator;
    }
    
    IUartPort* createUartPort() override {
        // Return nullptr for now - would implement MockUartPort
        return nullptr;
    }
    
    ITimerManager* createTimerManager() override {
        // Return nullptr for now - would implement MockTimerManager
        return nullptr;
    }
    
    // Test helpers for accessing mocks
    MockGpioPort* getMockGpio() { return &gpio_port; }
    MockAdcReader* getMockAdc() { return &adc_reader; }
    MockPwmGenerator* getMockPwm() { return &pwm_generator; }
};

#endif // HAL_MOCK_FACTORY_H
