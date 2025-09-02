/**
 * @file modbus.cpp
 * @brief MODBUS RTU Communication Framework Implementation
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Arduino Framework implementation of MODBUS RTU slave using native
 * Serial communication. Provides register-based interface for sonicator
 * control and monitoring via PLC/HMI systems.
 */

#include "modbus.h"
#include "modbus_registers.h"
#ifdef UNIT_TEST
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <ctime>
// Mock Arduino functions for unit testing
extern "C" {
    unsigned long millis() { return clock() / (CLOCKS_PER_SEC / 1000); }
    void delay(unsigned long ms) { /* stub */ }
}
#define Serial std::cout
#else
#include <Arduino.h>
#endif

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static modbus_config_t modbus_config;
static modbus_statistics_t modbus_stats;
static modbus_state_t modbus_current_state = MODBUS_STATE_IDLE;
static bool modbus_enabled = false;
static uint32_t last_communication_time = 0;
static uint32_t request_start_time = 0;

// Register map storage
static modbus_register_map_t register_map;

// Communication buffers
static uint8_t rx_buffer[256];
static uint16_t rx_length = 0;
static uint8_t tx_buffer[256];
static uint16_t tx_length = 0;

// ============================================================================
// PRIVATE FUNCTION DECLARATIONS
// ============================================================================

static uint16_t modbus_read_register_internal(uint16_t address);
static bool modbus_write_register_internal(uint16_t address, uint16_t value);
static void modbus_update_statistics(void);
static void modbus_handle_timeout(void);
static void modbus_handle_error(modbus_error_t error);
static uint16_t modbus_process_frame(uint8_t* frame, uint16_t length);
static bool modbus_validate_frame(uint8_t* frame, uint16_t length);

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

modbus_error_t modbus_init(const modbus_config_t* config) {
    if (!config) {
        return MODBUS_ERROR_INVALID_LENGTH;
    }
    
    // Copy configuration
    modbus_config = *config;
    
    // Initialize statistics
    memset(&modbus_stats, 0, sizeof(modbus_stats));
    
    // Initialize register map
    memset(&register_map, 0, sizeof(register_map));
    
    // Set initial system status
    register_map.system_status.system_status = SYSTEM_STATUS_OK;
    register_map.system_status.active_count = 0;
    register_map.system_status.active_mask = 0;
    register_map.system_status.watchdog_status = 1; // Watchdog OK
    register_map.system_status.comm_errors = 0;
    
#ifndef UNIT_TEST
    // Configure Serial for MODBUS communication
    Serial.begin(config->baud_rate);
#endif
    
    modbus_enabled = true;
    modbus_current_state = MODBUS_STATE_IDLE;
    last_communication_time = millis();
    
    return MODBUS_OK;
}

modbus_state_t modbus_process(void) {
    if (!modbus_enabled) {
        return MODBUS_STATE_IDLE;
    }
    
    // Check for communication timeout
    uint32_t current_time = millis();
    if ((current_time - last_communication_time) > modbus_config.timeout_ms) {
        modbus_handle_timeout();
        return MODBUS_STATE_TIMEOUT;
    }
    
#ifndef UNIT_TEST
    // Check for incoming data
    if (Serial.available()) {
        modbus_current_state = MODBUS_STATE_RECEIVING;
        last_communication_time = current_time;
        
        // Simple frame reading (production would need proper RTU timing)
        while (Serial.available() && rx_length < sizeof(rx_buffer)) {
            rx_buffer[rx_length++] = Serial.read();
            delay(1); // Simple inter-character delay
        }
        
        if (rx_length >= 4) { // Minimum MODBUS frame size
            modbus_current_state = MODBUS_STATE_PROCESSING;
            
            if (modbus_validate_frame(rx_buffer, rx_length)) {
                tx_length = modbus_process_frame(rx_buffer, rx_length);
                
                if (tx_length > 0) {
                    modbus_current_state = MODBUS_STATE_RESPONDING;
                    Serial.write(tx_buffer, tx_length);
                    modbus_stats.responses_sent++;
                }
            }
            
            rx_length = 0; // Reset for next frame
        }
    }
#endif
    
    if (modbus_current_state != MODBUS_STATE_TIMEOUT) {
        modbus_current_state = MODBUS_STATE_IDLE;
    }
    
    return modbus_current_state;
}

bool modbus_is_timeout(void) {
    uint32_t current_time = millis();
    return ((current_time - last_communication_time) > modbus_config.timeout_ms);
}

void modbus_reset_timeout(void) {
    last_communication_time = millis();
}

const modbus_statistics_t* modbus_get_statistics(void) {
    return &modbus_stats;
}

void modbus_reset_statistics(void) {
    memset(&modbus_stats, 0, sizeof(modbus_stats));
}

modbus_state_t modbus_get_state(void) {
    return modbus_current_state;
}

void modbus_force_error(modbus_error_t error_code) {
    modbus_handle_error(error_code);
}

bool modbus_validate_address(uint16_t address, uint8_t function_code) {
    // Check if address is within valid ranges
    if (!IS_VALID_REGISTER_ADDR(address)) {
        return false;
    }
    
    // For write operations, check if register is writable
    if ((function_code == MODBUS_FC_WRITE_SINGLE || 
         function_code == MODBUS_FC_WRITE_MULTIPLE) &&
        IS_READONLY_REGISTER(address)) {
        return false;
    }
    
    return true;
}

uint16_t modbus_calculate_crc(const uint8_t* data, uint16_t length) {
    if (!data || length == 0) {
        return 0xFFFF;
    }
    
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

void modbus_set_enabled(bool enabled) {
    modbus_enabled = enabled;
    if (enabled) {
        modbus_current_state = MODBUS_STATE_IDLE;
        last_communication_time = millis();
    } else {
        modbus_current_state = MODBUS_STATE_ERROR;
    }
}

bool modbus_is_enabled(void) {
    return modbus_enabled;
}

// ============================================================================
// PRIVATE FUNCTION IMPLEMENTATIONS
// ============================================================================

static uint16_t modbus_read_register_internal(uint16_t address) {
    // System Status Registers (0x0000-0x000F)
    if (address <= 0x000F) {
        uint16_t* status_regs = (uint16_t*)&register_map.system_status;
        return status_regs[address];
    }
    
    // Global Control Registers (0x0010-0x001F)
    if (address >= 0x0010 && address <= 0x001F) {
        uint16_t* control_regs = (uint16_t*)&register_map.global_control;
        return control_regs[address - 0x0010];
    }
    
    // Per-Sonicator Registers (0x0100-0x041F)
    if (address >= 0x0100 && address <= 0x041F) {
        uint16_t sonicator_id = (address - 0x0100) / MODBUS_REG_SONICATOR_STRIDE;
        uint16_t reg_offset = (address - 0x0100) % MODBUS_REG_SONICATOR_STRIDE;
        
        if (sonicator_id < MODBUS_MAX_SONICATORS) {
            uint16_t* son_regs = (uint16_t*)&register_map.sonicators[sonicator_id];
            return son_regs[reg_offset];
        }
    }
    
    // Use callback if available
    if (modbus_config.read_callback) {
        uint16_t value = 0;
        if (modbus_config.read_callback(address, &value) == MODBUS_OK) {
            return value;
        }
    }
    
    return 0; // Default value for invalid addresses
}

static bool modbus_write_register_internal(uint16_t address, uint16_t value) {
    // Global Control Registers (0x0010-0x001F)
    if (address >= 0x0010 && address <= 0x001F) {
        uint16_t* control_regs = (uint16_t*)&register_map.global_control;
        control_regs[address - 0x0010] = value;
        
        // Handle special control commands
        switch (address) {
            case MODBUS_REG_GLOBAL_ENABLE:
                if (value) {
                    register_map.system_status.system_status |= SYSTEM_STATUS_OK;
                } else {
                    register_map.system_status.system_status &= ~SYSTEM_STATUS_OK;
                }
                break;
                
            case MODBUS_REG_EMERGENCY_STOP:
                if (value) {
                    register_map.system_status.system_status |= SYSTEM_STATUS_EMERGENCY_STOP;
                }
                break;
        }
        
        return true;
    }
    
    // Per-Sonicator Control Registers (0xN00-0xN0F)
    if (address >= 0x0100 && address <= 0x041F) {
        uint16_t sonicator_id = (address - 0x0100) / MODBUS_REG_SONICATOR_STRIDE;
        uint16_t reg_offset = (address - 0x0100) % MODBUS_REG_SONICATOR_STRIDE;
        
        if (sonicator_id < MODBUS_MAX_SONICATORS && reg_offset < 0x10) {
            uint16_t* son_regs = (uint16_t*)&register_map.sonicators[sonicator_id];
            son_regs[reg_offset] = value;
            return true;
        }
    }
    
    // Use callback if available
    if (modbus_config.write_callback) {
        return (modbus_config.write_callback(address, value) == MODBUS_OK);
    }
    
    return false;
}

static void modbus_update_statistics(void) {
    modbus_stats.responses_sent++;
    
    uint32_t response_time = millis() - request_start_time;
    if (response_time > modbus_stats.max_response_time) {
        modbus_stats.max_response_time = response_time;
    }
    
    modbus_stats.last_request_time = request_start_time;
}

static void modbus_handle_timeout(void) {
    modbus_stats.timeout_errors++;
    modbus_current_state = MODBUS_STATE_TIMEOUT;
    
    register_map.system_status.system_status |= SYSTEM_STATUS_COMM_FAULT;
    register_map.system_status.comm_errors++;
    
    if (modbus_config.timeout_callback) {
        modbus_config.timeout_callback();
    }
}

static void modbus_handle_error(modbus_error_t error) {
    modbus_current_state = MODBUS_STATE_ERROR;
    
    switch (error) {
        case MODBUS_ERROR_CRC:
            modbus_stats.crc_errors++;
            break;
        case MODBUS_ERROR_ILLEGAL_FUNCTION:
            modbus_stats.illegal_function_errors++;
            break;
        case MODBUS_ERROR_ILLEGAL_ADDRESS:
            modbus_stats.illegal_address_errors++;
            break;
        case MODBUS_ERROR_SLAVE_FAILURE:
            modbus_stats.slave_failure_errors++;
            break;
        default:
            break;
    }
    
    if (modbus_config.error_callback) {
        modbus_config.error_callback(error);
    }
}

static bool modbus_validate_frame(uint8_t* frame, uint16_t length) {
    if (!frame || length < 4) {
        return false;
    }
    
    // Check slave ID
    if (frame[0] != modbus_config.slave_id) {
        return false;
    }
    
    // Check CRC
    uint16_t received_crc = (frame[length-1] << 8) | frame[length-2];
    uint16_t calculated_crc = modbus_calculate_crc(frame, length-2);
    
    if (received_crc != calculated_crc) {
        modbus_stats.crc_errors++;
        return false;
    }
    
    modbus_stats.requests_received++;
    return true;
}

static uint16_t modbus_process_frame(uint8_t* frame, uint16_t length) {
    request_start_time = millis();
    
    uint8_t function_code = frame[1];
    uint16_t response_length = 0;
    
    // Build response header
    tx_buffer[0] = modbus_config.slave_id;
    tx_buffer[1] = function_code;
    
    switch (function_code) {
        case MODBUS_FC_READ_HOLDING: {
            uint16_t start_addr = (frame[2] << 8) | frame[3];
            uint16_t count = (frame[4] << 8) | frame[5];
            
            if (count > 125) { // Maximum registers per request
                tx_buffer[1] |= 0x80; // Error response
                tx_buffer[2] = MODBUS_ERROR_ILLEGAL_VALUE;
                response_length = 3;
                break;
            }
            
            tx_buffer[2] = count * 2; // Byte count
            response_length = 3;
            
            for (uint16_t i = 0; i < count; i++) {
                uint16_t addr = start_addr + i;
                if (modbus_validate_address(addr, function_code)) {
                    uint16_t value = modbus_read_register_internal(addr);
                    tx_buffer[response_length++] = (value >> 8) & 0xFF;
                    tx_buffer[response_length++] = value & 0xFF;
                } else {
                    tx_buffer[1] |= 0x80; // Error response
                    tx_buffer[2] = MODBUS_ERROR_ILLEGAL_ADDRESS;
                    response_length = 3;
                    break;
                }
            }
            break;
        }
        
        case MODBUS_FC_WRITE_SINGLE: {
            uint16_t addr = (frame[2] << 8) | frame[3];
            uint16_t value = (frame[4] << 8) | frame[5];
            
            if (modbus_validate_address(addr, function_code)) {
                if (modbus_write_register_internal(addr, value)) {
                    // Echo request for successful write
                    tx_buffer[2] = frame[2];
                    tx_buffer[3] = frame[3];
                    tx_buffer[4] = frame[4];
                    tx_buffer[5] = frame[5];
                    response_length = 6;
                } else {
                    tx_buffer[1] |= 0x80;
                    tx_buffer[2] = MODBUS_ERROR_SLAVE_FAILURE;
                    response_length = 3;
                }
            } else {
                tx_buffer[1] |= 0x80;
                tx_buffer[2] = MODBUS_ERROR_ILLEGAL_ADDRESS;
                response_length = 3;
            }
            break;
        }
        
        default:
            tx_buffer[1] |= 0x80; // Error response
            tx_buffer[2] = MODBUS_ERROR_ILLEGAL_FUNCTION;
            response_length = 3;
            break;
    }
    
    // Add CRC
    uint16_t crc = modbus_calculate_crc(tx_buffer, response_length);
    tx_buffer[response_length++] = crc & 0xFF;
    tx_buffer[response_length++] = (crc >> 8) & 0xFF;
    
    modbus_update_statistics();
    
    return response_length;
}
