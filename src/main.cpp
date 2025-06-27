/**
 * @file main.cpp
 * @brief Multi Sonicator I/O Controller - Main Application
 * @author Cannasol Technologies
 * @date 2025-06-27
 * @version 1.0.0
 * 
 * Main application entry point for the Multi Sonicator I/O Controller
 * (Model: CAN-SON-CTRL-4) - Initial Setup Test
 */

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// Global variables for testing
unsigned long previousMillis = 0;
const long interval = 500; // 500ms blink interval
bool ledState = false;

/**
 * @brief Arduino setup function - runs once at startup
 */
void setup() {
    // Initialize status LEDs as outputs
    pinMode(LED_POWER_PIN, OUTPUT);
    pinMode(LED_RUN_PIN, OUTPUT);
    pinMode(LED_ERROR_PIN, OUTPUT);
    pinMode(LED_COMM_PIN, OUTPUT);
    
    // Initialize individual sonicator LEDs
    pinMode(LED_SONICATOR_1, OUTPUT);
    pinMode(LED_SONICATOR_2, OUTPUT);
    pinMode(LED_SONICATOR_3, OUTPUT);
    pinMode(LED_SONICATOR_4, OUTPUT);
    
    // Turn on power LED to indicate system is running
    digitalWrite(LED_POWER_PIN, HIGH);
    
    // Turn off all other LEDs initially
    digitalWrite(LED_RUN_PIN, LOW);
    digitalWrite(LED_ERROR_PIN, LOW);
    digitalWrite(LED_COMM_PIN, LOW);
    digitalWrite(LED_SONICATOR_1, LOW);
    digitalWrite(LED_SONICATOR_2, LOW);
    digitalWrite(LED_SONICATOR_3, LOW);
    digitalWrite(LED_SONICATOR_4, LOW);
    
    // Initialize I2C communication
    Wire.begin();
    
    // Initialize UART for MODBUS communication
    Serial.begin(MODBUS_BAUD_RATE);
    
    // Send startup message
    delay(100);
    Serial.println();
    Serial.println("=== Cannasol Multi Sonicator Controller ===");
    Serial.print("Model: ");
    Serial.println(PROJECT_MODEL);
    Serial.print("Version: ");
    Serial.println(PROJECT_VERSION);
    Serial.print("Build: ");
    Serial.println(PROJECT_BUILD_DATE);
    Serial.println();
    Serial.println("Phase 1.1 Initial Setup Test - SUCCESS");
    Serial.println("System initializing...");
    
    // Test sequence: Flash all LEDs once to verify hardware
    Serial.println("Running LED test sequence...");
    for(int i = 0; i < 3; i++) {
        digitalWrite(LED_RUN_PIN, HIGH);
        digitalWrite(LED_ERROR_PIN, HIGH);
        digitalWrite(LED_COMM_PIN, HIGH);
        digitalWrite(LED_SONICATOR_1, HIGH);
        digitalWrite(LED_SONICATOR_2, HIGH);
        digitalWrite(LED_SONICATOR_3, HIGH);
        digitalWrite(LED_SONICATOR_4, HIGH);
        
        delay(200);
        
        digitalWrite(LED_RUN_PIN, LOW);
        digitalWrite(LED_ERROR_PIN, LOW);
        digitalWrite(LED_COMM_PIN, LOW);
        digitalWrite(LED_SONICATOR_1, LOW);
        digitalWrite(LED_SONICATOR_2, LOW);
        digitalWrite(LED_SONICATOR_3, LOW);
        digitalWrite(LED_SONICATOR_4, LOW);
        
        delay(200);
    }
    
    Serial.println("LED test complete. System ready.");
    Serial.println("Commands: 'test', 'status', 'info', 'help'");
    
    // Final state: Power ON, Run blinking
    digitalWrite(LED_POWER_PIN, HIGH);
}

/**
 * @brief Arduino main loop function - runs continuously
 */
void loop() {
    unsigned long currentMillis = millis();
    
    // Blink the RUN LED to show the system is alive
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        ledState = !ledState;
        digitalWrite(LED_RUN_PIN, ledState);
    }
    
    // Handle incoming serial commands
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();
        
        if(command == "test") {
            Serial.println("Running LED test...");
            for(int i = 0; i < 2; i++) {
                digitalWrite(LED_ERROR_PIN, HIGH);
                digitalWrite(LED_COMM_PIN, HIGH);
                digitalWrite(LED_SONICATOR_1, HIGH);
                digitalWrite(LED_SONICATOR_2, HIGH);
                digitalWrite(LED_SONICATOR_3, HIGH);
                digitalWrite(LED_SONICATOR_4, HIGH);
                delay(300);
                digitalWrite(LED_ERROR_PIN, LOW);
                digitalWrite(LED_COMM_PIN, LOW);
                digitalWrite(LED_SONICATOR_1, LOW);
                digitalWrite(LED_SONICATOR_2, LOW);
                digitalWrite(LED_SONICATOR_3, LOW);
                digitalWrite(LED_SONICATOR_4, LOW);
                delay(300);
            }
            Serial.println("Test complete.");
        }
        else if(command == "status") {
            Serial.println("=== System Status ===");
            Serial.print("Uptime: ");
            Serial.print(millis() / 1000);
            Serial.println(" seconds");
            Serial.print("MCU: ");
            Serial.println(MCU_TYPE);
            Serial.print("Frequency: ");
            Serial.print(MCU_FREQUENCY / 1000000);
            Serial.println(" MHz");
        }
        else if(command == "info") {
            Serial.println("=== Project Information ===");
            Serial.print("Name: ");
            Serial.println(PROJECT_NAME);
            Serial.print("Model: ");
            Serial.println(PROJECT_MODEL);
            Serial.print("Manufacturer: ");
            Serial.println(MANUFACTURER);
            Serial.print("Max Sonicators: ");
            Serial.println(MAX_SONICATORS);
            Serial.print("MODBUS Baud: ");
            Serial.println(MODBUS_BAUD_RATE);
        }
        else if(command == "help") {
            Serial.println("=== Available Commands ===");
            Serial.println("test   - Run LED test sequence");
            Serial.println("status - Show system status");
            Serial.println("info   - Show project info");
            Serial.println("help   - Show this help");
        }
        else {
            Serial.print("Unknown command: ");
            Serial.println(command);
            Serial.println("Type 'help' for available commands.");
        }
        
        // Toggle comm LED on serial activity
        digitalWrite(LED_COMM_PIN, HIGH);
        delay(100);
        digitalWrite(LED_COMM_PIN, LOW);
    }
    
    // Add small delay to prevent overwhelming the loop
    delay(10);
}
