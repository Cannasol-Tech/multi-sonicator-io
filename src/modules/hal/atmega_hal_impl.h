/**
 * @file atmega_hal_impl.h
 * @brief ATmega32A HAL Implementation for Template-Based Design
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 1.0.0
 *
 * Concrete implementation of HAL operations for ATmega32A microcontroller.
 * Contains static methods that can be used with template-based HAL interfaces
 * for zero-overhead polymorphism.
 */

#ifndef ATMEGA_HAL_IMPL_H
#define ATMEGA_HAL_IMPL_H

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"
#include "adc.h"
#include "pwm.h"
#include "timer.h"
#include "uart.h"

class AtmegaHal {
public:
    // ============================================================================
    // GPIO IMPLEMENTATION
    // ============================================================================

    static bool gpioInit() {
        return gpio_init() == GPIO_OK;
    }

    static bool gpioSetPinMode(uint8_t pin, uint8_t mode) {
        gpio_mode_t gpio_mode;
        switch (mode) {
            case 0: gpio_mode = GPIO_MODE_INPUT; break;
            case 1: gpio_mode = GPIO_MODE_OUTPUT; break;
            case 2: gpio_mode = GPIO_MODE_INPUT_PULLUP; break;
            default: return false;
        }
        return gpio_set_pin_mode(pin, gpio_mode) == GPIO_OK;
    }

    static bool gpioDigitalWrite(uint8_t pin, uint8_t value) {
        gpio_state_t state = (value == 0) ? GPIO_LOW : GPIO_HIGH;
        return gpio_write_pin(pin, state) == GPIO_OK;
    }

    static uint8_t gpioDigitalRead(uint8_t pin) {
        gpio_state_t state;
        if (gpio_read_pin(pin, &state) != GPIO_OK) {
            return 0; // Default to LOW on error
        }
        return (state == GPIO_HIGH) ? 1 : 0;
    }

    static bool gpioTogglePin(uint8_t pin) {
        return gpio_toggle_pin(pin) == GPIO_OK;
    }

    // Sonicator-specific GPIO operations
    static bool gpioSonicatorStart(uint8_t sonicator_id) {
        return gpio_sonicator_start(sonicator_id) == GPIO_OK;
    }

    static bool gpioSonicatorStop(uint8_t sonicator_id) {
        return gpio_sonicator_stop(sonicator_id) == GPIO_OK;
    }

    static bool gpioSonicatorReset(uint8_t sonicator_id) {
        return gpio_sonicator_reset(sonicator_id) == GPIO_OK;
    }

    static bool gpioSonicatorReadOverload(uint8_t sonicator_id, bool* state) {
        return gpio_sonicator_read_overload(sonicator_id, state) == GPIO_OK;
    }

    static bool gpioSonicatorReadFreqLock(uint8_t sonicator_id, bool* state) {
        return gpio_sonicator_read_freq_lock(sonicator_id, state) == GPIO_OK;
    }

    static bool gpioStatusLed(bool state) {
        gpio_state_t gpio_state = state ? GPIO_HIGH : GPIO_LOW;
        return gpio_status_led(gpio_state) == GPIO_OK;
    }

    // ============================================================================
    // ADC IMPLEMENTATION
    // ============================================================================

    static bool adcInit() {
        return adc_init() == ADC_OK;
    }

    static bool adcReadChannel(uint8_t channel, uint16_t* value) {
        return adc_read_channel((adc_channel_t)channel, value) == ADC_OK;
    }

    static bool adcStartConversion(uint8_t channel) {
        return adc_start_conversion((adc_channel_t)channel) == ADC_OK;
    }

    static bool adcIsConversionComplete() {
        return adc_conversion_complete();
    }

    static float adcRawToVoltage(uint16_t raw_value) {
        float voltage;
        if (adc_to_voltage(raw_value, &voltage) == ADC_OK) {
            return voltage;
        }
        return 0.0f;
    }

    // Application-specific ADC operations
    static bool adcReadSonicatorPower(uint8_t sonicator_id, float* power_watts) {
        return adc_read_sonicator_power(sonicator_id, power_watts) == ADC_OK;
    }

    static bool adcReadFrequency(float* frequency_hz) {
        return adc_read_frequency(frequency_hz) == ADC_OK;
    }

    static bool adcCalibrate(float reference_voltage) {
        return adc_calibrate(reference_voltage) == ADC_OK;
    }

    // ============================================================================
    // PWM IMPLEMENTATION
    // ============================================================================

    static bool pwmInit() {
        return pwm_init() == PWM_OK;
    }

    static bool pwmSetFrequency(uint8_t channel, uint32_t frequency) {
        return pwm_set_frequency((pwm_channel_t)channel, frequency) == PWM_OK;
    }

    static bool pwmSetDutyCycle(uint8_t channel, uint8_t duty_percent) {
        return pwm_set_duty_cycle((pwm_channel_t)channel, duty_percent) == PWM_OK;
    }

    static bool pwmEnableChannel(uint8_t channel) {
        return pwm_enable_channel((pwm_channel_t)channel) == PWM_OK;
    }

    static bool pwmDisableChannel(uint8_t channel) {
        return pwm_disable_channel((pwm_channel_t)channel) == PWM_OK;
    }

    // Application-specific PWM operations
    static bool pwmSetAmplitude(uint8_t amplitude_percent) {
        return pwm_set_amplitude(amplitude_percent) == PWM_OK;
    }

    static bool pwmGetAmplitude(uint8_t* amplitude_percent) {
        return pwm_get_amplitude(amplitude_percent) == PWM_OK;
    }

    static bool pwmAmplitudeRamp(uint8_t target_percent, uint16_t ramp_time_ms) {
        return pwm_amplitude_ramp(target_percent, ramp_time_ms) == PWM_OK;
    }

    static bool pwmRampComplete(bool* complete) {
        return pwm_ramp_complete(complete) == PWM_OK;
    }

    static bool pwmEmergencyStop() {
        return pwm_emergency_stop() == PWM_OK;
    }

    // Diagnostic operations
    static bool pwmDutyToVoltage(uint8_t duty_percent, float* output_voltage) {
        return pwm_duty_to_voltage(duty_percent, output_voltage) == PWM_OK;
    }

    static bool pwmAmplitudeToVoltage(uint8_t amplitude_percent, float* output_voltage) {
        return pwm_amplitude_to_voltage(amplitude_percent, output_voltage) == PWM_OK;
    }

    static bool pwmTestPattern() {
        return pwm_test_pattern() == PWM_OK;
    }

    // ============================================================================
    // TIMER IMPLEMENTATION
    // ============================================================================

    static bool timerInit() {
        return timer_init() == TIMER_OK;
    }

    static bool timerConfigure(timer_id_t timer_id, const timer_config_t* config) {
        return timer_configure(timer_id, config) == TIMER_OK;
    }

    static bool timerStart(timer_id_t timer_id, uint32_t period_ms) {
        return timer_start(timer_id, period_ms) == TIMER_OK;
    }

    static bool timerStop(timer_id_t timer_id) {
        return timer_stop(timer_id) == TIMER_OK;
    }

    static bool timerExpired(timer_id_t timer_id, bool* expired) {
        return timer_expired(timer_id, expired) == TIMER_OK;
    }

    static bool timerReset(timer_id_t timer_id) {
        return timer_reset(timer_id) == TIMER_OK;
    }

    static bool timerGetValue(timer_id_t timer_id, uint16_t* value) {
        return timer_get_value(timer_id, value) == TIMER_OK;
    }

    static bool timerGetElapsed(timer_id_t timer_id, uint32_t* elapsed_ms) {
        return timer_get_elapsed(timer_id, elapsed_ms) == TIMER_OK;
    }

    // System timing operations
    static bool timerGetMillis(uint32_t* uptime_ms) {
        return timer_get_millis(uptime_ms) == TIMER_OK;
    }

    static bool timerGetMicros(uint32_t* uptime_us) {
        return timer_get_micros(uptime_us) == TIMER_OK;
    }

    static bool timerDelayMs(uint32_t delay_ms) {
        return timer_delay_ms(delay_ms) == TIMER_OK;
    }

    static bool timerDelayUs(uint32_t delay_us) {
        return timer_delay_us(delay_us) == TIMER_OK;
    }

    static bool timerDelayComplete(uint32_t start_time_ms, uint32_t delay_ms, bool* complete) {
        return timer_delay_complete(start_time_ms, delay_ms, complete) == TIMER_OK;
    }

    // Scheduler operations
    static bool timerSchedulerInit() {
        return timer_scheduler_init() == TIMER_OK;
    }

    static bool timerSchedulerTick(bool* tick_available) {
        return timer_scheduler_tick(tick_available) == TIMER_OK;
    }

    static bool timerSchedulerGetTicks(uint32_t* tick_count) {
        return timer_scheduler_get_ticks(tick_count) == TIMER_OK;
    }

    static bool timerSchedulerReset() {
        return timer_scheduler_reset() == TIMER_OK;
    }

    // Watchdog operations
    static bool timerWatchdogInit() {
        return timer_watchdog_init() == TIMER_OK;
    }

    static bool timerWatchdogReset() {
        return timer_watchdog_reset() == TIMER_OK;
    }

    static bool timerWatchdogExpired(bool* expired) {
        return timer_watchdog_expired(expired) == TIMER_OK;
    }

    static bool timerWatchdogTimeRemaining(uint32_t* time_remaining_ms) {
        return timer_watchdog_time_remaining(time_remaining_ms) == TIMER_OK;
    }

    // ============================================================================
    // UART IMPLEMENTATION
    // ============================================================================

    static bool uartInit(uint32_t baud_rate) {
        uart_config_t config = {
            .baud_rate = baud_rate,
            .data_bits = UART_DATA_8BIT,
            .parity = UART_PARITY_NONE,
            .stop_bits = UART_STOP_1BIT
        };
        return uart_init(&config) == UART_OK;
    }

    static bool uartSetBaudRate(uint32_t baud_rate) {
        return uart_set_baud_rate(baud_rate) == UART_OK;
    }

    static bool uartSendByte(uint8_t data) {
        return uart_send_byte(data) == UART_OK;
    }

    static bool uartReceiveByte(uint8_t* data) {
        return uart_receive_byte(data) == UART_OK;
    }

    static bool uartDataAvailable(bool* available) {
        return uart_data_available(available) == UART_OK;
    }

    static bool uartGetRxCount(uint16_t* count) {
        return uart_get_rx_count(count) == UART_OK;
    }

    static bool uartSendBuffer(const uint8_t* buffer, uint16_t length) {
        return uart_send_buffer(buffer, length) == UART_OK;
    }

    static bool uartReceiveBuffer(uint8_t* buffer, uint16_t max_length, uint16_t* received) {
        return uart_receive_buffer(buffer, max_length, received) == UART_OK;
    }

    static bool uartFlushTx() {
        return uart_flush_tx() == UART_OK;
    }

    static bool uartFlushRx() {
        return uart_flush_rx() == UART_OK;
    }

    // MODBUS RTU operations
    static bool uartSendModbusFrame(const uint8_t* frame, uint16_t length) {
        return uart_send_modbus_frame(frame, length) == UART_OK;
    }

    static bool uartReceiveModbusFrame(uint8_t* frame, uint16_t max_length,
                                      uint16_t* received, uint32_t timeout_ms) {
        return uart_receive_modbus_frame(frame, max_length, received, timeout_ms) == UART_OK;
    }

    static bool uartModbusGapDetected(bool* gap_detected) {
        return uart_modbus_gap_detected(gap_detected) == UART_OK;
    }

    static bool uartGetIdleTime(uint32_t* time_us) {
        return uart_get_idle_time(time_us) == UART_OK;
    }

    // Diagnostic operations
    static bool uartGetStatus(bool* tx_busy, bool* rx_overflow,
                            bool* frame_error, bool* parity_error) {
        return uart_get_status(tx_busy, rx_overflow, frame_error, parity_error) == UART_OK;
    }

    static bool uartGetTxFree(uint16_t* free_bytes) {
        return uart_get_tx_free(free_bytes) == UART_OK;
    }

    static bool uartClearErrors() {
        return uart_clear_errors() == UART_OK;
    }

    static bool uartGetCharTime(uint16_t* char_time_us) {
        return uart_get_char_time(char_time_us) == UART_OK;
    }

    static bool uartTestLoopback(bool* success) {
        return uart_test_loopback(success) == UART_OK;
    }
};

#endif // ATMEGA_HAL_IMPL_H
