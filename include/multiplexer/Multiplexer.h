/**
 * @file Multiplexer.h
 * @brief Manages multiple SonicatorInterface units
 * @author James (AI Developer)
 * @date 2025-09-16
 * @version 1.0.0
 *
 * @details
 * This class encapsulates the control and monitoring of four CT2000 sonicator
 * units. It follows an object-oriented approach, providing a clean interface
 * to manage the entire system. It is designed to replace the previous
 * procedural multi-sonicator logic.
 */

#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "sonicator/sonicator.h"
#include "system_config.h"

#define NUM_SONICATORS 4

class Multiplexer {
public:
    /**
     * @brief Get the singleton instance of the multiplexer.
     */
    static Multiplexer& getInstance() {
        static Multiplexer instance;
        return instance;
    }

    /**
     * @brief Initializes the multiplexer and all sonicator units.
     *
     * This method sets up the pin configurations for each of the four
     * sonicators and initializes them. It must be called in the setup
     * phase of the application.
     */
    inline void begin() {
        shared_amplitude_percent_ = 50;
        for (uint8_t i = 0; i < NUM_SONICATORS; i++) {
            if (sonicators[i]) {
                sonicators[i]->begin();
            }
        }
    }

    /**
     * @brief Main update loop for the multiplexer.
     *
     * This method should be called repeatedly from the main application loop.
     * It updates the state of all sonicator units and handles the shared
     * hardware resources, like the amplitude PWM.
     * 
     * @performance This function must complete within 1ms to meet real-time requirements.
     */
    inline void update() {
        for (uint8_t i = 0; i < NUM_SONICATORS; i++) {
            if (sonicators[i]) {
                sonicators[i]->update();
            }
        }
        updateSharedAmplitude();
    }

    /**
     * @brief Starts a specific sonicator.
     * @param index The index of the sonicator to start (0-3).
     * @return true if the start command was accepted, false otherwise.
     */
    inline bool start(uint8_t index) {
        if (index >= NUM_SONICATORS || !sonicators[index]) {
            return false;
        }
        return sonicators[index]->start();
    }

    /**
     * @brief Stops a specific sonicator.
     * @param index The index of the sonicator to stop (0-3).
     * @return true if the stop command was accepted, false otherwise.
     */
    inline bool stop(uint8_t index) {
        if (index >= NUM_SONICATORS || !sonicators[index]) {
            return false;
        }
        return sonicators[index]->stop();
    }

    /**
     * @brief Sets the shared amplitude for all sonicators.
     * @param amplitude_percent The desired amplitude (20-100%).
     * @return true if the amplitude was set, false if the value is out of range.
     */
    inline bool setAmplitude(uint8_t amplitude_percent) {
        if (amplitude_percent < 20 || amplitude_percent > 100) {
            return false;
        }
        shared_amplitude_percent_ = amplitude_percent;
        return true;
    }

    /**
     * @brief Resets a fault condition on a specific sonicator.
     * @param index The index of the sonicator to reset (0-3).
     * @return true if the reset command was accepted, false otherwise.
     */
    inline bool resetOverload(uint8_t index) {
        if (index >= NUM_SONICATORS || !sonicators[index]) {
            return false;
        }
        return sonicators[index]->resetOverload();
    }

    /**
     * @brief Gets the status of a specific sonicator.
     * @param index The index of the sonicator to query (0-3).
     * @return A pointer to the sonicator's state structure, or nullptr if the index is invalid.
     */
    inline const sonicator_status_t* getStatus(uint8_t index) const {
        if (index >= NUM_SONICATORS || !sonicators[index]) {
            return nullptr;
        }
        return sonicators[index]->getStatus();
    }

private:
    /**
     * @brief Private constructor for singleton pattern.
     */
    Multiplexer() : shared_amplitude_percent_(50) {
        for (uint8_t i = 0; i < NUM_SONICATORS; i++) {
            sonicators[i] = nullptr;
        }
    }

    /**
     * @brief Private destructor for singleton pattern.
     */
    ~Multiplexer() = default;

    // Delete copy constructor and assignment operator
    Multiplexer(const Multiplexer&) = delete;
    Multiplexer& operator=(const Multiplexer&) = delete;

    // An array to hold the four sonicator instances.
    // This will be properly initialized in the .cpp file.
    SonicatorInterface* sonicators[NUM_SONICATORS];

    // The shared amplitude setpoint.
    uint8_t shared_amplitude_percent_;

    /**
     * @brief Updates the shared amplitude PWM output.
     */
    inline void updateSharedAmplitude() {
        // PWM output implementation would go here
        // Example: analogWrite(AMPLITUDE_PWM_PIN, map(shared_amplitude_percent_, 0, 100, 0, 255));
    }
};

#endif // MULTIPLEXER_H
