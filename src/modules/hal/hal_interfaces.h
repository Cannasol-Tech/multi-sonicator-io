/**
 * @file hal_interfaces.h
 * @brief Hardware Abstraction Layer Interfaces for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * Following unity-testing-guide.md patterns for hardware abstraction interfaces
 * to enable proper unit testing with mock objects and dependency injection.
 */

#ifndef HAL_INTERFACES_H
#define HAL_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// GPIO INTERFACE
// ============================================================================

/**
 * @brief GPIO abstraction interface for testability
 * 
 * Provides abstract interface for GPIO operations to enable mocking
 * for unit tests while supporting real hardware implementation.
 */
class IGpioPort {
public:
    virtual ~IGpioPort() = default;
    
    // Core GPIO operations
    virtual bool init(void) = 0;
    virtual bool setPinMode(uint8_t pin, uint8_t mode) = 0;
    virtual bool digitalWrite(uint8_t pin, uint8_t value) = 0;
    virtual uint8_t digitalRead(uint8_t pin) = 0;
    virtual bool togglePin(uint8_t pin) = 0;
    
    // Sonicator-specific operations
    virtual bool sonicatorStart(uint8_t sonicator_id) = 0;
    virtual bool sonicatorStop(uint8_t sonicator_id) = 0;
    virtual bool sonicatorReset(uint8_t sonicator_id) = 0;
    virtual bool sonicatorReadOverload(uint8_t sonicator_id, bool* state) = 0;
    virtual bool sonicatorReadFreqLock(uint8_t sonicator_id, bool* state) = 0;
    virtual bool statusLed(bool state) = 0;
};

// ============================================================================
// ADC INTERFACE
// ============================================================================

/**
 * @brief ADC abstraction interface for testability
 */
class IAdcReader {
public:
    virtual ~IAdcReader() = default;
    
    // Core ADC operations
    virtual bool init(void) = 0;
    virtual bool readChannel(uint8_t channel, uint16_t* raw_value) = 0;
    virtual bool startConversion(uint8_t channel) = 0;
    virtual bool isConversionComplete(void) = 0;
    virtual float rawToVoltage(uint16_t raw_value) = 0;
    
    // Application-specific operations
    virtual bool readSonicatorPower(uint8_t sonicator_id, float* power_watts) = 0;
    virtual bool readFrequency(float* frequency_hz) = 0;
};

// ============================================================================
// PWM INTERFACE
// ============================================================================

/**
 * @brief PWM abstraction interface for testability
 */
class IPwmGenerator {
public:
    virtual ~IPwmGenerator() = default;
    
    // Core PWM operations
    virtual bool init(void) = 0;
    virtual bool setFrequency(uint8_t channel, uint32_t frequency) = 0;
    virtual bool setDutyCycle(uint8_t channel, uint8_t duty_percent) = 0;
    virtual bool enableChannel(uint8_t channel) = 0;
    virtual bool disableChannel(uint8_t channel) = 0;
    
    // Application-specific operations
    virtual bool setAmplitude(uint8_t amplitude_percent) = 0;
    virtual bool emergencyStop(void) = 0;
    virtual bool testPattern(void) = 0;
};

// ============================================================================
// UART INTERFACE
// ============================================================================

/**
 * @brief UART abstraction interface for testability
 */
class IUartPort {
public:
    virtual ~IUartPort() = default;
    
    // Core UART operations
    virtual bool init(uint32_t baud_rate) = 0;
    virtual bool sendByte(uint8_t data) = 0;
    virtual bool receiveByte(uint8_t* data) = 0;
    virtual bool dataAvailable(void) = 0;
    virtual bool sendBuffer(const uint8_t* buffer, uint16_t length) = 0;
    virtual bool receiveBuffer(uint8_t* buffer, uint16_t max_length, uint16_t* received) = 0;
    
    // Flow control and status
    virtual bool flushTx(void) = 0;
    virtual bool flushRx(void) = 0;
    virtual bool clearErrors(void) = 0;
    virtual bool testLoopback(bool* success) = 0;
};

// ============================================================================
// TIMER INTERFACE
// ============================================================================

/**
 * @brief Timer abstraction interface for testability
 */
class ITimerManager {
public:
    virtual ~ITimerManager() = default;
    
    // Core timer operations
    virtual bool init(void) = 0;
    virtual bool start(uint8_t timer_id, uint32_t period_ms) = 0;
    virtual bool stop(uint8_t timer_id) = 0;
    virtual bool isExpired(uint8_t timer_id) = 0;
    virtual bool getMillis(uint32_t* millis) = 0;
    virtual bool delayMs(uint32_t delay_ms) = 0;
    
    // Watchdog operations
    virtual bool watchdogReset(void) = 0;
    virtual bool testAll(bool* success) = 0;
};

// ============================================================================
// HAL FACTORY INTERFACE
// ============================================================================

/**
 * @brief Factory interface for creating HAL instances
 * 
 * Enables dependency injection pattern from unity-testing-guide.md
 * for creating either real hardware or mock implementations.
 */
class IHalFactory {
public:
    virtual ~IHalFactory() = default;
    
    virtual IGpioPort* createGpioPort() = 0;
    virtual IAdcReader* createAdcReader() = 0;
    virtual IPwmGenerator* createPwmGenerator() = 0;
    virtual IUartPort* createUartPort() = 0;
    virtual ITimerManager* createTimerManager() = 0;
};

// ============================================================================
// RESULT TYPES
// ============================================================================

typedef enum {
    HAL_INTERFACE_OK = 0,
    HAL_INTERFACE_ERROR,
    HAL_INTERFACE_INVALID_PARAM,
    HAL_INTERFACE_NOT_INITIALIZED,
    HAL_INTERFACE_TIMEOUT,
    HAL_INTERFACE_HARDWARE_FAULT
} hal_interface_result_t;

#endif // HAL_INTERFACES_H
