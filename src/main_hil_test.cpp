/**
 * @file main_hil_test.cpp
 * @brief Hardware-in-the-Loop Test Firmware for HAL Verification
 * @author Cannasol Technologies  
 * @date 2025-09-02
 * @version 1.0.0
 * @framework Arduino Framework
 *
 * @brief HIL test firmware for HAL module verification
 * @details Provides command interface for HIL framework to test GPIO, ADC, PWM, UART, Timer modules
 */

#include <Arduino.h>
#include "modules/hal/hal.h"
#include "config.h"

// Command buffer for HIL communication
#define CMD_BUFFER_SIZE 64
char cmd_buffer[CMD_BUFFER_SIZE];
int cmd_index = 0;

// Test state variables
bool hal_initialized = false;

void setup() {
    // Initialize serial communication for HIL framework
    Serial.begin(115200);
    
    // Send startup message
    Serial.println("HIL_TEST_READY");
    Serial.flush();
    
    // Initialize HAL system
    hal_result_t result = hal_init();
    if (result == HAL_OK) {
        hal_initialized = true;
        Serial.println("STATUS HAL_INITIALIZED");
    } else {
        Serial.print("ERROR HAL_INIT_FAILED ");
        Serial.println(result);
    }
    
    Serial.flush();
}

void loop() {
    // Process HIL commands
    if (Serial.available()) {
        char c = Serial.read();
        
        if (c == '\n' || c == '\r') {
            // Process complete command
            cmd_buffer[cmd_index] = '\0';
            processCommand(cmd_buffer);
            cmd_index = 0;
        } else if (cmd_index < CMD_BUFFER_SIZE - 1) {
            cmd_buffer[cmd_index++] = c;
        }
    }
}

void processCommand(const char* cmd) {
    // Parse command string
    char command[16];
    char param1[16];
    char param2[16];
    char param3[16];
    
    int parsed = sscanf(cmd, "%s %s %s %s", command, param1, param2, param3);
    
    // System commands
    if (strcmp(command, "PING") == 0) {
        Serial.println("OK PONG");
    }
    else if (strcmp(command, "STATUS") == 0) {
        Serial.print("OK HAL_STATUS ");
        Serial.println(hal_initialized ? "READY" : "NOT_READY");
    }
    
    // GPIO commands
    else if (strcmp(command, "GPIO_STATUS") == 0) {
        if (hal_initialized) {
            Serial.println("OK GPIO_OK");
        } else {
            Serial.println("ERROR GPIO_NOT_READY");
        }
    }
    else if (strcmp(command, "CONFIG_PIN") == 0 && parsed >= 3) {
        processGPIOConfig(param1, param2);
    }
    else if (strcmp(command, "SET_PIN") == 0 && parsed >= 3) {
        processGPIOSet(param1, param2);
    }
    else if (strcmp(command, "READ_PIN") == 0 && parsed >= 2) {
        processGPIORead(param1);
    }
    
    // ADC commands
    else if (strcmp(command, "READ_ADC") == 0 && parsed >= 2) {
        processADCRead(param1);
    }
    else if (strcmp(command, "READ_VOLTAGE") == 0 && parsed >= 2) {
        processVoltageRead(param1);
    }
    
    // PWM commands
    else if (strcmp(command, "SET_PWM") == 0 && parsed >= 4) {
        processPWMSet(param1, param2, param3);
    }
    else if (strcmp(command, "PWM_ENABLE") == 0 && parsed >= 2) {
        processPWMEnable(param1);
    }
    else if (strcmp(command, "PWM_DISABLE") == 0 && parsed >= 2) {
        processPWMDisable(param1);
    }
    
    // Timer commands
    else if (strcmp(command, "GET_MILLIS") == 0) {
        processGetMillis();
    }
    
    else {
        Serial.print("ERROR UNKNOWN_COMMAND ");
        Serial.println(command);
    }
    
    Serial.flush();
}

// GPIO command processors
void processGPIOConfig(const char* pin_name, const char* mode) {
    uint8_t pin = getPinFromName(pin_name);
    if (pin == 255) {
        Serial.print("ERROR INVALID_PIN ");
        Serial.println(pin_name);
        return;
    }
    
    gpio_mode_t gpio_mode;
    if (strcmp(mode, "INPUT") == 0) {
        gpio_mode = GPIO_MODE_INPUT;
    } else if (strcmp(mode, "INPUT_PULLUP") == 0) {
        gpio_mode = GPIO_MODE_INPUT_PULLUP;
    } else if (strcmp(mode, "OUTPUT") == 0) {
        gpio_mode = GPIO_MODE_OUTPUT;
    } else {
        Serial.print("ERROR INVALID_MODE ");
        Serial.println(mode);
        return;
    }
    
    gpio_result_t result = gpio_set_pin_mode(pin, gpio_mode);
    if (result == GPIO_OK) {
        Serial.print("OK PIN_CONFIGURED ");
        Serial.print(pin_name);
        Serial.print(" ");
        Serial.println(mode);
    } else {
        Serial.print("ERROR CONFIG_FAILED ");
        Serial.print(pin_name);
        Serial.print(" ");
        Serial.println(result);
    }
}

void processGPIOSet(const char* pin_name, const char* state) {
    uint8_t pin = getPinFromName(pin_name);
    if (pin == 255) {
        Serial.print("ERROR INVALID_PIN ");
        Serial.println(pin_name);
        return;
    }
    
    gpio_state_t pin_state = (strcmp(state, "HIGH") == 0 || strcmp(state, "1") == 0) ? GPIO_HIGH : GPIO_LOW;
    
    gpio_result_t result = gpio_write_pin(pin, pin_state);
    if (result == GPIO_OK) {
        Serial.print("OK PIN_SET ");
        Serial.print(pin_name);
        Serial.print(" ");
        Serial.println(state);
    } else {
        Serial.print("ERROR SET_FAILED ");
        Serial.print(pin_name);
        Serial.print(" ");
        Serial.println(result);
    }
}

void processGPIORead(const char* pin_name) {
    uint8_t pin = getPinFromName(pin_name);
    if (pin == 255) {
        Serial.print("ERROR INVALID_PIN ");
        Serial.println(pin_name);
        return;
    }
    
    gpio_state_t pin_state;
    gpio_result_t result = gpio_read_pin(pin, &pin_state);
    if (result == GPIO_OK) {
        Serial.print("OK PIN_READ ");
        Serial.print(pin_name);
        Serial.print(" ");
        Serial.println(pin_state == GPIO_HIGH ? "HIGH" : "LOW");
    } else {
        Serial.print("ERROR READ_FAILED ");
        Serial.print(pin_name);
        Serial.print(" ");
        Serial.println(result);
    }
}

// ADC command processors
void processADCRead(const char* channel_name) {
    adc_channel_t channel = getADCChannelFromName(channel_name);
    if (channel == ADC_CHANNEL_MAX) {
        Serial.print("ERROR INVALID_CHANNEL ");
        Serial.println(channel_name);
        return;
    }
    
    uint16_t adc_value;
    adc_result_t result = adc_read_channel(channel, &adc_value);
    if (result == ADC_OK) {
        Serial.print("OK ADC_READ ");
        Serial.print(channel_name);
        Serial.print(" ");
        Serial.println(adc_value);
    } else {
        Serial.print("ERROR ADC_READ_FAILED ");
        Serial.print(channel_name);
        Serial.print(" ");
        Serial.println(result);
    }
}

void processVoltageRead(const char* channel_name) {
    uint8_t channel = getADCChannelFromName(channel_name);
    if (channel == 255) {
        Serial.print("ERROR INVALID_CHANNEL ");
        Serial.println(channel_name);
        return;
    }
    
    float voltage;
    adc_result_t result = adc_read_voltage(channel, &voltage);
    if (result == ADC_OK) {
        Serial.print("OK VOLTAGE_READ ");
        Serial.print(channel_name);
        Serial.print(" ");
        Serial.println(voltage, 3);
    } else {
        Serial.print("ERROR VOLTAGE_READ_FAILED ");
        Serial.print(channel_name);
        Serial.print(" ");
        Serial.println(result);
    }
}

// PWM command processors
void processPWMSet(const char* channel_name, const char* freq_str, const char* duty_str) {
    uint8_t channel = getPWMChannelFromName(channel_name);
    if (channel == 255) {
        Serial.print("ERROR INVALID_PWM_CHANNEL ");
        Serial.println(channel_name);
        return;
    }
    
    uint32_t frequency = atol(freq_str);
    uint8_t duty_cycle = atoi(duty_str);
    
    pwm_result_t result1 = pwm_set_frequency(channel, frequency);
    pwm_result_t result2 = pwm_set_duty_cycle(channel, duty_cycle);
    
    if (result1 == PWM_OK && result2 == PWM_OK) {
        Serial.print("OK PWM_SET ");
        Serial.print(channel_name);
        Serial.print(" ");
        Serial.print(frequency);
        Serial.print(" ");
        Serial.println(duty_cycle);
    } else {
        Serial.print("ERROR PWM_SET_FAILED ");
        Serial.print(channel_name);
        Serial.print(" ");
        Serial.print(result1);
        Serial.print(" ");
        Serial.println(result2);
    }
}

void processPWMEnable(const char* channel_name) {
    uint8_t channel = getPWMChannelFromName(channel_name);
    if (channel == 255) {
        Serial.print("ERROR INVALID_PWM_CHANNEL ");
        Serial.println(channel_name);
        return;
    }
    
    pwm_result_t result = pwm_enable_channel(channel);
    if (result == PWM_OK) {
        Serial.print("OK PWM_ENABLED ");
        Serial.println(channel_name);
    } else {
        Serial.print("ERROR PWM_ENABLE_FAILED ");
        Serial.print(channel_name);
        Serial.print(" ");
        Serial.println(result);
    }
}

void processPWMDisable(const char* channel_name) {
    uint8_t channel = getPWMChannelFromName(channel_name);
    if (channel == 255) {
        Serial.print("ERROR INVALID_PWM_CHANNEL ");
        Serial.println(channel_name);
        return;
    }
    
    pwm_result_t result = pwm_disable_channel(channel);
    if (result == PWM_OK) {
        Serial.print("OK PWM_DISABLED ");
        Serial.println(channel_name);
    } else {
        Serial.print("ERROR PWM_DISABLE_FAILED ");
        Serial.print(channel_name);
        Serial.print(" ");
        Serial.println(result);
    }
}

// Timer command processors
void processGetMillis() {
    uint32_t millis = timer_get_millis();
    Serial.print("OK MILLIS ");
    Serial.println(millis);
}

// Helper functions for pin/channel mapping
uint8_t getPinFromName(const char* pin_name) {
    // Map pin names to physical pins based on pin matrix
    if (strcmp(pin_name, "START_4") == 0) return PIN_START_4;
    if (strcmp(pin_name, "RESET_4") == 0) return PIN_RESET_4;
    if (strcmp(pin_name, "OVERLOAD_4") == 0) return PIN_OVERLOAD_4;
    if (strcmp(pin_name, "FREQ_LOCK_4") == 0) return PIN_FREQ_LOCK_4;
    if (strcmp(pin_name, "STATUS_LED") == 0) return PIN_STATUS_LED;
    return 255; // Invalid pin
}

uint8_t getADCChannelFromName(const char* channel_name) {
    // Map channel names to ADC channels
    if (strcmp(channel_name, "POWER_SENSE_4") == 0) return ADC_CHANNEL_POWER_SENSE_4;
    return 255; // Invalid channel
}

uint8_t getPWMChannelFromName(const char* channel_name) {
    // Map channel names to PWM channels
    if (strcmp(channel_name, "AMPLITUDE_ALL") == 0) return PWM_CHANNEL_AMPLITUDE;
    return 255; // Invalid channel
}
