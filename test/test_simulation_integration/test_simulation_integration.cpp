/**
 * @file test_simulation_integration.cpp
 * @brief Integration Tests for Multi-Sonicator Simulation Framework
 * @author Cannasol Technologies
 * @date 2025-07-17
 */

#include <unity.h>
#include <Arduino.h>
#include "../simulation/virtual_sonicator.h"
#include "../simulation/serial_simulator.h"
#include "../simulation/test_scenarios.h"

SerialPortSimulator* serial_sim;
TestScenarioRunner* scenario_runner;

void setUp() {
    serial_sim = new SerialPortSimulator(2, 115200);
    scenario_runner = new TestScenarioRunner();
}

void tearDown() {
    delete serial_sim;
    delete scenario_runner;
    serial_sim = nullptr;
    scenario_runner = nullptr;
}

/**
 * @brief Test MODBUS simulator initialization
 */
void test_modbus_simulator_init() {
    TEST_ASSERT_TRUE(serial_sim->open());
    TEST_ASSERT_TRUE(serial_sim->isOpen());
    TEST_ASSERT_EQUAL(0, serial_sim->available());
    TEST_ASSERT_EQUAL(0, serial_sim->getBytesReceived());
    TEST_ASSERT_EQUAL(0, serial_sim->getBytesTransmitted());
    TEST_ASSERT_EQUAL(0, serial_sim->getErrorCount());
}

/**
 * @brief Test MODBUS read holding registers
 */
void test_modbus_read_holding_registers() {
    TEST_ASSERT_TRUE(serial_sim->open());
    
    // Construct MODBUS read request for register 40001 (1 register)
    uint8_t request[] = {0x02, 0x03, 0x9C, 0x41, 0x00, 0x01, 0x54, 0x08};
    
    // Send request
    size_t bytes_written = serial_sim->write(request, sizeof(request));
    TEST_ASSERT_EQUAL(sizeof(request), bytes_written);
    
    // Read response
    uint8_t response[32];
    size_t bytes_read = serial_sim->read(response, sizeof(response));
    
    // Should get response: slave_id + function_code + byte_count + data + crc
    TEST_ASSERT_GREATER_THAN(4, bytes_read);
    TEST_ASSERT_EQUAL(0x02, response[0]); // Slave address
    TEST_ASSERT_EQUAL(0x03, response[1]); // Function code
    TEST_ASSERT_EQUAL(0x02, response[2]); // Byte count (2 bytes for 1 register)
}

/**
 * @brief Test MODBUS write single register
 */
void test_modbus_write_single_register() {
    TEST_ASSERT_TRUE(serial_sim->open());
    
    // Construct MODBUS write request for register 40001 (amplitude) = 75
    uint8_t request[] = {0x02, 0x06, 0x9C, 0x41, 0x00, 0x4B, 0x99, 0xD6};
    
    // Send request
    size_t bytes_written = serial_sim->write(request, sizeof(request));
    TEST_ASSERT_EQUAL(sizeof(request), bytes_written);
    
    // Read response (should echo the request)
    uint8_t response[32];
    size_t bytes_read = serial_sim->read(response, sizeof(response));
    
    TEST_ASSERT_EQUAL(8, bytes_read); // Standard write response length
    TEST_ASSERT_EQUAL(0x02, response[0]); // Slave address
    TEST_ASSERT_EQUAL(0x06, response[1]); // Function code
}

/**
 * @brief Test scenario runner initialization
 */
void test_scenario_runner_init() {
    TEST_ASSERT_TRUE(scenario_runner->initialize());
    TEST_ASSERT_FALSE(scenario_runner->isRunning());
    
    // Check sonicator access
    for (int i = 0; i < 4; i++) {
        VirtualSonicator* sonicator = scenario_runner->getSonicator(i);
        TEST_ASSERT_NOT_NULL(sonicator);
        TEST_ASSERT_FALSE(sonicator->getIsRunning());
    }
    
    // Check MODBUS simulator access
    SerialPortSimulator& modbus_sim = scenario_runner->getModbusSimulator();
    TEST_ASSERT_TRUE(modbus_sim.isOpen());
}

/**
 * @brief Test normal operation scenario
 */
void test_normal_operation_scenario() {
    TEST_ASSERT_TRUE(scenario_runner->initialize());
    
    // Run normal operation scenario
    scenario_runner->runScenario(SCENARIO_NORMAL_OPERATION);
    TEST_ASSERT_TRUE(scenario_runner->isRunning());
    
    // Simulate for a few seconds
    for (int i = 0; i < 100; i++) {
        scenario_runner->update();
        delay(50); // 50ms updates
    }
    
    // Check results
    TestResults results = scenario_runner->getResults();
    TEST_ASSERT_EQUAL(SCENARIO_NORMAL_OPERATION, results.scenario);
    TEST_ASSERT_TRUE(results.status == TEST_PASSED || results.status == TEST_RUNNING);
    
    scenario_runner->stopScenario();
    TEST_ASSERT_FALSE(scenario_runner->isRunning());
}

/**
 * @brief Test overload recovery scenario
 */
void test_overload_recovery_scenario() {
    TEST_ASSERT_TRUE(scenario_runner->initialize());
    
    // Run overload recovery scenario
    scenario_runner->runScenario(SCENARIO_OVERLOAD_RECOVERY);
    TEST_ASSERT_TRUE(scenario_runner->isRunning());
    
    // Simulate for overload detection
    for (int i = 0; i < 50; i++) {
        scenario_runner->update();
        delay(100); // 100ms updates
        
        // Check if overload was detected
        TestResults results = scenario_runner->getResults();
        if (results.fault_count > 0) {
            break; // Overload detected
        }
    }
    
    // Continue simulation for recovery
    for (int i = 0; i < 100; i++) {
        scenario_runner->update();
        delay(50);
    }
    
    // Check results
    TestResults results = scenario_runner->getResults();
    TEST_ASSERT_EQUAL(SCENARIO_OVERLOAD_RECOVERY, results.scenario);
    TEST_ASSERT_GREATER_THAN(0, results.fault_count); // Should have detected overload
    
    scenario_runner->stopScenario();
}

/**
 * @brief Test emergency stop scenario
 */
void test_emergency_stop_scenario() {
    TEST_ASSERT_TRUE(scenario_runner->initialize());
    
    // Run emergency stop scenario
    scenario_runner->runScenario(SCENARIO_EMERGENCY_STOP);
    TEST_ASSERT_TRUE(scenario_runner->isRunning());
    
    // Simulate for emergency stop
    for (int i = 0; i < 60; i++) {
        scenario_runner->update();
        delay(50);
    }
    
    // Check results
    TestResults results = scenario_runner->getResults();
    TEST_ASSERT_EQUAL(SCENARIO_EMERGENCY_STOP, results.scenario);
    
    // All sonicators should be stopped
    for (int i = 0; i < 4; i++) {
        VirtualSonicator* sonicator = scenario_runner->getSonicator(i);
        TEST_ASSERT_FALSE(sonicator->getIsRunning());
    }
    
    scenario_runner->stopScenario();
}

/**
 * @brief Test communication stress scenario
 */
void test_communication_stress_scenario() {
    TEST_ASSERT_TRUE(scenario_runner->initialize());
    
    // Run communication stress scenario
    scenario_runner->runScenario(SCENARIO_COMMUNICATION_STRESS);
    TEST_ASSERT_TRUE(scenario_runner->isRunning());
    
    // Simulate for communication stress
    for (int i = 0; i < 100; i++) {
        scenario_runner->update();
        delay(50);
    }
    
    // Check communication statistics
    SerialPortSimulator& modbus_sim = scenario_runner->getModbusSimulator();
    uint32_t total_bytes = modbus_sim.getBytesReceived() + modbus_sim.getBytesTransmitted();
    
    // Should have some communication activity
    TEST_ASSERT_GREATER_THAN(0, total_bytes);
    
    scenario_runner->stopScenario();
}

/**
 * @brief Test fault injection scenario
 */
void test_fault_injection_scenario() {
    TEST_ASSERT_TRUE(scenario_runner->initialize());
    
    // Run fault injection scenario
    scenario_runner->runScenario(SCENARIO_FAULT_INJECTION);
    TEST_ASSERT_TRUE(scenario_runner->isRunning());
    
    // Simulate for fault injection
    for (int i = 0; i < 150; i++) {
        scenario_runner->update();
        delay(50);
    }
    
    // Check results
    TestResults results = scenario_runner->getResults();
    TEST_ASSERT_EQUAL(SCENARIO_FAULT_INJECTION, results.scenario);
    
    scenario_runner->stopScenario();
}

/**
 * @brief Test multiple scenarios sequentially
 */
void test_multiple_scenarios() {
    TEST_ASSERT_TRUE(scenario_runner->initialize());
    
    TestScenario scenarios[] = {
        SCENARIO_NORMAL_OPERATION,
        SCENARIO_EMERGENCY_STOP,
        SCENARIO_OVERLOAD_RECOVERY
    };
    
    for (int scenario_idx = 0; scenario_idx < 3; scenario_idx++) {
        scenario_runner->runScenario(scenarios[scenario_idx]);
        TEST_ASSERT_TRUE(scenario_runner->isRunning());
        
        // Run scenario for a short time
        for (int i = 0; i < 30; i++) {
            scenario_runner->update();
            delay(50);
        }
        
        scenario_runner->stopScenario();
        TEST_ASSERT_FALSE(scenario_runner->isRunning());
        
        // Check results
        TestResults results = scenario_runner->getResults();
        TEST_ASSERT_EQUAL(scenarios[scenario_idx], results.scenario);
    }
}

// Test runner
void setup() {
    delay(2000); // Wait for serial connection
    UNITY_BEGIN();
    
    RUN_TEST(test_modbus_simulator_init);
    RUN_TEST(test_modbus_read_holding_registers);
    RUN_TEST(test_modbus_write_single_register);
    RUN_TEST(test_scenario_runner_init);
    RUN_TEST(test_normal_operation_scenario);
    RUN_TEST(test_overload_recovery_scenario);
    RUN_TEST(test_emergency_stop_scenario);
    RUN_TEST(test_communication_stress_scenario);
    RUN_TEST(test_fault_injection_scenario);
    RUN_TEST(test_multiple_scenarios);
    
    UNITY_END();
}

void loop() {
    // Empty loop
}