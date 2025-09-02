/**
 * @file main_test.cpp
 * @brief Simple ATmega32A Hardware Test - Multiple Pin Toggle
 * @author Cannasol Technologies  
 * @date 2025-09-02
 * @version 1.0.0
 * @framework Arduino Framework
 *
 * @brief Hardware validation test
 * @details Tests multiple pins to ensure basic functionality regardless of clock speed
 */

#include <Arduino.h>

// Test multiple pins to ensure we see some activity
#define TEST_PIN_1 2   // PD2 (Status LED per pin matrix)
#define TEST_PIN_2 0   // PB0 (alternate test pin)
#define TEST_PIN_3 1   // PB1 (alternate test pin)

void setup() {
    // Set up multiple pins as outputs for testing
    DDRD |= (1 << TEST_PIN_1);  // PD2 as output
    DDRB |= (1 << TEST_PIN_2);  // PB0 as output  
    DDRB |= (1 << TEST_PIN_3);  // PB1 as output
    
    // Initialize serial at a very basic baud rate
    Serial.begin(9600);  // Lower baud rate for reliability
}

void loop() {
    // Toggle all test pins
    PORTD ^= (1 << TEST_PIN_1);  // Toggle PD2
    PORTB ^= (1 << TEST_PIN_2);  // Toggle PB0
    PORTB ^= (1 << TEST_PIN_3);  // Toggle PB1
    
    // Send basic serial message
    Serial.println("TEST");
    
    // Long delay that works regardless of clock speed
    // This creates visible blinking even if clock is wrong
    for(volatile long i = 0; i < 100000; i++) {
        // Busy wait - works regardless of crystal issues
    }
}
