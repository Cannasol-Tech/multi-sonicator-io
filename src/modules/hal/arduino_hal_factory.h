/**
 * @file arduino_hal_factory.h
 * @brief Arduino Framework HAL Factory Implementation
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 1.0.0
 *
 * Factory implementation for creating Arduino-based HAL instances.
 * Uses the ArduinoHal static methods to provide HAL functionality.
 */

#ifndef ARDUINO_HAL_FACTORY_H
#define ARDUINO_HAL_FACTORY_H

#include "hal_interfaces.h"
#include "arduino_hal_impl.h"

// ============================================================================
// ARDUINO IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Arduino GPIO implementation using Arduino framework
 */
class ArduinoGpioPort : public IGpioPort {
public:
    bool init(void) override {
        return ArduinoHal::gpioInit();
    }

    bool setPinMode(uint8_t pin, uint8_t mode) override {
        return ArduinoHal::gpioSetPinMode(pin, mode);
    }

    bool digitalWrite(uint8_t pin, uint8_t value) override {
        return ArduinoHal::gpioDigitalWrite(pin, value);
    }

    uint8_t digitalRead(uint8_t pin) override {
        return ArduinoHal::gpioDigitalRead(pin);
    }

    bool togglePin(uint8_t pin) override {
        return ArduinoHal::gpioTogglePin(pin);
    }

    bool sonicatorStart(uint8_t sonicator_id) override {
        return ArduinoHal::gpioSonicatorStart(sonicator_id);
    }

    bool sonicatorStop(uint8_t sonicator_id) override {
        return ArduinoHal::gpioSonicatorStop(sonicator_id);
    }

    bool sonicatorReset(uint8_t sonicator_id) override {
        return ArduinoHal::gpioSonicatorReset(sonicator_id);
    }

    bool sonicatorReadOverload(uint8_t sonicator_id, bool* state) override {
        return ArduinoHal::gpioSonicatorReadOverload(sonicator_id, state);
    }

    bool sonicatorReadFreqLock(uint8_t sonicator_id, bool* state) override {
        return ArduinoHal::gpioSonicatorReadFreqLock(sonicator_id, state);
    }

    bool statusLed(bool state) override {
        return ArduinoHal::gpioStatusLed(state);
    }
};

/**
 * @brief Arduino ADC implementation using Arduino framework
 */
class ArduinoAdcReader : public IAdcReader {
public:
    bool init(void) override {
        return ArduinoHal::adcInit();
    }

    bool readChannel(uint8_t channel, uint16_t* raw_value) override {
        return ArduinoHal::adcReadChannel(channel, raw_value);
    }

    bool startConversion(uint8_t channel) override {
        return ArduinoHal::adcStartConversion(channel);
    }

    bool isConversionComplete(void) override {
        return ArduinoHal::adcIsConversionComplete();
    }

    float rawToVoltage(uint16_t raw_value) override {
        return ArduinoHal::adcRawToVoltage(raw_value);
    }

    bool readSonicatorPower(uint8_t sonicator_id, float* power_watts) override {
        return ArduinoHal::adcReadSonicatorPower(sonicator_id, power_watts);
    }

    bool readFrequency(float* frequency_hz) override {
        return ArduinoHal::adcReadFrequency(frequency_hz);
    }
};

/**
 * @brief Arduino PWM implementation using Arduino framework
 */
class ArduinoPwmGenerator : public IPwmGenerator {
public:
    bool init(void) override {
        return ArduinoHal::pwmInit();
    }

    bool setFrequency(uint8_t channel, uint32_t frequency) override {
        return ArduinoHal::pwmSetFrequency(channel, frequency);
    }

    bool setDutyCycle(uint8_t channel, uint8_t duty_percent) override {
        return ArduinoHal::pwmSetDutyCycle(channel, duty_percent);
    }

    bool enableChannel(uint8_t channel) override {
        return ArduinoHal::pwmEnableChannel(channel);
    }

    bool disableChannel(uint8_t channel) override {
        return ArduinoHal::pwmDisableChannel(channel);
    }

    bool setAmplitude(uint8_t amplitude_percent) override {
        return ArduinoHal::pwmSetAmplitude(amplitude_percent);
    }

    bool emergencyStop(void) override {
        return ArduinoHal::pwmEmergencyStop();
    }

    bool testPattern(void) override {
        return ArduinoHal::pwmTestPattern();
    }
};

/**
 * @brief Arduino UART implementation using Arduino framework
 */
class ArduinoUartPort : public IUartPort {
public:
    bool init(uint32_t baud_rate) override {
        return ArduinoHal::uartInit(baud_rate);
    }

    bool sendByte(uint8_t data) override {
        return ArduinoHal::uartSendByte(data);
    }

    bool receiveByte(uint8_t* data) override {
        return ArduinoHal::uartReceiveByte(data);
    }

    bool dataAvailable(void) override {
        return ArduinoHal::uartDataAvailable();
    }

    bool sendBuffer(const uint8_t* buffer, uint16_t length) override {
        return ArduinoHal::uartSendBuffer(buffer, length);
    }

    bool receiveBuffer(uint8_t* buffer, uint16_t max_length, uint16_t* received) override {
        return ArduinoHal::uartReceiveBuffer(buffer, max_length, received);
    }

    bool flushTx(void) override {
        return ArduinoHal::uartFlushTx();
    }

    bool flushRx(void) override {
        return ArduinoHal::uartFlushRx();
    }

    bool clearErrors(void) override {
        return ArduinoHal::uartClearErrors();
    }

    bool testLoopback(bool* success) override {
        return ArduinoHal::uartTestLoopback(success);
    }
};

/**
 * @brief Arduino Timer implementation using Arduino framework
 */
class ArduinoTimerManager : public ITimerManager {
public:
    bool init(void) override {
        return ArduinoHal::timerInit();
    }

    bool start(uint8_t timer_id, uint32_t period_ms) override {
        return ArduinoHal::timerStart(timer_id, period_ms);
    }

    bool stop(uint8_t timer_id) override {
        return ArduinoHal::timerStop(timer_id);
    }

    bool isExpired(uint8_t timer_id) override {
        return ArduinoHal::timerIsExpired(timer_id);
    }

    bool getMillis(uint32_t* millis) override {
        return ArduinoHal::timerGetMillis(millis);
    }

    bool delayMs(uint32_t delay_ms) override {
        return ArduinoHal::timerDelayMs(delay_ms);
    }

    bool watchdogReset(void) override {
        return ArduinoHal::timerWatchdogReset();
    }

    bool testAll(bool* success) override {
        return ArduinoHal::timerTestAll(success);
    }
};

// ============================================================================
// ARDUINO FACTORY
// ============================================================================

/**
 * @brief Factory for creating Arduino-based HAL instances
 *
 * Implements the factory pattern for dependency injection of Arduino HAL implementations.
 */
class ArduinoHalFactory : public IHalFactory {
private:
    ArduinoGpioPort gpio_port;
    ArduinoAdcReader adc_reader;
    ArduinoPwmGenerator pwm_generator;
    ArduinoUartPort uart_port;
    ArduinoTimerManager timer_manager;

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

#endif // ARDUINO_HAL_FACTORY_H