/**
 * @file config.h
 * @brief Multi Sonicator I/O Controller configuration
 * @author Cannasol Technologies
 * @date 2025-06-27
 * @version 1.0.0
 * @framework Arduino Framework (chosen for rapid development and proven libraries)
 * @details
 * Configuration constants and compile-time settings for the
 * Multi Sonicator I/O Controller (Model: CAN-SON-CTRL-4).
 * Uses Arduino Framework for UART, I2C, PWM, and ADC operations.
 * See docs/planning/pin-matrix.md — SOLE SOURCE OF TRUTH for hardware pin assignments. This file mirrors the matrix; do not modify mappings here without updating the matrix first.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ----------------------------------------------------------------------------
// See docs/planning/pin-matrix.md — SOLE SOURCE OF TRUTH for hardware pin assignments. This file mirrors the matrix; do not modify mappings here without updating the matrix first.
// -----------------------------------------------------------------------------

// Backward-compatible aliases for unit tests targeting legacy pin names


#ifndef PWM_SONICATOR_1_PIN
#define PWM_SONICATOR_1_PIN  PWM_AMPLITUDE_CONTROL_PIN
#endif
#ifndef PWM_SONICATOR_2_PIN
#define PWM_SONICATOR_2_PIN  PWM_AMPLITUDE_CONTROL_PIN
#endif
#ifndef PWM_SONICATOR_3_PIN
#define PWM_SONICATOR_3_PIN  PWM_AMPLITUDE_CONTROL_PIN
#endif
#ifndef PWM_SONICATOR_4_PIN
#define PWM_SONICATOR_4_PIN  PWM_AMPLITUDE_CONTROL_PIN
#endif

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
#define MCU_VOLTAGE 5.0           // 5V power to the ATMEGA32

// System limitations
#define MAX_SONICATORS 4          // Maximum number of sonicators supported
#define MODBUS_SLAVE_ID 2          // MODBUS RTU slave address
#define MODBUS_BAUD_RATE 115200

// Safety timing requirements
#define EMERGENCY_STOP_RESPONSE_US 50
#define WATCHDOG_TIMEOUT_MS 1000
#define SAFETY_MONITOR_INTERVAL_MS 10

// ============================================================================
// PIN DEFINITIONS - ATmega32A (DIP-40) - Arduino Framework Compatible
// ============================================================================

// UART Communication (MODBUS RTU) - Arduino Serial uses these automatically
#define UART_RX_PIN 0     // PD0 (Physical pin 14) - DB9-0 Pin 8 (MODBUS RTU RX) - See pin-matrix.md
#define UART_TX_PIN 1     // PD1 (Physical pin 15) - DB9-0 Pin 9 (MODBUS RTU TX) - See pin-matrix.md

// I2C Interface (Available for Future Expansion) - Arduino Wire library compatible
#define I2C_SDA_PIN 22    // PC1 (Physical pin 22)
#define I2C_SCL_PIN 21    // PC0 (Physical pin 21)

// Crystal Oscillator (16MHz external - handled by Arduino framework)
#define XTAL1_PIN 13      // XTAL1 (Physical pin 13)
#define XTAL2_PIN 12      // XTAL2 (Physical pin 12)

// Status LED (single LED terminal) - Direct register access
#define STATUS_LED_PIN 2      // PD2 (Physical pin 16) - LED-TERM (DB9-0 Pin) - See pin-matrix.md

// Serial Communication Configuration
#define SERIAL_BAUD_RATE 115200  // Debug serial communication speed

/**
 * @defgroup SonicatorInterface Sonicator Interface Pin Mapping
 * @brief ATmega32A pin assignments for CT2000 sonicator control interfaces
 * @details Based on schematic design with 6N137 optocouplers and ULN2003A drivers.
 * See also: docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH) and PRD sections.
 * @{
 */

/**
 * @defgroup Sonicator1 Sonicator 1 Interface (DB9 Connector 1)
 * @brief Pin definitions for first sonicator interface
 * @{
 */
#define SON1_OVERLOAD_PIN     20   ///< PD6 (Physical pin 20) - Input via 6N137
#define SON1_FREQ_OUTPUT_PIN  4    ///< PB3 (Physical pin 4)  - FREQ ÷10 input
#define SON1_FREQ_LOCK_PIN    8    ///< PB7 (Physical pin 8)  - Input via 6N137
#define SON1_START_PIN        28   ///< PC6 (Physical pin 28) - Output via ULN2003A (aligned to pin-matrix)
#define SON1_RESET_PIN        29   ///< PC7 (Physical pin 29) - Output via ULN2003A (aligned to pin-matrix)
/** @} */

/**
 * @defgroup Sonicator2 Sonicator 2 Interface (DB9 Connector 2)
 * @brief Pin definitions for second sonicator interface
 * @{
 */
#define SON2_OVERLOAD_PIN     19   ///< PD5 (Physical pin 19) - Input via 6N137
#define SON2_FREQ_OUTPUT_PIN  3    ///< PB2 (Physical pin 3)  - FREQ ÷10 input
#define SON2_FREQ_LOCK_PIN    7    ///< PB6 (Physical pin 7)  - Input via 6N137
#define SON2_START_PIN        26   ///< PC4 (Physical pin 26) - Output via ULN2003A (aligned to pin-matrix)
#define SON2_RESET_PIN        27   ///< PC5 (Physical pin 27) - Output via ULN2003A (aligned to pin-matrix)
/** @} */

/**
 * @defgroup Sonicator3 Sonicator 3 Interface (DB9 Connector 3)
 * @brief Pin definitions for third sonicator interface
 * @{
 */
#define SON3_OVERLOAD_PIN     18   ///< PD4 (Physical pin 18) - Input via 6N137
#define SON3_FREQ_OUTPUT_PIN  2    ///< PB1 (Physical pin 2)  - FREQ ÷10 input
#define SON3_FREQ_LOCK_PIN    6    ///< PB5 (Physical pin 6)  - Input via 6N137
#define SON3_START_PIN        24   ///< PC2 (Physical pin 24) - Output via ULN2003A (aligned to pin-matrix)
#define SON3_RESET_PIN        25   ///< PC3 (Physical pin 25) - Output via ULN2003A (aligned to pin-matrix)
/** @} */

/**
 * @defgroup Sonicator4 Sonicator 4 Interface (DB9 Connector 4)
 * @brief Pin definitions for fourth sonicator interface
 * @{
 */
#define SON4_OVERLOAD_PIN     17   ///< PD3 (Physical pin 17) - Input via 6N137 (A2)
#define SON4_FREQ_OUTPUT_PIN  1    ///< PB0 (Physical pin 1)  - FREQ ÷10 input (D7)
#define SON4_FREQ_LOCK_PIN    5    ///< PB4 (Physical pin 5)  - Input via 6N137 (D8)
#define SON4_START_PIN        22   ///< PC0 (Physical pin 22) - Output via ULN2003A (A3)
#define SON4_RESET_PIN        23   ///< PC1 (Physical pin 23) - Output via ULN2003A (A4)
/** @} */

/** @} */

/**
 * @defgroup PWMControl PWM Amplitude Control Configuration
 * @brief Single PWM output for amplitude control of all 4 sonicators simultaneously
 * @details Based on schematic design with 1kΩ resistor + 10µF ceramic capacitor filter
 * @{
 */

/** @brief Single PWM pin for amplitude control (0-10V output) */
#define PWM_AMPLITUDE_CONTROL_PIN   21   ///< PD7 (Physical pin 21) - AMP_C per schematic

/**
 * @brief PWM Filter Configuration
 * - 1kΩ resistor + 10µF ceramic capacitor
 * - Cutoff frequency: fc = 1/(2πRC) = 16Hz
 * - LM358N op-amp provides 2x gain (0-5V PWM → 0-10V output)
 * - Controls all 4 sonicator amplitudes simultaneously to limit current draw
 */
/** @} */

/**
 * @defgroup ADCPowerMonitoring ADC Power Monitoring Configuration
 * @brief Built-in ADC pin assignments for sonicator power monitoring
 * @details 10kΩ voltage divider with input protection for 0-10.88V range
 * @{
 */

/** @brief ADC pin assignments for power monitoring */
#define ADC_SONICATOR_1_PIN   A4   ///< PA4 (Physical pin 36) - ADC4
#define ADC_SONICATOR_2_PIN   A5   ///< PA5 (Physical pin 35) - ADC5
#define ADC_SONICATOR_3_PIN   A6   ///< PA6 (Physical pin 34) - ADC6
#define ADC_SONICATOR_4_PIN   A7   ///< PA7 (Physical pin 33) - ADC7

/**
 * @brief ADC Configuration (per channel)
 * - 10kΩ voltage divider for input protection
 * - 0-10.88V input range (2000W × 5.44mV/W)
 * - 0-5.44V after divider, 0-1024 ADC counts
 * - Resolution: ~2 watts per count
 */
/** @} */

/**
 * @defgroup LM2907FrequencyConverter LM2907 Frequency-to-Voltage Converter
 * @brief Configuration for LM2907 frequency-to-voltage converter (Sonicator 4)
 * @details Converts CT2000 frequency output to 0-5V analog signal for ADC measurement
 * @{
 */

/** @brief LM2907 frequency-to-voltage converter configuration */
#define LM2907_FREQ_INPUT_PIN     A0   ///< PA0 (Physical pin 40) - ADC0
#define LM2907_FREQ_MIN_VOLTAGE   0.0  ///< Minimum voltage (0Hz)
#define LM2907_FREQ_MAX_VOLTAGE   5.0  ///< Maximum voltage (2500Hz)
#define LM2907_FREQ_SCALE_FACTOR  500  ///< Hz per volt (2500Hz / 5V)

/**
 * @brief LM2907 Scaling Configuration
 * - Input: 1.9-2.1 kHz (CT2000 frequency ÷ 10)
 * - Output: 1.9-2.1V (at 1V/kHz scaling)
 * - ADC Reading: 389-430 counts (at 5V reference)
 * - Frequency Resolution: ~5Hz per ADC count
 */
/** @} */

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

/**
 * @name Active Sonicator Summary Registers
 * @brief System-level summary of currently running sonicators
 * @details
 *  - REG_ACTIVE_SONICATOR_COUNT: Number of sonicators actively running (0-4)
 *  - REG_ACTIVE_SONICATOR_MASK:  Bitmask of running units (bit0=S1 .. bit3=S4)
 *  - Both are Read-Only holding registers and should reflect changes within
 *    100 ms of any start/stop or reset event.
 */
#define REG_ACTIVE_SONICATOR_COUNT    40035  ///< RO: 0-4 currently running
#define REG_ACTIVE_SONICATOR_MASK     40036  ///< RO: bitmask of running units

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
    ERROR_PWM_AMPLITUDE_FAILURE = 6,
    ERROR_ADC_POWER_FAILURE = 7,
    ERROR_WATCHDOG_TIMEOUT = 8,
    ERROR_EMERGENCY_STOP = 9,
    ERROR_SYSTEM_FAULT = 10
} error_code_t;

#endif // CONFIG_H
