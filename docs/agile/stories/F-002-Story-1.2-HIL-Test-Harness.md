# Story 1.2: HIL Test Harness Implementation

**Story ID**: S-1.2
**Feature**: F-002: HIL Testing Framework & Hardware Verification
**Priority**: Critical
**Owner**: Product Owner (Sarah)
**Created**: 2025-09-10
**Updated**: 2025-09-10

## Status

Draft

## Story

**As a** firmware test engineer,
**I want** a comprehensive Hardware-in-the-Loop test harness with Python controller,
**so that** I can execute automated hardware validation tests and simulate real-world operating conditions for the Multi-Sonicator system.

## Feature Context

**Feature F-002: HIL Testing Framework & Hardware Verification**

- Goal: Establish comprehensive Hardware-in-the-Loop testing framework and validate all hardware connections
- Enables automated testing of firmware against actual hardware in controlled conditions

## Story Context

**HIL Test Harness Architecture:**

- Python HIL controller interfaces with ATmega32A via Arduino Test Wrapper
- Real-time hardware signal monitoring and control capabilities
- Automated test execution with comprehensive logging and reporting
- Integration with BDD acceptance testing framework

**Integration Points:**

- Arduino Test Wrapper as hardware interface bridge
- Python-based test controller for automation
- ATmega32A target hardware for validation
- Test logging and reporting systems

## Acceptance Criteria

**HIL Controller Requirements:**

1. Python HIL controller successfully interfaces with ATmega32A via Arduino Test Wrapper
2. HIL controller can control all sonicator interface signals (START_X, RST_X, PWM amplitude)
3. Real-time monitoring of all hardware signals with <100ms latency
4. HIL controller integrates with `make test-acceptance` for automated execution

**Hardware Simulation Capabilities:**

5. CT2000 sonicator interface simulation (FREQ_DIV10_X signals, power feedback, overload conditions)
6. MODBUS master simulation for communication testing and register validation
7. Power monitoring simulation with configurable power levels (0-10.88V range)
8. Signal injection capabilities for testing error conditions and edge cases

**Test Execution and Reporting:**

9. Automated test sequences execute without manual intervention
10. Comprehensive test logging with timestamped hardware state capture
11. Test results include signal integrity analysis and timing validation
12. HIL test execution completes within <10 minutes for full validation suite

## Technical Implementation Details

### HIL Test Harness Architecture

```python
# test/integration/hil_test_harness.py
class HILTestHarness:
    def __init__(self):
        self.arduino_wrapper = ArduinoTestWrapper()
        self.modbus_master = MODBUSMasterSimulator()
        self.signal_monitor = SignalMonitor()
        
    def execute_hil_test_suite(self):
        """Execute complete HIL validation suite"""
        self.initialize_hardware()
        self.run_sonicator_tests()
        self.run_modbus_tests()
        self.run_power_monitoring_tests()
        self.generate_test_report()
```

### Hardware Interface Control

**Arduino Test Wrapper Interface:**
- Serial communication for command/response protocol
- Real-time signal control and monitoring
- Hardware state synchronization
- Error detection and recovery

**Sonicator Interface Simulation:**
- FREQ_DIV10_X signal generation for frequency monitoring
- Power level simulation via voltage dividers
- Overload condition simulation via optocoupler control
- CT2000 interface behavior modeling

### Make Target Integration

```bash
make hardware-sandbox    # Start HIL test environment
make test-hil           # Execute HIL test suite
make monitor-device     # Monitor hardware signals in real-time
```

## Definition of Done

- [ ] Python HIL controller interfaces successfully with Arduino Test Wrapper
- [ ] HIL controller controls all sonicator interface signals
- [ ] Real-time hardware signal monitoring operational (<100ms latency)
- [ ] HIL controller integrates with make test-acceptance target
- [ ] CT2000 sonicator interface simulation functional
- [ ] MODBUS master simulation operational for communication testing
- [ ] Power monitoring simulation with configurable levels
- [ ] Signal injection capabilities for error condition testing
- [ ] Automated test sequences execute without manual intervention
- [ ] Comprehensive test logging with timestamped state capture
- [ ] Signal integrity analysis and timing validation included
- [ ] HIL test execution completes within <10 minutes
- [ ] Ready for BDD acceptance testing framework integration (Story 1.3)

## Tasks / Subtasks

- [ ] **Task 1: Python HIL Controller Foundation** (AC: 1, 3)
  - [ ] Implement Python HIL controller with Arduino Test Wrapper interface
  - [ ] Establish serial communication protocol for hardware control
  - [ ] Implement real-time signal monitoring with <100ms latency

- [ ] **Task 2: Sonicator Interface Control** (AC: 2, 5)
  - [ ] Implement control of START_X and RST_X signals via HIL controller
  - [ ] Implement PWM amplitude control and verification
  - [ ] Create CT2000 sonicator interface simulation capabilities

- [ ] **Task 3: MODBUS and Communication Testing** (AC: 6)
  - [ ] Implement MODBUS master simulation for communication testing
  - [ ] Create register validation and communication protocol testing
  - [ ] Integrate communication testing with HIL framework

- [ ] **Task 4: Power Monitoring and Signal Simulation** (AC: 7, 8)
  - [ ] Implement power monitoring simulation (0-10.88V range)
  - [ ] Create signal injection capabilities for error conditions
  - [ ] Develop edge case and fault condition testing

- [ ] **Task 5: Test Automation and Integration** (AC: 4, 9)
  - [ ] Integrate HIL controller with make test-acceptance
  - [ ] Implement automated test sequences without manual intervention
  - [ ] Create test orchestration and execution framework

- [ ] **Task 6: Logging, Reporting, and Performance** (AC: 10, 11, 12)
  - [ ] Implement comprehensive test logging with timestamps
  - [ ] Create signal integrity analysis and timing validation
  - [ ] Optimize HIL test execution for <10 minute completion

## Dev Notes

### HIL Test Harness Architecture

**Controller Location**: `test/integration/hil_test_harness.py`
**Arduino Wrapper**: `test/integration/arduino_test_wrapper.py`

**Hardware Interface**: Arduino Test Wrapper provides bridge between Python controller and ATmega32A hardware
**Communication Protocol**: Serial-based command/response for real-time control

**Test Categories:**
1. **Hardware Interface Tests**: Pin control, signal generation, monitoring
2. **Sonicator Simulation Tests**: CT2000 interface behavior validation
3. **Communication Tests**: MODBUS protocol and register validation
4. **Power Monitoring Tests**: ADC accuracy and power measurement validation

### Testing Standards

**Test File Location**: `test/integration/test_hil_*.py`

**Testing Framework**: pytest with Hardware-in-the-Loop capabilities

**Testing Requirements:**
- Real-time hardware control and monitoring
- Automated test execution without manual intervention
- Comprehensive logging and state capture
- Signal integrity and timing analysis
- Performance validation (<10 minutes execution)

**Specific Testing Patterns:**
- Hardware state validation with expected vs actual comparison
- Signal timing analysis with tolerance checking
- Communication protocol validation with known message sequences
- Error condition injection and recovery validation

### Hardware Dependencies

**Required Hardware:**
- ATmega32A target hardware with complete pin matrix connectivity
- Arduino Test Wrapper configured as hardware interface bridge
- Signal monitoring capabilities for real-time analysis

**Interface Requirements:**
- Serial communication between Python controller and Arduino Wrapper
- Hardware signal control and monitoring capabilities
- CT2000 sonicator interface simulation hardware

## Dependencies

**Prerequisites:**
- Story 1.1: Hardware Verification & Pin Matrix Validation completed
- Arduino Test Wrapper setup and validated
- Python testing environment configured
- Hardware interface connectivity established

**Enables Next Stories:**
- Story 1.3: BDD Acceptance Testing Framework
- Story 1.4: Web UI for Testing & Monitoring

## Change Log

| Date | Version | Description | Author |
|------|---------|-------------|---------|
| 2025-09-10 | 1.0 | Initial story creation for F-002 | Product Owner (Sarah) |
