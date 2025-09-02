/**
 * @file modbus_example.cpp
 * @brief Example implementation showing MODBUS integration
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 * 
 * @details
 * Demonstrates how to integrate the MODBUS framework with the
 * sonicator control system. Shows proper initialization,
 * register handling, and communication management.
 */

#include <Arduino.h>
#include "modules/communication/modbus.h"
#include "modules/communication/modbus_register_manager.h"

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static uint32_t last_status_update = 0;
static uint32_t last_comm_check = 0;
static bool system_initialized = false;

// ============================================================================
// CALLBACK IMPLEMENTATIONS
// ============================================================================

/**
 * @brief Handle communication timeout events
 */
void handle_communication_timeout(void) {
    Serial.println("MODBUS communication timeout detected - activating failsafe");
    
    // Activate failsafe mode
    register_manager_update_system_status(SYSTEM_STATUS_COMM_FAULT, true);
    
    // Stop all sonicators for safety
    for (int i = 0; i < MODBUS_MAX_SONICATORS; i++) {
        register_manager_update_sonicator_status(i, 0, 0, 0, 0);
    }
    
    // Increment communication error count
    register_manager_increment_comm_errors();
}

/**
 * @brief Handle communication errors
 */
void handle_communication_error(modbus_error_t error_code) {
    Serial.print("MODBUS communication error: ");
    Serial.println(error_code);
    
    // Update system status based on error type
    switch (error_code) {
        case MODBUS_ERROR_CRC:
        case MODBUS_ERROR_TIMEOUT:
            register_manager_update_system_status(SYSTEM_STATUS_COMM_FAULT, true);
            break;
        case MODBUS_ERROR_SLAVE_FAILURE:
            register_manager_update_system_status(SYSTEM_STATUS_FAULT, true);
            break;
        default:
            break;
    }
    
    register_manager_increment_comm_errors();
}

/**
 * @brief Custom register read handler for extended functionality
 */
modbus_error_t handle_register_read(uint16_t address, uint16_t* value) {
    // Example: Add custom logic for specific registers
    switch (address) {
        case 0x0005: // Custom diagnostic register
            *value = millis() / 1000; // System uptime in seconds
            return MODBUS_OK;
        default:
            // Use default register map
            return MODBUS_ERROR_ILLEGAL_ADDRESS;
    }
}

/**
 * @brief Custom register write handler for extended functionality
 */
modbus_error_t handle_register_write(uint16_t address, uint16_t value) {
    // Example: Add custom logic for specific control commands
    switch (address) {
        case MODBUS_REG_SYSTEM_RESET:
            if (value == 0xABCD) { // Special reset code
                Serial.println("System reset requested via MODBUS");
                delay(100);
                // Perform controlled system reset
                return MODBUS_OK;
            }
            break;
        default:
            break;
    }
    
    return MODBUS_ERROR_ILLEGAL_VALUE;
}

// ============================================================================
// SONICATOR SIMULATION FUNCTIONS
// ============================================================================

/**
 * @brief Simulate sonicator operation and update status
 */
void update_sonicator_status(uint8_t sonicator_id) {
    if (sonicator_id >= MODBUS_MAX_SONICATORS) return;
    
    uint16_t start_stop, amplitude_sp;
    if (!register_manager_get_sonicator_control(sonicator_id, &start_stop, &amplitude_sp)) {
        return;
    }
    
    // Simulate sonicator operation
    uint16_t power_watts = 0;
    uint16_t frequency_hz = 20000;
    uint16_t amplitude_actual = 0;
    uint16_t status_flags = 0;
    
    if (start_stop && register_manager_is_global_enabled() && 
        !register_manager_is_emergency_stop_active()) {
        
        // Sonicator is running
        status_flags |= SON_STATUS_RUNNING;
        
        // Simulate power consumption based on amplitude
        power_watts = (amplitude_sp * 2); // 2W per percent amplitude
        
        // Simulate frequency variation
        frequency_hz = 20000 + (random(-50, 50));
        
        // Simulate amplitude tracking with some error
        amplitude_actual = amplitude_sp + random(-2, 2);
        if (amplitude_actual > 100) amplitude_actual = 100;
        if (amplitude_actual < 0) amplitude_actual = 0;
        
        // Simulate frequency lock after startup
        if (millis() % 10000 > 2000) { // After 2 seconds
            status_flags |= SON_STATUS_FREQ_LOCK;
        }
        
        // Simulate occasional overload
        if (random(0, 1000) < 5) { // 0.5% chance
            status_flags |= SON_STATUS_OVERLOAD;
            power_watts = 0;
        }
    }
    
    // Update register map
    register_manager_update_sonicator_status(sonicator_id, power_watts, 
                                           frequency_hz, amplitude_actual, status_flags);
}

// ============================================================================
// MAIN APPLICATION FUNCTIONS
// ============================================================================

/**
 * @brief Initialize the MODBUS communication system
 */
bool initialize_modbus_system(void) {
    Serial.println("Initializing MODBUS RTU Communication Framework...");
    
    // Initialize register manager
    if (!register_manager_init()) {
        Serial.println("ERROR: Failed to initialize register manager");
        return false;
    }
    
    // Configure MODBUS
    modbus_config_t config;
    config.slave_id = MODBUS_SLAVE_ID;
    config.baud_rate = MODBUS_BAUD_RATE;
    config.timeout_ms = MODBUS_TIMEOUT_MS;
    config.read_callback = handle_register_read;
    config.write_callback = handle_register_write;
    config.timeout_callback = handle_communication_timeout;
    config.error_callback = handle_communication_error;
    
    // Initialize MODBUS communication
    modbus_error_t result = modbus_init(&config);
    if (result != MODBUS_OK) {
        Serial.print("ERROR: Failed to initialize MODBUS: ");
        Serial.println(result);
        return false;
    }
    
    Serial.println("MODBUS RTU Communication initialized successfully");
    Serial.print("Slave ID: ");
    Serial.println(MODBUS_SLAVE_ID);
    Serial.print("Baud Rate: ");
    Serial.println(MODBUS_BAUD_RATE);
    Serial.println("Register map ready for PLC/HMI communication");
    
    return true;
}

/**
 * @brief Main application setup
 */
void setup(void) {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("=================================================");
    Serial.println("Multi-Sonicator MODBUS Communication Example");
    Serial.println("Epic 1 Story 3: MODBUS RTU Framework");
    Serial.println("=================================================");
    
    // Initialize MODBUS system
    system_initialized = initialize_modbus_system();
    
    if (system_initialized) {
        Serial.println("System ready for operation");
    } else {
        Serial.println("System initialization failed!");
    }
    
    last_status_update = millis();
    last_comm_check = millis();
}

/**
 * @brief Main application loop
 */
void loop(void) {
    if (!system_initialized) {
        delay(1000);
        return;
    }
    
    uint32_t current_time = millis();
    
    // Process MODBUS communication
    modbus_state_t state = modbus_process();
    
    // Check for communication timeout
    if (current_time - last_comm_check >= 500) { // Check every 500ms
        if (modbus_is_timeout()) {
            Serial.println("Communication timeout detected");
        }
        last_comm_check = current_time;
    }
    
    // Update sonicator status periodically
    if (current_time - last_status_update >= 100) { // Update every 100ms
        static uint8_t current_sonicator = 0;
        
        // Update one sonicator per cycle to spread the load
        update_sonicator_status(current_sonicator);
        current_sonicator = (current_sonicator + 1) % 4; // Update first 4 sonicators
        
        last_status_update = current_time;
    }
    
    // Print statistics periodically
    static uint32_t last_stats_print = 0;
    if (current_time - last_stats_print >= 10000) { // Every 10 seconds
        const modbus_statistics_t* stats = modbus_get_statistics();
        
        Serial.println("=== MODBUS Statistics ===");
        Serial.print("Requests received: ");
        Serial.println(stats->requests_received);
        Serial.print("Responses sent: ");
        Serial.println(stats->responses_sent);
        Serial.print("CRC errors: ");
        Serial.println(stats->crc_errors);
        Serial.print("Timeout errors: ");
        Serial.println(stats->timeout_errors);
        Serial.print("Max response time: ");
        Serial.print(stats->max_response_time);
        Serial.println("ms");
        
        // Print active sonicator status
        modbus_register_map_t* map = register_manager_get_map();
        Serial.print("Active sonicators: ");
        Serial.print(map->system_status.active_count);
        Serial.print(" (mask: 0x");
        Serial.print(map->system_status.active_mask, HEX);
        Serial.println(")");
        
        last_stats_print = current_time;
    }
    
    // Small delay to prevent overwhelming the system
    delay(10);
}
