/**
 * @file config.h
 * @brief Multi Sonicator I/O Controller - System Configuration
 * @author Cannasol Technologies
 * @date 2025-06-27
 * @version 1.0.0
 * 
 * Configuration constants and compile-time settings for the 
 * Multi Sonicator I/O Controller (Model: CAN-SON-CTRL-4)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// PROJECT INFORMATION
// ============================================================================
#define PROJECT_NAME "Multi Sonicator I/O Controller"
#define PROJECT_MODEL "CAN-SON-CTRL-4"
#define PROJECT_VERSION "1.0.0"
#define PROJECT_BUILD_DATE __DATE__ " " __TIME__
#define MANUFACTURER "Cannasol Technologies"

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

// Microcontroller specifications
#define MCU_TYPE "ATmega32A-PU"
#define MCU_FREQUENCY 16000000L
#define MCU_VOLTAGE 5.0

// System limitations
#define MAX_SONICATORS 4
#define MODBUS_SLAVE_ID 1
#define MODBUS_BAUD_RATE 115200

// Safety timing requirements
#define EMERGENCY_STOP_RESPONSE_US 50
#define WATCHDOG_TIMEOUT_MS 1000
#define SAFETY_MONITOR_INTERVAL_MS 10

// ============================================================================
// PIN DEFINITIONS - ATmega32A (DIP-40)
// ============================================================================

// UART Communication (MODBUS RTU)
#define UART_RX_PIN 14    // PD0 (Physical pin 14)
#define UART_TX_PIN 15    // PD1 (Physical pin 15)

// I2C Communication (DACs and ADCs)
#define I2C_SDA_PIN 22    // PC1 (Physical pin 22)
#define I2C_SCL_PIN 21    // PC0 (Physical pin 21)

// Crystal Oscillator
#define XTAL1_PIN 13      // XTAL1 (Physical pin 13)
#define XTAL2_PIN 12      // XTAL2 (Physical pin 12)

// Status LEDs
#define LED_POWER_PIN     8    // PB0 (Physical pin 1) 
#define LED_RUN_PIN       9    // PB1 (Physical pin 2)
#define LED_ERROR_PIN     10   // PB2 (Physical pin 3)
#define LED_COMM_PIN      11   // PB3 (Physical pin 4)

// Individual Sonicator Status LEDs (Front Panel)
#define LED_SONICATOR_1   24   // PC2 (Physical pin 24)
#define LED_SONICATOR_2   25   // PC3 (Physical pin 25)
#define LED_SONICATOR_3   26   // PC4 (Physical pin 26)
#define LED_SONICATOR_4   27   // PC5 (Physical pin 27)

// ============================================================================
// SONICATOR INTERFACE PIN MAPPING
// ============================================================================

// Sonicator 1 Interface
#define SON1_OVERLOAD_PIN     16   // PD2 (Physical pin 16) - Input
#define SON1_FREQ_OUTPUT_PIN  17   // PD3 (Physical pin 17) - Input  
#define SON1_FREQ_LOCK_PIN    18   // PD4 (Physical pin 18) - Input
#define SON1_START_PIN        19   // PD5 (Physical pin 19) - Output
#define SON1_RESET_PIN        20   // PD6 (Physical pin 20) - Output

// Sonicator 2 Interface  
#define SON2_OVERLOAD_PIN     21   // PD7 (Physical pin 21) - Input
#define SON2_FREQ_OUTPUT_PIN  1    // PB0 (Physical pin 1)  - Input
#define SON2_FREQ_LOCK_PIN    2    // PB1 (Physical pin 2)  - Input  
#define SON2_START_PIN        3    // PB2 (Physical pin 3)  - Output
#define SON2_RESET_PIN        4    // PB3 (Physical pin 4)  - Output

// Sonicator 3 Interface
#define SON3_OVERLOAD_PIN     23   // PC2 (Physical pin 23) - Input
#define SON3_FREQ_OUTPUT_PIN  24   // PC3 (Physical pin 24) - Input
#define SON3_FREQ_LOCK_PIN    25   // PC4 (Physical pin 25) - Input
#define SON3_START_PIN        22   // PC1 (Physical pin 22) - Output  
#define SON3_RESET_PIN        26   // PC5 (Physical pin 26) - Output

// Sonicator 4 Interface
#define SON4_OVERLOAD_PIN     27   // PC6 (Physical pin 27) - Input
#define SON4_FREQ_OUTPUT_PIN  28   // PC7 (Physical pin 28) - Input
#define SON4_FREQ_LOCK_PIN    29   // PC7 (Physical pin 29) - Input
#define SON4_START_PIN        5    // PB4 (Physical pin 5)  - Output
#define SON4_RESET_PIN        6    // PB5 (Physical pin 6)  - Output

// ============================================================================
// I2C DEVICE ADDRESSES  
// ============================================================================

// DAC Addresses (MCP4725) for Amplitude Control (0-10V)
#define DAC_SONICATOR_1_ADDR  0x60
#define DAC_SONICATOR_2_ADDR  0x61  
#define DAC_SONICATOR_3_ADDR  0x62
#define DAC_SONICATOR_4_ADDR  0x63

// ADC Addresses (ADS1115) for Power Monitoring
#define ADC_BANK_1_ADDR       0x48   // Sonicators 1 & 2
#define ADC_BANK_2_ADDR       0x49   // Sonicators 3 & 4

// ============================================================================
// CT2000 SONICATOR SPECIFICATIONS
// ============================================================================

// Operating parameters
#define SONICATOR_FREQ_NOMINAL    20000   // 20 kHz nominal frequency
#define SONICATOR_FREQ_TOLERANCE  20      // ±20 Hz tolerance (±0.1%)
#define SONICATOR_MAX_POWER       2000    // 2000W maximum power output
#define SONICATOR_MIN_AMPLITUDE   20      // 20% minimum amplitude
#define SONICATOR_MAX_AMPLITUDE   100     // 100% maximum amplitude

// Analog scaling factors
#define POWER_SCALING_MV_PER_WATT 5.44    // 5.44mV per watt (Pin 5)
#define AMPLITUDE_SCALE_V_PER_PCT 0.1     // 0.1V per percent (0-10V for 20-100%)
#define FREQUENCY_DIVIDER         10      // Pin 4 outputs frequency ÷ 10

// DB9 Pin Mapping (CT2000 Interface)
#define DB9_OVERLOAD_INDICATION   1       // Pin 1: Overload status (input)
#define DB9_OVERLOAD_RESET        2       // Pin 2: Reset overload (output)
#define DB9_FREQUENCY_LOCK        3       // Pin 3: Frequency lock (input)
#define DB9_FREQUENCY_OUTPUT      4       // Pin 4: Frequency out ÷10 (input)
#define DB9_POWER_OUTPUT          5       // Pin 5: Power level (input)
#define DB9_SIGNAL_GROUND         6       // Pin 6: Signal ground
#define DB9_SONICS_START          7       // Pin 7: Start control (output)
#define DB9_AMPLITUDE_CONTROL     8       // Pin 8: Amplitude 0-10V (output)
#define DB9_EMITTER_COMMON        9       // Pin 9: Open collector common

// ============================================================================
// MODBUS REGISTER MAPPING
// ============================================================================

// Control Registers (Read/Write)
#define REG_SONICATOR_1_AMPLITUDE    40001
#define REG_SONICATOR_2_AMPLITUDE    40002  
#define REG_SONICATOR_3_AMPLITUDE    40003
#define REG_SONICATOR_4_AMPLITUDE    40004
#define REG_SONICATOR_1_START_STOP   40005
#define REG_SONICATOR_2_START_STOP   40006
#define REG_SONICATOR_3_START_STOP   40007
#define REG_SONICATOR_4_START_STOP   40008
#define REG_SONICATOR_1_RESET        40009
#define REG_SONICATOR_2_RESET        40010
#define REG_SONICATOR_3_RESET        40011
#define REG_SONICATOR_4_RESET        40012

// Monitoring Registers (Read Only)
#define REG_SONICATOR_1_POWER        40013
#define REG_SONICATOR_2_POWER        40014
#define REG_SONICATOR_3_POWER        40015
#define REG_SONICATOR_4_POWER        40016
#define REG_SONICATOR_1_FREQUENCY    40017
#define REG_SONICATOR_2_FREQUENCY    40018
#define REG_SONICATOR_3_FREQUENCY    40019
#define REG_SONICATOR_4_FREQUENCY    40020
#define REG_SONICATOR_1_STATUS       40021
#define REG_SONICATOR_2_STATUS       40022
#define REG_SONICATOR_3_STATUS       40023
#define REG_SONICATOR_4_STATUS       40024

// System Registers
#define REG_SYSTEM_STATUS            40025
#define REG_EMERGENCY_STOP           40026
#define REG_SONICATOR_1_RUNTIME      40027
#define REG_SONICATOR_2_RUNTIME      40028
#define REG_SONICATOR_3_RUNTIME      40029
#define REG_SONICATOR_4_RUNTIME      40030
#define REG_SONICATOR_1_OVERLOAD_COUNT 40031
#define REG_SONICATOR_2_OVERLOAD_COUNT 40032
#define REG_SONICATOR_3_OVERLOAD_COUNT 40033
#define REG_SONICATOR_4_OVERLOAD_COUNT 40034

// ============================================================================
// TIMING AND PERFORMANCE CONSTANTS
// ============================================================================

#define MAIN_LOOP_INTERVAL_MS        10    // 10ms main loop (100 Hz)
#define MODBUS_TIMEOUT_MS            500   // 500ms MODBUS timeout
#define STATUS_UPDATE_INTERVAL_MS    100   // Status LED update rate
#define POWER_MONITOR_INTERVAL_MS    50    // Power monitoring rate
#define FREQUENCY_MEASURE_WINDOW_MS  100   // Frequency measurement window

// ============================================================================
// DEBUG AND DEVELOPMENT SETTINGS
// ============================================================================

#ifdef DEBUG_MODE
    #define DEBUG_SERIAL_SPEED       115200
    #define DEBUG_ENABLE_WATCHDOG    false
    #define DEBUG_VERBOSE_MODBUS     true
    #define DEBUG_LOG_LEVEL          3     // 0=none, 1=error, 2=warn, 3=info, 4=debug
#else
    #define DEBUG_ENABLE_WATCHDOG    true
    #define DEBUG_VERBOSE_MODBUS     false
    #define DEBUG_LOG_LEVEL          1
#endif

// ============================================================================
// ERROR CODES AND STATUS FLAGS
// ============================================================================

// System Status Bits (REG_SYSTEM_STATUS)
#define STATUS_SYSTEM_OK             0x0001
#define STATUS_EMERGENCY_STOP        0x0002
#define STATUS_COMMUNICATION_ERROR   0x0004
#define STATUS_POWER_FAULT           0x0008
#define STATUS_OVERLOAD_ACTIVE       0x0010
#define STATUS_WATCHDOG_RESET        0x0020

// Individual Sonicator Status Bits  
#define STATUS_SONICATOR_RUNNING     0x0001
#define STATUS_SONICATOR_OVERLOAD    0x0002
#define STATUS_SONICATOR_FREQ_LOCK   0x0004
#define STATUS_SONICATOR_COMM_ERROR  0x0008

// Error Codes
typedef enum {
    ERROR_NONE = 0,
    ERROR_MODBUS_TIMEOUT = 1,
    ERROR_MODBUS_CRC = 2,
    ERROR_SONICATOR_OVERLOAD = 3,
    ERROR_FREQUENCY_LOCK_LOST = 4,
    ERROR_POWER_OUT_OF_RANGE = 5,
    ERROR_I2C_DAC_FAILURE = 6,
    ERROR_I2C_ADC_FAILURE = 7,
    ERROR_WATCHDOG_TIMEOUT = 8,
    ERROR_EMERGENCY_STOP = 9,
    ERROR_SYSTEM_FAULT = 10
} error_code_t;

#endif // CONFIG_H
