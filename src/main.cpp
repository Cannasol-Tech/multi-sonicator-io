/**
 * @file main.c
 * @brief PLACEHOLDER - Simple ATmega32A pin toggle test
 * @author Cannasol Technologies
 * @date 2025-07-16
 *
 * @note This is a placeholder implementation for hardware testing only.
 *       The full Multi-Sonicator I/O Controller implementation is not yet underway.
 *       This simple LED blink test is used for initial hardware validation.
 *       See include/config.h and include/types.h for the planned full implementation.
 */

#include <Arduino.h>
#include "config.h"

/**
 * @brief Arduino setup function - runs once at startup
 * @details Initializes hardware for basic LED blink test using direct register access
 */
void setup() {
    // Initialize serial communication for debugging (optional)
    Serial.begin(115200);

    // Set PB0 (Physical pin 1) as output using direct register access
    // This avoids the pin mapping issue with ATmega32A
    DDRB |= (1 << PB0);

    // Print startup message
    Serial.println(F("Multi-Sonicator I/O Controller - Hardware Test"));
    Serial.println(F("Framework: Arduino"));
    Serial.println(F("MCU: ATmega32A @ 16MHz"));
    Serial.println(F("Status: LED Blink Test Active (Direct Register Access)"));
}

/**
 * @brief Arduino main loop function - runs continuously
 * @details Simple LED blink pattern for hardware validation using direct register access
 */
void loop() {
    // Turn LED on (PB0)
    PORTB |= (1 << PB0);
    Serial.println(F("LED ON"));

    // Delay 1 second (Arduino framework handles timing)
    delay(1000);

    // Turn LED off (PB0)
    PORTB &= ~(1 << PB0);
    Serial.println(F("LED OFF"));

    // Delay 1 second
    delay(1000);
}
