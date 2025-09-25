/**
 * @file Multiplexer.cpp
 * @brief SonicMultiplexer implementation: master controller for 4 sonicators
 */

#include "multiplexer/Multiplexer.h"
#include "system_config.h"
#include "modules/hal/pwm.h"
#include "modules/hal/gpio.h"
#include "modbus_register_manager.h"


// Define static members
uint8_t SonicMultiplexer::sonicator_count_m = 0;
uint8_t SonicMultiplexer::amplitude_ctrl_duty_m = SONICATOR_MIN_AMPLITUDE_PERCENT;

/**
 * @brief Clamps a percentage value to valid sonicator amplitude range
 * @param pct Percentage value to clamp
 * @return Clamped percentage within [SONICATOR_MIN_AMPLITUDE_PERCENT, SONICATOR_MAX_AMPLITUDE_PERCENT]
 */
static inline uint8_t clamp_percent(uint8_t pct) {
    if (pct < SONICATOR_MIN_AMPLITUDE_PERCENT) return SONICATOR_MIN_AMPLITUDE_PERCENT;
    if (pct > SONICATOR_MAX_AMPLITUDE_PERCENT) return SONICATOR_MAX_AMPLITUDE_PERCENT;
    return pct;
}

/**
 * @brief Converts amplitude percentage to PWM duty cycle value
 * @param amplitude_percent Amplitude percentage (20-100%)
 * @return PWM duty cycle value (0-255), or 0 if amplitude is below minimum
 */
static inline uint8_t amplitude_to_duty(uint8_t amplitude_percent) {
    if (amplitude_percent < SONICATOR_MIN_AMPLITUDE_PERCENT) {
        return 0;
    }
    return static_cast<uint8_t>(
        ((uint32_t)(amplitude_percent - SONICATOR_MIN_AMPLITUDE_PERCENT) * 255u) /
        (uint32_t)(SONICATOR_MAX_AMPLITUDE_PERCENT - SONICATOR_MIN_AMPLITUDE_PERCENT)
    );
}


/**
 * @brief Checks if any sonicator is running
 */
 bool SonicMultiplexer::anySonicatorRunning_() {
    for (uint8_t i = 0; i < sonicator_count_m && i < MAX_SONICATORS; ++i) {
        if (sonicators_m[i]) {
            const sonicator_status_t* st = sonicators_m[i]->getStatus();
            if (st && st->is_running) {
                // Any one running is enough
                    return true;
            }
        }
    }
    return false;
}

/**
 * @brief Checks if any sonicator is in a fault state
 * @return true if any sonicator is in a fault state, false otherwise
 * @details Checks the status of each sonicator and returns true if any are in a fault state
 * @note This method is not thread-safe and should only be called from the main loop context
 * @warning Do not call from interrupt context
 * @see sonicator_status_t for status flags
*/
bool SonicMultiplexer::anySonicatorFaulted_() {
    for (uint8_t i = 0; i < sonicator_count_m && i < MAX_SONICATORS; ++i) {
        if (sonicators_m[i]) {
            const sonicator_status_t* st = sonicators_m[i]->getStatus();
            if (st && st->fault_count > 0) {
                // Any one faulted is enough
                return true;
            }
        }
    }
    return false;
}


/**
 * @brief Updates the global LED status based on sonicator states
 * @details Sets the status LED to ON if any sonicator is running, OFF otherwise
 */
void SonicMultiplexer::update_led_state() noexcept {
    // Check if any sonicator is running
    bool any_running = anySonicatorRunning_();
    (void)gpio_status_led(any_running ? GPIO_HIGH : GPIO_LOW);
}

/**
 * @brief Initializes the multiplexer system
 * @details No-op implementation since HAL/PWM are initialized by hal_init().
 *          Ensures shared amplitude reflects current value.
 */
void SonicMultiplexer::begin() {
    // No-op: HAL/PWM are initialized by hal_init(). Ensure shared amplitude reflects current value.
    register_map_m = register_manager_get_map();
    updateSharedAmplitude_();
    update_led_state();
}

/**
 * @brief Sets the shared amplitude for all sonicators
 * @param amplitude_percent Desired amplitude percentage (will be clamped to valid range)
 * @return true on success
 * @details Updates all individual sonicator amplitude setpoints and the shared PWM output
 */
bool SonicMultiplexer::setAmplitude(uint8_t amplitude_percent) {
    amplitude_ctrl_duty_m = clamp_percent(register_map_m->global_control.global_amplitude_sp);
    updateSharedAmplitude_();
    return true;
}

/**
 * @brief Gets the status of a specific sonicator
 * @param index Zero-based index of the sonicator (0-3)
 * @return Pointer to sonicator status structure, or nullptr if invalid index
 */
const sonicator_status_t* SonicMultiplexer::getStatus(uint8_t index) const {
    if (index >= sonicator_count_m || !sonicators_m[index]) return nullptr;
    return sonicators_m[index]->getStatus();
}

/**
 * @brief Initializes all sonicator interfaces with their hardware pin configurations
 * @details Creates SonicatorInterface instances for each configured sonicator unit,
 *          sets up pin mappings from system_config.h, and initializes default amplitude
 */
void SonicMultiplexer::initSonicators_() {
    uint8_t sonicator_id = 0;

    SonicatorPins pins{
        .start_pin           = SON1_START_PIN,
        .reset_pin           = SON1_RESET_PIN,
        .overload_pin        = SON1_OVERLOAD_PIN,
        .freq_lock_pin       = SON1_FREQ_LOCK_PIN,
        .freq_div10_pin      = SON1_FREQ_OUTPUT_PIN,
        .power_sense_channel = ADC_CHANNEL_4
    };
    sonicators_m[0] = new SonicatorInterface(++sonicator_id, pins, &register_map_m->sonicators[0]);

    // ID 2
    if (sonicator_count_m > 1) {
        SonicatorPins pins_2{
            .start_pin           = SON2_START_PIN,
            .reset_pin           = SON2_RESET_PIN,
            .overload_pin        = SON2_OVERLOAD_PIN,
            .freq_lock_pin       = SON2_FREQ_LOCK_PIN,
            .freq_div10_pin      = SON2_FREQ_OUTPUT_PIN,
            .power_sense_channel = ADC_CHANNEL_5
        };
        sonicators_m[1] = new SonicatorInterface(++sonicator_id, pins_2, &register_map_m->sonicators[1]);
    }

    // ID 3
    if (sonicator_count_m > 2) {
        SonicatorPins pins_3{
            .start_pin           = SON3_START_PIN,
            .reset_pin           = SON3_RESET_PIN,
            .overload_pin        = SON3_OVERLOAD_PIN,
            .freq_lock_pin       = SON3_FREQ_LOCK_PIN,
            .freq_div10_pin      = SON3_FREQ_OUTPUT_PIN,
            .power_sense_channel = ADC_CHANNEL_6
        };
        sonicators_m[2] = new SonicatorInterface(++sonicator_id, pins_3, &register_map_m->sonicators[2]);
    }

    // ID 4
    if (sonicator_count_m > 3) {
        SonicatorPins pins_4{
            .start_pin           = SON4_START_PIN,
            .reset_pin           = SON4_RESET_PIN,
            .overload_pin        = SON4_OVERLOAD_PIN,
            .freq_lock_pin       = SON4_FREQ_LOCK_PIN,
            .freq_div10_pin      = SON4_FREQ_OUTPUT_PIN,
            .power_sense_channel = ADC_CHANNEL_7
        };
        sonicators_m[3] = new SonicatorInterface(++sonicator_id, pins_4, &register_map_m->sonicators[3]);
    }

    // Initialize default amplitude at construction time
    updateSharedAmplitude_();
}

/**
 * @brief Reads the MODBUS register for Global Amplitude Setpoint Control
 */
uint8_t SonicMultiplexer::readGlobalAmplitudeSP_() const {
    if (register_map_m) {
        amplitude_ctrl_duty_m = register_map_m->global_control.global_amplitude_sp;
    }
    return amplitude_ctrl_duty_m;
}


/**
 * @brief Updates the shared amplitude PWM output
 * @details Reads the current amplitude control value from the MODBUS control register,
 *          converts it to a PWM duty cycle and drives the hardware PWM channel for amplitude control
 */
void SonicMultiplexer::updateSharedAmplitude_() {
    // Shared amplitude PWM: drive HAL channel for amplitude output.
    const uint8_t duty = amplitude_to_duty(register_map_m->global_control.global_amplitude_sp);
    (void)pwm_set_duty_cycle(PWM_CHANNEL_AMPLITUDE, duty);
}

/**
* @brief Updates all sonicators and shared amplitude control
* @details Calls update() on each active sonicator and synchronizes the shared amplitude PWM
*/
void SonicMultiplexer::update() {
   // Update each sonicator (includes its own MODBUS sync once integrated at unit level)
   for (uint8_t i = 0; i < sonicator_count_m && i < MAX_SONICATORS; ++i) {
       if (sonicators_m[i]) {
           (void)sonicators_m[i]->update();
       }
   }

   // Keep shared amplitude PWM in sync
    updateSharedAmplitude_();

   // Keep Global LED Status Accurate
    update_led_state();
}

