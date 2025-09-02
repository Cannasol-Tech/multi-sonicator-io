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
#define FREQ_DIV10_4_PIN    7   // D7  -> PB0 (FREQ_DIV10_4)
#define FREQ_LOCK_4_PIN     8   // D8  -> PB4 (FREQ_LOCK_4)
#define OVERLOAD_4_PIN      A2  // A2  -> PD3 (OVERLOAD_4)
#define START_4_PIN         A3  // A3  -> PC0 (START_4)
#define RESET_4_PIN         A4  // A4  -> PC1 (RESET_4)
#define POWER_SENSE_4_PIN   A1  // A1  -> PA7 (POWER_SENSE_4)
#define AMPLITUDE_ALL_PIN   9   // D9  -> PD7 (AMPLITUDE_ALL - PWM)
#define UART_RXD_PIN        10  // D10 -> PD0 (UART_RXD - drive to DUT RX)
#define UART_TXD_PIN        11  // D11 -> PD1 (UART_TXD - read from DUT TX)
#define STATUS_LED_PIN      12  // D12 -> PD2 (STATUS_LED)

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
    
    // Set some default register values
    modbus_registers[0x0000] = 0x0001;  // System status - online
    modbus_registers[0x0001] = 0x0000;  // Fault status - no faults
    modbus_registers[0x0002] = 0x0001;  // Mode register - normal mode
    
    initializePins();
}

void ATmegaInterface::initializePins() {
    // Configure pins according to pin matrix
    // Inputs (from ATmega32A perspective, outputs from Arduino perspective)
    pinMode(START_4_PIN, OUTPUT);       // START_4 - Arduino drives this
    pinMode(RESET_4_PIN, OUTPUT);       // RESET_4 - Arduino drives this
    pinMode(AMPLITUDE_ALL_PIN, OUTPUT); // AMPLITUDE_ALL - Arduino drives PWM
    pinMode(UART_RXD_PIN, OUTPUT);      // UART_RXD - Arduino drives to DUT RX
    pinMode(STATUS_LED_PIN, OUTPUT);    // STATUS_LED - Arduino drives this
    
    // Outputs (from ATmega32A perspective, inputs to Arduino)
    pinMode(FREQ_DIV10_4_PIN, INPUT);   // FREQ_DIV10_4 - Arduino reads this
    pinMode(FREQ_LOCK_4_PIN, INPUT);    // FREQ_LOCK_4 - Arduino reads this
    pinMode(OVERLOAD_4_PIN, INPUT);     // OVERLOAD_4 - Arduino reads this
    pinMode(UART_TXD_PIN, INPUT);       // UART_TXD - Arduino reads from DUT TX
    
    // Analog inputs
    pinMode(POWER_SENSE_4_PIN, INPUT);  // POWER_SENSE_4 - Arduino reads ADC
    
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
    // Simplified PWM measurement
    // In a real implementation, this would use input capture or timing
    
    // For now, return simulated values based on what was set
    frequency = 1000.0;  // Assume 1kHz
    
    // Read current pin state and estimate duty cycle
    int analog_value = analogRead(pin);
    duty_cycle = map(analog_value, 0, 1023, 0, 100);
    
    return true;
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

    // Update PWM pin (read current PWM value)
    pin_states[AMPLITUDE_ALL_PIN].analog_value = analogRead(AMPLITUDE_ALL_PIN);
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

    // PWM output pin
    status += "\"AMPLITUDE_ALL\":" + String(analogRead(AMPLITUDE_ALL_PIN)) + ",";

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
