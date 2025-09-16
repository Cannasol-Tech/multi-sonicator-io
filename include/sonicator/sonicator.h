/**
 * @file sonicator/sonicator.h
 * @brief Sonicator IO Control and Monitoring API
 * @author Cannasol Technologies
 * @date 2025-09-16
 * @version 2.0.0
 *
 * @details
 * Provides a clean, object-oriented API for single sonicator control.
 * This version is refactored to be generic, accepting pin configurations
 * to allow management of multiple sonicator units by a multiplexer.
 */

#ifndef SONICATOR_H
#define SONICATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "modules/hal/hal.h"
#include "sonicator/types/state.h"
#include "sonicator/types/errors.h"
#include "sonicator/types/control.h"

// ============================================================================
// TIMING CONSTANTS
// ============================================================================

#define SONICATOR_STOP_DELAY_MS         100
#define SONICATOR_START_DELAY_MS        50
#define SONICATOR_RESET_PULSE_MS        20
#define SONICATOR_COMM_TIMEOUT_MS       2000
#define SONICATOR_FAULT_DEBOUNCE_MS     10
#define SONICATOR_WATCHDOG_TIMEOUT_MS   1000

// Amplitude limits
#define SONICATOR_MIN_AMPLITUDE_PERCENT 20
#define SONICATOR_MAX_AMPLITUDE_PERCENT 100

// ============================================================================
// SONICATOR CONTROLLER CLASS
// ============================================================================

CONFIGURE_SONICATOR_PINS(id, _p) \
    _p.start_pin = SON##id##_START_PIN; \
    _p.reset_pin = SON##id##_RESET_PIN; \
    _p.overload_pin = SON##id##_OVERLOAD_PIN; \
    _p.freq_lock_pin = SON##id##_FREQ_LOCK_PIN; \
    _p.freq_div10_pin = SON##id##_FREQ_DIV10_PIN; \
    _p.power_sense_channel = ADC_SONICATOR_##id##_PIN;


/**
 * @class SonicatorInterface
 * @brief Manages a single sonicator channel via a defined pin structure.
 * @details This class represents a group of pins that are used to control and monitor a 
 * single sonicator from the ATmega32A Multi Sonicator IO Controller
 * 
 * @note This class is the base Sonicator Interface and there should be four instances of it,
 *       one for each sonicator.  This pin definitions are defined in the hardware-config.yaml 
 *       and pin-matrix.md
 *
 * @def sonicator_1 pins = {
 *        SON1_START_PIN, SON1_RESET_PIN, SON1_OVERLOAD_PIN,
 *       SON1_FREQ_LOCK_PIN, SON1_FREQ_DIV10_PIN, ADC_SONICATOR_1_PIN
 * }
 * @def sonicator_2 
 * @def sonicator_3 
 * @def sonicator_4 
 */
class SonicatorInterface {
private:
    // Member variables
    uint8_t id_;
    bool simulation_mode_;
    sonicator_status_t state_;
    const sonicator_pins_s pins_;
public:
    /**
     * @brief Constructor for SonicatorInterface.
     * @param pins A struct containing the pin definitions for this sonicator.
     */
    SonicatorInterface(uint8_t id) : id(id_) {
        CONFIGURE_SONICATOR_PINS(id, pins_);
    };

    /**
     * @brief Destructor.
     */
    ~SonicatorInterface();

    bool start(void);
    bool stop(void);
    bool setAmplitude(uint8_t amplitude_percent);
    bool resetOverload(void);
    bool emergencyStop(void);
    sonicator_state_t update(void);
    const sonicator_status_t* getStatus(void) const;
    static const char* stateToString(sonicator_state_t state);
    bool isSafe(void) const;
    void resetStatistics(void);

    // Advanced functions for testing
    void setSimulationMode(bool enable);
    bool forceState(const sonicator_status_t& newState);
    bool injectFault(const sonicator_fault_t& faultMask);

private:
    // Testing Only
    bool simulation_mode_;

    // Register Space for this Sonicator
    sonicator_status_t status_regs_;
    sonicator_control_t control_regs_;

    // Hardware Pin Definitions
    const SonicatorPins pins_;

    // Private utility methods
    uint32_t getTimestampMs() const;
    bool isTimeout(uint32_t start_time, uint32_t timeout_ms) const;
    uint8_t clampAmplitude(uint8_t amplitude) const;
    uint8_t amplitudeToPwm(uint8_t amplitude_percent) const;

    // Private HAL interface methods
    void halGpioWriteSafe(uint8_t pin, bool state);
    bool halGpioReadSafe(uint8_t pin);
    void halPwmSetSafe(uint8_t pin, uint8_t duty_cycle);
    uint16_t halAdcReadSafe(adc_channel_t channel);

    // Private hardware interface methods
    void updateHardwareControl();
    void updateHardwareStatus();
    void handleHardwareFaults();

    // Private state machine method
    void processStateMachine();
};

#endif // SONICATOR_H

 