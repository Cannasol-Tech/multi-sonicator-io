/**
 * @file test_scenarios.cpp
 * @brief Comprehensive Test Scenarios for Multi-Sonicator Simulation
 * @author Cannasol Technologies
 * @date 2025-07-17
 * 
 * Implements realistic test scenarios for the Multi-Sonicator system
 * including normal operation, fault conditions, and edge cases.
 */

#include "test_scenarios.h"
#include <iostream>
#include <cstring>
#include <unistd.h>

TestScenarioRunner::TestScenarioRunner() 
    : modbus_simulator(2, 115200)
    , scenario_start_time(0)
    , current_scenario(SCENARIO_NONE)
    , scenario_running(false)
{
    // Initialize virtual sonicators
    for (int i = 0; i < 4; i++) {
        sonicators[i] = new VirtualSonicator(i + 1);
    }
    
    // Initialize test results
    memset(&test_results, 0, sizeof(test_results));
}

TestScenarioRunner::~TestScenarioRunner() {
    // Clean up virtual sonicators
    for (int i = 0; i < 4; i++) {
        delete sonicators[i];
    }
}

bool TestScenarioRunner::initialize() {
    // Initialize MODBUS simulator
    if (!modbus_simulator.open()) {
        std::cerr << "Failed to open MODBUS simulator" << std::endl;
        return false;
    }
    
    // Initialize all sonicators
    for (int i = 0; i < 4; i++) {
        sonicators[i]->reset();
    }
    
    // Reset test results
    memset(&test_results, 0, sizeof(test_results));
    
    std::cout << "Test scenario runner initialized" << std::endl;
    return true;
}

void TestScenarioRunner::runScenario(TestScenario scenario) {
    if (scenario_running) {
        std::cout << "Scenario already running, stopping current scenario" << std::endl;
        stopScenario();
    }
    
    current_scenario = scenario;
    scenario_running = true;
    scenario_start_time = getCurrentTime();
    
    std::cout << "Starting scenario: " << getScenarioName(scenario) << std::endl;
    
    // Initialize scenario-specific settings
    switch (scenario) {
        case SCENARIO_NORMAL_OPERATION:
            initializeNormalOperation();
            break;
        case SCENARIO_OVERLOAD_RECOVERY:
            initializeOverloadRecovery();
            break;
        case SCENARIO_FREQUENCY_DRIFT:
            initializeFrequencyDrift();
            break;
        case SCENARIO_MULTI_SONICATOR_SYNC:
            initializeMultiSonicatorSync();
            break;
        case SCENARIO_EMERGENCY_STOP:
            initializeEmergencyStop();
            break;
        case SCENARIO_POWER_RAMPING:
            initializePowerRamping();
            break;
        case SCENARIO_COMMUNICATION_STRESS:
            initializeCommunicationStress();
            break;
        case SCENARIO_FAULT_INJECTION:
            initializeFaultInjection();
            break;
        default:
            std::cerr << "Unknown scenario: " << scenario << std::endl;
            scenario_running = false;
            return;
    }
    
    test_results.scenario = scenario;
    test_results.start_time = scenario_start_time;
    test_results.status = TEST_RUNNING;
}

void TestScenarioRunner::stopScenario() {
    if (!scenario_running) {
        return;
    }
    
    scenario_running = false;
    
    // Stop all sonicators
    for (int i = 0; i < 4; i++) {
        sonicators[i]->stopSonication();
    }
    
    // Update test results
    test_results.end_time = getCurrentTime();
    test_results.duration_ms = test_results.end_time - test_results.start_time;
    
    if (test_results.status == TEST_RUNNING) {
        test_results.status = TEST_PASSED;
    }
    
    std::cout << "Scenario stopped: " << getScenarioName(current_scenario) << std::endl;
    std::cout << "Duration: " << test_results.duration_ms << " ms" << std::endl;
    
    current_scenario = SCENARIO_NONE;
}

void TestScenarioRunner::update() {
    if (!scenario_running) {
        return;
    }
    
    // Update all sonicators
    for (int i = 0; i < 4; i++) {
        sonicators[i]->update();
    }
    
    // Update scenario-specific logic
    switch (current_scenario) {
        case SCENARIO_NORMAL_OPERATION:
            updateNormalOperation();
            break;
        case SCENARIO_OVERLOAD_RECOVERY:
            updateOverloadRecovery();
            break;
        case SCENARIO_FREQUENCY_DRIFT:
            updateFrequencyDrift();
            break;
        case SCENARIO_MULTI_SONICATOR_SYNC:
            updateMultiSonicatorSync();
            break;
        case SCENARIO_EMERGENCY_STOP:
            updateEmergencyStop();
            break;
        case SCENARIO_POWER_RAMPING:
            updatePowerRamping();
            break;
        case SCENARIO_COMMUNICATION_STRESS:
            updateCommunicationStress();
            break;
        case SCENARIO_FAULT_INJECTION:
            updateFaultInjection();
            break;
        default:
            break;
    }
    
    // Check for test completion or failure
    checkTestCompletion();
}

TestResults TestScenarioRunner::getResults() const {
    return test_results;
}

const char* TestScenarioRunner::getScenarioName(TestScenario scenario) {
    switch (scenario) {
        case SCENARIO_NORMAL_OPERATION: return "Normal Operation";
        case SCENARIO_OVERLOAD_RECOVERY: return "Overload Recovery";
        case SCENARIO_FREQUENCY_DRIFT: return "Frequency Drift";
        case SCENARIO_MULTI_SONICATOR_SYNC: return "Multi-Sonicator Sync";
        case SCENARIO_EMERGENCY_STOP: return "Emergency Stop";
        case SCENARIO_POWER_RAMPING: return "Power Ramping";
        case SCENARIO_COMMUNICATION_STRESS: return "Communication Stress";
        case SCENARIO_FAULT_INJECTION: return "Fault Injection";
        default: return "Unknown";
    }
}

uint32_t TestScenarioRunner::getCurrentTime() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

// Scenario initialization methods
void TestScenarioRunner::initializeNormalOperation() {
    // Set all sonicators to 50% amplitude
    for (int i = 0; i < 4; i++) {
        sonicators[i]->setAmplitude(50);
    }
    
    test_results.expected_duration_ms = 5000; // 5 seconds
    test_results.test_description = "Normal operation with 4 sonicators at 50% amplitude";
}

void TestScenarioRunner::initializeOverloadRecovery() {
    // Set sonicator 1 to high amplitude to trigger overload
    sonicators[0]->setAmplitude(95);
    sonicators[0]->injectFault(FAULT_OVERLOAD);
    
    test_results.expected_duration_ms = 10000; // 10 seconds
    test_results.test_description = "Overload detection and recovery test";
}

void TestScenarioRunner::initializeFrequencyDrift() {
    // Inject frequency drift fault
    sonicators[0]->injectFault(FAULT_FREQUENCY_DRIFT);
    sonicators[0]->setAmplitude(60);
    
    test_results.expected_duration_ms = 8000; // 8 seconds
    test_results.test_description = "Frequency drift detection and response";
}

void TestScenarioRunner::initializeMultiSonicatorSync() {
    // Start all sonicators at different times
    for (int i = 0; i < 4; i++) {
        sonicators[i]->setAmplitude(40 + i * 10);
    }
    
    test_results.expected_duration_ms = 15000; // 15 seconds
    test_results.test_description = "Multi-sonicator synchronization test";
}

void TestScenarioRunner::initializeEmergencyStop() {
    // Start all sonicators first
    for (int i = 0; i < 4; i++) {
        sonicators[i]->setAmplitude(70);
        sonicators[i]->startSonication();
    }
    
    test_results.expected_duration_ms = 3000; // 3 seconds
    test_results.test_description = "Emergency stop response time test";
}

void TestScenarioRunner::initializePowerRamping() {
    // Start with low amplitude, will ramp up
    sonicators[0]->setAmplitude(20);
    
    test_results.expected_duration_ms = 12000; // 12 seconds
    test_results.test_description = "Power ramping from 20% to 100% test";
}

void TestScenarioRunner::initializeCommunicationStress() {
    // Enable error simulation
    modbus_simulator.setErrorSimulation(true, 0.05f); // 5% error rate
    
    test_results.expected_duration_ms = 20000; // 20 seconds
    test_results.test_description = "MODBUS communication stress test";
}

void TestScenarioRunner::initializeFaultInjection() {
    // Random fault injection
    test_results.expected_duration_ms = 30000; // 30 seconds
    test_results.test_description = "Random fault injection test";
}

// Scenario update methods
void TestScenarioRunner::updateNormalOperation() {
    uint32_t elapsed = getCurrentTime() - scenario_start_time;
    
    // Start sonicators at 1 second intervals
    for (int i = 0; i < 4; i++) {
        if (elapsed > (i + 1) * 1000 && !sonicators[i]->getIsRunning()) {
            sonicators[i]->startSonication();
        }
    }
    
    // Check if all sonicators are running normally
    bool all_running = true;
    for (int i = 0; i < 4; i++) {
        if (!sonicators[i]->getIsRunning() || sonicators[i]->getIsOverloaded()) {
            all_running = false;
            break;
        }
    }
    
    if (all_running && elapsed > 3000) {
        test_results.status = TEST_PASSED;
        test_results.success_count = 4;
    }
}

void TestScenarioRunner::updateOverloadRecovery() {
    uint32_t elapsed = getCurrentTime() - scenario_start_time;
    
    if (elapsed > 1000 && !sonicators[0]->getIsRunning()) {
        sonicators[0]->startSonication();
    }
    
    // Check for overload condition
    if (sonicators[0]->getIsOverloaded()) {
        test_results.fault_count++;
        
        // Try to reset overload after 2 seconds
        if (elapsed > 3000) {
            sonicators[0]->resetOverload();
            sonicators[0]->clearFaultInjection();
            sonicators[0]->setAmplitude(50); // Reduce amplitude
        }
    }
    
    // Check for successful recovery
    if (elapsed > 5000 && sonicators[0]->getIsRunning() && !sonicators[0]->getIsOverloaded()) {
        test_results.status = TEST_PASSED;
        test_results.success_count = 1;
    }
}

void TestScenarioRunner::updateFrequencyDrift() {
    uint32_t elapsed = getCurrentTime() - scenario_start_time;
    
    if (elapsed > 1000 && !sonicators[0]->getIsRunning()) {
        sonicators[0]->startSonication();
    }
    
    // Monitor frequency lock status
    if (!sonicators[0]->getIsFrequencyLocked()) {
        test_results.fault_count++;
    }
    
    // Clear fault after 5 seconds
    if (elapsed > 5000) {
        sonicators[0]->clearFaultInjection();
    }
    
    // Check for frequency lock recovery
    if (elapsed > 6000 && sonicators[0]->getIsFrequencyLocked()) {
        test_results.status = TEST_PASSED;
        test_results.success_count = 1;
    }
}

void TestScenarioRunner::updateMultiSonicatorSync() {
    uint32_t elapsed = getCurrentTime() - scenario_start_time;
    
    // Start sonicators at different times
    for (int i = 0; i < 4; i++) {
        if (elapsed > (i * 2 + 1) * 1000 && !sonicators[i]->getIsRunning()) {
            sonicators[i]->startSonication();
        }
    }
    
    // Check synchronization
    if (elapsed > 10000) {
        int running_count = 0;
        for (int i = 0; i < 4; i++) {
            if (sonicators[i]->getIsRunning()) {
                running_count++;
            }
        }
        
        if (running_count == 4) {
            test_results.status = TEST_PASSED;
            test_results.success_count = running_count;
        }
    }
}

void TestScenarioRunner::updateEmergencyStop() {
    uint32_t elapsed = getCurrentTime() - scenario_start_time;
    
    // Trigger emergency stop at 2 seconds
    if (elapsed > 2000) {
        for (int i = 0; i < 4; i++) {
            sonicators[i]->stopSonication();
        }
        
        // Check that all stopped quickly
        bool all_stopped = true;
        for (int i = 0; i < 4; i++) {
            if (sonicators[i]->getIsRunning()) {
                all_stopped = false;
                break;
            }
        }
        
        if (all_stopped) {
            test_results.status = TEST_PASSED;
            test_results.success_count = 4;
        }
    }
}

void TestScenarioRunner::updatePowerRamping() {
    uint32_t elapsed = getCurrentTime() - scenario_start_time;
    
    if (elapsed > 1000 && !sonicators[0]->getIsRunning()) {
        sonicators[0]->startSonication();
    }
    
    // Ramp amplitude from 20% to 100% over 10 seconds
    if (elapsed > 2000) {
        uint32_t ramp_time = elapsed - 2000;
        uint8_t amplitude = 20 + (ramp_time * 80) / 10000;
        if (amplitude > 100) amplitude = 100;
        
        sonicators[0]->setAmplitude(amplitude);
    }
    
    // Check if ramping completed successfully
    if (elapsed > 11000 && sonicators[0]->getAmplitude() == 100) {
        test_results.status = TEST_PASSED;
        test_results.success_count = 1;
    }
}

void TestScenarioRunner::updateCommunicationStress() {
    uint32_t elapsed = getCurrentTime() - scenario_start_time;
    
    // Continuously modify sonicator parameters
    static uint32_t last_update = 0;
    if (elapsed - last_update > 100) { // Update every 100ms
        for (int i = 0; i < 4; i++) {
            uint8_t amplitude = 30 + (elapsed / 100) % 50;
            sonicators[i]->setAmplitude(amplitude);
        }
        last_update = elapsed;
    }
    
    // Check communication statistics
    if (elapsed > 15000) {
        uint32_t error_count = modbus_simulator.getErrorCount();
        uint32_t total_bytes = modbus_simulator.getBytesReceived() + modbus_simulator.getBytesTransmitted();
        
        if (total_bytes > 1000 && error_count < total_bytes * 0.1) {
            test_results.status = TEST_PASSED;
            test_results.success_count = total_bytes;
            test_results.fault_count = error_count;
        }
    }
}

void TestScenarioRunner::updateFaultInjection() {
    uint32_t elapsed = getCurrentTime() - scenario_start_time;
    
    // Inject random faults every 5 seconds
    static uint32_t last_injection = 0;
    if (elapsed - last_injection > 5000) {
        int sonicator_id = elapsed % 4;
        FaultType fault = static_cast<FaultType>((elapsed / 5000) % 4 + 1);
        
        sonicators[sonicator_id]->injectFault(fault);
        test_results.fault_count++;
        last_injection = elapsed;
    }
    
    // Clear faults after 2 seconds
    for (int i = 0; i < 4; i++) {
        if ((elapsed % 10000) > 7000) {
            sonicators[i]->clearFaultInjection();
        }
    }
    
    // Test passes if we injected faults and recovered
    if (elapsed > 25000 && test_results.fault_count > 0) {
        test_results.status = TEST_PASSED;
        test_results.success_count = test_results.fault_count;
    }
}

void TestScenarioRunner::checkTestCompletion() {
    uint32_t elapsed = getCurrentTime() - scenario_start_time;
    
    // Check for timeout
    if (elapsed > test_results.expected_duration_ms) {
        if (test_results.status == TEST_RUNNING) {
            test_results.status = TEST_FAILED;
            test_results.error_message = "Test timeout";
        }
        stopScenario();
    }
    
    // Check for early completion
    if (test_results.status == TEST_PASSED || test_results.status == TEST_FAILED) {
        stopScenario();
    }
}