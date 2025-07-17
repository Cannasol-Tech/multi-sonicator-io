/**
 * @file serial_simulator.h
 * @brief Serial Port Simulator Header for MODBUS RTU Testing
 * @author Cannasol Technologies
 * @date 2025-07-17
 * 
 * Provides virtual serial port simulation for MODBUS RTU communication
 * testing when physical hardware is not available.
 */

#ifndef SERIAL_SIMULATOR_H
#define SERIAL_SIMULATOR_H

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include "../../include/config.h"
#include "../../include/types.h"

/**
 * @brief Serial Port Simulator Class
 * 
 * Simulates a serial port for MODBUS RTU communication testing.
 * Provides realistic timing, error simulation, and full MODBUS
 * protocol support for the Multi-Sonicator register map.
 */
class SerialPortSimulator {
private:
    static const size_t BUFFER_SIZE = 512;
    static const size_t MAX_REGISTERS = 34;
    
    // Port configuration
    uint8_t slave_address;
    uint32_t baud_rate;
    bool is_open;
    
    // Buffers for data transfer
    uint8_t rx_buffer[BUFFER_SIZE];
    uint8_t tx_buffer[BUFFER_SIZE];
    size_t rx_buffer_head;
    size_t rx_buffer_tail;
    size_t tx_buffer_head;
    size_t tx_buffer_tail;
    
    // Statistics
    uint32_t bytes_received;
    uint32_t bytes_transmitted;
    uint32_t error_count;
    
    // Simulation parameters
    uint32_t response_delay_ms;
    bool simulate_errors;
    float error_rate;
    
    // MODBUS register storage
    uint16_t registers[MAX_REGISTERS];
    
    // Current frame being processed
    modbus_frame_t current_frame;
    
    // CRC calculation
    uint16_t crc_table[256];
    
    // Private methods
    void initializeRegisterMap();
    void initializeCrcTable();
    uint16_t calculateCrc(const uint8_t* data, size_t length);
    
    void processReceivedData();
    bool parseModbusFrame();
    void processModbusFrame();
    
    size_t generateReadResponse(uint8_t* response);
    size_t generateWriteSingleResponse(uint8_t* response);
    size_t generateWriteMultipleResponse(uint8_t* response);
    size_t generateErrorResponse(uint8_t* response, uint8_t error_code);
    
    void sendResponse(const uint8_t* response, size_t length);
    
    uint16_t getRegisterValue(uint16_t address);
    void setRegisterValue(uint16_t address, uint16_t value);
    void handleRegisterUpdate(uint16_t address, uint16_t value);
    void updateSonicatorStatus(uint8_t sonicator_id, bool running);
    
public:
    /**
     * @brief Constructor
     * @param slave_address MODBUS slave address (default: 2)
     * @param baud_rate Serial baud rate (default: 115200)
     */
    SerialPortSimulator(uint8_t slave_address = 2, uint32_t baud_rate = 115200);
    
    /**
     * @brief Destructor
     */
    ~SerialPortSimulator();
    
    /**
     * @brief Open the simulated serial port
     * @return true if successful
     */
    bool open();
    
    /**
     * @brief Close the simulated serial port
     */
    void close();
    
    /**
     * @brief Check if port is open
     * @return true if open
     */
    bool isOpen() const;
    
    /**
     * @brief Write data to the simulator (as if from master)
     * @param data Data to write
     * @param length Number of bytes to write
     * @return Number of bytes actually written
     */
    size_t write(const uint8_t* data, size_t length);
    
    /**
     * @brief Read data from the simulator (response to master)
     * @param buffer Buffer to read into
     * @param length Maximum bytes to read
     * @return Number of bytes actually read
     */
    size_t read(uint8_t* buffer, size_t length);
    
    /**
     * @brief Check how many bytes are available to read
     * @return Number of bytes available
     */
    size_t available();
    
    /**
     * @brief Configure error simulation
     * @param enabled Enable/disable error simulation
     * @param error_rate Error rate (0.0 to 1.0)
     */
    void setErrorSimulation(bool enabled, float error_rate = 0.01f);
    
    /**
     * @brief Set response delay simulation
     * @param delay_ms Delay in milliseconds
     */
    void setResponseDelay(uint32_t delay_ms);
    
    /**
     * @brief Get bytes received counter
     * @return Total bytes received
     */
    uint32_t getBytesReceived() const;
    
    /**
     * @brief Get bytes transmitted counter
     * @return Total bytes transmitted
     */
    uint32_t getBytesTransmitted() const;
    
    /**
     * @brief Get error counter
     * @return Total errors encountered
     */
    uint32_t getErrorCount() const;
    
    /**
     * @brief Reset communication statistics
     */
    void resetStatistics();
};

#endif // SERIAL_SIMULATOR_H