/**
 * @file types.h
 * @brief Multi Sonicator I/O Controller - Common Type Definitions
 * @author Cannasol Technologies
 * @date 2025-06-27
 * @version 1.0.0
 * @framework Arduino Framework (chosen for rapid development and proven libraries)
 *
 * Common data structures, enumerations, and type definitions
 * used throughout the Multi Sonicator I/O Controller system.
 * Optimized for Arduino Framework with ATmega32A microcontroller.
 */

#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>
#include "system_config.h"

// Include the proper header files to avoid redeclaration conflicts
#include "sonicator_control.h"
#include "modules/hal/hal.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================
class CT2000Sonicator;
class SystemController;
class ModbusRTUSlave;
class SafetyMonitor;
class DiagnosticsSystem;

// ============================================================================
// ENUMERATION TYPES
// ============================================================================

/**
 * @brief System operating modes
 */
typedef enum {
    SYSTEM_MODE_INIT = 0,
    SYSTEM_MODE_NORMAL = 1,
    SYSTEM_MODE_SAFETY = 2,
    SYSTEM_MODE_MAINTENANCE = 3,
    SYSTEM_MODE_ERROR = 4
} system_mode_t;

/**
 * @brief MODBUS function codes (subset for this application)
 */
typedef enum {
    MODBUS_FUNC_READ_HOLDING_REGISTERS = 0x03,
    MODBUS_FUNC_WRITE_SINGLE_REGISTER = 0x06,
    MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS = 0x10
} modbus_function_t;

/**
 * @brief Safety event types for logging and alarm management
 */
typedef enum {
    SAFETY_EVENT_NONE = 0,
    SAFETY_EVENT_EMERGENCY_STOP = 1,
    SAFETY_EVENT_OVERLOAD_DETECTED = 2,
    SAFETY_EVENT_FREQUENCY_LOCK_LOST = 3,
    SAFETY_EVENT_POWER_FAULT = 4,
    SAFETY_EVENT_COMMUNICATION_TIMEOUT = 5,
    SAFETY_EVENT_WATCHDOG_TIMEOUT = 6,
    SAFETY_EVENT_SYSTEM_FAULT = 7
} safety_event_t;

// ============================================================================
// STRUCTURE DEFINITIONS
// ============================================================================

/**
 * @brief System-wide status and health information
 */
typedef struct {
    // Overall system state
    system_mode_t operating_mode;        //< Current system mode
    uint16_t system_status_flags;        //< Status bit field (see system_config.h)
    bool emergency_stop_active;          //< Master emergency stop state
    uint32_t system_uptime_seconds;      //< Total system runtime
    
    // Connected sonicators
    uint8_t sonicator_count;             //< Number of connected sonicators
    uint8_t active_sonicator_mask;       //< Bitmask of active sonicators
    
    // Communication status
    bool modbus_communication_ok;        //< MODBUS RTU communication status
    uint32_t modbus_request_count;       //< Total MODBUS requests received
    uint32_t modbus_error_count;         //< MODBUS communication errors
    uint32_t last_modbus_activity;       //< Timestamp of last MODBUS activity
    
    // Performance monitoring
    uint16_t cpu_usage_percent;          //< Estimated CPU utilization
    uint16_t memory_usage_bytes;         //< Dynamic memory usage
    uint16_t main_loop_frequency_hz;     //< Actual main loop rate
    
    // Error tracking
    error_code_t last_system_error;      //< Most recent system error
    uint32_t total_error_count;          //< Total error events
    uint32_t watchdog_reset_count;       //< Number of watchdog resets
} system_status_t;

/**
 * @brief MODBUS RTU frame structure for parsing
 */
typedef struct {
    uint8_t slave_address;               //< Target slave address
    uint8_t function_code;               //< MODBUS function code
    uint16_t starting_address;           //< Register starting address
    uint16_t quantity;                   //< Number of registers
    uint8_t byte_count;                  //< Data byte count
    uint8_t* data;                       //< Pointer to data payload
    uint16_t crc;                        //< CRC-16 checksum
    uint8_t frame_length;                //< Total frame length in bytes
} modbus_frame_t;

/**
 * @brief Safety event log entry for audit trail
 */
typedef struct {
    uint32_t timestamp;                  //< Event timestamp (seconds since boot)
    safety_event_t event_type;           //< Type of safety event
    uint8_t sonicator_id;                //< Associated sonicator (0 = system-wide)
    uint16_t event_data;                 //< Event-specific data
    char description[32];                //< Human-readable description
} safety_event_log_t;

/**
 * @brief Hardware abstraction layer pin configuration
 */
typedef struct {
    uint8_t overload_input_pin;          //< Overload indication input
    uint8_t frequency_output_pin;        //< Frequency output input  
    uint8_t frequency_lock_pin;          //< Frequency lock input
    uint8_t start_output_pin;            //< Start control output
    uint8_t reset_output_pin;            //< Overload reset output
    uint8_t pwm_amplitude_pin;           //< PWM pin for amplitude control
    uint8_t adc_power_pin;               //< Built-in ADC pin for power monitoring
    uint8_t adc_channel;                 //< ADC channel for power monitoring
    uint8_t status_led_pin;              //< Individual status LED pin
} sonicator_hardware_config_t;

/**
 * @brief Diagnostics and performance metrics
 */
typedef struct {
    // Timing measurements
    uint32_t main_loop_min_us;           //< Minimum main loop execution time
    uint32_t main_loop_max_us;           //< Maximum main loop execution time
    uint32_t main_loop_avg_us;           //< Average main loop execution time
    
    // Interrupt statistics  
    uint32_t timer_interrupt_count;      //< Timer interrupt counter
    uint32_t safety_interrupt_count;     //< Safety interrupt counter
    uint32_t modbus_interrupt_count;     //< MODBUS UART interrupt counter
    
    // Communication statistics
    uint32_t i2c_transaction_count;      //< I2C transaction counter
    uint32_t i2c_error_count;            //< I2C communication errors
    uint32_t uart_byte_count;            //< Total UART bytes transferred
    uint32_t uart_error_count;           //< UART communication errors
    
    // Memory and resource usage
    uint16_t stack_usage_bytes;          //< Estimated stack usage
    uint16_t heap_usage_bytes;           //< Dynamic memory usage
    uint8_t free_ram_percent;            //< Percentage of free RAM
} diagnostics_metrics_t;

// ============================================================================
// CALLBACK FUNCTION TYPES
// ============================================================================

/**
 * @brief Callback function type for safety events
 * @param event_type Type of safety event that occurred
 * @param sonicator_id ID of associated sonicator (0 for system events)
 * @param event_data Additional event-specific data
 */
typedef void (*safety_event_callback_t)(safety_event_t event_type, uint8_t sonicator_id, uint16_t event_data);

/**
 * @brief Callback function type for MODBUS register access
 * @param register_address MODBUS register address
 * @param value Register value for write operations
 * @param is_write True for write operations, false for read
 * @return Register value for read operations, 0 for write operations
 */
typedef uint16_t (*modbus_register_callback_t)(uint16_t register_address, uint16_t value, bool is_write);

/**
 * @brief Callback function type for error notifications
 * @param error_code Error code that occurred
 * @param source_module Name of module that generated the error
 * @param additional_info Additional error context information
 */
typedef void (*error_notification_callback_t)(error_code_t error_code, const char* source_module, uint16_t additional_info);

// ============================================================================
// COMPILE-TIME CONSTANTS AND LIMITS
// ============================================================================

// Array sizing constants
#define MAX_SAFETY_EVENT_LOG_ENTRIES     50
#define MAX_MODBUS_FRAME_SIZE             256
#define MAX_MODULE_NAME_LENGTH            16
#define MAX_ERROR_DESCRIPTION_LENGTH      32

// Validation limits
#define MIN_VALID_FREQUENCY_HZ            19900
#define MAX_VALID_FREQUENCY_HZ            20100
#define MIN_VALID_POWER_WATTS             0
#define MAX_VALID_POWER_WATTS             2200
#define MIN_VALID_AMPLITUDE_PERCENT       20
#define MAX_VALID_AMPLITUDE_PERCENT       100

// Timing constraints
#define MAX_ACCEPTABLE_LOOP_TIME_US       5000   // 5ms max loop time
#define MIN_MODBUS_RESPONSE_TIME_US       1000   // 1ms min response time
#define MAX_MODBUS_RESPONSE_TIME_US       50000  // MODBUS bottleneck - system responds faster

#endif // TYPES_H
