/**
 * @file test_scenarios.h
 * @brief Test Scenarios Header for Multi-Sonicator Simulation
 * @author Cannasol Technologies
 * @date 2025-07-17
 * 
 * Defines comprehensive test scenarios for the Multi-Sonicator system
 * including normal operation, fault conditions, and edge cases.
 */

#ifndef TEST_SCENARIOS_H
#define TEST_SCENARIOS_H

#include <stdint.h>
#include <time.h>
#include "virtual_sonicator.h"
#include "serial_simulator.h"

/**
 * @brief Test scenario types
 */
typedef enum {
    SCENARIO_NONE = 0,
    SCENARIO_NORMAL_OPERATION = 1,
    SCENARIO_OVERLOAD_RECOVERY = 2,
    SCENARIO_FREQUENCY_DRIFT = 3,
    SCENARIO_MULTI_SONICATOR_SYNC = 4,
    SCENARIO_EMERGENCY_STOP = 5,
    SCENARIO_POWER_RAMPING = 6,
    SCENARIO_COMMUNICATION_STRESS = 7,
    SCENARIO_FAULT_INJECTION = 8
} TestScenario;

/**
 * @brief Test result status
 */
typedef enum {
    TEST_NOT_RUN = 0,
    TEST_RUNNING = 1,
    TEST_PASSED = 2,
    TEST_FAILED = 3
} TestStatus;

/**
 * @brief Test results structure
 */
typedef struct {
    TestScenario scenario;
    TestStatus status;
    uint32_t start_time;
    uint32_t end_time;
    uint32_t duration_ms;
    uint32_t expected_duration_ms;
    
    uint32_t success_count;
    uint32_t fault_count;
    
    const char* test_description;
    const char* error_message;
} TestResults;

/**
 * @brief Test Scenario Runner Class
 * 
 * Manages and executes comprehensive test scenarios for the
 * Multi-Sonicator system using virtual hardware simulation.
 */
class TestScenarioRunner {
private:
    // Virtual hardware
    VirtualSonicator* sonicators[4];
    SerialPortSimulator modbus_simulator;
    
    // Test execution state
    uint32_t scenario_start_time;
    TestScenario current_scenario;
    bool scenario_running;
    
    // Test results
    TestResults test_results;
    
    // Private methods
    uint32_t getCurrentTime();
    const char* getScenarioName(TestScenario scenario);
    
    // Scenario initialization methods
    void initializeNormalOperation();
    void initializeOverloadRecovery();
    void initializeFrequencyDrift();
    void initializeMultiSonicatorSync();
    void initializeEmergencyStop();
    void initializePowerRamping();
    void initializeCommunicationStress();
    void initializeFaultInjection();
    
    // Scenario update methods
    void updateNormalOperation();
    void updateOverloadRecovery();
    void updateFrequencyDrift();
    void updateMultiSonicatorSync();
    void updateEmergencyStop();
    void updatePowerRamping();
    void updateCommunicationStress();
    void updateFaultInjection();
    
    void checkTestCompletion();
    
public:
    /**
     * @brief Constructor
     */
    TestScenarioRunner();
    
    /**
     * @brief Destructor
     */
    ~TestScenarioRunner();
    
    /**
     * @brief Initialize the test scenario runner
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Run a specific test scenario
     * @param scenario Test scenario to run
     */
    void runScenario(TestScenario scenario);
    
    /**
     * @brief Stop the current scenario
     */
    void stopScenario();
    
    /**
     * @brief Update scenario execution (call periodically)
     */
    void update();
    
    /**
     * @brief Check if a scenario is currently running
     * @return true if scenario is running
     */
    bool isRunning() const { return scenario_running; }
    
    /**
     * @brief Get test results
     * @return Test results structure
     */
    TestResults getResults() const;
    
    /**
     * @brief Get access to MODBUS simulator
     * @return Reference to MODBUS simulator
     */
    SerialPortSimulator& getModbusSimulator() { return modbus_simulator; }
    
    /**
     * @brief Get access to specific sonicator
     * @param index Sonicator index (0-3)
     * @return Pointer to virtual sonicator
     */
    VirtualSonicator* getSonicator(int index) { 
        return (index >= 0 && index < 4) ? sonicators[index] : nullptr; 
    }
};

#endif // TEST_SCENARIOS_H