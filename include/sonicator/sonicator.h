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

#include "modbus_registers.h"

/** @brief Delay after stop command before transitioning to idle state (ms) */
#define SONICATOR_STOP_DELAY_MS         100

/** @brief Delay after start command before transitioning to running state (ms) */
#define SONICATOR_START_DELAY_MS        50

/** @brief Duration of reset pulse sent to CT2000 (ms) */
#define SONICATOR_RESET_PULSE_MS        20

/** @brief Communication timeout threshold (ms) */
#define SONICATOR_COMM_TIMEOUT_MS       2000

/** @brief Fault condition debounce time (ms) */
#define SONICATOR_FAULT_DEBOUNCE_MS     10

/** @brief Watchdog timeout threshold (ms) */
#define SONICATOR_WATCHDOG_TIMEOUT_MS   1000

/** @brief Minimum allowed amplitude percentage */
#define SONICATOR_MIN_AMPLITUDE_PERCENT 20

/** @brief Maximum allowed amplitude percentage */
#define SONICATOR_MAX_AMPLITUDE_PERCENT 100

/**
 * @brief Pin bundle describing a single sonicator channel
 *
 * This structure contains all the hardware pin assignments needed to control
 * and monitor a single CT2000 sonicator channel.
 */
typedef struct {
    // Pin assignments
    uint8_t       start_pin;           /**< Digital output pin to start sonics (ULN2003A -> CT2000) */
    uint8_t       reset_pin;           /**< Digital output pin to reset overload (ULN2003A -> CT2000) */
    uint8_t       overload_pin;        /**< Digital input (6N137 -> CT2000 overload) */
    uint8_t       freq_lock_pin;       /**< Digital input (6N137 -> CT2000 frequency lock) */
    uint8_t       freq_div10_pin;      /**< Digital input for frequency ÷10 (pin change ISR channel select) */
    adc_channel_t power_sense_channel; /**< ADC channel for 0-10V power monitor (raw ADC stored) */
} SonicatorPins;

/**
 * @class SonicatorInterface
 * @brief Manages a single sonicator channel via a defined pin structure.
 *
 * This class provides a complete interface for controlling and monitoring a single
 * CT2000 sonicator channel. It handles state machine logic, fault detection,
 * hardware I/O, and MODBUS communication.
 *
 * @par Usage Example:
 * @code
 * SonicatorPins p{1, SON1_START_PIN, SON1_RESET_PIN, SON1_OVERLOAD_PIN,
 *                 SON1_FREQ_LOCK_PIN, SON1_FREQ_OUTPUT_PIN, ADC_SONICATOR_1_PIN};
 * SonicatorInterface s1(p);
 * s1.update(); // reads control from MODBUS, applies to HAL, writes status to MODBUS
 * @endcode
 */
class SonicatorInterface {
public:
    /**
     * @brief Construct a new SonicatorInterface object
     * @param pins Pin configuration for this sonicator channel
     */
    explicit SonicatorInterface(uint8_t sonicator_id, const SonicatorPins& pins, const sonicator_registers_t* registers);

    /**
     * @brief Destroy the SonicatorInterface object
     */
    ~SonicatorInterface();

    // Control API (logical)

    /**
     * @brief Start the sonicator
     * @note this sets the gpio value of pins_.start_pin to true
     */
    void start(void);

    /**
     * @brief Stop the sonicator
     * @note this sets the gpio value of pins_.start_pin to false
     */
     void stop(void);

    /**
     * @brief Reset overload condition
     * @note this sets the gpio value of pins_.reset_pin to true for 20ms
     */
    void resetOverload(void);

    /**
     * @brief Emergency stop - immediately halt operation
     * @note this sets the gpio value of pins_.start_pin to false
     */
    void emergencyStop(void);


    /**
      * @brief Apply a start/stop command from control register
      *
      * Reads the persistent state from the MODBUS control register:
      * - 1 = sonicator should be running (state-based)
      * - 0 = sonicator should be stopped (state-based)
      * - other values = no change
      *
      * This helper routes the command through the SonicatorInterface safety layer
      * (start()/stop()) so simulation, logging, and policy are consistently applied.
      * The register value persists and directly controls the operating state
      * @note this uses the current state_.start_stop value to determine if a change is needed
      */
    void setStartStop();

    /**
     * @brief Apply an overload reset command from control register
     *
     * Reads the overload reset command from the MODBUS control register and
     * consumes it (write-one-to-consume pattern). If a reset was requested,
     * triggers the resetOverload() function to pulse the reset pin.
     */
    void setResetOverload();

    // Main periodic update (called by multiplexer)

    /**
     * @brief Main periodic update function
     *
     * This function should be called regularly to:
     * - Read MODBUS control commands
     * - Update state machine
     * - Process hardware I/O
     * - Detect and handle faults
     * - Publish status to MODBUS
     *
     * @return Current sonicator state
     */
    sonicator_state_t update(void);

    // Status / diagnostics

    /**
     * @brief Get current status
     * @return Pointer to current status structure
     */
    const sonicator_status_t* getStatus(void) const;

    /**
     * @brief Convert state enum to human-readable string
     * @param state State to convert
     * @return String representation of the state
     */
    static const char* stateToString(sonicator_state_t state);

    /**
     * @brief Check if sonicator is in a safe operating condition
     * @return true if safe, false if faults are present
     */
    bool isSafe(void) const;

    /**
     * @brief Reset runtime statistics counters
     */
    void resetStatistics(void);

    // Test hooks

    /**
     * @brief Enable or disable simulation mode for testing
     * @param enable true to enable simulation mode, false for normal operation
     */
    void setSimulationMode(bool enable);

    /**
     * @brief Force sonicator to a specific state (for testing)
     * @param newState Desired state to force
     * @return true if state was forced successfully, false otherwise
     */
    bool forceState(const sonicator_status_t& newState);

    /**
     * @brief Inject a fault condition (for testing)
     * @param faultMask Fault condition(s) to inject
     * @return true if fault was injected successfully, false otherwise
     */
    bool injectFault(const sonicator_fault_t& faultMask);

private:
    /** @brief Pin configuration for this sonicator channel */
    uint8_t sonicator_id_;
    const SonicatorPins pins_;
    const sonicator_registers_t* registers_m;

    /**
     * @brief Internal runtime state structure
     *
     * This structure contains all the internal state variables needed
     * for sonicator operation, including control state, monitoring data,
     * fault tracking, and statistics
     */
    struct RuntimeState {

        // Control/state machine
        sonicator_state_t   state{SONICATOR_STATE_UNKNOWN};          /**< Current state machine state */
        sonicator_state_t   previous_state{SONICATOR_STATE_UNKNOWN}; /**< Previous state for transition tracking */
        uint32_t            state_entry_time{0};                     /**< Timestamp when current state was entered */

        // Control inputs and setpoints
        uint8_t             amplitude_percent{SONICATOR_MIN_AMPLITUDE_PERCENT}; /**< Current amplitude setpoint (%) */
        bool                start_stop{0};                           /**< Start/stop command (1=start, 0=stop) */
        bool                reset_requested{false};                  /**< Reset command pending */

        // Derived/monitoring
        bool                is_running{false};                       /**< True when sonicator is actively running */
        bool                overload_active{false};                  /**< True when overload condition detected */
        bool                frequency_locked{false};                 /**< True when frequency is locked */
        float               power_watts{0.0f};                       /**< Power reading (stores raw ADC as float) */
        uint16_t            frequency_hz{0};                         /**< Measured frequency in Hz */

        // Faults and watches
        sonicator_fault_t   active_faults{(sonicator_fault_t)0};     /**< Currently active fault conditions */
        uint32_t            fault_count{0};                          /**< Total number of faults encountered */
        uint32_t            last_fault_time{0};                      /**< Timestamp of last fault occurrence */
        uint32_t            last_update_time{0};                     /**< Timestamp of last update call */
        uint32_t            watchdog_last_reset{0};                  /**< Timestamp of last watchdog reset */
        bool                safety_override{false};                  /**< Safety override flag */

        // History
        uint32_t            start_count{0};                          /**< Total number of start operations */
        uint32_t            total_runtime_ms{0};                     /**< Cumulative runtime in milliseconds */
        uint32_t            last_start_time{0};                      /**< Timestamp of last start operation */
    };

    bool                 simulation_mode_{false};    /**< True when in simulation mode */
    RuntimeState         state_{};                   /**< Internal runtime state */
    mutable sonicator_status_t status_view_{};       /**< Public status view built on demand */

    // Utilities

    /**
     * @brief Get current timestamp in milliseconds
     * @return Current timestamp
     */
    uint32_t getTimestampMs() const;

    /**
     * @brief Check if a timeout has occurred
     * @param start_time Starting timestamp
     * @param timeout_ms Timeout duration in milliseconds
     * @return true if timeout has occurred, false otherwise
     */
    bool isTimeout(uint32_t start_time, uint32_t timeout_ms) const;

    /**
     * @brief Convert amplitude percentage to PWM duty cycle
     * @param amplitude_percent Amplitude percentage (20-100%)
     * @return PWM duty cycle value (0-255)
     */
    uint8_t amplitudeToPwm(uint8_t amplitude_percent) const;

    // HAL adapters (safe wrappers)

    /**
     * @brief Safe GPIO write operation
     * @param pin GPIO pin number
     * @param state Desired pin state
     */
    void halGpioWriteSafe(uint8_t pin, bool state);

    /**
     * @brief Safe GPIO read operation
     * @param pin GPIO pin number
     * @return Current pin state
     */
    bool halGpioReadSafe(uint8_t pin);

    /**
     * @brief Safe PWM set operation
     * @param duty_cycle PWM duty cycle (0-255)
     */
    void halPwmSetSafe(uint8_t duty_cycle);

    /**
     * @brief Safe ADC read operation
     * @param channel ADC channel to read
     * @return ADC reading value
     */
    uint16_t halAdcReadSafe(adc_channel_t channel);

    // Hardware interaction

    /**
     * @brief Update all hardware outputs based on current state
     */
    void updateHardwareOutputs();

    /**
     * @brief Read all hardware inputs and update state
     */
    void readHardwareInputs();

    // Fault management

    /**
     * @brief Check for fault conditions
     * @return Bitmask of detected fault conditions
     */
    sonicator_fault_t checkFaultConditions();

    /**
     * @brief Handle detected fault conditions
     * @param faults Bitmask of fault conditions to handle
     */
    void    handleFaultConditions(sonicator_fault_t faults);

    // State machine

    /**
     * @brief Process state machine transitions
     */
    void    processStateMachine();

    /**
     * @brief Build public status view from internal runtime state
     */
    void    buildStatusView_() const;
    
    /**
     * @brief Get the fault count
     * @return The total number of faults encountered
     */
    uint32_t getFaultCount() const;
};

#endif // SONICATOR_H