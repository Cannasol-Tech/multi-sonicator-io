/**
 * @file modbus_register_manager.cpp
 * @brief MODBUS Register Map Management Implementation
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

#include "modbus_register_manager.h"
#include <string.h>

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================

static modbus_register_map_t register_map;
static bool manager_initialized = false;

// ============================================================================
// PUBLIC FUNCTION IMPLEMENTATIONS
// ============================================================================

bool register_manager_init(void) {
    // Initialize register map to default values
    memset(&register_map, 0, sizeof(register_map));
    
    // Set default system status
    register_map.system_status.system_status = SYSTEM_STATUS_OK;
    register_map.system_status.active_count = 0;
    register_map.system_status.active_mask = 0;
    register_map.system_status.watchdog_status = 1; // Watchdog OK
    register_map.system_status.comm_errors = 0;
    
    // Initialize all sonicators to safe defaults
    for (int i = 0; i < MODBUS_MAX_SONICATORS; i++) {
        register_map.sonicators[i].start_stop = 0;          // Stopped
        register_map.sonicators[i].amplitude_setpoint = 50;  // 50% default
        register_map.sonicators[i].overload_reset = 0;
        register_map.sonicators[i].power_watts = 0;
        register_map.sonicators[i].frequency_hz = 20000;    // 20kHz default
        register_map.sonicators[i].status_flags = 0;
        register_map.sonicators[i].amplitude_actual = 0;
    }
    
    manager_initialized = true;
    return true;
}

modbus_register_map_t* register_manager_get_map(void) {
    if (!manager_initialized) {
        register_manager_init();
    }
    return &register_map;
}

void register_manager_update_system_status(uint16_t status_bits, bool set) {
    if (!manager_initialized) return;
    
    if (set) {
        register_map.system_status.system_status |= status_bits;
    } else {
        register_map.system_status.system_status &= ~status_bits;
    }
}

void register_manager_update_sonicator_status(uint8_t sonicator_id,
                                             uint16_t power_watts,
                                             uint16_t frequency_hz,
                                             uint16_t amplitude_actual,
                                             uint16_t status_flags) {
    if (!manager_initialized || sonicator_id >= MODBUS_MAX_SONICATORS) {
        return;
    }
    
    register_map.sonicators[sonicator_id].power_watts = power_watts;
    register_map.sonicators[sonicator_id].frequency_hz = frequency_hz;
    register_map.sonicators[sonicator_id].amplitude_actual = amplitude_actual;
    register_map.sonicators[sonicator_id].status_flags = status_flags;
    
    // Update system active count and mask
    uint16_t active_count = 0;
    uint16_t active_mask = 0;
    
    for (int i = 0; i < MODBUS_MAX_SONICATORS; i++) {
        if (register_map.sonicators[i].status_flags & SON_STATUS_RUNNING) {
            active_count++;
            active_mask |= (1 << i);
        }
    }
    
    register_map.system_status.active_count = active_count;
    register_map.system_status.active_mask = active_mask;
}

bool register_manager_get_sonicator_control(uint8_t sonicator_id,
                                           uint16_t* start_stop,
                                           uint16_t* amplitude_sp) {
    if (!manager_initialized || sonicator_id >= MODBUS_MAX_SONICATORS ||
        !start_stop || !amplitude_sp) {
        return false;
    }
    
    *start_stop = register_map.sonicators[sonicator_id].start_stop;
    *amplitude_sp = register_map.sonicators[sonicator_id].amplitude_setpoint;
    
    return true;
}

bool register_manager_is_global_enabled(void) {
    if (!manager_initialized) return false;
    return (register_map.global_control.global_enable != 0);
}

bool register_manager_is_emergency_stop_active(void) {
    if (!manager_initialized) return false;
    return (register_map.global_control.emergency_stop != 0) ||
           (register_map.system_status.system_status & SYSTEM_STATUS_EMERGENCY_STOP);
}

void register_manager_clear_emergency_stop(void) {
    if (!manager_initialized) return;
    
    register_map.global_control.emergency_stop = 0;
    register_manager_update_system_status(SYSTEM_STATUS_EMERGENCY_STOP, false);
}

void register_manager_increment_comm_errors(void) {
    if (!manager_initialized) return;
    register_map.system_status.comm_errors++;
}

void register_manager_reset_comm_errors(void) {
    if (!manager_initialized) return;
    register_map.system_status.comm_errors = 0;
}

bool register_manager_validate_value(uint16_t address, uint16_t value) {
    if (!manager_initialized) return false;
    
    // Validate based on register type
    if (address >= 0x0100 && address <= 0x041F) {
        uint16_t reg_offset = (address - 0x0100) % MODBUS_REG_SONICATOR_STRIDE;
        
        switch (reg_offset) {
            case MODBUS_REG_SON_START_STOP:
                // Start/stop must be 0 or 1
                return (value <= 1);
                
            case MODBUS_REG_SON_AMPLITUDE_SP:
                // Amplitude must be 20-100%
                return (value >= 20 && value <= 100);
                
            case MODBUS_REG_SON_OVERLOAD_RESET:
                // Reset command must be 0 or 1
                return (value <= 1);
                
            default:
                // Other registers accept any 16-bit value
                return true;
        }
    }
    
    // Global control registers
    if (address >= 0x0010 && address <= 0x001F) {
        switch (address) {
            case MODBUS_REG_GLOBAL_ENABLE:
            case MODBUS_REG_EMERGENCY_STOP:
            case MODBUS_REG_SYSTEM_RESET:
                // Control commands must be 0 or 1
                return (value <= 1);
                
            default:
                return true;
        }
    }
    
    // System status registers are read-only, but value validation not needed
    return true;
}
