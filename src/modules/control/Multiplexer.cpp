/**
 * @file Multiplexer.cpp
 * @brief SonicMultiplexer implementation: master controller for 4 sonicators
 */

#include "multiplexer/Multiplexer.h"
#include "system_config.h"
#include "modules/hal/pwm.h"

// Local helpers
static inline uint8_t clamp_percent(uint8_t pct) {
    if (pct < SONICATOR_MIN_AMPLITUDE_PERCENT) return SONICATOR_MIN_AMPLITUDE_PERCENT;
    if (pct > SONICATOR_MAX_AMPLITUDE_PERCENT) return SONICATOR_MAX_AMPLITUDE_PERCENT;
    return pct;
}

static inline uint8_t amplitude_to_duty(uint8_t amplitude_percent) {
    if (amplitude_percent < SONICATOR_MIN_AMPLITUDE_PERCENT) {
        return 0;
    }
    return static_cast<uint8_t>(
        ((uint32_t)(amplitude_percent - SONICATOR_MIN_AMPLITUDE_PERCENT) * 255u) /
        (uint32_t)(SONICATOR_MAX_AMPLITUDE_PERCENT - SONICATOR_MIN_AMPLITUDE_PERCENT)
    );
}

void SonicMultiplexer::begin() {
    // No-op: HAL/PWM are initialized by hal_init(). Ensure shared amplitude reflects current value.
    updateSharedAmplitude_();
}

void SonicMultiplexer::update() {
    // Update each sonicator (includes its own MODBUS sync once integrated at unit level)
    for (uint8_t i = 0; i < sonicator_count_m && i < MAX_SONICATORS; ++i) {
        if (sonicators_m[i]) {
            (void)sonicators_m[i]->update();
        }
    }

    // Keep shared amplitude PWM in sync
    updateSharedAmplitude_();
}

/**
 * @brief Sets the shared amplitude for all sonicators
 */
bool SonicMultiplexer::setAmplitude(uint8_t amplitude_percent) {
    amplitude_ctrl_duty_m = clamp_percent(amplitude_percent);

    // Push to each unit so it knows the setpoint (even if stopped)
    for (uint8_t i = 0; i < sonicator_count_m && i < MAX_SONICATORS; ++i) {
        if (sonicators_m[i]) {
            (void)sonicators_m[i]->setAmplitude(amplitude_ctrl_duty_m);
        }
    }

    // Update shared PWM
    updateSharedAmplitude_();
    return true;
}

const sonicator_status_t* SonicMultiplexer::getStatus(uint8_t index) const {
    if (index >= sonicator_count_m || !sonicators_m[index]) return nullptr;
    return sonicators_m[index]->getStatus();
}

void SonicMultiplexer::initSonicators_() {
    // Instantiate per-unit SonicatorInterface with pins derived from system_config.h
    // ID 1
    {
        SonicatorPins pins{
            .sonicator_id        = 1,
            .start_pin           = SON1_START_PIN,
            .reset_pin           = SON1_RESET_PIN,
            .overload_pin        = SON1_OVERLOAD_PIN,
            .freq_lock_pin       = SON1_FREQ_LOCK_PIN,
            .freq_div10_pin      = SON1_FREQ_OUTPUT_PIN,
            .power_sense_channel = ADC_CHANNEL_4
        };
        sonicators_m[0] = new SonicatorInterface(pins);
    }

    // ID 2
    if (sonicator_count_m > 1) {
        SonicatorPins pins{
            .sonicator_id        = 2,
            .start_pin           = SON2_START_PIN,
            .reset_pin           = SON2_RESET_PIN,
            .overload_pin        = SON2_OVERLOAD_PIN,
            .freq_lock_pin       = SON2_FREQ_LOCK_PIN,
            .freq_div10_pin      = SON2_FREQ_OUTPUT_PIN,
            .power_sense_channel = ADC_CHANNEL_5
        };
        sonicators_m[1] = new SonicatorInterface(pins);
    }

    // ID 3
    if (sonicator_count_m > 2) {
        SonicatorPins pins{
            .sonicator_id        = 3,
            .start_pin           = SON3_START_PIN,
            .reset_pin           = SON3_RESET_PIN,
            .overload_pin        = SON3_OVERLOAD_PIN,
            .freq_lock_pin       = SON3_FREQ_LOCK_PIN,
            .freq_div10_pin      = SON3_FREQ_OUTPUT_PIN,
            .power_sense_channel = ADC_CHANNEL_6
        };
        sonicators_m[2] = new SonicatorInterface(pins);
    }

    // ID 4
    if (sonicator_count_m > 3) {
        SonicatorPins pins{
            .sonicator_id        = 4,
            .start_pin           = SON4_START_PIN,
            .reset_pin           = SON4_RESET_PIN,
            .overload_pin        = SON4_OVERLOAD_PIN,
            .freq_lock_pin       = SON4_FREQ_LOCK_PIN,
            .freq_div10_pin      = SON4_FREQ_OUTPUT_PIN,
            .power_sense_channel = ADC_CHANNEL_7
        };
        sonicators_m[3] = new SonicatorInterface(pins);
    }

    // Initialize default amplitude at construction time
    amplitude_ctrl_duty_m = SONICATOR_MIN_AMPLITUDE_PERCENT;
    for (uint8_t i = 0; i < sonicator_count_m && i < MAX_SONICATORS; ++i) {
        if (sonicators_m[i]) {
            (void)sonicators_m[i]->setAmplitude(amplitude_ctrl_duty_m);
        }
    }
    updateSharedAmplitude_();
}

void SonicMultiplexer::updateSharedAmplitude_() {
    // Shared amplitude PWM: drive HAL channel for amplitude output.
    const uint8_t duty = amplitude_to_duty(amplitude_ctrl_duty_m);
    (void)pwm_set_duty_cycle(PWM_CHANNEL_AMPLITUDE, duty);
}
