/**
 * @file sonicator.cpp
 * @brief Single Sonicator Control Implementation for Epic 1 Story 1.4
 * @author Cannasol Technologies  
 * @date 2025-09-12
 * @version 1.0.0
 * 
 * @details
 * Implementation of single sonicator control logic with state machine,
 * safety monitoring, and HAL integration. Designed for Sonicator 4
 * (zero-based ID 3) per pin matrix configuration.
 * 
 * Features:
 * - Non-blocking state machine for 1kHz execution
 * - Hardware abstraction via HAL interfaces  
 * - Safety monitoring with fault detection
 * - MODBUS register integration ready
 * - Comprehensive error handling and recovery
 */

#include "sonicator_control.h"
#include "config.h"
#include "modules/hal/gpio.h"
#include "modules/hal/pwm.h"
#include "modules/hal/adc.h"
#include <Arduino.h> // For millis() timing
#include <string.h> // For memset

// ============================================================================
// COMPILE-TIME CONFIGURATION
// ============================================================================

// Enable simulation mode for unit testing (bypasses HAL calls)
#ifndef SONICATOR_SIMULATION_MODE
#define SONICATOR_SIMULATION_MODE 0
#endif

// Debug logging (only in DEBUG builds)
#ifdef DEBUG_MODE
#define SONICATOR_LOG(msg) Serial.println(msg)
#else
#define SONICATOR_LOG(msg) ((void)0)
#endif

// ============================================================================
// STATIC VARIABLES (single sonicator instance)
// ============================================================================

static sonicator_state_control_t g_sonicator = {0};
static bool g_simulation_mode = SONICATOR_SIMULATION_MODE;
static bool g_initialized = false;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Get current system timestamp in milliseconds
 * @return Current timestamp
 */
static inline uint32_t get_timestamp_ms(void) {
    return millis();
}

/**
 * @brief Check if timeout period has elapsed
 * @param start_time Start timestamp
 * @param timeout_ms Timeout period in milliseconds  
 * @return true if timeout elapsed
 */
static inline bool is_timeout(uint32_t start_time, uint32_t timeout_ms) {
    return (get_timestamp_ms() - start_time) >= timeout_ms;
}

/**
 * @brief Clamp amplitude to valid range (20-100%)
 * @param amplitude Input amplitude
 * @return Clamped amplitude
 */
static inline uint8_t clamp_amplitude(uint8_t amplitude) {
    if (amplitude < SONICATOR_MIN_AMPLITUDE_PERCENT) {
        return SONICATOR_MIN_AMPLITUDE_PERCENT;
    }
    if (amplitude > SONICATOR_MAX_AMPLITUDE_PERCENT) {
        return SONICATOR_MAX_AMPLITUDE_PERCENT; 
    }
    return amplitude;
}

/**
 * @brief Convert amplitude percentage to PWM duty cycle
 * @param amplitude_percent Amplitude (20-100%)
 * @return PWM value (0-255)
 */
static inline uint8_t amplitude_to_pwm(uint8_t amplitude_percent) {
    // Scale 20-100% to 0-255 PWM range
    // PWM = (amplitude - 20) * 255 / (100 - 20)
    if (amplitude_percent < SONICATOR_MIN_AMPLITUDE_PERCENT) {
        return 0;
    }
    return ((amplitude_percent - SONICATOR_MIN_AMPLITUDE_PERCENT) * 255) / 
           (SONICATOR_MAX_AMPLITUDE_PERCENT - SONICATOR_MIN_AMPLITUDE_PERCENT);
}

// ============================================================================
// HAL INTERFACE WRAPPERS (with simulation mode support)
// ============================================================================

/**
 * @brief Write GPIO pin with simulation mode support
 * @param pin Pin identifier  
 * @param state Pin state (true=HIGH, false=LOW)
 */
static void hal_gpio_write_safe(uint8_t pin, bool state) {
    if (!g_simulation_mode) {
        gpio_write_pin(pin, state ? GPIO_HIGH : GPIO_LOW);
    }
    SONICATOR_LOG(state ? "GPIO HIGH" : "GPIO LOW");
}

/**
 * @brief Read GPIO pin with simulation mode support
 * @param pin Pin identifier
 * @return Pin state (true=HIGH, false=LOW)
 */
static bool hal_gpio_read_safe(uint8_t pin) {
    if (g_simulation_mode) {
        // In simulation mode, return safe defaults
        if (pin == SON4_OVERLOAD_PIN) return false; // No overload
        if (pin == SON4_FREQ_LOCK_PIN) return true; // Frequency locked
        return false;
    }
    gpio_state_t state;
    gpio_read_pin(pin, &state);
    return (state == GPIO_HIGH);
}

/**
 * @brief Set PWM duty cycle with simulation mode support
 * @param pin PWM pin identifier
 * @param duty_cycle Duty cycle (0-255)
 */
static void hal_pwm_set_safe(uint8_t pin, uint8_t duty_cycle) {
    if (!g_simulation_mode) {
        pwm_set_duty_cycle(PWM_CHANNEL_AMPLITUDE, duty_cycle);
    }
    SONICATOR_LOG("PWM SET");
}

/**
 * @brief Read ADC channel with simulation mode support  
 * @param channel ADC channel
 * @return ADC value (0-1023)
 */
static uint16_t hal_adc_read_safe(adc_channel_t channel) {
    if (g_simulation_mode) {
        // Return simulated power reading (500W)
        return (uint16_t)((500UL * 1023UL) / 2000UL); // Scale to ADC range
    }
    uint16_t value;
    if (adc_read_channel(channel, &value) == ADC_OK) {
        return value;
    }
    return 0;
}

// ============================================================================
// HARDWARE INTERFACE FUNCTIONS
// ============================================================================

/**
 * @brief Update hardware outputs based on current state
 */
static void update_hardware_outputs(void) {
    sonicator_state_control_t* ctrl = &g_sonicator;
    
    // Update START signal (PC0)
    bool start_signal = (ctrl->state == SONICATOR_STATE_RUNNING ||
                        ctrl->state == SONICATOR_STATE_STARTING);
    hal_gpio_write_safe(SON4_START_PIN, start_signal);
    
    // Update amplitude PWM (PD7) - shared for all sonicators
    uint8_t pwm_value = 0;
    if (ctrl->state == SONICATOR_STATE_RUNNING) {
        pwm_value = amplitude_to_pwm(ctrl->amplitude_percent);
    }
    hal_pwm_set_safe(PWM_AMPLITUDE_CONTROL_PIN, pwm_value);
    
    // Handle reset pulse timing (PC1)
    static uint32_t reset_pulse_start = 0;
    static bool reset_pulse_active = false;
    
    if (ctrl->reset_requested && !reset_pulse_active) {
        hal_gpio_write_safe(SON4_RESET_PIN, true);
        reset_pulse_start = get_timestamp_ms();
        reset_pulse_active = true;
        ctrl->reset_requested = false;
        SONICATOR_LOG("Reset pulse started");
    }
    
    if (reset_pulse_active && is_timeout(reset_pulse_start, SONICATOR_RESET_PULSE_MS)) {
        hal_gpio_write_safe(SON4_RESET_PIN, false);
        reset_pulse_active = false;
        SONICATOR_LOG("Reset pulse ended");
    }
}

/**
 * @brief Read hardware inputs and update status
 */
static void read_hardware_inputs(void) {
    sonicator_state_control_t* ctrl = &g_sonicator;
    
    // Read overload status (PD3)
    ctrl->overload_active = hal_gpio_read_safe(SON4_OVERLOAD_PIN);
    
    // Read frequency lock status (PB4)  
    ctrl->frequency_locked = hal_gpio_read_safe(SON4_FREQ_LOCK_PIN);
    
    // Read power measurement (PA7)
    uint16_t adc_value = hal_adc_read_safe(ADC_CHANNEL_7);
    // Convert ADC to watts: ADC range 0-1023 maps to 0-2000W
    ctrl->power_watts = (float)(adc_value * 2000) / 1023.0f;
    
    // Frequency measurement (simplified - would need timer capture in full implementation)
    if (ctrl->frequency_locked) {
        ctrl->frequency_hz = 20000; // Nominal CT2000 frequency
    } else {
        ctrl->frequency_hz = 0;
    }
}

/**
 * @brief Check for fault conditions
 * @return Active fault mask
 */
static sonicator_fault_t check_fault_conditions(void) {
    sonicator_state_control_t* ctrl = &g_sonicator;
    sonicator_fault_t faults = SONICATOR_FAULT_NONE;
    uint32_t now = get_timestamp_ms();
    
    // Check overload condition with debouncing
    static uint32_t overload_detected_time = 0;
    static bool overload_debounce = false;
    
    if (ctrl->overload_active && !overload_debounce) {
        overload_detected_time = now;
        overload_debounce = true;
    } else if (!ctrl->overload_active) {
        overload_debounce = false;
    }
    
    if (overload_debounce && is_timeout(overload_detected_time, SONICATOR_FAULT_DEBOUNCE_MS)) {
        faults |= SONICATOR_FAULT_OVERLOAD;
    }
    
    // Check frequency lock loss (only when running)
    if ((ctrl->state == SONICATOR_STATE_RUNNING) && !ctrl->frequency_locked) {
        faults |= SONICATOR_FAULT_FREQ_UNLOCK;
    }
    
    // Check communication timeout
    if (is_timeout(ctrl->last_update_time, SONICATOR_COMM_TIMEOUT_MS)) {
        faults |= SONICATOR_FAULT_COMM_TIMEOUT;
    }
    
    // Check watchdog timeout  
    if (is_timeout(ctrl->watchdog_last_reset, SONICATOR_WATCHDOG_TIMEOUT_MS)) {
        faults |= SONICATOR_FAULT_WATCHDOG;
    }
    
    return faults;
}

/**
 * @brief Handle fault detection and emergency stop
 * @param faults Active fault conditions
 */
static void handle_fault_conditions(sonicator_fault_t faults) {
    sonicator_state_control_t* ctrl = &g_sonicator;
    
    if (faults != SONICATOR_FAULT_NONE) {
        // Emergency stop - immediate hardware shutdown
        hal_gpio_write_safe(SON4_START_PIN, false);
        hal_pwm_set_safe(PWM_AMPLITUDE_CONTROL_PIN, 0);
        
        // Update state and fault tracking
        ctrl->previous_state = ctrl->state;
        ctrl->state = SONICATOR_STATE_FAULT;
        ctrl->state_entry_time = get_timestamp_ms();
        ctrl->active_faults = faults;
        ctrl->fault_count++;
        ctrl->last_fault_time = ctrl->state_entry_time;
        ctrl->is_running = false;
        
        SONICATOR_LOG("FAULT DETECTED - Emergency stop activated");
    }
}

// ============================================================================
// STATE MACHINE IMPLEMENTATION  
// ============================================================================

/**
 * @brief Process state machine transitions
 */
static void process_state_machine(void) {
    sonicator_state_control_t* ctrl = &g_sonicator;
    uint32_t now = get_timestamp_ms();
    uint32_t state_duration = now - ctrl->state_entry_time;
    
    switch (ctrl->state) {
        case SONICATOR_STATE_IDLE:
            ctrl->is_running = false;
            
            // Transition: IDLE → STARTING
            if (ctrl->start_requested && (ctrl->active_faults == SONICATOR_FAULT_NONE)) {
                ctrl->previous_state = ctrl->state;
                ctrl->state = SONICATOR_STATE_STARTING;
                ctrl->state_entry_time = now;
                ctrl->start_requested = false;
                SONICATOR_LOG("State: IDLE -> STARTING");
            }
            break;
            
        case SONICATOR_STATE_STARTING:
            ctrl->is_running = false;
            
            // Transition: STARTING → RUNNING (after delay)
            if (state_duration >= SONICATOR_START_DELAY_MS) {
                ctrl->previous_state = ctrl->state;
                ctrl->state = SONICATOR_STATE_RUNNING;
                ctrl->state_entry_time = now;
                ctrl->is_running = true;
                ctrl->start_count++;
                ctrl->last_start_time = now;
                SONICATOR_LOG("State: STARTING -> RUNNING");
            }
            break;
            
        case SONICATOR_STATE_RUNNING:
            ctrl->is_running = true;
            
            // Update runtime statistics
            static uint32_t last_runtime_update = 0;
            if (now - last_runtime_update >= 1000) { // Update every second
                ctrl->total_runtime_ms += (now - last_runtime_update);
                last_runtime_update = now;
            }
            
            // Transition: RUNNING → STOPPING
            if (ctrl->stop_requested) {
                ctrl->previous_state = ctrl->state;
                ctrl->state = SONICATOR_STATE_STOPPING;
                ctrl->state_entry_time = now;
                ctrl->stop_requested = false;
                SONICATOR_LOG("State: RUNNING -> STOPPING");
            }
            break;
            
        case SONICATOR_STATE_STOPPING:
            ctrl->is_running = false;
            
            // Transition: STOPPING → IDLE (after delay)
            if (state_duration >= SONICATOR_STOP_DELAY_MS) {
                ctrl->previous_state = ctrl->state;
                ctrl->state = SONICATOR_STATE_IDLE;
                ctrl->state_entry_time = now;
                SONICATOR_LOG("State: STOPPING -> IDLE");
            }
            break;
            
        case SONICATOR_STATE_FAULT:
            ctrl->is_running = false;
            
            // Transition: FAULT → IDLE (when faults cleared)
            if (ctrl->active_faults == SONICATOR_FAULT_NONE) {
                ctrl->previous_state = ctrl->state;
                ctrl->state = SONICATOR_STATE_IDLE;
                ctrl->state_entry_time = now;
                SONICATOR_LOG("State: FAULT -> IDLE");
            }
            break;
            
        default:
            // Unknown state - go to fault
            ctrl->state = SONICATOR_STATE_FAULT;
            ctrl->state_entry_time = now;
            ctrl->active_faults |= SONICATOR_FAULT_HARDWARE;
            SONICATOR_LOG("State: UNKNOWN -> FAULT");
            break;
    }
}

// ============================================================================
// PUBLIC API IMPLEMENTATION
// ============================================================================

bool sonicator_begin(void) {
    if (g_initialized) {
        return true;
    }
    
    // Initialize control structure
    memset(&g_sonicator, 0, sizeof(g_sonicator));
    g_sonicator.state = SONICATOR_STATE_IDLE;
    g_sonicator.previous_state = SONICATOR_STATE_UNKNOWN;
    g_sonicator.state_entry_time = get_timestamp_ms();
    g_sonicator.amplitude_percent = SONICATOR_MIN_AMPLITUDE_PERCENT;
    g_sonicator.last_update_time = g_sonicator.state_entry_time;
    g_sonicator.watchdog_last_reset = g_sonicator.state_entry_time;
    
    // Initialize HAL if not in simulation mode
    if (!g_simulation_mode) {
        if (hal_init() != HAL_OK) {
            SONICATOR_LOG("HAL initialization failed");
            return false;
        }
    }
    
    g_initialized = true;
    SONICATOR_LOG("Sonicator control initialized");
    return true;
}

bool sonicator_start(void) {
    if (!g_initialized) {
        return false;
    }
    
    sonicator_state_control_t* ctrl = &g_sonicator;
    
    // Can only start from IDLE state with no active faults
    if (ctrl->state != SONICATOR_STATE_IDLE || ctrl->active_faults != SONICATOR_FAULT_NONE) {
        return false;
    }
    
    ctrl->start_requested = true;
    return true;
}

bool sonicator_stop(void) {
    if (!g_initialized) {
        return false;
    }
    
    sonicator_state_control_t* ctrl = &g_sonicator;
    
    // Can stop from RUNNING or STARTING states
    if (ctrl->state == SONICATOR_STATE_RUNNING || ctrl->state == SONICATOR_STATE_STARTING) {
        ctrl->stop_requested = true;
        return true;
    }
    
    return false;
}

bool sonicator_set_amplitude(uint8_t amplitude_percent) {
    if (!g_initialized) {
        return false;
    }
    
    uint8_t clamped_amplitude = clamp_amplitude(amplitude_percent);
    g_sonicator.amplitude_percent = clamped_amplitude;
    
    return (clamped_amplitude == amplitude_percent); // Return false if clamping occurred
}

bool sonicator_reset_overload(void) {
    if (!g_initialized) {
        return false;
    }
    
    sonicator_state_control_t* ctrl = &g_sonicator;
    
    // Can only reset if in FAULT state due to overload
    if (ctrl->state == SONICATOR_STATE_FAULT && 
        (ctrl->active_faults & SONICATOR_FAULT_OVERLOAD)) {
        
        // Clear overload fault and request reset pulse
        ctrl->active_faults &= ~SONICATOR_FAULT_OVERLOAD;
        ctrl->reset_requested = true;
        return true;
    }
    
    return false;
}

bool sonicator_emergency_stop(void) {
    if (!g_initialized) {
        return false;
    }
    
    sonicator_state_control_t* ctrl = &g_sonicator;
    
    // Immediate emergency stop
    hal_gpio_write_safe(SON4_START_PIN, false);
    hal_pwm_set_safe(PWM_AMPLITUDE_CONTROL_PIN, 0);
    
    // Force to FAULT state
    ctrl->previous_state = ctrl->state;
    ctrl->state = SONICATOR_STATE_FAULT;
    ctrl->state_entry_time = get_timestamp_ms();
    ctrl->active_faults |= SONICATOR_FAULT_HARDWARE;
    ctrl->is_running = false;
    
    SONICATOR_LOG("EMERGENCY STOP");
    return true;
}

sonicator_state_t sonicator_update(void) {
    if (!g_initialized) {
        return SONICATOR_STATE_UNKNOWN;
    }
    
    uint32_t now = get_timestamp_ms();
    sonicator_state_control_t* ctrl = &g_sonicator;
    
    // Update timestamps
    ctrl->last_update_time = now;
    ctrl->watchdog_last_reset = now;
    
    // Read hardware inputs
    read_hardware_inputs();
    
    // Check for fault conditions
    sonicator_fault_t faults = check_fault_conditions();
    if (faults != SONICATOR_FAULT_NONE) {
        handle_fault_conditions(faults);
    }
    
    // Process state machine
    process_state_machine();
    
    // Update hardware outputs
    update_hardware_outputs();
    
    return ctrl->state;
}

const sonicator_state_control_t* sonicator_get_status(void) {
    return g_initialized ? &g_sonicator : NULL;
}

const char* sonicator_state_to_string(sonicator_state_t state) {
    switch (state) {
        case SONICATOR_STATE_IDLE: return "IDLE";
        case SONICATOR_STATE_STARTING: return "STARTING";  
        case SONICATOR_STATE_RUNNING: return "RUNNING";
        case SONICATOR_STATE_STOPPING: return "STOPPING";
        case SONICATOR_STATE_FAULT: return "FAULT";
        case SONICATOR_STATE_UNKNOWN: return "UNKNOWN";
        default: return "INVALID";
    }
}

bool sonicator_is_safe(void) {
    if (!g_initialized) {
        return false;
    }
    
    const sonicator_state_control_t* ctrl = &g_sonicator;
    
    // Check all safety conditions
    bool no_faults = (ctrl->active_faults == SONICATOR_FAULT_NONE);
    bool watchdog_ok = !is_timeout(ctrl->watchdog_last_reset, SONICATOR_WATCHDOG_TIMEOUT_MS);
    bool comm_ok = !is_timeout(ctrl->last_update_time, SONICATOR_COMM_TIMEOUT_MS);
    bool state_ok = (ctrl->state != SONICATOR_STATE_FAULT && ctrl->state != SONICATOR_STATE_UNKNOWN);
    
    return no_faults && watchdog_ok && comm_ok && state_ok;
}

void sonicator_reset_statistics(void) {
    if (!g_initialized) {
        return;
    }
    
    sonicator_state_control_t* ctrl = &g_sonicator;
    
    // Reset statistics but preserve current operational state
    ctrl->start_count = 0;
    ctrl->total_runtime_ms = 0;
    ctrl->fault_count = 0;
    ctrl->last_fault_time = 0;
    ctrl->last_start_time = 0;
}

// ============================================================================
// ADVANCED FUNCTIONS (testing and diagnostics)
// ============================================================================

bool sonicator_force_state(sonicator_state_t new_state) {
    if (!g_initialized) {
        return false;
    }
    
    sonicator_state_control_t* ctrl = &g_sonicator;
    ctrl->previous_state = ctrl->state;
    ctrl->state = new_state;
    ctrl->state_entry_time = get_timestamp_ms();
    
    SONICATOR_LOG("FORCED STATE CHANGE (TESTING)");
    return true;
}

bool sonicator_inject_fault(sonicator_fault_t fault_mask) {
    if (!g_initialized) {
        return false;
    }
    
    g_sonicator.active_faults |= fault_mask;
    SONICATOR_LOG("FAULT INJECTED (TESTING)");
    return true;
}

void sonicator_set_simulation_mode(bool enable) {
    g_simulation_mode = enable;
    SONICATOR_LOG(enable ? "Simulation mode enabled" : "Simulation mode disabled");
}
