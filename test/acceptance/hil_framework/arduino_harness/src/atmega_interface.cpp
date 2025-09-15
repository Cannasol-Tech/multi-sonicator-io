/*
 * ATmega Interface Implementation
 * 
 * Implementation of the ATmega32A interface for HIL testing.
 * Pin assignments follow docs/planning/pin-matrix.md exactly.
 * 
 * Author: Cannasol Technologies
 * License: Proprietary
 */

#include "atmega_interface.h"

// Pin mapping from pin matrix (Sonicator 4 - single channel HIL)
// Arduino Pin -> ATmega32A Pin (Signal Name)
#define FREQ_DIV10_4_PIN    7   // D7  -> PB0 (FREQ_DIV10_4) - See pin-matrix.md
#define FREQ_LOCK_4_PIN     8   // D8  -> PB4 (FREQ_LOCK_4) - See pin-matrix.md
#define OVERLOAD_4_PIN      A2  // A2  -> PD3 (OVERLOAD_4) - See pin-matrix.md
#define START_4_PIN         A3  // A3  -> PC0 (START_4) - See pin-matrix.md
#define RESET_4_PIN         A4  // A4  -> PC1 (RESET_4) - See pin-matrix.md
#define POWER_SENSE_4_PIN   A1  // A1  -> PA7 (POWER_SENSE_4) - See pin-matrix.md
#define AMPLITUDE_ALL_PIN   9   // D9  -> PD7 (AMPLITUDE_ALL - PWM) - See pin-matrix.md
#define UART_RXD_PIN        10  // D10 -> PD0 (UART_RXD - drive to DUT RX) - See pin-matrix.md
#define UART_TXD_PIN        11  // D11 -> PD1 (UART_TXD - read from DUT TX) - See pin-matrix.md
#define STATUS_LED_PIN      12  // D12 -> PD2 (STATUS_LED) - See pin-matrix.md

void ATmegaInterface::begin() {
    sandbox_mode_active = false;
    monitoring_enabled = false;
    last_monitor_update = 0;
    modbus_slave_id = 2;  // Default slave ID
    modbus_baud_rate = 115200;  // Default baud rate
    
    // Initialize pin states array
    for (int i = 0; i < 20; i++) {
        pin_states[i].digital_state = false;
        pin_states[i].analog_value = 0;
        pin_states[i].last_update = 0;
    }
    
    // Initialize MODBUS registers
    for (int i = 0; i < 256; i++) {
        modbus_registers[i] = 0;
    }

    // Initialize test-control flags
    for (int i = 0; i < 4; ++i) {
        start_inhibit_flags[i] = false;
    }
    
    // Set some default register values
    modbus_registers[0x0000] = 0x0001;  // System status - online
    modbus_registers[0x0001] = 0x0000;  // Fault status - no faults
    modbus_registers[0x0002] = 0x0001;  // Mode register - normal mode
    
    initializePins();
}

void ATmegaInterface::initializePins() {
    // Configure pins according to pin matrix (see docs/planning/pin-matrix.md)
    // Inputs (from ATmega32A perspective, outputs from Arduino perspective)
    pinMode(START_4_PIN, OUTPUT);       // A3 -> PC0 (START_4)
    pinMode(RESET_4_PIN, OUTPUT);       // A4 -> PC1 (RESET_4)
    pinMode(AMPLITUDE_ALL_PIN, INPUT);  // D9 <- PD7 (AMPLITUDE_ALL - PWM READ)
    pinMode(UART_RXD_PIN, OUTPUT);      // D10 -> PD0 (UART_RXD)
    pinMode(STATUS_LED_PIN, OUTPUT);    // D12 -> PD2 (STATUS_LED)

    // Outputs (from ATmega32A perspective, inputs to Arduino)
    pinMode(FREQ_DIV10_4_PIN, INPUT);   // D7 -> PB0 (FREQ_DIV10_4)
    pinMode(FREQ_LOCK_4_PIN, INPUT);    // D8 -> PB4 (FREQ_LOCK_4)
    pinMode(OVERLOAD_4_PIN, INPUT);     // A2 -> PD3 (OVERLOAD_4)
    pinMode(UART_TXD_PIN, INPUT);       // D11 -> PD1 (UART_TXD)

    // Analog inputs
    pinMode(POWER_SENSE_4_PIN, INPUT);  // A1 -> PA7 (POWER_SENSE_4)

    // Initialize outputs to safe states
    digitalWrite(START_4_PIN, LOW);
    digitalWrite(RESET_4_PIN, LOW);
    digitalWrite(AMPLITUDE_ALL_PIN, LOW);
    digitalWrite(UART_RXD_PIN, LOW);
    digitalWrite(STATUS_LED_PIN, LOW);
}

bool ATmegaInterface::resetATmega() {
    // Reset ATmega32A by pulsing reset pin
    digitalWrite(RESET_4_PIN, LOW);
    delay(100);  // Hold reset for 100ms
    digitalWrite(RESET_4_PIN, HIGH);
    delay(10);
    digitalWrite(RESET_4_PIN, LOW);
    
    // Wait for target to boot
    delay(2000);
    
    return true;  // Assume successful for now
}

void ATmegaInterface::enterSandboxMode() {
    sandbox_mode_active = true;
}

void ATmegaInterface::exitSandboxMode() {
    sandbox_mode_active = false;
    monitoring_enabled = false;
}

void ATmegaInterface::enablePinMonitoring() {
    monitoring_enabled = true;
}

void ATmegaInterface::disablePinMonitoring() {
    monitoring_enabled = false;
}

bool ATmegaInterface::readPin(uint8_t pin) {
    return digitalRead(pin);
}

void ATmegaInterface::writePin(uint8_t pin, bool state) {
    digitalWrite(pin, state ? HIGH : LOW);
    
    // Update pin state tracking
    if (pin < 20) {
        pin_states[pin].digital_state = state;
        pin_states[pin].last_update = millis();
    }
}

uint16_t ATmegaInterface::readADC(uint8_t channel) {
    return analogRead(channel);
}

void ATmegaInterface::setPWM(uint8_t pin, uint8_t duty_percent, uint16_t frequency) {
    // Simple PWM implementation using analogWrite
    // Note: Arduino's analogWrite doesn't allow frequency control easily
    // This is a simplified implementation
    
    if (pin == AMPLITUDE_ALL_PIN) {
        int pwm_value = map(duty_percent, 0, 100, 0, 255);
        analogWrite(pin, pwm_value);
    } else {
        // For non-PWM pins, just set digital high/low based on duty cycle
        if (duty_percent > 50) {
            digitalWrite(pin, HIGH);
        } else if (duty_percent == 0) {
            digitalWrite(pin, LOW);
        } else {
            digitalWrite(pin, HIGH);
        }
    }
}

void ATmegaInterface::disablePWM(uint8_t pin) {
    if (pin == AMPLITUDE_ALL_PIN) {
        analogWrite(pin, 0);
    } else {
        digitalWrite(pin, LOW);
    }
}

bool ATmegaInterface::measurePWM(uint8_t pin, float &frequency, float &duty_cycle) {
    // PWM measurement for AMPLITUDE_ALL pin
    if (pin == AMPLITUDE_ALL_PIN) {
        // Use pulseIn to measure PWM duty cycle
        unsigned long high_time = pulseIn(pin, HIGH, 50000); // 50ms timeout
        unsigned long low_time = pulseIn(pin, LOW, 50000);   // 50ms timeout

        if (high_time == 0 && low_time == 0) {
            // No PWM signal detected, check if pin is constantly high or low
            if (digitalRead(pin) == HIGH) {
                duty_cycle = 100.0; // Constant HIGH = 100% duty cycle
            } else {
                duty_cycle = 0.0;   // Constant LOW = 0% duty cycle
            }
            frequency = 0.0; // No frequency if constant
            return true;
        }

        if (high_time == 0) {
            duty_cycle = 0.0; // No high pulse detected
            frequency = 0.0;
            return true;
        }

        if (low_time == 0) {
            duty_cycle = 100.0; // No low pulse detected (always high)
            frequency = 0.0;
            return true;
        }

        // Calculate duty cycle and frequency
        unsigned long total_period = high_time + low_time;
        duty_cycle = (float)high_time / (float)total_period * 100.0;
        frequency = 1000000.0 / (float)total_period; // Convert microseconds to Hz

        return true;
    }

    // For other pins, return false
    return false;
}

void ATmegaInterface::updatePinStates() {
    // Update digital input pin states (from ATmega32A perspective)
    updatePinState(FREQ_DIV10_4_PIN);
    updatePinState(FREQ_LOCK_4_PIN);
    updatePinState(OVERLOAD_4_PIN);
    updatePinState(UART_TXD_PIN);

    // Update digital output pin states (Arduino drives these)
    updatePinState(START_4_PIN);
    updatePinState(RESET_4_PIN);
    updatePinState(STATUS_LED_PIN);
    updatePinState(UART_RXD_PIN);

    // Update analog pins
    pin_states[POWER_SENSE_4_PIN].analog_value = analogRead(POWER_SENSE_4_PIN);
    pin_states[POWER_SENSE_4_PIN].last_update = millis();

    // Update PWM pin (measure PWM duty cycle)
    float frequency, duty_cycle;
    if (measurePWM(AMPLITUDE_ALL_PIN, frequency, duty_cycle)) {
        pin_states[AMPLITUDE_ALL_PIN].analog_value = (uint16_t)(duty_cycle * 10); // Store duty cycle * 10 for precision
    } else {
        pin_states[AMPLITUDE_ALL_PIN].analog_value = 0;
    }
    pin_states[AMPLITUDE_ALL_PIN].last_update = millis();
}

void ATmegaInterface::updatePinState(uint8_t pin) {
    if (pin < 20) {
        pin_states[pin].digital_state = digitalRead(pin);
        pin_states[pin].last_update = millis();
    }
}

void ATmegaInterface::sendPinStatus() {
    String status = formatPinStatus();
    Serial.println(status);
}

String ATmegaInterface::formatPinStatus() {
    // Format pin status as complete JSON string with all pins
    String status = "{";

    // Digital input pins (from ATmega32A perspective)
    status += "\"FREQ_DIV10_4\":" + String(pin_states[FREQ_DIV10_4_PIN].digital_state ? "true" : "false") + ",";
    status += "\"FREQ_LOCK_4\":" + String(pin_states[FREQ_LOCK_4_PIN].digital_state ? "true" : "false") + ",";
    status += "\"OVERLOAD_4\":" + String(pin_states[OVERLOAD_4_PIN].digital_state ? "true" : "false") + ",";
    status += "\"UART_TXD\":" + String(pin_states[UART_TXD_PIN].digital_state ? "true" : "false") + ",";

    // Digital output pins (Arduino drives these)
    status += "\"START_4\":" + String(pin_states[START_4_PIN].digital_state ? "true" : "false") + ",";
    status += "\"RESET_4\":" + String(pin_states[RESET_4_PIN].digital_state ? "true" : "false") + ",";
    status += "\"STATUS_LED\":" + String(pin_states[STATUS_LED_PIN].digital_state ? "true" : "false") + ",";
    status += "\"UART_RXD\":" + String(digitalRead(UART_RXD_PIN) ? "true" : "false") + ",";

    // PWM output pin (duty cycle percentage)
    float frequency, duty_cycle;
    if (measurePWM(AMPLITUDE_ALL_PIN, frequency, duty_cycle)) {
        status += "\"AMPLITUDE_ALL\":\"" + String(duty_cycle, 1) + "%\",";
    } else {
        status += "\"AMPLITUDE_ALL\":\"0.0%\",";
    }

    // Analog input pin
    status += "\"POWER_SENSE_4\":" + String(pin_states[POWER_SENSE_4_PIN].analog_value) + ",";

    // Add timestamp and connection info
    status += "\"timestamp\":\"" + String(millis()) + "\",";
    status += "\"connection_status\":\"CONNECTED\"";

    status += "}";

    return status;
}

int ATmegaInterface::mapPinName(String name) {
    name.toUpperCase();
    
    if (name == "FREQ_DIV10_4" || name == "D7") return FREQ_DIV10_4_PIN;
    if (name == "FREQ_LOCK_4" || name == "D8") return FREQ_LOCK_4_PIN;
    if (name == "OVERLOAD_4" || name == "A2") return OVERLOAD_4_PIN;
    if (name == "START_4" || name == "A3") return START_4_PIN;
    if (name == "RESET_4" || name == "A4") return RESET_4_PIN;
    if (name == "POWER_SENSE_4" || name == "A1") return POWER_SENSE_4_PIN;
    if (name == "AMPLITUDE_ALL" || name == "D9") return AMPLITUDE_ALL_PIN;
    if (name == "UART_RXD" || name == "D10") return UART_RXD_PIN;
    if (name == "UART_TXD" || name == "D11") return UART_TXD_PIN;
    if (name == "STATUS_LED" || name == "D12") return STATUS_LED_PIN;
    
    // Try parsing as direct pin number
    if (name.startsWith("D")) {
        return name.substring(1).toInt();
    }
    if (name.startsWith("A")) {
        return A0 + name.substring(1).toInt();
    }
    
    return -1;  // Invalid pin name
}

int ATmegaInterface::mapADCChannel(String name) {
    name.toUpperCase();
    
    if (name == "POWER_SENSE_4" || name == "A1") return A1;
    if (name == "OVERLOAD_4" || name == "A2") return A2;
    if (name == "START_4" || name == "A3") return A3;
    if (name == "RESET_4" || name == "A4") return A4;
    
    // Try parsing as direct ADC channel
    if (name.startsWith("A")) {
        int channel = name.substring(1).toInt();
        return A0 + channel;
    }
    
    return -1;  // Invalid ADC channel
}

bool ATmegaInterface::isReady() {
    return true;  // Always ready for now
}

uint16_t ATmegaInterface::modbusRead(uint16_t address) {
    if (address < 256) {
        return modbus_registers[address];
    }
    return 0;  // Invalid address
}

bool ATmegaInterface::modbusWrite(uint16_t address, uint16_t value) {
    if (address < 256) {
        modbus_registers[address] = value;
        
        // Handle special registers that control hardware
        if (address == 0x0100) {  // Sonicator 4 start register
            writePin(START_4_PIN, value != 0);
        } else if (address == 0x0101) {  // Sonicator 4 reset register
            writePin(RESET_4_PIN, value != 0);
        } else if (address == 0x0102) {  // Status LED register
            writePin(STATUS_LED_PIN, value != 0);
        }
        
        return true;
    }
    return false;  // Invalid address
}

void ATmegaInterface::setModbusSlaveId(uint8_t slave_id) {
    modbus_slave_id = slave_id;
}

void ATmegaInterface::setModbusBaudRate(uint32_t baud_rate) {
    modbus_baud_rate = baud_rate;
}

void ATmegaInterface::setStartInhibit(uint8_t unit_1_based, bool enable) {
    if (unit_1_based >= 1 && unit_1_based <= 4) {
        start_inhibit_flags[unit_1_based - 1] = enable;
    }
}

bool ATmegaInterface::getStartInhibit(uint8_t unit_1_based) const {
    if (unit_1_based >= 1 && unit_1_based <= 4) {
        return start_inhibit_flags[unit_1_based - 1];
    }
    return false;
}
