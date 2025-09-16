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


/**
 * @class SonicMultiplexer
 * @brief Manages multiple CT2000 SonicatorInterface units running Simultaneously
 * @details This class manages multiple Concurrent CT2000 SonicatorInterface units and provides a clean interface to manage the entire system.
 * @param count_ - The number of Sonicators that will be managed by this instance of the Multiplexer
 * @note This class is a singleton and should be used as a global instance.
 * @note This class is responsible for managing the shared resources such as the amplitude PWM output.
 * @note Product Names Multi-Sonicator I/O Controller and Sonic Multiplexer
 * @
 */
class SonicMultiplexer {
private:
    uint8_t sonicator_count_m;
    uint8_t amplitude_ctrl_duty_m;    ///<! The duty cycle of the amplitude control signal (20-100%)

    sonicator_interface_t sonicators_m;

public:
    /**
     * @brief Constructor for the Multiplexer class.
     */
    SonicMultiplexer(uint8_t count_) {
        sonicator_count_m = count_;
        initSonicators_();
     };

    /**
     * @brief Destructor for the Multiplexer class.
     */
    ~SonicMultiplexer() { for (uint8_t i = 0; i < sonicator_count_m; i++) { sonicators_m[i].~sonicator_interface_t(); } }

    /**
     * @brief Initializes the multiplexer and all sonicator units.
     *
     * This method sets up the pin configurations for each of the four
     * sonicators and initializes them. It must be called in the setup
     * phase of the application.
     */
    static inline svoid begin() { for (uint8_t i = 0; i < sonicator_count_m; i++) { sonicators_m[i].begin(); } }

    /**
     * @brief Main update loop for the multiplexer.
     *
     * This method should be called repeatedly from the main application loop.
     * It updates the state of all sonicator units and handles the shared
     * hardware resources, like the amplitude PWM.
     */
    static inline void update() { for (uint8_t i = 0; i < sonicator_count_m; i++) { sonicators_m[i].update(); } }

    /**
     * @brief Starts a specific sonicator.
     * @param index The index of the sonicator to start (0-3).
     * @return true if the start command was accepted, false otherwise.
     */
    static inline bool start(uint8_t index) { return sonicators_m[index].start(); }

    /**
     * @brief Stops a specific sonicator.
     * @param index The index of the sonicator to stop (0-3).
     * @return true if the stop command was accepted, false otherwise.
     */
    static inline bool stop(uint8_t index) { return sonicators_m[index].stop(); }

    /**
     * @brief Sets the shared amplitude for all sonicators.
     * @param amplitude_percent The desired amplitude (20-100%).
     * @return true if the amplitude was set, false if the value is out of range.
     */
    static inline bool setAmplitude(uint8_t amplitude_percent) { return amplitude_percent >= 20 && amplitude_percent <= 100; }

    /**
     * @brief Resets a fault condition on a specific sonicator.
     * @param index The index of the sonicator to reset (0-3).
     * @return true if the reset command was accepted, false otherwise.
     */
    bool resetOverload(uint8_t index);

    /**
     * @brief Retrieves the status of a specific sonicator.
     * @param index The index of the sonicator to query (0-3).
     * @return A pointer to the status structure of the sonicator, or nullptr if the index is invalid.
     */
    const sonicator_status_t* getStatus(uint8_t index) const;

private:

    inline constexpr initSonicators_(uint8_t _count) : numSonicators_m(_count) {
        sonicators_m<_count> = new sonicator_interface_t();
    }

    sonicator_interface_t sonicators_m[sonicator_count_m];
    uint8_t amplitude_percent;
    void initSonicators_();
    void updateSharedAmplitude_();
};

#endif // MULTIPLEXER_H