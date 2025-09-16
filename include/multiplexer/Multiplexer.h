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
     * @brief Constructor for the Multiplexer class.
     */
    Multiplexer();

    /**
     * @brief Destructor for the Multiplexer class.
     */
    ~Multiplexer();

    /**
     * @brief Initializes the multiplexer and all sonicator units.
     *
     * This method sets up the pin configurations for each of the four
     * sonicators and initializes them. It must be called in the setup
     * phase of the application.
     */
    void begin();

    /**
     * @brief Main update loop for the multiplexer.
     *
     * This method should be called repeatedly from the main application loop.
     * It updates the state of all sonicator units and handles the shared
     * hardware resources, like the amplitude PWM.
     */
    void update();

    /**
     * @brief Starts a specific sonicator.
     * @param index The index of the sonicator to start (0-3).
     * @return true if the start command was accepted, false otherwise.
     */
    bool start(uint8_t index);

    /**
     * @brief Stops a specific sonicator.
     * @param index The index of the sonicator to stop (0-3).
     * @return true if the stop command was accepted, false otherwise.
     */
    bool stop(uint8_t index);

    /**
     * @brief Sets the shared amplitude for all sonicators.
     * @param amplitude_percent The desired amplitude (20-100%).
     * @return true if the amplitude was set, false if the value is out of range.
     */
    bool setAmplitude(uint8_t amplitude_percent);

    /**
     * @brief Resets a fault condition on a specific sonicator.
     * @param index The index of the sonicator to reset (0-3).
     * @return true if the reset command was accepted, false otherwise.
     */
    bool resetOverload(uint8_t index);

    /**
     * @brief Gets the status of a specific sonicator.
     * @param index The index of the sonicator to query (0-3).
     * @return A pointer to the sonicator's state structure, or nullptr if the index is invalid.
     */
    const sonicator_status_t* getStatus(uint8_t index) const;

private:
    // An array to hold the four sonicator instances.
    // This will be properly initialized in the .cpp file.
    SonicatorInterface* sonicators[NUM_SONICATORS];

    // The shared amplitude setpoint.
    uint8_t shared_amplitude_percent_;

    /**
     * @brief Updates the shared amplitude PWM output.
     */
    void updateSharedAmplitude();
};

#endif // MULTIPLEXER_H
