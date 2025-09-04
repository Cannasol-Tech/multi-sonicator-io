/*
 * ATmega Interface - Core interface for controlling ATmega32A from Arduino Test Harness
 * 
 * This module provides the interface between the Arduino Test Harness and the 
 * ATmega32A target for HIL testing. Pin assignments follow the pin matrix exactly.
 * 
 * Author: Cannasol Technologies
 * License: Proprietary
 */

#ifndef ATMEGA_INTERFACE_H
#define ATMEGA_INTERFACE_H

#include "Arduino.h"

class ATmegaInterface {
public:
    void begin();
    void programATmega(const char* firmware_path);
    bool resetATmega();
    void enterSandboxMode();
    void exitSandboxMode();
    
    // Pin monitoring and control
    void enablePinMonitoring();
    void disablePinMonitoring();
    bool readPin(uint8_t pin);
    void writePin(uint8_t pin, bool state);
    uint16_t readADC(uint8_t channel);
    
    // PWM control
    void setPWM(uint8_t pin, uint8_t duty_percent, uint16_t frequency);
    void disablePWM(uint8_t pin);
    bool measurePWM(uint8_t pin, float &frequency, float &duty_cycle);
    
    // Real-time monitoring
    void updatePinStates();
    void sendPinStatus();
    
    // Pin mapping utilities
    int mapPinName(String name);
    int mapADCChannel(String name);
    
    // Status
    bool isReady();
    
    // MODBUS simulation
    uint16_t modbusRead(uint16_t address);
    bool modbusWrite(uint16_t address, uint16_t value);
    void setModbusSlaveId(uint8_t slave_id);
    void setModbusBaudRate(uint32_t baud_rate);
    
private:
    bool sandbox_mode_active;
    bool monitoring_enabled;
    unsigned long last_monitor_update;
    
    // Pin state tracking
    struct PinState {
        bool digital_state;
        uint16_t analog_value;
        unsigned long last_update;
    };
    
    PinState pin_states[20];  // Track up to 20 pins
    
    // MODBUS simulation state
    uint8_t modbus_slave_id;
    uint32_t modbus_baud_rate;
    uint16_t modbus_registers[256];  // Simulate 256 registers
    
    // Internal methods
    void initializePins();
    void updatePinState(uint8_t pin);
    String formatPinStatus();
};

#endif // ATMEGA_INTERFACE_H
