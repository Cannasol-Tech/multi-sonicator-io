/**
 * @file sonicator/sonicator.h
 * @brief Sonicator IO Control and Monitoring API (single-unit interface)
 *
 * This header defines the SonicatorInterface class which encapsulates the control
 * and monitoring logic for a single CT2000 sonicator channel. It is designed to be
 * instantiated four times by the SonicMultiplexer, each with its own pin mapping.
 *
 * Architecture:
 * - Multiplexer owns four SonicatorInterface instances
 * - Multiplexer.update() calls each SonicatorInterface::update()
 * - Each SonicatorInterface reads its own MODBUS control, applies HAL actions,
 *   and publishes status back to MODBUS (bridge in the unit)
 */

#ifndef SONICATOR_H
#define SONICATOR_H

#include <stdint.h>
#include <stdbool.h>

#include "modules/hal/adc.h"
#include "modules/hal/hal.h"

#include "sonicator/types/state.h"
#include "sonicator/types/errors.h"
#include "sonicator/types/control.h"

// Timing constants (kept here for unit-level logic; must match implementation)
#define SONICATOR_STOP_DELAY_MS         100
#define SONICATOR_START_DELAY_MS        50
#define SONICATOR_RESET_PULSE_MS        20
#define SONICATOR_COMM_TIMEOUT_MS       2000
#define SONICATOR_FAULT_DEBOUNCE_MS     10
#define SONICATOR_WATCHDOG_TIMEOUT_MS   1000

// Amplitude limits
#define SONICATOR_MIN_AMPLITUDE_PERCENT 20
#define SONICATOR_MAX_AMPLITUDE_PERCENT 100

/**
 * @brief Pin bundle describing a single sonicator channel
 */
typedef struct {
    uint8_t       sonicator_id;        // 1..4 logical ID (used for MODBUS index and frequency channel select)
    uint8_t       start_pin;           // Digital output pin to start sonics (ULN2003A -> CT2000)
    uint8_t       reset_pin;           // Digital output pin to reset overload (ULN2003A -> CT2000)
    uint8_t       overload_pin;        // Digital input (6N137 -> CT2000 overload)
    uint8_t       freq_lock_pin;       // Digital input (6N137 -> CT2000 frequency lock)
    uint8_t       freq_div10_pin;      // Digital input for frequency ÷10 (pin change ISR channel select)
    adc_channel_t power_sense_channel; // ADC channel for 0-10V power monitor (raw ADC stored)
} SonicatorPins;

/**
 * @class SonicatorInterface
 * @brief Manages a single sonicator channel via a defined pin structure.
 *
 * Usage:
 *   SonicatorPins p{1, SON1_START_PIN, SON1_RESET_PIN, SON1_OVERLOAD_PIN, SON1_FREQ_LOCK_PIN, SON1_FREQ_OUTPUT_PIN, ADC_SONICATOR_1_PIN};
 *   SonicatorInterface s1(p);
 *   s1.update(); // reads control from MODBUS, applies to HAL, writes status to MODBUS
 */
class SonicatorInterface {
public:
    explicit SonicatorInterface(const SonicatorPins& pins);
    ~SonicatorInterface();

    // Control API (logical)
    bool start(void);
    bool stop(void);
    bool setAmplitude(uint8_t amplitude_percent);
    bool resetOverload(void);
    bool emergencyStop(void);

    // Main periodic update (called by multiplexer)
    sonicator_state_t update(void);

    // Status / diagnostics
    const sonicator_status_t* getStatus(void) const;
    static const char* stateToString(sonicator_state_t state);
    bool isSafe(void) const;
    void resetStatistics(void);

    // Test hooks
    void setSimulationMode(bool enable);
    bool forceState(const sonicator_status_t& newState);
    bool injectFault(const sonicator_fault_t& faultMask);

private:
    // Configuration
    const SonicatorPins pins_;

    // Internal runtime state (implementation detail)
    struct RuntimeState {
        
        // Control/state machine
        sonicator_state_t   state{SONICATOR_STATE_UNKNOWN};
        sonicator_state_t   previous_state{SONICATOR_STATE_UNKNOWN};
        uint32_t            state_entry_time{0};

        // Control inputs and setpoints
        uint8_t             amplitude_percent{SONICATOR_MIN_AMPLITUDE_PERCENT};
        bool                start_requested{false};
        bool                stop_requested{false};
        bool                reset_requested{false};

        // Derived/monitoring
        bool                is_running{false};
        bool                overload_active{false};
        bool                frequency_locked{false};
        float               power_watts{0.0f};       // stores raw ADC as float for simplicity
        uint16_t            frequency_hz{0};

        // Faults and watches
        sonicator_fault_t   active_faults{(sonicator_fault_t)0};
        uint32_t            fault_count{0};
        uint32_t            last_fault_time{0};
        uint32_t            last_update_time{0};
        uint32_t            watchdog_last_reset{0};
        bool                safety_override{false};

        // History
        uint32_t            start_count{0};
        uint32_t            total_runtime_ms{0};
        uint32_t            last_start_time{0};
    };

    bool                 simulation_mode_{false};
    RuntimeState         state_{};
    mutable sonicator_status_t status_view_{}; // built on demand from runtime

    // Utilities
    uint32_t getTimestampMs() const;
    bool     isTimeout(uint32_t start_time, uint32_t timeout_ms) const;
    uint8_t  clampAmplitude(uint8_t amplitude) const;
    uint8_t  amplitudeToPwm(uint8_t amplitude_percent) const;

    // HAL adapters (safe wrappers)
    void     halGpioWriteSafe(uint8_t pin, bool state);
    bool     halGpioReadSafe(uint8_t pin);
    void     halPwmSetSafe(uint8_t pin, uint8_t duty_cycle);
    uint16_t halAdcReadSafe(adc_channel_t channel);

    // Hardware interaction
    void     updateHardwareOutputs();
    void     readHardwareInputs();

    // Fault management
    sonicator_fault_t checkFaultConditions();
    void              handleFaultConditions(sonicator_fault_t faults);

    // State machine
    void     processStateMachine();

    // Build public status view (maps RuntimeState -> sonicator_status_t)
    void     buildStatusView_() const;
};

#endif // SONICATOR_H