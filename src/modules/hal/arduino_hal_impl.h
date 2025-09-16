/**
 * @file arduino_hal_impl.h
 * @brief Arduino Framework HAL Implementation for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 1.0.0
 *
 * Concrete implementation of HAL operations using Arduino framework functions.
 * Provides the same interface as AtmegaHal but uses Arduino pinMode/digitalWrite/etc.
 * for more readable and maintainable code.
 */

#ifndef ARDUINO_HAL_IMPL_H
#define ARDUINO_HAL_IMPL_H

#include <stdint.h>
#include <stdbool.h>
#include <Arduino.h>
#include "gpio.h"
#include "adc.h"
#include "pwm.h"
#include "timer.h"
#include "uart.h"

class ArduinoHal {
public:
    // ============================================================================
    // GPIO IMPLEMENTATION
    // ============================================================================

    static bool gpioInit() {
        // Initialize all pins used in the system
        // Map ATMega32A pins to Arduino pins
        pinMode(arduinoPin(PB1), INPUT);  // FREQ_DIV10_3
        pinMode(arduinoPin(PB0), INPUT);  // FREQ_DIV10_4
        pinMode(arduinoPin(PB2), INPUT);  // FREQ_DIV10_2
        pinMode(arduinoPin(PB3), INPUT);  // FREQ_DIV10_1
        pinMode(arduinoPin(PB4), INPUT);  // FREQ_LOCK_4
        pinMode(arduinoPin(PB5), INPUT);  // FREQ_LOCK_3
        pinMode(arduinoPin(PB6), INPUT);  // FREQ_LOCK_2
        pinMode(arduinoPin(PB7), INPUT);  // FREQ_LOCK_1

        pinMode(arduinoPin(PC0), OUTPUT); // START_4
        pinMode(arduinoPin(PC1), OUTPUT); // RESET_4
        pinMode(arduinoPin(PC2), OUTPUT); // START_3
        pinMode(arduinoPin(PC3), OUTPUT); // RESET_3
        pinMode(arduinoPin(PC4), OUTPUT); // START_2
        pinMode(arduinoPin(PC5), OUTPUT); // RESET_2
        pinMode(arduinoPin(PC6), OUTPUT); // START_1
        pinMode(arduinoPin(PC7), OUTPUT); // RESET_1

        pinMode(arduinoPin(PD0), INPUT);  // UART_RXD
        pinMode(arduinoPin(PD1), OUTPUT); // UART_TXD
        pinMode(arduinoPin(PD2), OUTPUT); // STATUS_LED
        pinMode(arduinoPin(PD3), INPUT);  // OVERLOAD_4
        pinMode(arduinoPin(PD4), INPUT);  // OVERLOAD_3
        pinMode(arduinoPin(PD5), INPUT);  // OVERLOAD_2
        pinMode(arduinoPin(PD6), INPUT);  // OVERLOAD_1
        pinMode(arduinoPin(PD7), OUTPUT); // AMPLITUDE_ALL (PWM)

        // Analog pins don't need pinMode for ADC
        return true;
    }

    static bool gpioSetPinMode(uint8_t pin, uint8_t mode) {
        uint8_t arduino_pin = arduinoPin(pin);
        switch (mode) {
            case 0: pinMode(arduino_pin, INPUT); break;
            case 1: pinMode(arduino_pin, OUTPUT); break;
            case 2: pinMode(arduino_pin, INPUT_PULLUP); break;
            default: return false;
        }
        return true;
    }

    static bool gpioDigitalWrite(uint8_t pin, uint8_t value) {
        digitalWrite(arduinoPin(pin), value ? HIGH : LOW);
        return true;
    }

    static uint8_t gpioDigitalRead(uint8_t pin) {
        return digitalRead(arduinoPin(pin)) == HIGH ? 1 : 0;
    }

    static bool gpioTogglePin(uint8_t pin) {
        uint8_t current = gpioDigitalRead(pin);
        return gpioDigitalWrite(pin, !current);
    }

    // Sonicator-specific GPIO operations
    static bool gpioSonicatorStart(uint8_t sonicator_id) {
        uint8_t pin = getSonicatorStartPin(sonicator_id);
        if (pin == 0xFF) return false;
        return gpioDigitalWrite(pin, HIGH);
    }

    static bool gpioSonicatorStop(uint8_t sonicator_id) {
        uint8_t pin = getSonicatorStartPin(sonicator_id);
        if (pin == 0xFF) return false;
        return gpioDigitalWrite(pin, LOW);
    }

    static bool gpioSonicatorReset(uint8_t sonicator_id) {
        uint8_t pin = getSonicatorResetPin(sonicator_id);
        if (pin == 0xFF) return false;
        // Pulse reset: HIGH then LOW
        gpioDigitalWrite(pin, HIGH);
        delayMicroseconds(100); // Short pulse
        gpioDigitalWrite(pin, LOW);
        return true;
    }

    static bool gpioSonicatorReadOverload(uint8_t sonicator_id, bool* state) {
        uint8_t pin = getSonicatorOverloadPin(sonicator_id);
        if (pin == 0xFF) return false;
        *state = (gpioDigitalRead(pin) == 1);
        return true;
    }

    static bool gpioSonicatorReadFreqLock(uint8_t sonicator_id, bool* state) {
        uint8_t pin = getSonicatorFreqLockPin(sonicator_id);
        if (pin == 0xFF) return false;
        *state = (gpioDigitalRead(pin) == 1);
        return true;
    }

    static bool gpioStatusLed(bool state) {
        return gpioDigitalWrite(PD2, state ? 1 : 0);
    }

    // ============================================================================
    // ADC IMPLEMENTATION
    // ============================================================================

    static bool adcInit() {
        // Arduino analogRead doesn't need explicit initialization
        // Set ADC reference if needed
        analogReference(DEFAULT);
        return true;
    }

    static bool adcReadChannel(uint8_t channel, uint16_t* value) {
        int arduino_pin = getAnalogPin(channel);
        if (arduino_pin == -1) return false;
        *value = analogRead(arduino_pin);
        return true;
    }

    static bool adcStartConversion(uint8_t channel) {
        // Arduino analogRead is blocking, so conversion starts immediately
        return true;
    }

    static bool adcIsConversionComplete() {
        // Arduino analogRead is synchronous
        return true;
    }


    // Application-specific ADC operations
    static bool adcReadSonicatorPower(uint8_t sonicator_id, uint16_t* voltage) {
        uint8_t channel = getSonicatorPowerChannel(sonicator_id);
        if (channel == AMPLITUDE_ALL_DISABLE) return false;

        uint16_t raw_value;
        if (!adcReadChannel(channel, &raw_value)) return false;

        // Convert to voltage then to power (5.44 mV/W scaling)
        float voltage = adcRawToVoltage(raw_value);
    }

    static bool adcReadFrequency(float* frequency_hz) {
        // This would require frequency measurement implementation
        // For now, return a placeholder
        *frequency_hz = 20000.0; // 20 kHz default
        return true;
    }

    // ============================================================================
    // PWM IMPLEMENTATION
    // ============================================================================

    static bool pwmInit() {
        // PWM pins are initialized in gpioInit
        return true;
    }

    static bool pwmSetFrequency(uint8_t channel, uint32_t frequency) {
        // Arduino standard PWM is fixed frequency (~490Hz or ~980Hz)
        // For variable frequency, would need custom implementation
        // For now, just acknowledge
        return true;
    }

    static bool pwmSetDutyCycle(uint8_t channel, uint8_t duty_percent) {
        int arduino_pin = getPwmPin(channel);
        if (arduino_pin == -1) return false;

        // Convert percentage to 0-255 range
        int duty_value = map(duty_percent, 0, 100, 0, 255);
        analogWrite(arduino_pin, duty_value);
        return true;
    }

    static bool pwmEnableChannel(uint8_t channel) {
        // PWM is enabled by writing non-zero duty cycle
        return pwmSetDutyCycle(channel, 0); // Start with 0% duty
    }

    static bool pwmDisableChannel(uint8_t channel) {
        int arduino_pin = getPwmPin(channel);
        if (arduino_pin == -1) return false;
        analogWrite(arduino_pin, 0); // 0 duty cycle effectively disables
        return true;
    }

    // Application-specific PWM operations
    static bool pwmSetAmplitude(uint8_t amplitude_percent) {
        // Use PD7 (AMPLITUDE_ALL) for shared amplitude control
        return pwmSetDutyCycle(0, amplitude_percent); // Channel 0 for amplitude
    }

    static bool pwmEmergencyStop() {
        // Set amplitude to 0
        return pwmSetAmplitude(0);
    }

    static bool pwmTestPattern() {
        // Simple test pattern: ramp amplitude
        for (int i = 0; i <= 100; i += 10) {
            pwmSetAmplitude(i);
            delay(100);
        }
        for (int i = 100; i >= 0; i -= 10) {
            pwmSetAmplitude(i);
            delay(100);
        }
        return true;
    }

    // ============================================================================
    // TIMER IMPLEMENTATION
    // ============================================================================

    static bool timerInit() {
        // Arduino millis/micros are always available
        return true;
    }

    static bool timerStart(uint8_t timer_id, uint32_t period_ms) {
        // For simple timing, we can use millis()
        // More complex timers would need custom implementation
        return true;
    }

    static bool timerStop(uint8_t timer_id) {
        return true;
    }

    static bool timerIsExpired(uint8_t timer_id) {
        // Would need to track start times
        return false;
    }

    static bool timerGetMillis(uint32_t* millis) {
        *millis = ::millis();
        return true;
    }

    static bool timerDelayMs(uint32_t delay_ms) {
        delay(delay_ms);
        return true;
    }

    // ============================================================================
    // UART IMPLEMENTATION
    // ============================================================================

    static bool uartInit(uint32_t baud_rate) {
        Serial.begin(baud_rate);
        return true;
    }

    static bool uartSendByte(uint8_t data) {
        Serial.write(data);
        return true;
    }

    static bool uartReceiveByte(uint8_t* data) {
        if (Serial.available()) {
            *data = Serial.read();
            return true;
        }
        return false;
    }

    static bool uartDataAvailable() {
        return Serial.available() > 0;
    }

    static bool uartSendBuffer(const uint8_t* buffer, uint16_t length) {
        Serial.write(buffer, length);
        return true;
    }

    static bool uartReceiveBuffer(uint8_t* buffer, uint16_t max_length, uint16_t* received) {
        *received = 0;
        while (*received < max_length && Serial.available()) {
            buffer[*received] = Serial.read();
            (*received)++;
        }
        return true;
    }

    static bool uartFlushTx() {
        Serial.flush();
        return true;
    }

    static bool uartFlushRx() {
        while (Serial.available()) {
            Serial.read();
        }
        return true;
    }

private:
    // ============================================================================
    // PIN MAPPING HELPERS
    // ============================================================================

    // Map ATMega32A pin numbers to Arduino pin numbers
    static uint8_t arduinoPin(uint8_t atmega_pin) {
        // PORTB: PB0-PB7 (pins 1-8) -> Arduino digital 8-15
        if (atmega_pin >= 1 && atmega_pin <= 8) {
            return atmega_pin + 7; // PB0=8, PB1=9, ..., PB7=15
        }
        // PORTC: PC0-PC7 (pins 22-29) -> Arduino analog A0-A7
        else if (atmega_pin >= 22 && atmega_pin <= 29) {
            return atmega_pin - 22; // PC0=A0, PC1=A1, ..., PC7=A7
        }
        // PORTD: PD0-PD7 (pins 14-21) -> Arduino digital 0-7
        else if (atmega_pin >= 14 && atmega_pin <= 21) {
            return atmega_pin - 14; // PD0=0, PD1=1, ..., PD7=7
        }
        // PORTA: PA0-PA7 (pins 33-40) -> Arduino analog A8-A15 (if supported)
        else if (atmega_pin >= 33 && atmega_pin <= 40) {
            return atmega_pin - 25; // PA0=A8, etc. (assuming extended analog pins)
        }
        return 0xFF; // Invalid pin
    }

    // Get Arduino analog pin for ADC channel
    static int getAnalogPin(uint8_t channel) {
        // Map ADC channels to Arduino analog pins
        // Assuming standard Arduino Uno mapping
        if (channel >= 0 && channel <= 7) {
            return A0 + channel; // A0-A7
        }
        return -1;
    }

    // Get PWM pin for channel
    static int getPwmPin(uint8_t channel) {
        // Arduino Uno PWM pins: 3, 5, 6, 9, 10, 11
        // For amplitude control, use PD7 which maps to digital pin 7
        // But digital pin 7 is not PWM capable, so use pin 9 (PB1)
        if (channel == 0) {
            return 9; // Digital pin 9 for amplitude PWM
        }
        return -1;
    }

    // Sonicator pin mappings
    static uint8_t getSonicatorStartPin(uint8_t sonicator_id) {
        switch (sonicator_id) {
            case 1: return PC6; // START_1
            case 2: return PC4; // START_2
            case 3: return PC2; // START_3
            case 4: return PC0; // START_4
            default: return 0xFF;
        }
    }

    static uint8_t getSonicatorResetPin(uint8_t sonicator_id) {
        switch (sonicator_id) {
            case 1: return PC7; // RESET_1
            case 2: return PC5; // RESET_2
            case 3: return PC3; // RESET_3
            case 4: return PC1; // RESET_4
            default: return 0xFF;
        }
    }

    static uint8_t getSonicatorOverloadPin(uint8_t sonicator_id) {
        switch (sonicator_id) {
            case 1: return PD6; // OVERLOAD_1
            case 2: return PD5; // OVERLOAD_2
            case 3: return PD4; // OVERLOAD_3
            case 4: return PD3; // OVERLOAD_4
            default: return 0xFF;
        }
    }

    static uint8_t getSonicatorFreqLockPin(uint8_t sonicator_id) {
        switch (sonicator_id) {
            case 1: return PB7; // FREQ_LOCK_1
            case 2: return PB6; // FREQ_LOCK_2
            case 3: return PB5; // FREQ_LOCK_3
            case 4: return PB4; // FREQ_LOCK_4
            default: return 0xFF;
        }
    }


    static uint8_t getSonicatorPowerChannel(uint8_t sonicator_id) {
        // Map to ADC channels (0-7 for A0-A7)
        switch (sonicator_id) {
            case 1: return ADC_CHANNEL_5; // PA4 -> A4
            case 2: return ADC_CHANNEL_6; // PA5 -> A5
            case 3: return ADC_CHANNEL_7; // PA6 -> A6
            case 4: return ADC_CHANNEL_8; // PA7 -> A7
            default: return AMPLITUDE_CTRL_DISABLE;  // AMPLITUDE_CTRL_DISABLE
        }
    }

    // ATMega32A pin definitions (from pins.h)
    static constexpr uint8_t PB0 = 1;
    static constexpr uint8_t PB1 = 2;
    static constexpr uint8_t PB2 = 3;
    static constexpr uint8_t PB3 = 4;
    static constexpr uint8_t PB4 = 5;
    static constexpr uint8_t PB5 = 6;
    static constexpr uint8_t PB6 = 7;
    static constexpr uint8_t PB7 = 8;

    static constexpr uint8_t PC0 = 22;
    static constexpr uint8_t PC1 = 23;
    static constexpr uint8_t PC2 = 24;
    static constexpr uint8_t PC3 = 25;
    static constexpr uint8_t PC4 = 26;
    static constexpr uint8_t PC5 = 27;
    static constexpr uint8_t PC6 = 28;
    static constexpr uint8_t PC7 = 29;

    static constexpr uint8_t PD0 = 14;
    static constexpr uint8_t PD1 = 15;
    static constexpr uint8_t PD2 = 16;
    static constexpr uint8_t PD3 = 17;
    static constexpr uint8_t PD4 = 18;
    static constexpr uint8_t PD5 = 19;
    static constexpr uint8_t PD6 = 20;
    static constexpr uint8_t PD7 = 21;

    static constexpr uint8_t PA4 = 36;
    static constexpr uint8_t PA5 = 35;
    static constexpr uint8_t PA6 = 34;
    static constexpr uint8_t PA7 = 33;
};

#endif // ARDUINO_HAL_IMPL_H