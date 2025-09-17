/**
 * @file hal_hardware_factory.h
 * @brief Hardware Factory Implementation for Real ATmega32A Hardware
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * Concrete factory implementation for creating real hardware HAL instances.
 * Following dependency injection pattern from unity-testing-guide.md.
 */

#ifndef HAL_HARDWARE_FACTORY_H
#define HAL_HARDWARE_FACTORY_H

#include "hal_interfaces.h"

// ============================================================================
// HARDWARE IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Real ATmega32A GPIO implementation
 * 
 * Provides real hardware implementation for GPIO operations.
 */
class HardwareGpioPort : public IGpioPort {
public:
    bool init(void) override;
    bool togglePin(uint8_t pin) override;
    uint8_t digitalRead(uint8_t pin) override;
    bool setPinMode(uint8_t pin, uint8_t mode) override;
    bool digitalWrite(uint8_t pin, uint8_t value) override;
    
    bool sonicatorStart(uint8_t sonicator_id) override;
    bool sonicatorStop(uint8_t sonicator_id) override;
    bool sonicatorReset(uint8_t sonicator_id) override;
    bool sonicatorReadOverload(uint8_t sonicator_id, bool* state) override;
    bool sonicatorReadFreqLock(uint8_t sonicator_id, bool* state) override;
    bool statusLed(bool state) override;
};

/**
 * @brief Real ATmega32A ADC implementation
 */
class HardwareAdcReader : public IAdcReader {
public:
    bool init(void) override;
    bool readChannel(uint8_t channel, uint16_t* raw_value) override;
    bool startConversion(uint8_t channel) override;
    bool isConversionComplete(void) override;
    float rawToVoltage(uint16_t raw_value) override;
    
    bool readSonicatorPower(uint8_t sonicator_id, float* power_watts) override;
    bool readFrequency(float* frequency_hz) override;
};

/**
 * @brief Real ATmega32A PWM implementation
 */
class HardwarePwmGenerator : public IPwmGenerator {
public:
    bool init(void) override;
    bool setFrequency(uint8_t channel, uint32_t frequency) override;
    bool setDutyCycle(uint8_t channel, uint8_t duty_percent) override;
    bool enableChannel(uint8_t channel) override;
    bool disableChannel(uint8_t channel) override;
    
    bool setAmplitude(uint8_t amplitude_percent) override;
    bool emergencyStop(void) override;
    bool testPattern(void) override;
};

/**
 * @brief Real ATmega32A UART implementation
 */
class HardwareUartPort : public IUartPort {
public:
    bool init(uint32_t baud_rate) override;
    bool sendByte(uint8_t data) override;
    bool receiveByte(uint8_t* data) override;
    bool dataAvailable(void) override;
    bool sendBuffer(const uint8_t* buffer, uint16_t length) override;
    bool receiveBuffer(uint8_t* buffer, uint16_t max_length, uint16_t* received) override;
    
    bool flushTx(void) override;
    bool flushRx(void) override;
    bool clearErrors(void) override;
    bool testLoopback(bool* success) override;
};

/**
 * @brief Real ATmega32A Timer implementation
 */
class HardwareTimerManager : public ITimerManager {
public:
    bool init(void) override;
    bool start(uint8_t timer_id, uint32_t period_ms) override;
    bool stop(uint8_t timer_id) override;
    bool isExpired(uint8_t timer_id) override;
    bool getMillis(uint32_t* millis) override;
    bool delayMs(uint32_t delay_ms) override;
    
    bool watchdogReset(void) override;
    bool testAll(bool* success) override;
};

// ============================================================================
// HARDWARE FACTORY
// ============================================================================

/**
 * @brief Factory for creating real hardware HAL instances
 * 
 * Implements the factory pattern from unity-testing-guide.md
 * for dependency injection of real hardware implementations.
 */
class HardwareHalFactory : public IHalFactory {
private:
    HardwareGpioPort gpio_port;
    HardwareAdcReader adc_reader;
    HardwarePwmGenerator pwm_generator;
    HardwareUartPort uart_port;
    HardwareTimerManager timer_manager;

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
        return &uart_port;
    }
    
    ITimerManager* createTimerManager() override {
        return &timer_manager;
    }
};

#endif // HAL_HARDWARE_FACTORY_H
