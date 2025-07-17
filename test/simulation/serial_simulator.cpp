/**
 * @file serial_simulator.cpp
 * @brief Serial Port Simulator for MODBUS RTU Testing
 * @author Cannasol Technologies
 * @date 2025-07-17
 * 
 * Provides virtual serial port simulation for MODBUS RTU communication
 * testing when physical hardware is not available.
 */

#include "serial_simulator.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

SerialPortSimulator::SerialPortSimulator(uint8_t slave_address, uint32_t baud_rate)
    : slave_address(slave_address)
    , baud_rate(baud_rate)
    , is_open(false)
    , rx_buffer_head(0)
    , rx_buffer_tail(0)
    , tx_buffer_head(0)
    , tx_buffer_tail(0)
    , bytes_received(0)
    , bytes_transmitted(0)
    , error_count(0)
    , response_delay_ms(10)
    , simulate_errors(false)
    , error_rate(0.0f)
{
    // Initialize register map with default values
    initializeRegisterMap();
    
    // Initialize CRC table
    initializeCrcTable();
}

SerialPortSimulator::~SerialPortSimulator() {
    close();
}

bool SerialPortSimulator::open() {
    if (is_open) {
        return true;
    }
    
    // Clear buffers
    rx_buffer_head = 0;
    rx_buffer_tail = 0;
    tx_buffer_head = 0;
    tx_buffer_tail = 0;
    
    // Reset statistics
    bytes_received = 0;
    bytes_transmitted = 0;
    error_count = 0;
    
    is_open = true;
    return true;
}

void SerialPortSimulator::close() {
    is_open = false;
}

bool SerialPortSimulator::isOpen() const {
    return is_open;
}

void SerialPortSimulator::initializeRegisterMap() {
    // Initialize all registers to 0
    memset(registers, 0, sizeof(registers));
    
    // Set default values for specific registers
    registers[REG_SONICATOR_1_AMPLITUDE - 40001] = 50; // Default 50% amplitude
    registers[REG_SONICATOR_2_AMPLITUDE - 40001] = 50;
    registers[REG_SONICATOR_3_AMPLITUDE - 40001] = 50;
    registers[REG_SONICATOR_4_AMPLITUDE - 40001] = 50;
    
    // Set default frequencies
    registers[REG_SONICATOR_1_FREQUENCY - 40001] = SONICATOR_FREQ_NOMINAL;
    registers[REG_SONICATOR_2_FREQUENCY - 40001] = SONICATOR_FREQ_NOMINAL;
    registers[REG_SONICATOR_3_FREQUENCY - 40001] = SONICATOR_FREQ_NOMINAL;
    registers[REG_SONICATOR_4_FREQUENCY - 40001] = SONICATOR_FREQ_NOMINAL;
    
    // Set system status
    registers[REG_SYSTEM_STATUS - 40001] = STATUS_SYSTEM_OK;
}

void SerialPortSimulator::initializeCrcTable() {
    // Initialize CRC-16 lookup table for MODBUS
    uint16_t crc;
    for (int i = 0; i < 256; i++) {
        crc = i;
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
        crc_table[i] = crc;
    }
}

uint16_t SerialPortSimulator::calculateCrc(const uint8_t* data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc = (crc >> 8) ^ crc_table[(crc ^ data[i]) & 0xFF];
    }
    return crc;
}

size_t SerialPortSimulator::write(const uint8_t* data, size_t length) {
    if (!is_open || length == 0) {
        return 0;
    }
    
    // Add data to receive buffer (from simulator perspective)
    size_t bytes_written = 0;
    for (size_t i = 0; i < length; i++) {
        size_t next_head = (rx_buffer_head + 1) % BUFFER_SIZE;
        if (next_head != rx_buffer_tail) {
            rx_buffer[rx_buffer_head] = data[i];
            rx_buffer_head = next_head;
            bytes_written++;
        } else {
            // Buffer full
            break;
        }
    }
    
    bytes_received += bytes_written;
    
    // Process received data for MODBUS frames
    processReceivedData();
    
    return bytes_written;
}

size_t SerialPortSimulator::read(uint8_t* buffer, size_t length) {
    if (!is_open || length == 0) {
        return 0;
    }
    
    // Read data from transmit buffer (from simulator perspective)
    size_t bytes_read = 0;
    while (bytes_read < length && tx_buffer_tail != tx_buffer_head) {
        buffer[bytes_read] = tx_buffer[tx_buffer_tail];
        tx_buffer_tail = (tx_buffer_tail + 1) % BUFFER_SIZE;
        bytes_read++;
    }
    
    return bytes_read;
}

size_t SerialPortSimulator::available() {
    if (!is_open) {
        return 0;
    }
    
    // Return number of bytes available in transmit buffer
    if (tx_buffer_head >= tx_buffer_tail) {
        return tx_buffer_head - tx_buffer_tail;
    } else {
        return BUFFER_SIZE - tx_buffer_tail + tx_buffer_head;
    }
}

void SerialPortSimulator::processReceivedData() {
    // Look for complete MODBUS frames in receive buffer
    while (rx_buffer_tail != rx_buffer_head) {
        // Try to parse a frame starting at current position
        if (parseModbusFrame()) {
            // Frame parsed successfully, process it
            processModbusFrame();
        } else {
            // No complete frame found, advance by one byte
            rx_buffer_tail = (rx_buffer_tail + 1) % BUFFER_SIZE;
        }
    }
}

bool SerialPortSimulator::parseModbusFrame() {
    // Calculate available bytes
    size_t available_bytes;
    if (rx_buffer_head >= rx_buffer_tail) {
        available_bytes = rx_buffer_head - rx_buffer_tail;
    } else {
        available_bytes = BUFFER_SIZE - rx_buffer_tail + rx_buffer_head;
    }
    
    // Need at least 8 bytes for minimum MODBUS frame
    if (available_bytes < 8) {
        return false;
    }
    
    // Extract frame data
    uint8_t frame_data[256];
    size_t frame_pos = 0;
    size_t buffer_pos = rx_buffer_tail;
    
    // Copy available bytes to frame buffer
    while (frame_pos < available_bytes && frame_pos < 256) {
        frame_data[frame_pos] = rx_buffer[buffer_pos];
        buffer_pos = (buffer_pos + 1) % BUFFER_SIZE;
        frame_pos++;
    }
    
    // Check if this is for our slave address
    if (frame_data[0] != slave_address) {
        return false;
    }
    
    // Parse function code and determine frame length
    uint8_t function_code = frame_data[1];
    size_t expected_length = 0;
    
    switch (function_code) {
        case 0x03: // Read Holding Registers
            expected_length = 8;
            break;
        case 0x06: // Write Single Register
            expected_length = 8;
            break;
        case 0x10: // Write Multiple Registers
            if (frame_pos >= 7) {
                expected_length = 9 + frame_data[6]; // 9 bytes header + data bytes
            }
            break;
        default:
            return false;
    }
    
    // Check if we have enough bytes for complete frame
    if (available_bytes < expected_length) {
        return false;
    }
    
    // Verify CRC
    uint16_t received_crc = (frame_data[expected_length - 1] << 8) | frame_data[expected_length - 2];
    uint16_t calculated_crc = calculateCrc(frame_data, expected_length - 2);
    
    if (received_crc != calculated_crc) {
        error_count++;
        return false;
    }
    
    // Parse frame details
    current_frame.slave_address = frame_data[0];
    current_frame.function_code = frame_data[1];
    current_frame.starting_address = (frame_data[2] << 8) | frame_data[3];
    current_frame.quantity = (frame_data[4] << 8) | frame_data[5];
    current_frame.crc = received_crc;
    current_frame.frame_length = expected_length;
    
    if (function_code == 0x10) {
        current_frame.byte_count = frame_data[6];
        current_frame.data = &frame_data[7];
    }
    
    // Advance buffer tail past processed frame
    rx_buffer_tail = (rx_buffer_tail + expected_length) % BUFFER_SIZE;
    
    return true;
}

void SerialPortSimulator::processModbusFrame() {
    // Simulate processing delay
    usleep(response_delay_ms * 1000);
    
    // Check for simulated errors
    if (simulate_errors && ((rand() % 100) < (error_rate * 100))) {
        error_count++;
        return; // Don't send response
    }
    
    // Generate response based on function code
    uint8_t response[256];
    size_t response_length = 0;
    
    switch (current_frame.function_code) {
        case 0x03: // Read Holding Registers
            response_length = generateReadResponse(response);
            break;
        case 0x06: // Write Single Register
            response_length = generateWriteSingleResponse(response);
            break;
        case 0x10: // Write Multiple Registers
            response_length = generateWriteMultipleResponse(response);
            break;
        default:
            response_length = generateErrorResponse(response, 0x01); // Illegal Function
            break;
    }
    
    // Send response
    sendResponse(response, response_length);
}

size_t SerialPortSimulator::generateReadResponse(uint8_t* response) {
    // Validate register range
    if (current_frame.starting_address < 40001 || 
        current_frame.starting_address > 40034 ||
        current_frame.quantity == 0 ||
        current_frame.quantity > 125) {
        return generateErrorResponse(response, 0x02); // Illegal Data Address
    }
    
    // Build response
    response[0] = slave_address;
    response[1] = 0x03;
    response[2] = current_frame.quantity * 2; // Byte count
    
    // Add register values
    size_t response_pos = 3;
    for (uint16_t i = 0; i < current_frame.quantity; i++) {
        uint16_t reg_addr = current_frame.starting_address + i;
        uint16_t reg_value = getRegisterValue(reg_addr);
        response[response_pos++] = (reg_value >> 8) & 0xFF;
        response[response_pos++] = reg_value & 0xFF;
    }
    
    // Add CRC
    uint16_t crc = calculateCrc(response, response_pos);
    response[response_pos++] = crc & 0xFF;
    response[response_pos++] = (crc >> 8) & 0xFF;
    
    return response_pos;
}

size_t SerialPortSimulator::generateWriteSingleResponse(uint8_t* response) {
    // Validate register address
    if (current_frame.starting_address < 40001 || 
        current_frame.starting_address > 40034) {
        return generateErrorResponse(response, 0x02); // Illegal Data Address
    }
    
    // Update register value
    setRegisterValue(current_frame.starting_address, current_frame.quantity);
    
    // Echo back the request (standard MODBUS response)
    response[0] = slave_address;
    response[1] = 0x06;
    response[2] = (current_frame.starting_address >> 8) & 0xFF;
    response[3] = current_frame.starting_address & 0xFF;
    response[4] = (current_frame.quantity >> 8) & 0xFF;
    response[5] = current_frame.quantity & 0xFF;
    
    // Add CRC
    uint16_t crc = calculateCrc(response, 6);
    response[6] = crc & 0xFF;
    response[7] = (crc >> 8) & 0xFF;
    
    return 8;
}

size_t SerialPortSimulator::generateWriteMultipleResponse(uint8_t* response) {
    // Validate register range
    if (current_frame.starting_address < 40001 || 
        current_frame.starting_address > 40034 ||
        current_frame.quantity == 0 ||
        current_frame.quantity > 123) {
        return generateErrorResponse(response, 0x02); // Illegal Data Address
    }
    
    // Update register values
    for (uint16_t i = 0; i < current_frame.quantity; i++) {
        uint16_t reg_addr = current_frame.starting_address + i;
        uint16_t reg_value = (current_frame.data[i * 2] << 8) | current_frame.data[i * 2 + 1];
        setRegisterValue(reg_addr, reg_value);
    }
    
    // Build response
    response[0] = slave_address;
    response[1] = 0x10;
    response[2] = (current_frame.starting_address >> 8) & 0xFF;
    response[3] = current_frame.starting_address & 0xFF;
    response[4] = (current_frame.quantity >> 8) & 0xFF;
    response[5] = current_frame.quantity & 0xFF;
    
    // Add CRC
    uint16_t crc = calculateCrc(response, 6);
    response[6] = crc & 0xFF;
    response[7] = (crc >> 8) & 0xFF;
    
    return 8;
}

size_t SerialPortSimulator::generateErrorResponse(uint8_t* response, uint8_t error_code) {
    response[0] = slave_address;
    response[1] = current_frame.function_code | 0x80; // Set error bit
    response[2] = error_code;
    
    // Add CRC
    uint16_t crc = calculateCrc(response, 3);
    response[3] = crc & 0xFF;
    response[4] = (crc >> 8) & 0xFF;
    
    return 5;
}

void SerialPortSimulator::sendResponse(const uint8_t* response, size_t length) {
    // Add response to transmit buffer
    for (size_t i = 0; i < length; i++) {
        size_t next_head = (tx_buffer_head + 1) % BUFFER_SIZE;
        if (next_head != tx_buffer_tail) {
            tx_buffer[tx_buffer_head] = response[i];
            tx_buffer_head = next_head;
            bytes_transmitted++;
        } else {
            // Buffer full, drop data
            error_count++;
            break;
        }
    }
}

uint16_t SerialPortSimulator::getRegisterValue(uint16_t address) {
    if (address >= 40001 && address <= 40034) {
        return registers[address - 40001];
    }
    return 0;
}

void SerialPortSimulator::setRegisterValue(uint16_t address, uint16_t value) {
    if (address >= 40001 && address <= 40034) {
        registers[address - 40001] = value;
        
        // Handle special register updates
        handleRegisterUpdate(address, value);
    }
}

void SerialPortSimulator::handleRegisterUpdate(uint16_t address, uint16_t value) {
    // Handle special register updates that trigger actions
    switch (address) {
        case REG_SONICATOR_1_START_STOP:
        case REG_SONICATOR_2_START_STOP:
        case REG_SONICATOR_3_START_STOP:
        case REG_SONICATOR_4_START_STOP:
            // Update status register to reflect running state
            updateSonicatorStatus(address - REG_SONICATOR_1_START_STOP + 1, value != 0);
            break;
            
        case REG_EMERGENCY_STOP:
            if (value != 0) {
                // Emergency stop all sonicators
                for (int i = 1; i <= 4; i++) {
                    updateSonicatorStatus(i, false);
                }
                registers[REG_SYSTEM_STATUS - 40001] |= STATUS_EMERGENCY_STOP;
            } else {
                registers[REG_SYSTEM_STATUS - 40001] &= ~STATUS_EMERGENCY_STOP;
            }
            break;
    }
}

void SerialPortSimulator::updateSonicatorStatus(uint8_t sonicator_id, bool running) {
    if (sonicator_id >= 1 && sonicator_id <= 4) {
        uint16_t status_reg = REG_SONICATOR_1_STATUS + (sonicator_id - 1) - 40001;
        
        if (running) {
            registers[status_reg] |= STATUS_SONICATOR_RUNNING;
            // Simulate some power output
            uint16_t power_reg = REG_SONICATOR_1_POWER + (sonicator_id - 1) - 40001;
            uint16_t amplitude_reg = REG_SONICATOR_1_AMPLITUDE + (sonicator_id - 1) - 40001;
            uint16_t amplitude = registers[amplitude_reg];
            registers[power_reg] = (amplitude * SONICATOR_MAX_POWER) / 100;
        } else {
            registers[status_reg] &= ~STATUS_SONICATOR_RUNNING;
            // Clear power output
            uint16_t power_reg = REG_SONICATOR_1_POWER + (sonicator_id - 1) - 40001;
            registers[power_reg] = 0;
        }
    }
}

// Configuration and testing methods
void SerialPortSimulator::setErrorSimulation(bool enabled, float error_rate) {
    simulate_errors = enabled;
    this->error_rate = error_rate;
}

void SerialPortSimulator::setResponseDelay(uint32_t delay_ms) {
    response_delay_ms = delay_ms;
}

uint32_t SerialPortSimulator::getBytesReceived() const {
    return bytes_received;
}

uint32_t SerialPortSimulator::getBytesTransmitted() const {
    return bytes_transmitted;
}

uint32_t SerialPortSimulator::getErrorCount() const {
    return error_count;
}

void SerialPortSimulator::resetStatistics() {
    bytes_received = 0;
    bytes_transmitted = 0;
    error_count = 0;
}