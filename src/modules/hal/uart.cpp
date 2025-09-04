/**
 * @file uart.cpp
 * @brief UART Hardware Abstraction Layer Implementation for ATmega32A
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

#include "uart.h"
#include "config.h"
#include <Arduino.h>
#ifndef NATIVE_TEST
#include <avr/interrupt.h>
#endif

// ============================================================================
// PRIVATE VARIABLES
// ============================================================================

static bool uart_initialized = false;
static uint32_t current_baud_rate = UART_DEFAULT_BAUD;

// Circular buffer structures
static volatile uint8_t tx_buffer[UART_TX_BUFFER_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;
static volatile uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;

// Status flags
static volatile bool tx_busy = false;
static volatile bool rx_overflow = false;
static volatile bool frame_error = false;
static volatile bool parity_error = false;

// MODBUS timing
static volatile uint32_t last_rx_time = 0;
static uint16_t char_time_us = 0;

// ============================================================================
// PRIVATE FUNCTION DECLARATIONS
// ============================================================================

static uart_result_t calculate_baud_settings(uint32_t baud_rate, uint16_t* ubrr_value);
static uint16_t next_buffer_index(uint16_t index, uint16_t size);
static bool is_tx_buffer_full(void);
static bool is_rx_buffer_empty(void);
static uint16_t get_tx_buffer_count(void);
static uint16_t get_rx_buffer_count(void);
static void calculate_char_time(void);

// ============================================================================
// INTERRUPT SERVICE ROUTINES
// ============================================================================

// NOTE: When building with Arduino core, we rely on HardwareSerial for UART.
// The HAL functions below can delegate to Serial to avoid ISR conflicts.

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

uart_result_t uart_init(void) {
    uart_config_t default_config = {
        .baud_rate = UART_DEFAULT_BAUD,
        .data_bits = UART_DATA_8BIT,
        .parity = UART_PARITY_NONE,
        .stop_bits = UART_STOP_1BIT
    };

    return uart_init_config(&default_config);
}

uart_result_t uart_init_config(const uart_config_t* config) {
    if (config == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    // Calculate baud rate settings
    uint16_t ubrr_value;
    uart_result_t result = calculate_baud_settings(config->baud_rate, &ubrr_value);
    if (result != UART_OK) {
        return result;
    }

    // Disable UART during configuration
    UCSRB = 0;

    // Set baud rate
    UBRRH = (uint8_t)(ubrr_value >> 8);
    UBRRL = (uint8_t)ubrr_value;

    // Configure frame format
    uint8_t ucsrc_value = (1 << URSEL); // Select UCSRC register

    // Initialize Arduino Serial if not already
#ifdef NATIVE_TEST
    // No real Serial in native tests; mark initialized and return
    current_baud_rate = config->baud_rate;
    uart_initialized = true;
    return UART_OK;
#else
    Serial.begin(config->baud_rate);
    current_baud_rate = config->baud_rate;
    uart_initialized = true;
    return UART_OK;
#endif

    // Data bits
    switch (config->data_bits) {
        case UART_DATA_5BIT:
            // UCSZ2:0 = 000 (default)
            break;
        case UART_DATA_6BIT:
            ucsrc_value |= (1 << UCSZ0);
            break;
        case UART_DATA_7BIT:
            ucsrc_value |= (1 << UCSZ1);
            break;
        case UART_DATA_8BIT:
            ucsrc_value |= (1 << UCSZ1) | (1 << UCSZ0);
            break;
        case UART_DATA_9BIT:
            UCSRB |= (1 << UCSZ2);
            ucsrc_value |= (1 << UCSZ1) | (1 << UCSZ0);
            break;
        default:
            return UART_ERROR_INVALID_CONFIG;
    }

    // Parity
    switch (config->parity) {
        case UART_PARITY_NONE:
            // UPM1:0 = 00 (default)
            break;
        case UART_PARITY_EVEN:
            ucsrc_value |= (1 << UPM1);
            break;
        case UART_PARITY_ODD:
            ucsrc_value |= (1 << UPM1) | (1 << UPM0);
            break;
        default:
            return UART_ERROR_INVALID_CONFIG;
    }

    // Stop bits
    if (config->stop_bits == UART_STOP_2BIT) {
        ucsrc_value |= (1 << USBS);
    }

    // Write configuration to UCSRC
    UCSRC = ucsrc_value;

    // Enable RX, TX, and RX interrupt
    UCSRB |= (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);

    // Initialize buffers
    tx_head = tx_tail = 0;
    rx_head = rx_tail = 0;

    // Clear status flags
    tx_busy = false;
    rx_overflow = false;
    frame_error = false;
    parity_error = false;

    // Store current baud rate and calculate timing
    current_baud_rate = config->baud_rate;
    calculate_char_time();

    // Enable global interrupts if not already enabled
#ifndef NATIVE_TEST
    sei();
#endif

    uart_initialized = true;
    return UART_OK;
}

uart_result_t uart_set_baud_rate(uint32_t baud_rate) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    uint16_t ubrr_value;
    uart_result_t result = calculate_baud_settings(baud_rate, &ubrr_value);
    if (result != UART_OK) {
        return result;
    }

    // Wait for transmission to complete
    uart_flush_tx();

    // Update baud rate registers
    UBRRH = (uint8_t)(ubrr_value >> 8);
    UBRRL = (uint8_t)ubrr_value;

    current_baud_rate = baud_rate;
    calculate_char_time();

    return UART_OK;
}

uart_result_t uart_send_byte(uint8_t data) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

#ifdef NATIVE_TEST
    // In native tests, simulate success without real UART
    (void)data;
    return UART_OK;
#else
    // Directly use Arduino Serial in Arduino build to save RAM
    (void)Serial.write(&data, 1);
    return UART_OK;
#endif
}

uart_result_t uart_receive_byte(uint8_t* data) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (data == nullptr || is_rx_buffer_empty()) {
        return UART_ERROR_BUFFER_EMPTY;
    }

#ifdef NATIVE_TEST
    // No real RX data in native tests
    (void)data;
    return UART_ERROR_BUFFER_EMPTY;
#else
    if (Serial.available() <= 0) {
        return UART_ERROR_BUFFER_EMPTY;
    }

    *data = (uint8_t)Serial.read();
    return UART_OK;
#endif
}

uart_result_t uart_data_available(bool* available) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (available == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

#ifdef NATIVE_TEST
    *available = false;
    return UART_OK;
#else
    *available = (Serial.available() > 0);
    return UART_OK;
#endif
}

uart_result_t uart_get_rx_count(uint16_t* count) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (count == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    *count = get_rx_buffer_count();
    return UART_OK;
}

uart_result_t uart_send_buffer(const uint8_t* buffer, uint16_t length) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (buffer == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    for (uint16_t i = 0; i < length; i++) {
        // Direct write in Arduino build
        uart_result_t result = uart_send_byte(buffer[i]);
        if (result != UART_OK) {
            return result;
        }
    }

    return UART_OK;
}

uart_result_t uart_receive_buffer(uint8_t* buffer, uint16_t max_length, uint16_t* received) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (buffer == nullptr || received == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    *received = 0;

#ifdef NATIVE_TEST
    (void)buffer; (void)max_length; (void)received;
    return UART_OK;
#else
    while (*received < max_length && Serial.available() > 0) {
        uart_result_t result = uart_receive_byte(&buffer[*received]);
        if (result == UART_OK) {
            (*received)++;
        } else {
            break;
        }
    }

    return UART_OK;
#endif
}

uart_result_t uart_flush_tx(void) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    uint32_t timeout = millis() + UART_TIMEOUT_MS;

#ifdef NATIVE_TEST
    return UART_OK;
#else
    // In Arduino build, flush the Serial TX buffer
    Serial.flush();
    return UART_OK;
#endif
}

uart_result_t uart_flush_rx(void) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

#ifndef NATIVE_TEST
    cli();
#endif
    rx_head = rx_tail = 0;
#ifndef NATIVE_TEST
    sei();
#endif

    return UART_OK;
}

// ============================================================================
// MODBUS RTU FUNCTIONS
// ============================================================================

uart_result_t uart_send_modbus_frame(const uint8_t* frame, uint16_t length) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (frame == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    // Ensure 3.5 character gap before transmission
    delayMicroseconds(UART_MODBUS_T3_5_US);

    // Send frame data
    uart_result_t result = uart_send_buffer(frame, length);
    if (result != UART_OK) {
        return result;
    }

    // Wait for transmission to complete
    return uart_flush_tx();
}

uart_result_t uart_receive_modbus_frame(uint8_t* frame, uint16_t max_length,
                                       uint16_t* received, uint32_t timeout_ms) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (frame == nullptr || received == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    *received = 0;
    uint32_t start_time = millis();
    uint32_t last_char_time = micros();

    while (millis() - start_time < timeout_ms) {
        uint8_t data;
        if (uart_receive_byte(&data) == UART_OK) {
            if (*received < max_length) {
                frame[*received] = data;
                (*received)++;
                last_char_time = micros();
            }
        } else {
            // Check for frame gap (3.5 character times)
            if (*received > 0 && (micros() - last_char_time) > UART_MODBUS_T3_5_US) {
                break; // End of frame detected
            }
        }
    }

    return (*received > 0) ? UART_OK : UART_ERROR_TIMEOUT;
}

uart_result_t uart_modbus_gap_detected(bool* gap_detected) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (gap_detected == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    uint32_t idle_time = micros() - last_rx_time;
    *gap_detected = (idle_time > UART_MODBUS_T3_5_US);

    return UART_OK;
}

uart_result_t uart_get_idle_time(uint32_t* time_us) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (time_us == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    *time_us = micros() - last_rx_time;
    return UART_OK;
}

// ============================================================================
// DIAGNOSTIC FUNCTIONS
// ============================================================================

uart_result_t uart_get_status(bool* tx_busy_status, bool* rx_overflow_status,
                             bool* frame_error_status, bool* parity_error_status) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (tx_busy_status) *tx_busy_status = tx_busy;
    if (rx_overflow_status) *rx_overflow_status = rx_overflow;
    if (frame_error_status) *frame_error_status = frame_error;
    if (parity_error_status) *parity_error_status = parity_error;

    return UART_OK;
}

uart_result_t uart_get_tx_free(uint16_t* free_bytes) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (free_bytes == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    *free_bytes = UART_TX_BUFFER_SIZE - get_tx_buffer_count() - 1;
    return UART_OK;
}

uart_result_t uart_clear_errors(void) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

#ifndef NATIVE_TEST
    cli();
#endif
    rx_overflow = false;
    frame_error = false;
    parity_error = false;
#ifndef NATIVE_TEST
    sei();
#endif

    return UART_OK;
}

uart_result_t uart_get_char_time(uint16_t* char_time_us_ptr) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (char_time_us_ptr == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    *char_time_us_ptr = char_time_us;
    return UART_OK;
}

uart_result_t uart_test_loopback(bool* success) {
    if (!uart_initialized) {
        return UART_ERROR_NOT_INITIALIZED;
    }

    if (success == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    // Clear buffers
    uart_flush_rx();

    // Send test pattern
    uint8_t test_data = 0x55; // Alternating bit pattern
    uart_result_t result = uart_send_byte(test_data);
    if (result != UART_OK) {
        *success = false;
        return result;
    }

    // Wait for transmission and potential reception
    delay(10);

    // Check if data was received
    uint8_t received_data;
    result = uart_receive_byte(&received_data);
    *success = (result == UART_OK) && (received_data == test_data);

    return UART_OK;
}

// ============================================================================
// PRIVATE FUNCTION IMPLEMENTATIONS
// ============================================================================

static uart_result_t calculate_baud_settings(uint32_t baud_rate, uint16_t* ubrr_value) {
    if (ubrr_value == nullptr) {
        return UART_ERROR_INVALID_CONFIG;
    }

    if (baud_rate < UART_MIN_BAUD || baud_rate > UART_MAX_BAUD) {
        return UART_ERROR_INVALID_BAUD;
    }

    // Calculate UBRR value for normal mode (U2X = 0)
    uint32_t ubrr = (F_CPU / (16UL * baud_rate)) - 1;

    if (ubrr > 4095) { // 12-bit register
        return UART_ERROR_INVALID_BAUD;
    }

    *ubrr_value = (uint16_t)ubrr;
    return UART_OK;
}

static uint16_t next_buffer_index(uint16_t index, uint16_t size) {
    return (index + 1) % size;
}

static bool is_tx_buffer_full(void) {
    return next_buffer_index(tx_head, UART_TX_BUFFER_SIZE) == tx_tail;
}

static bool is_rx_buffer_empty(void) {
    return rx_head == rx_tail;
}

static uint16_t get_tx_buffer_count(void) {
    if (tx_head >= tx_tail) {
        return tx_head - tx_tail;
    } else {
        return UART_TX_BUFFER_SIZE - tx_tail + tx_head;
    }
}

static uint16_t get_rx_buffer_count(void) {
    if (rx_head >= rx_tail) {
        return rx_head - rx_tail;
    } else {
        return UART_RX_BUFFER_SIZE - rx_tail + rx_head;
    }
}

static void calculate_char_time(void) {
    // Calculate time for one character (11 bits: 1 start + 8 data + 1 parity + 1 stop)
    char_time_us = (11 * 1000000UL) / current_baud_rate;
}
