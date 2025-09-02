/**
 * @file modbus.h
 * @brief MODBUS RTU Communication Framework for Multi-Sonicator System
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Implements MODBUS RTU slave interface for PLC/HMI integration.
 * Provides register access for sonicator control and monitoring.
 * 
 * Features:
 * - MODBUS RTU slave with configurable slave ID (default: 2)
 * - 115200 baud, 8N1 frame format
 * - Function codes: 03, 06, 16 (Read/Write Holding Registers)
 * - CRC-16 error detection and validation
 * - Communication timeout detection with failsafe activation
 * - Response time <100ms per NFR2 requirements
 * 
 * Register Map:
 * - 0x0000-0x000F: System Status (Read Only)
 * - 0x0010-0x001F: Global Control (Read/Write)
 * - 0x0100-0x041F: Per-Sonicator Registers (Read/Write + Read Only)
 */

#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>
#include <stdbool.h>
#include "modbus_registers.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// MODBUS CONFIGURATION
// ============================================================================

#define MODBUS_SLAVE_ID             2           ///< MODBUS slave ID per system requirements
#define MODBUS_BAUD_RATE            115200      ///< Communication baud rate
#define MODBUS_TIMEOUT_MS           1000        ///< Communication timeout (1 second)
#define MODBUS_RESPONSE_TIMEOUT_MS  100         ///< Maximum response time per NFR2

// Supported MODBUS function codes
#define MODBUS_FC_READ_HOLDING      0x03        ///< Read Holding Registers
#define MODBUS_FC_WRITE_SINGLE      0x06        ///< Write Single Register
#define MODBUS_FC_WRITE_MULTIPLE    0x10        ///< Write Multiple Registers

// ============================================================================
// ERROR CODES
// ============================================================================

typedef enum {
    MODBUS_OK = 0,                  ///< Operation successful
    MODBUS_ERROR_ILLEGAL_FUNCTION,  ///< Unsupported function code
    MODBUS_ERROR_ILLEGAL_ADDRESS,   ///< Invalid register address
    MODBUS_ERROR_ILLEGAL_VALUE,     ///< Invalid register value
    MODBUS_ERROR_SLAVE_FAILURE,     ///< Slave device failure
    MODBUS_ERROR_ACKNOWLEDGE,       ///< Acknowledge error
    MODBUS_ERROR_SLAVE_BUSY,        ///< Slave device busy
    MODBUS_ERROR_MEMORY_PARITY,     ///< Memory parity error
    MODBUS_ERROR_GATEWAY_PATH,      ///< Gateway path unavailable
    MODBUS_ERROR_GATEWAY_TARGET,    ///< Gateway target failed to respond
    MODBUS_ERROR_TIMEOUT,           ///< Communication timeout
    MODBUS_ERROR_CRC,               ///< CRC validation failed
    MODBUS_ERROR_INVALID_LENGTH     ///< Invalid frame length
} modbus_error_t;

// ============================================================================
// COMMUNICATION STATE
// ============================================================================

typedef enum {
    MODBUS_STATE_IDLE,              ///< Waiting for request
    MODBUS_STATE_RECEIVING,         ///< Processing incoming frame
    MODBUS_STATE_PROCESSING,        ///< Executing MODBUS function
    MODBUS_STATE_RESPONDING,        ///< Sending response frame
    MODBUS_STATE_ERROR,             ///< Handling communication errors
    MODBUS_STATE_TIMEOUT            ///< Communication timeout detected
} modbus_state_t;

// ============================================================================
// CALLBACK FUNCTION TYPES
// ============================================================================

/**
 * @brief Callback for register read operations
 * @param address Register address being read
 * @param value Pointer to store the read value
 * @return MODBUS_OK on success, error code on failure
 */
typedef modbus_error_t (*modbus_read_callback_t)(uint16_t address, uint16_t* value);

/**
 * @brief Callback for register write operations
 * @param address Register address being written
 * @param value Value to write to the register
 * @return MODBUS_OK on success, error code on failure
 */
typedef modbus_error_t (*modbus_write_callback_t)(uint16_t address, uint16_t value);

/**
 * @brief Callback for communication timeout events
 */
typedef void (*modbus_timeout_callback_t)(void);

/**
 * @brief Callback for communication error events
 * @param error_code Error code that occurred
 */
typedef void (*modbus_error_callback_t)(modbus_error_t error_code);

// ============================================================================
// MODBUS CONFIGURATION STRUCTURE
// ============================================================================

typedef struct {
    uint8_t slave_id;                           ///< MODBUS slave ID
    uint32_t baud_rate;                         ///< Communication baud rate
    uint16_t timeout_ms;                        ///< Communication timeout
    modbus_read_callback_t read_callback;      ///< Register read callback
    modbus_write_callback_t write_callback;    ///< Register write callback
    modbus_timeout_callback_t timeout_callback; ///< Timeout event callback
    modbus_error_callback_t error_callback;    ///< Error event callback
} modbus_config_t;

// ============================================================================
// MODBUS STATISTICS
// ============================================================================

typedef struct {
    uint32_t requests_received;         ///< Total requests received
    uint32_t responses_sent;            ///< Total responses sent
    uint32_t crc_errors;                ///< CRC validation errors
    uint32_t timeout_errors;            ///< Communication timeouts
    uint32_t illegal_function_errors;   ///< Illegal function code errors
    uint32_t illegal_address_errors;    ///< Illegal address errors
    uint32_t slave_failure_errors;      ///< Slave failure errors
    uint32_t last_request_time;         ///< Timestamp of last request
    uint32_t max_response_time;         ///< Maximum response time recorded
} modbus_statistics_t;

// ============================================================================
// PUBLIC FUNCTION DECLARATIONS
// ============================================================================

/**
 * @brief Initialize MODBUS RTU communication
 * @param config MODBUS configuration structure
 * @return MODBUS_OK on success, error code on failure
 */
modbus_error_t modbus_init(const modbus_config_t* config);

/**
 * @brief Process MODBUS communication (call from main loop)
 * @return Current MODBUS state
 */
modbus_state_t modbus_process(void);

/**
 * @brief Check if communication timeout occurred
 * @return true if timeout detected, false otherwise
 */
bool modbus_is_timeout(void);

/**
 * @brief Reset communication timeout timer
 */
void modbus_reset_timeout(void);

/**
 * @brief Get MODBUS communication statistics
 * @return Pointer to statistics structure
 */
const modbus_statistics_t* modbus_get_statistics(void);

/**
 * @brief Reset MODBUS communication statistics
 */
void modbus_reset_statistics(void);

/**
 * @brief Get current MODBUS state
 * @return Current communication state
 */
modbus_state_t modbus_get_state(void);

/**
 * @brief Force MODBUS into error state for testing
 * @param error_code Error code to simulate
 */
void modbus_force_error(modbus_error_t error_code);

/**
 * @brief Validate MODBUS register address
 * @param address Register address to validate
 * @param function_code MODBUS function code
 * @return true if address is valid for the function, false otherwise
 */
bool modbus_validate_address(uint16_t address, uint8_t function_code);

/**
 * @brief Calculate CRC-16 for MODBUS frame
 * @param data Pointer to data buffer
 * @param length Length of data in bytes
 * @return Calculated CRC-16 value
 */
uint16_t modbus_calculate_crc(const uint8_t* data, uint16_t length);

/**
 * @brief Enable/disable MODBUS communication
 * @param enabled true to enable, false to disable
 */
void modbus_set_enabled(bool enabled);

/**
 * @brief Check if MODBUS communication is enabled
 * @return true if enabled, false if disabled
 */
bool modbus_is_enabled(void);

#ifdef __cplusplus
}
#endif

#endif // MODBUS_H
