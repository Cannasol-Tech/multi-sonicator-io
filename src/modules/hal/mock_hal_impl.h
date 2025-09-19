/**
 * @file mock_hal_impl.h
 * @brief Mock HAL Implementation for Unit Testing
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 1.0.0
 *
 * Mock implementation of HAL operations for unit testing.
 * Provides deterministic behavior and state tracking for test validation.
 */

#ifndef MOCK_HAL_IMPL_H
#define MOCK_HAL_IMPL_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "timer.h"

class MockHal {
private:
    // GPIO mock state
    static bool gpio_initialized;
    static uint8_t pin_states[32];
    static uint8_t pin_modes[32];

    // ADC mock state
    static bool adc_initialized;
    static uint16_t adc_channel_values[8];
    static bool adc_conversion_started;
    static bool adc_conversion_complete;

    // PWM mock state
    static bool pwm_initialized;
    static uint8_t pwm_duty_cycles[4];
    static uint32_t pwm_frequencies[4];
    static bool pwm_channels_enabled[4];

    // Timer mock state
    static bool timer_initialized;
    static bool timer_expired_flags[4];
    static uint32_t timer_start_times[4];
    static uint32_t timer_periods[4];
    static uint16_t timer_values[4];
    static uint32_t mock_millis;
    static uint32_t mock_micros;

    // UART mock state
    static bool uart_initialized;
    static uint32_t uart_baud_rate;
    static uint8_t uart_tx_buffer[256];
    static uint8_t uart_rx_buffer[256];
    static uint16_t uart_tx_head;
    static uint16_t uart_rx_head;
    static uint16_t uart_tx_tail;
    static uint16_t uart_rx_tail;

public:
    // ============================================================================
    // GPIO MOCK IMPLEMENTATION
    // ============================================================================

    static bool gpioInit() {
        gpio_initialized = true;
        memset(pin_states, 0, sizeof(pin_states));
        memset(pin_modes, 0, sizeof(pin_modes));
        return true;
    }

    static bool gpioSetPinMode(uint8_t pin, uint8_t mode) {
        if (!gpio_initialized || pin >= 32) return false;
        pin_modes[pin] = mode;
        return true;
    }

    static bool gpioDigitalWrite(uint8_t pin, uint8_t value) {
        if (!gpio_initialized || pin >= 32) return false;
        pin_states[pin] = value;
        return true;
    }

    static uint8_t gpioDigitalRead(uint8_t pin) {
        if (!gpio_initialized || pin >= 32) return 0;
        return pin_states[pin];
    }

    static bool gpioTogglePin(uint8_t pin) {
        if (!gpio_initialized || pin >= 32) return false;
        pin_states[pin] = !pin_states[pin];
        return true;
    }

    // Sonicator-specific GPIO mock operations
    static bool gpioSonicatorStart(uint8_t sonicator_id) {
        if (!gpio_initialized || sonicator_id < 1 || sonicator_id > 4) return false;
        // Mock: set corresponding start pin high
        uint8_t pin = (sonicator_id == 1) ? 22 : (sonicator_id == 2) ? 23 : (sonicator_id == 3) ? 24 : 25;
        return gpioDigitalWrite(pin, 1);
    }

    static bool gpioSonicatorStop(uint8_t sonicator_id) {
        if (!gpio_initialized || sonicator_id < 1 || sonicator_id > 4) return false;
        uint8_t pin = (sonicator_id == 1) ? 22 : (sonicator_id == 2) ? 23 : (sonicator_id == 3) ? 24 : 25;
        return gpioDigitalWrite(pin, 0);
    }

    static bool gpioSonicatorReset(uint8_t sonicator_id) {
        if (!gpio_initialized || sonicator_id < 1 || sonicator_id > 4) return false;
        uint8_t pin = (sonicator_id == 1) ? 26 : (sonicator_id == 2) ? 27 : (sonicator_id == 3) ? 28 : 29;
        return gpioDigitalWrite(pin, 1); // Pulse high for reset
    }

    static bool gpioSonicatorReadOverload(uint8_t sonicator_id, bool* state) {
        if (!gpio_initialized || sonicator_id < 1 || sonicator_id > 4 || !state) return false;
        uint8_t pin = (sonicator_id == 1) ? 30 : ((sonicator_id == 2) ? 31 : ((sonicator_id == 3) ? 0 : 1));
        *state = (gpioDigitalRead(pin) == 1); // Mock: return false (no overload)
        return true;
    }

    static bool gpioSonicatorReadFreqLock(uint8_t sonicator_id, bool* state) {
        if (!gpio_initialized || sonicator_id < 1 || sonicator_id > 4 || !state) return false;
        uint8_t pin = (sonicator_id == 1) ? 2 : (sonicator_id == 2) ? 3 : (sonicator_id == 3) ? 4 : 5;
        *state = (gpioDigitalRead(pin) == 1); // Mock: return true (frequency locked)
        return true;
    }

    static bool gpioStatusLed(bool state) {
        if (!gpio_initialized) return false;
        return gpioDigitalWrite(13, state ? 1 : 0); // Status LED on pin 13
    }

    // ============================================================================
    // ADC MOCK IMPLEMENTATION
    // ============================================================================

    static bool adcInit() {
        adc_initialized = true;
        memset(adc_channel_values, 0, sizeof(adc_channel_values));
        adc_conversion_started = false;
        adc_conversion_complete = false;
        return true;
    }

    static bool adcReadChannel(uint8_t channel, uint16_t* value) {
        if (!adc_initialized || channel >= 8 || !value) return false;
        *value = adc_channel_values[channel];
        return true;
    }

    static bool adcStartConversion(uint8_t channel) {
        if (!adc_initialized || channel >= 8) return false;
        adc_conversion_started = true;
        adc_conversion_complete = false;
        return true;
    }

    static bool adcIsConversionComplete() {
        if (!adc_initialized) return false;
        if (adc_conversion_started && !adc_conversion_complete) {
            adc_conversion_complete = true; // Mock: conversion completes immediately
        }
        return adc_conversion_complete;
    }

    static float adcRawToVoltage(uint16_t raw_value) {
        if (!adc_initialized) return 0.0f;
        return (raw_value * 5.0f) / 1023.0f; // 10-bit ADC, 5V reference
    }

    // Application-specific ADC mock operations
    static bool adcReadSonicatorPower(uint8_t sonicator_id, float* power_watts) {
        if (!adc_initialized || sonicator_id < 1 || sonicator_id > 4 || !power_watts) return false;
        // Mock: return 500W for sonicator power
        *power_watts = 500.0f;
        return true;
    }

    static bool adcReadFrequency(float* frequency_hz) {
        if (!adc_initialized || !frequency_hz) return false;
        *frequency_hz = 20000.0f; // Mock: 20kHz frequency
        return true;
    }

    static bool adcCalibrate(float reference_voltage) {
        if (!adc_initialized) return false;
        // Mock: calibration always succeeds
        return true;
    }

    // ============================================================================
    // PWM MOCK IMPLEMENTATION
    // ============================================================================

    static bool pwmInit() {
        pwm_initialized = true;
        memset(pwm_duty_cycles, 0, sizeof(pwm_duty_cycles));
        memset(pwm_frequencies, 0, sizeof(pwm_frequencies));
        memset(pwm_channels_enabled, 0, sizeof(pwm_channels_enabled));
        return true;
    }

    static bool pwmSetFrequency(uint8_t channel, uint32_t frequency) {
        if (!pwm_initialized || channel >= 4) return false;
        pwm_frequencies[channel] = frequency;
        return true;
    }

    static bool pwmSetDutyCycle(uint8_t channel, uint8_t duty_percent) {
        if (!pwm_initialized || channel >= 4 || duty_percent > 100) return false;
        pwm_duty_cycles[channel] = duty_percent;
        return true;
    }

    static bool pwmEnableChannel(uint8_t channel) {
        if (!pwm_initialized || channel >= 4) return false;
        pwm_channels_enabled[channel] = true;
        return true;
    }

    static bool pwmDisableChannel(uint8_t channel) {
        if (!pwm_initialized || channel >= 4) return false;
        pwm_channels_enabled[channel] = false;
        pwm_duty_cycles[channel] = 0;
        return true;
    }

    // Application-specific PWM mock operations
    static bool pwmSetAmplitude(uint8_t amplitude_percent) {
        if (!pwm_initialized || amplitude_percent > 100) return false;
        return pwmSetDutyCycle(0, amplitude_percent); // Use channel 0 for amplitude
    }

    static bool pwmGetAmplitude(uint8_t* amplitude_percent) {
        if (!pwm_initialized || !amplitude_percent) return false;
        *amplitude_percent = pwm_duty_cycles[0];
        return true;
    }

    static bool pwmAmplitudeRamp(uint8_t target_percent, uint16_t ramp_time_ms) {
        if (!pwm_initialized || target_percent > 100) return false;
        // Mock: ramp completes immediately
        return pwmSetAmplitude(target_percent);
    }

    static bool pwmRampComplete(bool* complete) {
        if (!pwm_initialized || !complete) return false;
        *complete = true; // Mock: ramp always complete
        return true;
    }

    static bool pwmEmergencyStop() {
        if (!pwm_initialized) return false;
        memset(pwm_duty_cycles, 0, sizeof(pwm_duty_cycles));
        memset(pwm_channels_enabled, 0, sizeof(pwm_channels_enabled));
        return true;
    }

    // Diagnostic operations
    static bool pwmDutyToVoltage(uint8_t duty_percent, float* output_voltage) {
        if (!pwm_initialized || duty_percent > 100 || !output_voltage) return false;
        *output_voltage = (duty_percent * 5.0f) / 100.0f;
        return true;
    }

    static bool pwmAmplitudeToVoltage(uint8_t amplitude_percent, float* output_voltage) {
        if (!pwm_initialized || amplitude_percent > 100 || !output_voltage) return false;
        // Linear mapping: 20% → 2V, 100% → 10V
        *output_voltage = 2.0f + ((amplitude_percent - 20) * 8.0f) / (100 - 20);
        return true;
    }

    static bool pwmTestPattern() {
        if (!pwm_initialized) return false;
        // Mock: test pattern completes successfully
        return true;
    }

    // ============================================================================
    // TIMER MOCK IMPLEMENTATION
    // ============================================================================

    static bool timerInit() {
        timer_initialized = true;
        memset(timer_expired_flags, 0, sizeof(timer_expired_flags));
        memset(timer_start_times, 0, sizeof(timer_start_times));
        memset(timer_periods, 0, sizeof(timer_periods));
        memset(timer_values, 0, sizeof(timer_values));
        mock_millis = 0;
        mock_micros = 0;
        return true;
    }

    static bool timerConfigure(timer_id_t timer_id, const timer_config_t* config) {
        if (!timer_initialized || !config) return false;
        // Mock: configuration always succeeds
        return true;
    }

    static bool timerStart(timer_id_t timer_id, uint32_t period_ms) {
        if (!timer_initialized) return false;
        timer_start_times[timer_id] = mock_millis;
        timer_periods[timer_id] = period_ms;
        timer_expired_flags[timer_id] = false;
        return true;
    }

    static bool timerStop(timer_id_t timer_id) {
        if (!timer_initialized) return false;
        timer_expired_flags[timer_id] = false;
        return true;
    }

    static bool timerExpired(timer_id_t timer_id, bool* expired) {
        if (!timer_initialized || !expired) return false;
        uint32_t elapsed = mock_millis - timer_start_times[timer_id];
        *expired = (elapsed >= timer_periods[timer_id]);
        timer_expired_flags[timer_id] = *expired;
        return true;
    }

    static bool timerReset(timer_id_t timer_id) {
        if (!timer_initialized) return false;
        timer_start_times[timer_id] = mock_millis;
        timer_expired_flags[timer_id] = false;
        return true;
    }

    static bool timerGetValue(timer_id_t timer_id, uint16_t* value) {
        if (!timer_initialized || !value) return false;
        *value = timer_values[timer_id];
        return true;
    }

    static bool timerGetElapsed(timer_id_t timer_id, uint32_t* elapsed_ms) {
        if (!timer_initialized || !elapsed_ms) return false;
        *elapsed_ms = mock_millis - timer_start_times[timer_id];
        return true;
    }

    // System timing mock operations
    static bool timerGetMillis(uint32_t* uptime_ms) {
        if (!uptime_ms) return false;
        *uptime_ms = mock_millis;
        return true;
    }

    static bool timerGetMicros(uint32_t* uptime_us) {
        if (!uptime_us) return false;
        *uptime_us = mock_micros;
        return true;
    }

    static bool timerDelayMs(uint32_t delay_ms) {
        mock_millis += delay_ms;
        mock_micros += (delay_ms * 1000);
        return true;
    }

    static bool timerDelayUs(uint32_t delay_us) {
        mock_micros += delay_us;
        mock_millis += (delay_us / 1000);
        return true;
    }

    static bool timerDelayComplete(uint32_t start_time_ms, uint32_t delay_ms, bool* complete) {
        if (!complete) return false;
        *complete = (mock_millis - start_time_ms) >= delay_ms;
        return true;
    }

    // Mock time advancement for testing
    static void advanceTimeMs(uint32_t ms) {
        mock_millis += ms;
        mock_micros += (ms * 1000);
    }

    static void advanceTimeUs(uint32_t us) {
        mock_micros += us;
        mock_millis += (us / 1000);
    }
};

// Static member definitions
#endif // MOCK_HAL_IMPL_H
