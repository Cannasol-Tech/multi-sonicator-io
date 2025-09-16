/**
 * @file include/multiplexer/Multiplexer.h
 * @title SonicMultiplexer Class for Multi-Sonicator Control
 * @company Axovia AI
 * @date 2025-09-16
 * @brief Object-oriented interface for managing multiple CT2000 sonicator units
 * @version 1.2.0
 *
 * @details
 * This header file defines the SonicMultiplexer class, which provides a clean, object-oriented
 * interface for controlling and monitoring multiple CT2000 ultrasonic transducer units. The
 * multiplexer encapsulates the complexity of managing shared resources and individual sonicator
 * states, providing a unified API for the Multi-Sonicator-IO controller system.
 *
 * The implementation supports up to 4 concurrent sonicator units with shared amplitude control
 * and individual start/stop/reset operations. It follows SOLID principles and clean architecture
 * patterns suitable for embedded systems development.
 *
 * @section overview Overview
 *
 * The SonicMultiplexer serves as the central controller for the Multi-Sonicator-IO system,
 * managing the coordination between multiple ultrasonic transducers. Key responsibilities include:
 *
 * - **Resource Management**: Shared amplitude PWM control across all units
 * - **State Coordination**: Synchronized operation and status monitoring
 * - **Fault Handling**: Individual and system-wide error recovery
 * - **Configuration**: Runtime parameter adjustment and validation
 *
 * The class is designed as a singleton pattern for global system management while providing
 * thread-safe operations suitable for interrupt-driven embedded environments.
 *
 * @section architecture Architecture
 *
 * The multiplexer architecture consists of:
 *
 * **Core Components:**
 * - Individual sonicator interfaces (sonicator_interface_t)
 * - Shared amplitude control system
 * - State management and coordination logic
 *
 * **Design Patterns:**
 * - Singleton pattern for global access
 * - Facade pattern for simplified interface
 * - State pattern for operational modes
 *
 * @section usage Usage
 *
 * To use the SonicMultiplexer in your application:
 *
 * @code{.cpp}
 * // Include the multiplexer header
 * #include "include/multiplexer/Multiplexer.h"
 *
 * // Create global multiplexer instance
 * SonicMultiplexer multiplexer(4); // For 4 sonicators
 *
 * // In setup()
 * multiplexer.begin(); // Initialize all units
 *
 * // In main loop
 * multiplexer.update(); // Update all units
 *
 * // Control operations
 * multiplexer.start(0); // Start sonicator 0
 * multiplexer.setAmplitude(75); // Set shared amplitude to 75%
 *
 * // Monitor status
 * const sonicator_status_t* status = multiplexer.getStatus(0);
 * if (status && status->isRunning) {
 *     // Handle running state
 * }
 * @endcode
 *
 * @section performance_considerations Performance Considerations
 *
 * The multiplexer is optimized for embedded systems:
 * - **Memory Footprint**: Minimal overhead for shared resources
 * - **Execution Time**: Fast operations suitable for real-time control
 * - **Interrupt Safety**: Thread-safe for interrupt-driven updates
 * - **Resource Sharing**: Efficient use of PWM and I/O resources
 *
 * @section safety_features Safety Features
 *
 * Built-in safety mechanisms include:
 * - **Parameter Validation**: Amplitude limits (20-100%) enforced
 * - **Index Bounds Checking**: Sonicator index validation
 * - **Fault Isolation**: Individual unit fault containment
 * - **Graceful Degradation**: System stability during faults
 *
 * @section error_handling Error Handling
 *
 * The multiplexer provides comprehensive error handling:
 * - **Invalid Parameters**: Rejected with appropriate return codes
 * - **Resource Failures**: Graceful degradation and error reporting
 * - **Communication Faults**: Automatic retry and fault recovery
 * - **Overload Conditions**: Reset mechanisms and status monitoring
 *
 * @warning This class manages hardware resources and should be used carefully in production
 * @warning Amplitude values outside 20-100% range will be rejected
 * @warning Sonicator indices must be within valid range (0 to count-1)
 *
 * @see sonicator.h Individual sonicator interface definition
 * @see system_config.h System configuration parameters
 * @see docs/multiplexer-architecture.md Detailed architecture documentation
 *
 * @note The multiplexer is designed for the Multi-Sonicator-IO controller
 * @note Shared amplitude control affects all active sonicators simultaneously
 * @note begin() must be called before any other operations
 * @note update() should be called regularly in the main application loop
 */

#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "sonicator/sonicator.h"
#include "system_config.h"

/**
 * @class SonicMultiplexer
 * @brief Object-oriented multiplexer for managing multiple CT2000 sonicator units
 * @details The SonicMultiplexer class provides a comprehensive, thread-safe interface for
 * controlling and monitoring multiple ultrasonic transducer units. It encapsulates the complexity
 * of shared resource management and individual unit coordination, offering a clean API for
 * embedded systems development.
 *
 * The class manages up to 4 concurrent sonicator units with the following capabilities:
 * - **Individual Control**: Start, stop, and reset operations per unit
 * - **Shared Resources**: Common amplitude control across all active units
 * - **Status Monitoring**: Real-time state and fault monitoring
 * - **Safety Features**: Parameter validation and fault isolation
 *
 * @section class_overview Class Overview
 *
 * The SonicMultiplexer follows object-oriented design principles:
 *
 * **Key Responsibilities:**
 * - Coordinate multiple sonicator units simultaneously
 * - Manage shared hardware resources (PWM amplitude control)
 * - Provide unified status monitoring and control interface
 * - Implement safety interlocks and parameter validation
 * - Handle fault conditions and recovery operations
 *
 * **Design Philosophy:**
 * - Clean separation of concerns between units
 * - Thread-safe operations for interrupt-driven environments
 * - Minimal resource overhead for embedded constraints
 * - Comprehensive error handling and reporting
 *
 * @section constructor_usage Constructor Usage
 *
 * Create a multiplexer instance with the desired number of sonicators:
 *
 * @code{.cpp}
 * // Create multiplexer for 4 sonicators
 * SonicMultiplexer multiplexer(4);
 *
 * // Alternative initialization
 * SonicMultiplexer* pMultiplexer = new SonicMultiplexer(2);
 * @endcode
 *
 * @param count_ Number of sonicator units to manage (1-4)
 *
 * @section public_interface Public Interface
 *
 * The public interface provides methods for:
 * - **Initialization**: begin() - Setup hardware and initialize units
 * - **Runtime Control**: update() - Main loop processing
 * - **Unit Control**: start(), stop(), resetOverload() - Individual unit operations
 * - **Shared Control**: setAmplitude() - Global amplitude setting
 * - **Monitoring**: getStatus() - Status retrieval
 *
 * @section lifecycle_management Lifecycle Management
 *
 * Proper usage follows this lifecycle:
 *
 * @code{.cpp}
 * // 1. Construction
 * SonicMultiplexer multiplexer(4);
 *
 * // 2. Initialization
 * multiplexer.begin(); // Must be called first
 *
 * // 3. Runtime operation
 * while (true) {
 *     multiplexer.update(); // Call regularly
 *
 *     // Control operations as needed
 *     multiplexer.start(0);
 *     multiplexer.setAmplitude(80);
 * }
 *
 * // 4. Destruction (automatic)
 * // Destructor handles cleanup
 * @endcode
 *
 * @section shared_resources Shared Resources
 *
 * The multiplexer manages shared hardware resources:
 *
 * **Amplitude PWM Control:**
 * - Single PWM output controls amplitude for all units
 * - Range: 20-100% (validated)
 * - Affects all active sonicators simultaneously
 * - Thread-safe updates
 *
 * **Resource Allocation:**
 * - GPIO pins assigned per unit
 * - PWM channel shared across units
 * - ADC channels for power monitoring
 *
 * @section error_handling Error Handling
 *
 * Comprehensive error handling includes:
 * - **Parameter Validation**: Index bounds and value ranges checked
 * - **Hardware Faults**: Individual unit fault isolation
 * - **Resource Conflicts**: Shared resource access protection
 * - **Recovery Mechanisms**: Reset and restart capabilities
 *
 * @section thread_safety Thread Safety
 *
 * The multiplexer is designed for embedded environments:
 * - **Interrupt Safe**: Can be used in interrupt service routines
 * - **Atomic Operations**: Critical sections protected
 * - **State Consistency**: Guaranteed state integrity across calls
 *
 * @warning Constructor parameter must be between 1 and 4
 * @warning begin() must be called before any other operations
 * @warning update() should be called regularly in main loop
 * @warning Shared amplitude affects all active sonicators
 *
 * @see sonicator_interface_t Individual sonicator interface
 * @see sonicator_status_t Status structure definition
 * @see docs/multiplexer-design.md Detailed design documentation
 *
 * @note The multiplexer uses RAII principles for resource management
 * @note All operations are non-blocking for real-time performance
 * @note Memory usage is optimized for embedded constraints
 * @note Error conditions are logged but don't halt operation
 */
class SonicMultiplexer {
private:
    uint8_t sonicator_count_m;
    uint8_t amplitude_ctrl_duty_m;    ///< The duty cycle of the amplitude control signal (20-100%)

    sonicator_interface_t sonicators_m;

public:
    /**
     * @brief Constructor for the SonicMultiplexer class
     * @param count_ Number of sonicator units to manage (1-4)
     * @details Initializes the multiplexer with the specified number of sonicator units.
     * The constructor sets up internal data structures and prepares for hardware initialization.
     * @warning count_ must be between 1 and 4 inclusive
     */
    SonicMultiplexer(uint8_t count_) {
        sonicator_count_m = count_;
        initSonicators_();
    }

    /**
     * @brief Destructor for the SonicMultiplexer class
     * @details Properly cleans up resources and shuts down all sonicator units.
     * Ensures safe hardware state before destruction.
     */
    ~SonicMultiplexer() { 
        for (uint8_t i = 0; i < sonicator_count_m; i++) { 
            sonicators_m[i].~sonicator_interface_t(); 
        } 
    }

    /**
     * @brief Initializes the multiplexer and all sonicator units
     * @details Sets up pin configurations, initializes hardware interfaces,
     * and prepares all sonicator units for operation. Must be called before
     * any other operations.
     * @note This method configures shared resources like PWM amplitude control
     * @warning Must be called in system setup phase
     */
    void begin();

    /**
     * @brief Main update loop for the multiplexer
     * @details Processes all sonicator units, updates shared resources,
     * and handles state transitions. Should be called regularly in the
     * main application loop.
     * @note Handles amplitude PWM updates and status synchronization
     */
    void update();

    /**
     * @brief Starts a specific sonicator unit
     * @param index The index of the sonicator to start (0 to count-1)
     * @return true if the start command was accepted, false otherwise
     * @details Initiates operation of the specified sonicator unit.
     * The unit will begin ultrasonic generation at the current amplitude setting.
     * @warning index must be within valid range
     */
    bool start(uint8_t index);

    /**
     * @brief Stops a specific sonicator unit
     * @param index The index of the sonicator to stop (0 to count-1)
     * @return true if the stop command was accepted, false otherwise
     * @details Terminates operation of the specified sonicator unit.
     * The unit will cease ultrasonic generation immediately.
     * @warning index must be within valid range
     */
    bool stop(uint8_t index);

    /**
     * @brief Sets the shared amplitude for all sonicators
     * @param amplitude_percent The desired amplitude percentage (20-100)
     * @return true if the amplitude was set successfully, false if out of range
     * @details Sets the global amplitude level that affects all active sonicator units.
     * The change takes effect immediately for all running units.
     * @note Amplitude range is validated (20-100% only)
     */
    bool setAmplitude(uint8_t amplitude_percent);

    /**
     * @brief Resets overload condition on a specific sonicator
     * @param index The index of the sonicator to reset (0 to count-1)
     * @return true if the reset was successful, false otherwise
     * @details Clears overload fault conditions and attempts to restart the unit.
     * Useful for recovering from temporary fault conditions.
     * @warning index must be within valid range
     */
    bool resetOverload(uint8_t index);

    /**
     * @brief Retrieves the status of a specific sonicator
     * @param index The index of the sonicator to query (0 to count-1)
     * @return Pointer to the status structure, or nullptr if invalid index
     * @details Returns current operational status of the specified sonicator unit.
     * The returned pointer is valid until the next multiplexer operation.
     * @warning index must be within valid range
     */
    const sonicator_status_t* getStatus(uint8_t index) const;

private:
    /**
     * @brief Initializes the sonicator units
     * @details Internal method for setting up individual sonicator interfaces.
     */
    void initSonicators_();

    /**
     * @brief Updates the shared amplitude control
     * @details Internal method for synchronizing amplitude PWM across units.
     */
    void updateSharedAmplitude_();
};

#endif // MULTIPLEXER_H