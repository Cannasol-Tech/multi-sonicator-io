/**
 * @file main.cpp
 * @brief Multi-Sonicator I/O Controller - ATmega32A Firmware Entry Point
 * @author Cannasol Technologies  
 * @date 2025-09-02
 * @version 1.0.0
 * @framework Arduino Framework
 *
 * @brief Project Structure & Build System Setup Implementation
 * @details Initial main.cpp with basic functionality:
 *          - System initialization and watchdog setup
 *          - Status LED control on PD2 (Pin 16) per pin matrix
 *          - Serial communication at 115200 baud for debugging
 *          - Timer setup for future cooperative scheduler
 *          - Pin assignments follow docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)
 */

#include <Arduino.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "config.h"
#include "types.h"

/**
 * @brief System initialization function
 * @details Initializes all hardware subsystems for Multi-Sonicator I/O Controller
 *          - Watchdog timer configuration
 *          - GPIO initialization per pin matrix
 *          - Serial communication setup
 *          - Timer configuration for scheduler
 */
void setup() {
    // Initialize watchdog timer for system reliability
    wdt_enable(WDTO_2S);
    
    // Initialize serial communication for debugging
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial && millis() < 2000); // Wait for serial with timeout
    
    // Configure Status LED (PD2, Pin 16) as output per pin matrix
    DDRD |= (1 << STATUS_LED_PIN);
    
    // Initialize system timer for cooperative scheduler
    // Timer1 configured for 1ms tick
    TCCR1A = 0;
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // CTC mode, prescaler 64
    OCR1A = (F_CPU / 64 / 1000) - 1; // 1ms interval at 16MHz
    TIMSK |= (1 << OCIE1A); // Enable Timer1 compare interrupt
    
    // Enable global interrupts
    sei();
    
    // Print system startup information
    Serial.println(F("========================================"));
    Serial.println(F("Multi-Sonicator I/O Controller v1.0.0"));
    Serial.println(F("Framework: Arduino"));
    Serial.println(F("MCU: ATmega32A @ 16MHz"));
    Serial.println(F("Status: System Initialized"));
    Serial.println(F("Pin Matrix: docs/planning/pin-matrix.md"));
    Serial.println(F("========================================"));
    
    // Perform initial system diagnostic
    Serial.print(F("Status LED Pin: PD"));
    Serial.println(STATUS_LED_PIN);
    Serial.println(F("System Ready - Starting main loop"));
}

/**
 * @brief Main application loop
 * @details Implements basic LED blink pattern for hardware validation
 *          - 1Hz blink pattern on Status LED (PD2)
 *          - Serial output for debugging
 *          - Watchdog timer reset to prevent system reset
 */
void loop() {
    static unsigned long lastToggle = 0;
    static bool ledState = false;
    
    // Reset watchdog timer
    wdt_reset();
    
    // Toggle LED every 1000ms (1Hz blink)
    if (millis() - lastToggle >= 1000) {
        lastToggle = millis();
        
        if (ledState) {
            // Turn Status LED ON (PD2)
            PORTD |= (1 << STATUS_LED_PIN);
            Serial.println(F("Status LED: ON"));
        } else {
            // Turn Status LED OFF (PD2)
            PORTD &= ~(1 << STATUS_LED_PIN);
            Serial.println(F("Status LED: OFF"));
        }
        
        ledState = !ledState;
        
        // Print system status
        Serial.print(F("Uptime: "));
        Serial.print(millis() / 1000);
        Serial.println(F(" seconds"));
    }
    
    // Small delay to prevent excessive CPU usage
    delay(10);
}

/**
 * @brief Timer1 Compare Interrupt Service Routine
 * @details 1ms system tick for cooperative scheduler (future use)
 */
ISR(TIMER1_COMPA_vect) {
    // System tick for future cooperative scheduler
    // Currently unused - ready for future implementation
}
