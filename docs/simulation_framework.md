# Multi-Sonicator Hardware Simulation Framework

This document describes the comprehensive hardware simulation framework for testing the Multi-Sonicator I/O Controller without physical hardware.

## Overview

The simulation framework provides realistic behavior modeling for:
- **CT2000 Sonicator Units**: Individual sonicator simulation with amplitude control, power output, frequency monitoring, and overload conditions
- **MODBUS RTU Communication**: Complete MODBUS protocol implementation with register mapping and error simulation
- **Test Scenarios**: Comprehensive test scenarios covering normal operation, fault conditions, and edge cases

## Components

### 1. Virtual Sonicator (`test/simulation/virtual_sonicator.cpp`)

Simulates the behavior of individual CT2000 sonicator units.

**Key Features:**
- Realistic frequency drift and power output simulation
- Overload condition detection and recovery
- Frequency lock simulation
- Fault injection for testing edge cases
- Runtime statistics tracking

**Usage Example:**
```cpp
VirtualSonicator sonicator(1); // Create sonicator with ID 1
sonicator.setAmplitude(75);    // Set 75% amplitude
sonicator.startSonication();   // Start operation
sonicator.update();            // Update simulation (call periodically)

// Monitor status
bool running = sonicator.getIsRunning();
bool overloaded = sonicator.getIsOverloaded();
uint16_t frequency = sonicator.getFrequency();
uint16_t power = sonicator.getPower();
```

### 2. Serial Port Simulator (`test/simulation/serial_simulator.cpp`)

Provides MODBUS RTU communication simulation.

**Key Features:**
- Complete MODBUS protocol implementation
- CRC validation and error handling
- Register mapping for all Multi-Sonicator registers
- Communication statistics and error simulation
- Configurable response delays

**Usage Example:**
```cpp
SerialPortSimulator modbus(2, 115200); // Slave ID 2, 115200 baud
modbus.open();

// Send MODBUS request
uint8_t request[] = {0x02, 0x03, 0x9C, 0x41, 0x00, 0x01, 0x54, 0x08};
modbus.write(request, sizeof(request));

// Read response
uint8_t response[256];
size_t bytes = modbus.read(response, sizeof(response));
```

### 3. Test Scenario Runner (`test/simulation/test_scenarios.cpp`)

Manages comprehensive test scenarios combining multiple sonicators and MODBUS communication.

**Available Scenarios:**
- `SCENARIO_NORMAL_OPERATION`: Normal 4-sonicator operation
- `SCENARIO_OVERLOAD_RECOVERY`: Overload detection and recovery
- `SCENARIO_FREQUENCY_DRIFT`: Frequency stability testing
- `SCENARIO_MULTI_SONICATOR_SYNC`: Multi-unit synchronization
- `SCENARIO_EMERGENCY_STOP`: Emergency stop response time
- `SCENARIO_POWER_RAMPING`: Power ramping from 20% to 100%
- `SCENARIO_COMMUNICATION_STRESS`: MODBUS communication stress testing
- `SCENARIO_FAULT_INJECTION`: Random fault injection testing

**Usage Example:**
```cpp
TestScenarioRunner runner;
runner.initialize();

// Run normal operation scenario
runner.runScenario(SCENARIO_NORMAL_OPERATION);
while (runner.isRunning()) {
    runner.update();
    delay(10);
}

TestResults results = runner.getResults();
```

## Test Integration

### Unit Tests

**Virtual Sonicator Tests** (`test/test_virtual_sonicator/`)
- Sonicator initialization and state management
- Amplitude control and validation
- Overload detection and recovery
- Fault injection and clearing
- Simulation behavior verification

**Integration Tests** (`test/test_simulation_integration/`)
- MODBUS communication protocol testing
- Multi-sonicator scenario execution
- Communication stress testing
- End-to-end system simulation

### Running Tests

```bash
# Run all virtual sonicator tests
pio test -e test_desktop --filter test_virtual_sonicator

# Run integration tests
pio test -e test_desktop --filter test_simulation_integration

# Run all simulation tests
pio test -e test_desktop --filter test_virtual_sonicator test_simulation_integration

# Run with verbose output
pio test -e test_desktop --filter test_virtual_sonicator --verbose
```

## CI/CD Integration

The simulation framework is integrated into the CI/CD pipeline:

### Hardware Detection
- Automatically detects if physical hardware is available
- Falls back to simulation mode in CI environments
- Configurable simulation parameters

### Test Execution
- Unit tests run in simulation mode by default
- Integration tests use hardware when available
- Nightly tests include extended simulation scenarios

### Error Reporting
- Detailed test failure analysis
- Performance metrics and timing analysis
- Communication statistics and error rates

## Configuration

### Simulation Parameters

**Frequency Simulation:**
- Nominal frequency: 20,000 Hz
- Frequency drift rate: ±0.1 Hz/s
- Lock time: 100-500 ms
- Tolerance: ±20 Hz

**Power Simulation:**
- Power scaling: Linear with amplitude
- Noise level: 5% of theoretical power
- Overload threshold: 95% of maximum power

**Communication Simulation:**
- Response delay: 10 ms (configurable)
- Error simulation: 0-10% error rate
- Buffer sizes: 512 bytes each direction

### Environment Variables

```bash
# Enable simulation mode
export SIMULATION_MODE=1

# Set error simulation rate
export SIMULATION_ERROR_RATE=0.05

# Set response delay
export SIMULATION_DELAY_MS=10
```

## Fault Injection

The framework supports comprehensive fault injection for testing edge cases:

### Fault Types
- `FAULT_OVERLOAD`: Trigger overload condition
- `FAULT_FREQUENCY_DRIFT`: Accelerate frequency drift
- `FAULT_POWER_SPIKE`: Generate power spike
- `FAULT_FREQUENCY_LOCK_LOSS`: Lose frequency lock

### Usage
```cpp
VirtualSonicator sonicator(1);
sonicator.injectFault(FAULT_OVERLOAD);
sonicator.update(); // Fault takes effect
sonicator.clearFaultInjection(); // Clear fault
```

## Performance Metrics

The simulation framework provides comprehensive performance metrics:

### Timing Metrics
- Simulation update frequency
- Response time measurements
- Test execution duration

### Communication Metrics
- MODBUS request/response pairs
- Communication error rates
- Throughput measurements

### System Metrics
- Memory usage tracking
- CPU utilization estimates
- Error event logging

## Advanced Features

### Realistic Behavior Modeling
- Frequency lock acquisition time
- Power output settling time
- Overload recovery procedures
- Multi-sonicator interference effects

### Statistical Analysis
- Runtime statistics collection
- Failure rate analysis
- Performance trend monitoring
- Reliability metrics

### Debugging Support
- Detailed logging output
- State machine visualization
- Communication protocol analysis
- Error condition replay

## Integration with Physical Hardware

The simulation framework is designed to seamlessly integrate with physical hardware:

### Hardware Detection
- Automatic hardware presence detection
- Seamless fallback to simulation
- Hybrid testing capabilities

### Compatibility
- Identical API for real and virtual hardware
- Consistent test results
- Parallel testing capability

## Best Practices

### Test Development
1. Start with unit tests for individual components
2. Use integration tests for multi-component scenarios
3. Include both positive and negative test cases
4. Test edge cases and error conditions
5. Validate performance requirements

### Simulation Accuracy
1. Calibrate simulation parameters with real hardware
2. Validate timing characteristics
3. Test communication protocols thoroughly
4. Include realistic noise and drift
5. Model hardware limitations accurately

### CI/CD Integration
1. Use simulation for rapid feedback
2. Include hardware tests in nightly builds
3. Generate comprehensive test reports
4. Track performance metrics over time
5. Automate regression testing

## Troubleshooting

### Common Issues
1. **Simulation not starting**: Check PlatformIO environment setup
2. **Tests failing**: Verify simulation parameters and timing
3. **Communication errors**: Check MODBUS frame construction
4. **Performance issues**: Adjust simulation update frequency

### Debug Mode
Enable debug output for detailed simulation information:
```bash
export DEBUG_SIMULATION=1
pio test -e test_desktop --filter test_virtual_sonicator --verbose
```

### Log Analysis
The simulation framework generates detailed logs for analysis:
- Simulation state changes
- Communication transactions
- Error conditions and recovery
- Performance measurements

## Future Enhancements

### Planned Features
- Real-time visualization of simulation state
- Web-based simulation control interface
- Advanced fault injection scenarios
- Performance regression testing
- Multi-platform simulation support

### Extension Points
- Custom sonicator models
- Additional communication protocols
- External hardware interfaces
- Cloud-based simulation services

## Conclusion

The Multi-Sonicator hardware simulation framework provides comprehensive testing capabilities without requiring physical hardware. It enables rapid development cycles, thorough testing coverage, and reliable CI/CD pipeline execution while maintaining compatibility with real hardware systems.