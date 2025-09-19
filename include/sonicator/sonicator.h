/**
 * @file sonicator.h
 * @brief Sonicator IO Control and Monitoring API
 * @author Cannasol Technologies
 * @date 2025-09-18
 * @version 2.1.0
 *
 * @details
 * Public interface for a single sonicator channel. The implementation lives in
 * src/modules/control/sonicator.cpp. This header is the canonical declaration
 * consumed by the multiplexer and MODBUS bridge layers.
 */

#ifndef SONICATOR_SONICATOR_H
#define SONICATOR_SONICATOR_H

#include <stdint.h>
#include <stdbool.h>

#include "sonicator/types/state.h"
#include "sonicator/types/errors.h"
#include "sonicator/types/pins.h"
#include "modules/hal/adc.h"

#ifndef SONICATOR_MIN_AMPLITUDE_PERCENT
#define SONICATOR_MIN_AMPLITUDE_PERCENT 20
#endif
#ifndef SONICATOR_MAX_AMPLITUDE_PERCENT
#define SONICATOR_MAX_AMPLITUDE_PERCENT 100
#endif

/**
 * @class SonicatorInterface
 * @brief Manages one CT2000 sonicator channel via DB9 interface pins.
 */
class SonicatorInterface {
public:
    /**
     * @brief Construct a SonicatorInterface with a given pin map
     * @param pins Hardware pin configuration for this channel
     */
    explicit SonicatorInterface(const SonicatorPins& pins);

    /**
     * @brief Destructor
     */
    ~SonicatorInterface();

    // Control
    bool start();
    bool stop();
    bool setAmplitude(uint8_t amplitude_percent);
    bool resetOverload();
    bool emergencyStop();

    // Runtime
    sonicator_state_t update();
    const sonicator_status_t* getStatus() const;

    // Utilities
    static const char* stateToString(sonicator_state_t state);
    bool isSafe() const;
    void resetStatistics();
    void setSimulationMode(bool enable);

    // Test hooks
    bool forceState(const sonicator_status_t& newState);
    bool injectFault(const sonicator_fault_t& faultMask);

private:
    // State and configuration
    sonicator_status_t state_;
    const SonicatorPins pins_;
    bool simulation_mode_;

    // Timing helpers
    uint32_t getTimestampMs() const;
    bool isTimeout(uint32_t start_time, uint32_t timeout_ms) const;

    // Value helpers
    uint8_t clampAmplitude(uint8_t amplitude) const;
    uint8_t amplitudeToPwm(uint8_t amplitude_percent) const;

    // Safe HAL accessors
    void halGpioWriteSafe(uint8_t pin, bool state);
    bool halGpioReadSafe(uint8_t pin);
    void halPwmSetSafe(uint8_t pin, uint8_t duty_cycle);
    uint16_t halAdcReadSafe(adc_channel_t channel);

    // Internal behaviors
    void updateHardwareOutputs();
    void readHardwareInputs();
    sonicator_fault_t checkFaultConditions();
    void handleFaultConditions(sonicator_fault_t faults);
    void processStateMachine();
};

#endif // SONICATOR_SONICATOR_H