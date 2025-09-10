/**
 * @file uart.h
 * @brief UART Hardware Abstraction Layer for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Provides hardware-agnostic UART interface for MODBUS RTU communication.
 * Uses USART0 on PD0 (RXD) and PD1 (TXD) for RS-485 communication at 115200 baud.
 * 
 * Supports interrupt-driven transmission/reception with circular buffers
 * for reliable MODBUS frame handling without blocking main application.
 * Pin assignments per docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH).
 */

#ifndef HAL_UART_H
#define HAL_UART_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TYPE DEFINITIONS
// ============================================================================

/**
 * @brief UART parity settings
 */
typedef enum {
    UART_PARITY_NONE = 0,       //< No parity
    UART_PARITY_EVEN = 2,       //< Even parity
    UART_PARITY_ODD = 3         //< Odd parity
} uart_parity_t;

/**
 * @brief UART stop bit settings
 */
typedef enum {
    UART_STOP_1BIT = 0,         //< 1 stop bit
    UART_STOP_2BIT = 1          //< 2 stop bits
} uart_stop_bits_t;

/**
 * @brief UART data bit settings
 */
typedef enum {
    UART_DATA_5BIT = 0,         //< 5 data bits
    UART_DATA_6BIT = 1,         //< 6 data bits
    UART_DATA_7BIT = 2,         //< 7 data bits
    UART_DATA_8BIT = 3,         //< 8 data bits
    UART_DATA_9BIT = 7          //< 9 data bits
} uart_data_bits_t;

/**
 * @brief UART configuration structure
 */
typedef struct {
    uint32_t baud_rate;         //< Baud rate (bps)
    uart_data_bits_t data_bits; //< Number of data bits
    uart_parity_t parity;       //< Parity setting
    uart_stop_bits_t stop_bits; //< Number of stop bits
} uart_config_t;

/**
 * @brief HAL function return codes
 */
typedef enum {
    UART_OK = 0,                //< Operation successful
    UART_ERROR_INVALID_BAUD,    //< Invalid baud rate
    UART_ERROR_INVALID_CONFIG,  //< Invalid configuration
    UART_ERROR_BUFFER_FULL,     //< Transmit buffer full
    UART_ERROR_BUFFER_EMPTY,    //< Receive buffer empty
    UART_ERROR_TIMEOUT,         //< Operation timeout
    UART_ERROR_NOT_INITIALIZED  //< UART not initialized
} uart_result_t;

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

#define UART_DEFAULT_BAUD       115200      //< Default baud rate for MODBUS
#define UART_MIN_BAUD           9600        //< Minimum supported baud rate
#define UART_MAX_BAUD           230400      //< Maximum supported baud rate

#define UART_TX_BUFFER_SIZE     256         //< Transmit buffer size
#define UART_RX_BUFFER_SIZE     256         //< Receive buffer size

#define UART_TIMEOUT_MS         1000        //< Default timeout (1 second)
#define UART_CHAR_TIMEOUT_MS    10          //< Character timeout for MODBUS

// MODBUS RTU timing requirements (at 115200 baud)
#define UART_MODBUS_T1_5_US     130         //< 1.5 character time (μs)
#define UART_MODBUS_T3_5_US     304         //< 3.5 character time (μs)

// ============================================================================
// PUBLIC INTERFACE
// ============================================================================

/**
 * @brief Initialize UART hardware abstraction layer
 * 
 * Configures USART0 with default settings:
 * - 115200 baud, 8N1 (8 data, no parity, 1 stop)
 * - Interrupt-driven RX/TX with circular buffers
 * - MODBUS RTU compatible timing
 * 
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_init(void);

/**
 * @brief Initialize UART with custom configuration
 * 
 * @param config Pointer to UART configuration structure
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_init_config(const uart_config_t* config);

/**
 * @brief Configure UART baud rate
 * 
 * @param baud_rate Desired baud rate (9600-230400)
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_set_baud_rate(uint32_t baud_rate);

/**
 * @brief Send single byte via UART
 * 
 * @param data Byte to transmit
 * @return UART_OK on success, error code on failure
 * 
 * @note Non-blocking if TX buffer has space
 */
uart_result_t uart_send_byte(uint8_t data);

/**
 * @brief Receive single byte from UART
 * 
 * @param data Pointer to store received byte
 * @return UART_OK on success, UART_ERROR_BUFFER_EMPTY if no data
 */
uart_result_t uart_receive_byte(uint8_t* data);

/**
 * @brief Check if UART data is available for reading
 * 
 * @param available Pointer to store availability status
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_data_available(bool* available);

/**
 * @brief Get number of bytes available in receive buffer
 * 
 * @param count Pointer to store byte count
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_get_rx_count(uint16_t* count);

/**
 * @brief Send buffer of bytes via UART
 * 
 * @param buffer Pointer to data buffer
 * @param length Number of bytes to send
 * @return UART_OK on success, error code on failure
 * 
 * @note Blocks if buffer cannot accommodate all data
 */
uart_result_t uart_send_buffer(const uint8_t* buffer, uint16_t length);

/**
 * @brief Receive buffer of bytes from UART
 * 
 * @param buffer Pointer to destination buffer
 * @param max_length Maximum bytes to receive
 * @param received Pointer to store actual bytes received
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_receive_buffer(uint8_t* buffer, uint16_t max_length, uint16_t* received);

/**
 * @brief Flush transmit buffer (wait for all data to be sent)
 * 
 * @return UART_OK on success, UART_ERROR_TIMEOUT on timeout
 */
uart_result_t uart_flush_tx(void);

/**
 * @brief Clear receive buffer
 * 
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_flush_rx(void);

// ============================================================================
// MODBUS RTU SPECIFIC FUNCTIONS
// ============================================================================

/**
 * @brief Send MODBUS RTU frame
 * 
 * Handles proper frame timing with 3.5 character gaps.
 * Automatically calculates and appends CRC16.
 * 
 * @param frame Pointer to MODBUS frame (without CRC)
 * @param length Frame length (excluding CRC)
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_send_modbus_frame(const uint8_t* frame, uint16_t length);

/**
 * @brief Receive MODBUS RTU frame
 * 
 * Waits for frame with proper inter-character timing.
 * Validates CRC16 and removes it from returned frame.
 * 
 * @param frame Pointer to destination buffer
 * @param max_length Maximum frame length
 * @param received Pointer to store actual frame length
 * @param timeout_ms Timeout in milliseconds
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_receive_modbus_frame(uint8_t* frame, uint16_t max_length, 
                                       uint16_t* received, uint32_t timeout_ms);

/**
 * @brief Check for MODBUS frame gap (3.5 character silence)
 * 
 * @param gap_detected Pointer to store gap detection status
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_modbus_gap_detected(bool* gap_detected);

/**
 * @brief Get time since last received character
 * 
 * @param time_us Pointer to store time in microseconds
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_get_idle_time(uint32_t* time_us);

// ============================================================================
// DIAGNOSTIC AND STATUS FUNCTIONS
// ============================================================================

/**
 * @brief Get UART status flags
 * 
 * @param tx_busy Pointer to store TX busy status
 * @param rx_overflow Pointer to store RX overflow status
 * @param frame_error Pointer to store frame error status
 * @param parity_error Pointer to store parity error status
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_get_status(bool* tx_busy, bool* rx_overflow, 
                             bool* frame_error, bool* parity_error);

/**
 * @brief Get TX buffer space available
 * 
 * @param free_bytes Pointer to store free byte count
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_get_tx_free(uint16_t* free_bytes);

/**
 * @brief Clear error flags
 * 
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_clear_errors(void);

/**
 * @brief Calculate character time for current baud rate
 * 
 * @param char_time_us Pointer to store character time in microseconds
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_get_char_time(uint16_t* char_time_us);

/**
 * @brief Test UART loopback (TX connected to RX)
 * 
 * @param success Pointer to store test result
 * @return UART_OK on success, error code on failure
 */
uart_result_t uart_test_loopback(bool* success);

#ifdef __cplusplus
}
#endif

#endif // HAL_UART_H
