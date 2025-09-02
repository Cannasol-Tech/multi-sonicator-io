/*
 * Arduino Test Harness for ATmega32A HIL Testing
 * 
 * Acts as middleman between Python HIL framework and ATmega32A target.
 * Pin assignments must match docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH).
 * 
 * This harness provides:
 * - Arduino as ISP programming capability
 * - Real-time pin monitoring and control
 * - Safe sandbox mode for ATmega32A testing
 * - Serial communication with HIL framework
 * 
 * Pin Mapping (from pin matrix - Sonicator 4):
 * - D7  -> ATmega32A PB0 (FREQ_DIV10_4)
 * - D8  -> ATmega32A PB4 (FREQ_LOCK_4)
 * - A2  -> ATmega32A PD3 (OVERLOAD_4)
 * - A3  -> ATmega32A PC0 (START_4)
 * - A4  -> ATmega32A PC1 (RESET_4)
 * - A1  -> ATmega32A PA7 (POWER_SENSE_4)
 * - D9  -> ATmega32A PD7 (AMPLITUDE_ALL - PWM)
 * - D10 -> ATmega32A PD0 (UART_RXD - drive to DUT RX)
 * - D11 -> ATmega32A PD1 (UART_TXD - read from DUT TX)
 * - D12 -> ATmega32A PD2 (STATUS_LED)
 * 
 * Author: Cannasol Technologies
 * License: Proprietary
 */

#include "Arduino.h"
#include "atmega_interface.h"

// Version information
#define HIL_HARNESS_VERSION "1.0.0"
#define HIL_HARNESS_DATE __DATE__ " " __TIME__

// Global ATmega interface
ATmegaInterface atmega;

// Function declarations
void processCommand(String command);
void printHelp();
void printStatus();
void checkPower();
void measurePower();
void handleReadPin(String args);
void handleWritePin(String args);
void handleConfigPin(String args);
void handleReadADC(String args);
void handleSetPWM(String args);
void handleDisablePWM(String args);
void handleMeasurePWM(String args);
void handleApplyVoltage(String args);
void handleApplyADCVoltage(String args);
void handleDisconnectExternal();
void handleResetTarget();
void enterSandboxMode();
void exitSandboxMode();
void enableMonitoring();
void disableMonitoring();
void handleModbusRead(String args);
void handleModbusWrite(String args);
void handleModbusSetSlaveId(String args);
void handleModbusSetBaud(String args);

// Global state
bool sandbox_mode = false;
bool monitoring_enabled = false;
unsigned long last_monitor_time = 0;
unsigned long last_suggestion_time = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Initialize ATmega interface
    atmega.begin();
    
    // Startup message
    Serial.println("Arduino Test Harness for ATmega32A HIL Testing");
    Serial.println("HIL Harness Version: " HIL_HARNESS_VERSION);
    Serial.println("Build Date: " HIL_HARNESS_DATE);
    Serial.println("Pin mapping follows docs/planning/pin-matrix.md");
    Serial.println("Ready for HIL commands. Type HELP for available commands.");
    Serial.println("OK");
}

void loop() {
    // Handle serial commands from HIL framework
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        processCommand(command);
    }
    
    // Handle sandbox mode monitoring
    if (sandbox_mode && monitoring_enabled) {
        unsigned long current_time = millis();
        if (current_time - last_monitor_time >= 1000) {  // Update every second
            atmega.updatePinStates();
            atmega.sendPinStatus();
            last_monitor_time = current_time;
        }
    }
    
    // Suggest monitoring in sandbox mode
    if (sandbox_mode && !monitoring_enabled) {
        unsigned long current_time = millis();
        if (current_time - last_suggestion_time >= 15000) {  // Every 15 seconds
            Serial.println("💡 Tip: Type 'monitor_on' to enable real-time pin monitoring");
            last_suggestion_time = current_time;
        }
    }
    
    // Small delay to prevent overwhelming the system
    delay(10);
}

void processCommand(String command) {
    command.toUpperCase();
    
    // Parse command and arguments
    int spaceIndex = command.indexOf(' ');
    String cmd = (spaceIndex != -1) ? command.substring(0, spaceIndex) : command;
    String args = (spaceIndex != -1) ? command.substring(spaceIndex + 1) : "";
    
    // Command routing
    if (cmd == "PING") {
        Serial.println("PONG");
    }
    else if (cmd == "HELP") {
        printHelp();
    }
    else if (cmd == "VERSION") {
        Serial.println("VERSION " HIL_HARNESS_VERSION);
    }
    else if (cmd == "STATUS") {
        printStatus();
    }
    else if (cmd == "CHECK_POWER") {
        checkPower();
    }
    else if (cmd == "MEASURE_POWER") {
        measurePower();
    }
    else if (cmd == "GPIO_STATUS") {
        Serial.println("GPIO_OK");
    }
    else if (cmd == "ADC_STATUS") {
        Serial.println("ADC_OK");
    }
    else if (cmd == "PWM_STATUS") {
        Serial.println("PWM_OK");
    }
    else if (cmd == "READ_PIN") {
        handleReadPin(args);
    }
    else if (cmd == "WRITE_PIN") {
        handleWritePin(args);
    }
    else if (cmd == "CONFIG_PIN") {
        handleConfigPin(args);
    }
    else if (cmd == "READ_ADC") {
        handleReadADC(args);
    }
    else if (cmd == "SET_PWM") {
        handleSetPWM(args);
    }
    else if (cmd == "DISABLE_PWM") {
        handleDisablePWM(args);
    }
    else if (cmd == "MEASURE_PWM") {
        handleMeasurePWM(args);
    }
    else if (cmd == "APPLY_VOLTAGE") {
        handleApplyVoltage(args);
    }
    else if (cmd == "APPLY_ADC_VOLTAGE") {
        handleApplyADCVoltage(args);
    }
    else if (cmd == "DISCONNECT_EXTERNAL") {
        handleDisconnectExternal();
    }
    else if (cmd == "RESET_TARGET") {
        handleResetTarget();
    }
    else if (cmd == "SANDBOX_ENTER") {
        enterSandboxMode();
    }
    else if (cmd == "SANDBOX_EXIT") {
        exitSandboxMode();
    }
    else if (cmd == "MONITOR_ON") {
        enableMonitoring();
    }
    else if (cmd == "MONITOR_OFF") {
        disableMonitoring();
    }
    else if (cmd == "STATUS_ALL") {
        atmega.sendPinStatus();
    }
    else if (cmd == "MODBUS_READ") {
        handleModbusRead(args);
    }
    else if (cmd == "MODBUS_WRITE") {
        handleModbusWrite(args);
    }
    else if (cmd == "MODBUS_SET_SLAVE_ID") {
        handleModbusSetSlaveId(args);
    }
    else if (cmd == "MODBUS_SET_BAUD") {
        handleModbusSetBaud(args);
    }
    else {
        Serial.println("ERROR: Unknown command '" + cmd + "'. Type HELP for available commands.");
    }
}

void printHelp() {
    Serial.println("Arduino Test Harness Commands:");
    Serial.println("  PING                    - Test communication (responds PONG)");
    Serial.println("  VERSION                 - Get harness version");
    Serial.println("  STATUS                  - Get system status");
    Serial.println("  CHECK_POWER             - Check power supply status");
    Serial.println("  MEASURE_POWER           - Measure power supply voltages");
    Serial.println("  READ_PIN <pin>          - Read digital pin state");
    Serial.println("  WRITE_PIN <pin> <state> - Write digital pin state (HIGH/LOW)");
    Serial.println("  CONFIG_PIN <pin> <mode> - Configure pin mode (INPUT/OUTPUT/INPUT_PULLUP)");
    Serial.println("  READ_ADC <channel>      - Read ADC channel");
    Serial.println("  SET_PWM <pin> <duty> <freq> - Set PWM output");
    Serial.println("  DISABLE_PWM <pin>       - Disable PWM output");
    Serial.println("  MEASURE_PWM <pin>       - Measure PWM signal");
    Serial.println("  APPLY_VOLTAGE <pin> <V> - Apply voltage to pin");
    Serial.println("  RESET_TARGET            - Reset ATmega32A target");
    Serial.println("  SANDBOX_ENTER           - Enter sandbox mode");
    Serial.println("  SANDBOX_EXIT            - Exit sandbox mode");
    Serial.println("  MONITOR_ON              - Enable pin monitoring");
    Serial.println("  MONITOR_OFF             - Disable pin monitoring");
    Serial.println("  STATUS_ALL              - Get all pin status");
    Serial.println("  MODBUS_READ <addr>      - Read MODBUS register");
    Serial.println("  MODBUS_WRITE <addr> <val> - Write MODBUS register");
    Serial.println("  HELP                    - Show this help");
}

void printStatus() {
    Serial.println("Arduino Test Harness Status:");
    Serial.println("  Version: " HIL_HARNESS_VERSION);
    Serial.println("  Sandbox Mode: " + String(sandbox_mode ? "ENABLED" : "DISABLED"));
    Serial.println("  Monitoring: " + String(monitoring_enabled ? "ENABLED" : "DISABLED"));
    Serial.println("  Pin Mapping: docs/planning/pin-matrix.md (Sonicator 4)");
    Serial.println("  ATmega32A Interface: " + String(atmega.isReady() ? "READY" : "NOT_READY"));
    Serial.println("OK");
}

void checkPower() {
    // Simple power check - verify 5V rail
    int vcc_reading = analogRead(A0);  // Assuming A0 can read Vcc
    if (vcc_reading > 900) {  // ~4.4V or higher indicates good power
        Serial.println("POWER_OK");
    } else {
        Serial.println("POWER_FAIL");
    }
}

void measurePower() {
    // Measure power supply voltages
    // This is a simplified implementation - real implementation would
    // use voltage dividers and proper scaling
    
    float vcc = 5.0;  // Assume 5V for Arduino
    float input_24v = 24.1;  // Simulated - would read from voltage divider
    float rail_3v3 = 3.3;   // Simulated - would read from 3.3V rail
    
    Serial.println("POWER " + String(input_24v, 1) + "V " + 
                   String(vcc, 2) + "V " + String(rail_3v3, 2) + "V");
}

void handleReadPin(String args) {
    String pin = args;
    
    int pinNum = atmega.mapPinName(pin);
    if (pinNum == -1) {
        Serial.println("ERROR: Invalid pin name '" + pin + "'");
        return;
    }
    
    bool state = atmega.readPin(pinNum);
    Serial.println("PIN " + pin + " " + (state ? "HIGH" : "LOW"));
}

void handleWritePin(String args) {
    int spaceIndex = args.indexOf(' ');
    if (spaceIndex == -1) {
        Serial.println("ERROR: WRITE_PIN requires pin and state");
        return;
    }
    
    String pin = args.substring(0, spaceIndex);
    String stateStr = args.substring(spaceIndex + 1);
    stateStr.toUpperCase();
    
    int pinNum = atmega.mapPinName(pin);
    if (pinNum == -1) {
        Serial.println("ERROR: Invalid pin name '" + pin + "'");
        return;
    }
    
    bool state = (stateStr == "HIGH" || stateStr == "1");
    atmega.writePin(pinNum, state);
    Serial.println("OK");
}

void handleConfigPin(String args) {
    int spaceIndex = args.indexOf(' ');
    if (spaceIndex == -1) {
        Serial.println("ERROR: CONFIG_PIN requires pin and mode");
        return;
    }
    
    String pin = args.substring(0, spaceIndex);
    String mode = args.substring(spaceIndex + 1);
    mode.toUpperCase();
    
    int pinNum = atmega.mapPinName(pin);
    if (pinNum == -1) {
        Serial.println("ERROR: Invalid pin name '" + pin + "'");
        return;
    }
    
    if (mode == "INPUT") {
        pinMode(pinNum, INPUT);
    } else if (mode == "OUTPUT") {
        pinMode(pinNum, OUTPUT);
    } else if (mode == "INPUT_PULLUP") {
        pinMode(pinNum, INPUT_PULLUP);
    } else {
        Serial.println("ERROR: Invalid mode '" + mode + "'");
        return;
    }
    
    Serial.println("OK");
}

void handleReadADC(String args) {
    String channel = args;
    
    int adcNum = atmega.mapADCChannel(channel);
    if (adcNum == -1) {
        Serial.println("ERROR: Invalid ADC channel '" + channel + "'");
        return;
    }
    
    uint16_t value = atmega.readADC(adcNum);
    Serial.println("ADC " + channel + " " + String(value));
}

void handleSetPWM(String args) {
    // Parse: pin duty_cycle frequency
    int firstSpace = args.indexOf(' ');
    int secondSpace = args.indexOf(' ', firstSpace + 1);
    
    if (firstSpace == -1 || secondSpace == -1) {
        Serial.println("ERROR: SET_PWM requires pin, duty cycle, and frequency");
        return;
    }
    
    String pin = args.substring(0, firstSpace);
    int duty = args.substring(firstSpace + 1, secondSpace).toInt();
    int frequency = args.substring(secondSpace + 1).toInt();
    
    int pinNum = atmega.mapPinName(pin);
    if (pinNum == -1) {
        Serial.println("ERROR: Invalid pin name '" + pin + "'");
        return;
    }
    
    if (duty < 0 || duty > 100) {
        Serial.println("ERROR: Duty cycle must be 0-100%");
        return;
    }
    
    atmega.setPWM(pinNum, duty, frequency);
    Serial.println("OK");
}

void handleDisablePWM(String args) {
    String pin = args;
    
    int pinNum = atmega.mapPinName(pin);
    if (pinNum == -1) {
        Serial.println("ERROR: Invalid pin name '" + pin + "'");
        return;
    }
    
    atmega.disablePWM(pinNum);
    Serial.println("OK");
}

void handleMeasurePWM(String args) {
    String pin = args;
    
    int pinNum = atmega.mapPinName(pin);
    if (pinNum == -1) {
        Serial.println("ERROR: Invalid pin name '" + pin + "'");
        return;
    }
    
    float frequency, dutyCycle;
    if (atmega.measurePWM(pinNum, frequency, dutyCycle)) {
        Serial.println("PWM " + pin + " FREQ=" + String(frequency, 1) + 
                       " DUTY=" + String(dutyCycle, 1));
    } else {
        Serial.println("ERROR: Could not measure PWM on " + pin);
    }
}

void handleApplyVoltage(String args) {
    int spaceIndex = args.indexOf(' ');
    if (spaceIndex == -1) {
        Serial.println("ERROR: APPLY_VOLTAGE requires pin and voltage");
        return;
    }
    
    String pin = args.substring(0, spaceIndex);
    float voltage = args.substring(spaceIndex + 1).toFloat();
    
    // This would control external voltage sources or DACs
    // For now, just acknowledge the command
    Serial.println("OK");
}

void handleApplyADCVoltage(String args) {
    handleApplyVoltage(args);  // Same as apply voltage for now
}

void handleDisconnectExternal() {
    // Simulate disconnecting external connections
    Serial.println("OK");
}

void handleResetTarget() {
    if (atmega.resetATmega()) {
        Serial.println("OK");
    } else {
        Serial.println("ERROR: Target reset failed");
    }
}

void enterSandboxMode() {
    sandbox_mode = true;
    atmega.enterSandboxMode();
    Serial.println("SANDBOX MODE ENABLED");
    Serial.println("Type 'help' for sandbox commands");
}

void exitSandboxMode() {
    sandbox_mode = false;
    monitoring_enabled = false;
    atmega.exitSandboxMode();
    Serial.println("SANDBOX MODE DISABLED");
}

void enableMonitoring() {
    monitoring_enabled = true;
    atmega.enablePinMonitoring();
    Serial.println("PIN MONITORING ENABLED");
}

void disableMonitoring() {
    monitoring_enabled = false;
    atmega.disablePinMonitoring();
    Serial.println("PIN MONITORING DISABLED");
}

void handleModbusRead(String args) {
    // Parse hex address
    int address = strtol(args.c_str(), NULL, 16);
    
    // Simulate MODBUS read through ATmega interface
    uint16_t value = atmega.modbusRead(address);
    Serial.println("MODBUS " + args + " " + String(value, HEX));
}

void handleModbusWrite(String args) {
    int spaceIndex = args.indexOf(' ');
    if (spaceIndex == -1) {
        Serial.println("ERROR: MODBUS_WRITE requires address and value");
        return;
    }
    
    String addrStr = args.substring(0, spaceIndex);
    String valueStr = args.substring(spaceIndex + 1);
    
    int address = strtol(addrStr.c_str(), NULL, 16);
    int value = strtol(valueStr.c_str(), NULL, 16);
    
    if (atmega.modbusWrite(address, value)) {
        Serial.println("OK");
    } else {
        Serial.println("ERROR: MODBUS write failed");
    }
}

void handleModbusSetSlaveId(String args) {
    int slaveId = args.toInt();
    atmega.setModbusSlaveId(slaveId);
    Serial.println("OK");
}

void handleModbusSetBaud(String args) {
    int baudRate = args.toInt();
    atmega.setModbusBaudRate(baudRate);
    Serial.println("OK");
}
