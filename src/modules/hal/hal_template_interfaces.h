/**
 * @file hal_template_interfaces.h
 * @brief Template-Based HAL Interfaces for Zero-Overhead Polymorphism
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 1.0.0
 *
 * Template-based HAL interfaces that provide abstraction and testability
 * without the runtime overhead of virtual methods. Uses compile-time
 * polymorphism through C++ templates.
 */

#ifndef HAL_TEMPLATE_INTERFACES_H
#define HAL_TEMPLATE_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>

// ============================================================================
// TEMPLATE-BASED GPIO INTERFACE
// ============================================================================

/**
 * @brief GPIO HAL Interface (Template-based)
 *
 * Template parameter allows different implementations at compile time
 * with zero runtime overhead.
 */
template<typename HalImpl>
class GpioHal {
public:
    // Core GPIO operations - direct calls to implementation
    bool init() { return HalImpl::gpioInit(); }
    bool setPinMode(uint8_t pin, uint8_t mode) { return HalImpl::gpioSetPinMode(pin, mode); }
    bool digitalWrite(uint8_t pin, uint8_t value) { return HalImpl::gpioDigitalWrite(pin, value); }
    uint8_t digitalRead(uint8_t pin) { return HalImpl::gpioDigitalRead(pin); }
    bool togglePin(uint8_t pin) { return HalImpl::gpioTogglePin(pin); }

    // Sonicator-specific operations
    bool sonicatorStart(uint8_t id) { return HalImpl::gpioSonicatorStart(id); }
    bool sonicatorStop(uint8_t id) { return HalImpl::gpioSonicatorStop(id); }
    bool sonicatorReset(uint8_t id) { return HalImpl::gpioSonicatorReset(id); }
    bool sonicatorReadOverload(uint8_t id, bool* state) { return HalImpl::gpioSonicatorReadOverload(id, state); }
    bool sonicatorReadFreqLock(uint8_t id, bool* state) { return HalImpl::gpioSonicatorReadFreqLock(id, state); }
    bool statusLed(bool state) { return HalImpl::gpioStatusLed(state); }
};

// ============================================================================
// TEMPLATE-BASED ADC INTERFACE
// ============================================================================

/**
 * @brief ADC HAL Interface (Template-based)
 */
template<typename HalImpl>
class AdcHal {
public:
    // Core ADC operations
    bool init() { return HalImpl::adcInit(); }
    bool readChannel(uint8_t channel, uint16_t* value) { return HalImpl::adcReadChannel(channel, value); }
    bool startConversion(uint8_t channel) { return HalImpl::adcStartConversion(channel); }
    bool isConversionComplete() { return HalImpl::adcIsConversionComplete(); }
    float rawToVoltage(uint16_t raw) { return HalImpl::adcRawToVoltage(raw); }

    // Application-specific operations
    bool readSonicatorPower(uint8_t id, float* power) { return HalImpl::adcReadSonicatorPower(id, power); }
    bool readFrequency(float* frequency) { return HalImpl::adcReadFrequency(frequency); }
    bool calibrate(float reference_voltage) { return HalImpl::adcCalibrate(reference_voltage); }
};

// ============================================================================
// TEMPLATE-BASED PWM INTERFACE
// ============================================================================

/**
 * @brief PWM HAL Interface (Template-based)
 */
template<typename HalImpl>
class PwmHal {
public:
    // Core PWM operations
    bool init() { return HalImpl::pwmInit(); }
    bool setFrequency(uint8_t channel, uint32_t frequency) { return HalImpl::pwmSetFrequency(channel, frequency); }
    bool setDutyCycle(uint8_t channel, uint8_t duty) { return HalImpl::pwmSetDutyCycle(channel, duty); }
    bool enableChannel(uint8_t channel) { return HalImpl::pwmEnableChannel(channel); }
    bool disableChannel(uint8_t channel) { return HalImpl::pwmDisableChannel(channel); }

    // Application-specific operations
    bool setAmplitude(uint8_t amplitude) { return HalImpl::pwmSetAmplitude(amplitude); }
    bool getAmplitude(uint8_t* amplitude) { return HalImpl::pwmGetAmplitude(amplitude); }
    bool amplitudeRamp(uint8_t target, uint16_t ramp_time) { return HalImpl::pwmAmplitudeRamp(target, ramp_time); }
    bool rampComplete(bool* complete) { return HalImpl::pwmRampComplete(complete); }
    bool emergencyStop() { return HalImpl::pwmEmergencyStop(); }

    // Diagnostic operations
    bool dutyToVoltage(uint8_t duty, float* voltage) { return HalImpl::pwmDutyToVoltage(duty, voltage); }
    bool amplitudeToVoltage(uint8_t amplitude, float* voltage) { return HalImpl::pwmAmplitudeToVoltage(amplitude, voltage); }
    bool testPattern() { return HalImpl::pwmTestPattern(); }
};

// ============================================================================
// TEMPLATE-BASED TIMER INTERFACE
// ============================================================================

/**
 * @brief Timer HAL Interface (Template-based)
 */
template<typename HalImpl>
class TimerHal {
public:
    // Core timer operations
    bool init() { return HalImpl::timerInit(); }
    bool configure(timer_id_t timer_id, const timer_config_t* config) { return HalImpl::timerConfigure(timer_id, config); }
    bool start(timer_id_t timer_id, uint32_t period_ms) { return HalImpl::timerStart(timer_id, period_ms); }
    bool stop(timer_id_t timer_id) { return HalImpl::timerStop(timer_id); }
    bool expired(timer_id_t timer_id, bool* expired) { return HalImpl::timerExpired(timer_id, expired); }
    bool reset(timer_id_t timer_id) { return HalImpl::timerReset(timer_id); }
    bool getValue(timer_id_t timer_id, uint16_t* value) { return HalImpl::timerGetValue(timer_id, value); }
    bool getElapsed(timer_id_t timer_id, uint32_t* elapsed_ms) { return HalImpl::timerGetElapsed(timer_id, elapsed_ms); }

    // System timing operations
    bool getMillis(uint32_t* uptime_ms) { return HalImpl::timerGetMillis(uptime_ms); }
    bool getMicros(uint32_t* uptime_us) { return HalImpl::timerGetMicros(uptime_us); }
    bool delayMs(uint32_t delay_ms) { return HalImpl::timerDelayMs(delay_ms); }
    bool delayUs(uint32_t delay_us) { return HalImpl::timerDelayUs(delay_us); }
    bool delayComplete(uint32_t start_time_ms, uint32_t delay_ms, bool* complete) {
        return HalImpl::timerDelayComplete(start_time_ms, delay_ms, complete);
    }

    // Scheduler operations
    bool schedulerInit() { return HalImpl::timerSchedulerInit(); }
    bool schedulerTick(bool* tick_available) { return HalImpl::timerSchedulerTick(tick_available); }
    bool schedulerGetTicks(uint32_t* tick_count) { return HalImpl::timerSchedulerGetTicks(tick_count); }
    bool schedulerReset() { return HalImpl::timerSchedulerReset(); }

    // Watchdog operations
    bool watchdogInit() { return HalImpl::timerWatchdogInit(); }
    bool watchdogReset() { return HalImpl::timerWatchdogReset(); }
    bool watchdogExpired(bool* expired) { return HalImpl::timerWatchdogExpired(expired); }
    bool watchdogTimeRemaining(uint32_t* time_remaining_ms) { return HalImpl::timerWatchdogTimeRemaining(time_remaining_ms); }
};

// ============================================================================
// TEMPLATE-BASED UART INTERFACE
// ============================================================================

/**
 * @brief UART HAL Interface (Template-based)
 */
template<typename HalImpl>
class UartHal {
public:
    // Core UART operations
    bool init(uint32_t baud_rate) { return HalImpl::uartInit(baud_rate); }
    bool setBaudRate(uint32_t baud_rate) { return HalImpl::uartSetBaudRate(baud_rate); }
    bool sendByte(uint8_t data) { return HalImpl::uartSendByte(data); }
    bool receiveByte(uint8_t* data) { return HalImpl::uartReceiveByte(data); }
    bool dataAvailable(bool* available) { return HalImpl::uartDataAvailable(available); }
    bool getRxCount(uint16_t* count) { return HalImpl::uartGetRxCount(count); }
    bool sendBuffer(const uint8_t* buffer, uint16_t length) { return HalImpl::uartSendBuffer(buffer, length); }
    bool receiveBuffer(uint8_t* buffer, uint16_t max_length, uint16_t* received) {
        return HalImpl::uartReceiveBuffer(buffer, max_length, received);
    }
    bool flushTx() { return HalImpl::uartFlushTx(); }
    bool flushRx() { return HalImpl::uartFlushRx(); }

    // MODBUS RTU operations
    bool sendModbusFrame(const uint8_t* frame, uint16_t length) { return HalImpl::uartSendModbusFrame(frame, length); }
    bool receiveModbusFrame(uint8_t* frame, uint16_t max_length, uint16_t* received, uint32_t timeout_ms) {
        return HalImpl::uartReceiveModbusFrame(frame, max_length, received, timeout_ms);
    }
    bool modbusGapDetected(bool* gap_detected) { return HalImpl::uartModbusGapDetected(gap_detected); }
    bool getIdleTime(uint32_t* time_us) { return HalImpl::uartGetIdleTime(time_us); }

    // Diagnostic operations
    bool getStatus(bool* tx_busy, bool* rx_overflow, bool* frame_error, bool* parity_error) {
        return HalImpl::uartGetStatus(tx_busy, rx_overflow, frame_error, parity_error);
    }
    bool getTxFree(uint16_t* free_bytes) { return HalImpl::uartGetTxFree(free_bytes); }
    bool clearErrors() { return HalImpl::uartClearErrors(); }
    bool getCharTime(uint16_t* char_time_us) { return HalImpl::uartGetCharTime(char_time_us); }
    bool testLoopback(bool* success) { return HalImpl::uartTestLoopback(success); }
};

// ============================================================================
// COMPLETE HAL TEMPLATE (COMBINES ALL PERIPHERALS)
// ============================================================================

/**
 * @brief Complete HAL Template (combines all peripherals)
 *
 * Provides a unified interface to all HAL peripherals using templates
 * for zero-overhead polymorphism.
 */
template<typename HalImpl>
class HalTemplate : public GpioHal<HalImpl>,
                   public AdcHal<HalImpl>,
                   public PwmHal<HalImpl>,
                   public TimerHal<HalImpl>,
                   public UartHal<HalImpl> {
public:
    // Unified initialization
    bool begin() {
        bool gpio_ok = GpioHal<HalImpl>::init();
        bool adc_ok = AdcHal<HalImpl>::init();
        bool pwm_ok = PwmHal<HalImpl>::init();
        bool timer_ok = TimerHal<HalImpl>::init();
        // UART initialization requires baud rate, so skip here
        return gpio_ok && adc_ok && pwm_ok && timer_ok;
    }

    // Individual peripheral initialization
    bool beginGpio() { return GpioHal<HalImpl>::init(); }
    bool beginAdc() { return AdcHal<HalImpl>::init(); }
    bool beginPwm() { return PwmHal<HalImpl>::init(); }
    bool beginTimer() { return TimerHal<HalImpl>::init(); }
    bool beginUart(uint32_t baud_rate) { return UartHal<HalImpl>::init(baud_rate); }
};

#endif // HAL_TEMPLATE_INTERFACES_H
