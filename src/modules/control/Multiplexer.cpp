/**
 * @file Multiplexer.cpp
 * @brief Implementation of the Multiplexer class
 * @author James (AI Developer)
 * @date 2025-09-16
 * @version 1.0.0
 */

#include "multiplexer/Multiplexer.h"
#include "system_config.h"
#include "modules/hal/pwm.h"
/**
 * @brief Pin configuration for each sonicator
 * @details 
 * @param sonicator_1 
 *      Sonicator 1 - Verified by Product Owner
 *       #	DUT Pin (Port/Pad)	Header Ref	Signal	Dir	Scale/Notes	Wrapper Pin	Test Point
 *       1	PB3 (Pin 4)	DB9-1 Pin 4	FREQ_DIV10_1	IN	÷10 frequency (S1)	N/A	TBD
        2	PB7 (Pin 8)	DB9-1 Pin 3	FREQ_LOCK_1	IN	FLCK_1 via opto	N/A	TBD
        3	PD6 (Pin 20)	DB9-1 Pin 1	OVERLOAD_1	IN	OL_1 via opto	N/A	TBD
        4	PC6 (Pin 28)	DB9-1 Pin 7	START_1	OUT	ULN2003A open-collector	N/A	TBD
        5	PC7 (Pin 29)	DB9-1 Pin 2	RESET_1	OUT	ULN2003A open-collector	N/A	TBD
        6	PA4/A4 (Pin 36)	DB9-1 Pin 5	POWER_SENSE_1	ANALOG	5.44 mV/W scaling	N/A	TBD
        7	PD7 (Pin 21)	DB9-1 Pin 8	AMPLITUDE_ALL	OUT	Shared AMP_C (0–10V), common	N/A	TBD


 * @param sonicator_2

Sonicator 2 - Verified by Product Owner
#	DUT Pin (Port/Pad)	Header Ref	Signal	Dir	Scale/Notes	Wrapper Pin	Test Point
1	PB2 (Pin 3)	DB9-2 Pin 4	FREQ_DIV10_2	IN	÷10 frequency (S2)	N/A (Uno R4 single-channel)	N/A
2	PB6 (Pin 7)	DB9-2 Pin 3	FREQ_LOCK_2	IN	FLCK_2 via opto	N/A	N/A
3	PD5 (Pin 19)	DB9-2 Pin 1	OVERLOAD_2	IN	OL_2 via opto	N/A	N/A
4	PC4 (Pin 26)	DB9-2 Pin 7	START_2	OUT	ULN2003A open-collector	N/A	N/A
5	PC5 (Pin 27)	DB9-2 Pin 2	RESET_2	OUT	ULN2003A open-collector	N/A	N/A
6	PA5/A5 (Pin 35)	DB9-2 Pin 5	POWER_SENSE_2	ANALOG	5.44 mV/W scaling	N/A	N/A
7	PD7 (Pin 21)	DB9-2 Pin 8	AMPLITUDE_ALL	OUT	Shared AMP_C (0–10V), common	N/A	N/A
 * @param sonicator_3
 Sonicator 3 - Verified by Product Owner
#	DUT Pin (Port/Pad)	Header Ref	Signal	Dir	Scale/Notes	Wrapper Pin	Test Point
1	PB1 (Pin 2)	DB9-3 Pin 4	FREQ_DIV10_3	IN	÷10 frequency (S3)	N/A	N/A
2	PB5 (Pin 6)	DB9-3 Pin 3	FREQ_LOCK_3	IN	FLCK_3 via opto	N/A	N/A
3	PD4 (Pin 18)	DB9-3 Pin 1	OVERLOAD_3	IN	OL_3 via opto	N/A	N/A
4	PC2 (Pin 24)	DB9-3 Pin 7	START_3	OUT	ULN2003A open-collector	N/A	N/A
5	PC3 (Pin 25)	DB9-3 Pin 2	RESET_3	OUT	ULN2003A open-collector	N/A	N/A
6	PA6/A6 (Pin 34)	DB9-3 Pin 5	POWER_SENSE_3	ANALOG	5.44 mV/W scaling	N/A	N/A
 * @param sonicator_4

Sonicator 4 - Verified by Product Owner
#	DUT Pin (Port/Pad)	Header Ref	Signal	Dir	Scale/Notes	Wrapper Pin	Test Point
1	PB0 (Pin 1)	DB9-4 Pin 4	FREQ_DIV10_4	IN	÷10 frequency (S4)	D7	TBD
2	PB4 (Pin 5)	DB9-4 Pin 3	FREQ_LOCK_4	IN	FLCK_4 via opto	D8	TBD
3	PD3 (Pin 17)	DB9-4 Pin 1	OVERLOAD_4	IN	OL_4 via opto	A2	TBD
4	PC0 (Pin 22)	DB9-4 Pin 7	START_4	OUT	ULN2003A open-collector	A3	TBD
5	PC1 (Pin 23)	DB9-4 Pin 2	RESET_4	OUT	ULN2003A open-collector	A4	TBD
6	PA7/A7 (Pin 33)	DB9-4 Pin 5	POWER_SENSE_4	ANALOG	5.44 mV/W scaling	A1	TBD
7	PD7 (Pin 21)	DB9-4 Pin 8	AMPLITUDE_ALL	OUT	Shared AMP_C (0–10V), common	D9 (PWM)	TBD
*/


SonicMultiplexer::Multiplexer() : shared_amplitude_percent_(SONICATOR_MIN_AMPLITUDE_PERCENT) {
    // Pin configurations for each sonicator, from system_config.h
    static soncicator
    };

    // Allocate and initialize each sonicator
    for (int i = 0; i < NUM_SONICATORS; ++i) {
        sonicators[i] = new SonicatorInterface(pins[i]);
    }
    updateSharedAmplitude();
}

Multiplexer::~Multiplexer() {
    for (int i = 0; i < NUM_SONICATORS; ++i) {
        delete sonicators[i];
    }
    updateSharedAmplitude();
}

void Multiplexer::begin() {
    // Initialization is handled in the constructor
    for (int i = 0; i < NUM_SONICATORS; ++i) {
        sonicators[i] = new SonicatorInterface(pins[i]);
    }
    updateSharedAmplitude();
}

void Multiplexer::update() {
    for (int i = 0; i < NUM_SONICATORS; ++i) {
        if (sonicators[i]) {
            sonicators[i]->update();
        }
    }
}

bool Multiplexer::start(uint8_t index) {
    if (index < NUM_SONICATORS && sonicators[index]) {
        return sonicators[index]->start();
    }
    return false;
}

bool Multiplexer::stop(uint8_t index) {
    if (index < NUM_SONICATORS && sonicators[index]) {
        return sonicators[index]->stop();
    }
    return false;
}

bool Multiplexer::setAmplitude(uint8_t amplitude_percent) {
    if (amplitude_percent >= SONICATOR_MIN_AMPLITUDE_PERCENT && amplitude_percent <= SONICATOR_MAX_AMPLITUDE_PERCENT) {
        shared_amplitude_percent_ = amplitude_percent;
        // Apply the amplitude to all sonicator state machines
        for (int i = 0; i < NUM_SONICATORS; ++i) {
            if (sonicators[i]) {
                sonicators[i]->setAmplitude(shared_amplitude_percent_);
            }
        }
        return true;
    }
    return false;
}

void Multiplexer::update() {
    for (int i = 0; i < NUM_SONICATORS; ++i) {
        if (sonicators[i]) {
            sonicators[i]->update();
        }
    }
}

bool Multiplexer::start(uint8_t index) {
    if (index < NUM_SONICATORS && sonicators[index]) {
        return sonicators[index]->start();
    }
    return false;
}

bool Multiplexer::stop(uint8_t index) {
    if (index < NUM_SONICATORS && sonicators[index]) {
        return sonicators[index]->stop();
    }
    return false;
}

bool Multiplexer::setAmplitude(uint8_t amplitude_percent) {
    if (amplitude_percent >= SONICATOR_MIN_AMPLITUDE_PERCENT && amplitude_percent <= SONICATOR_MAX_AMPLITUDE_PERCENT) {
        shared_amplitude_percent_ = amplitude_percent;
        // Apply the amplitude to all sonicator state machines
        for (int i = 0; i < NUM_SONICATORS; ++i) {
            if (sonicators[i]) {
                sonicators[i]->setAmplitude(shared_amplitude_percent_);
            }
        }

        // Directly update the PWM HAL using the proper amplitude control function
        if (pwm_set_amplitude(shared_amplitude_percent_) != PWM_OK) {
            // PWM amplitude setting failed, but continue with state machine updates
            // Log error if logging is available
            return false;
        }

        return true;
    }
    return false;
}

bool Multiplexer::resetOverload(uint8_t index) {
    if (index < NUM_SONICATORS && sonicators[index]) {
        return sonicators[index]->resetOverload();
    }
    return false;
}

const sonicator_status_t* Multiplexer::getStatus(uint8_t index) const {
    if (index < NUM_SONICATORS && sonicators[index]) {
        return sonicators[index]->getStatus();
    }
    return nullptr;
}

const sonicator_status_t* Multiplexer::getStatus(uint8_t index) const {
    if (index < NUM_SONICATORS && sonicators[index]) {
        return sonicators[index]->getStatus();
    }
    return nullptr;
}

void Multiplexer::updateSharedAmplitude() {
    // The amplitude PWM is shared. We set it based on the shared value,
    // but only if at least one sonicator is in the RUNNING state.
    // The individual sonicator's updateHardwareOutputs will handle the logic
    // of whether to use this PWM value.
    bool any_running = false;
    for (int i = 0; i < NUM_SONICATORS; ++i) {
        const sonicator_status_t* status = getStatus(i);
        if (status && status->state == SONICATOR_STATE_RUNNING) {
            any_running = true;
            break;
        }
    }

    if (any_running) {
        // Use the proper PWM amplitude control function
        pwm_set_amplitude(shared_amplitude_percent_);
    } else {
        // No sonicators running, set to minimum safe amplitude
        pwm_set_amplitude(PWM_AMPLITUDE_MIN);
    }
}
