# PlatformIO Embedded Testing Standard

> **Note:** This document defines the official testing standard for all PlatformIO embedded applications within the organization.

## Overview

PlatformIO embedded applications require specialized testing approaches that account for hardware constraints, real-time requirements, and cross-platform microcontroller compatibility. This standard defines the **official testing requirements** for all PlatformIO projects, emphasizing Unity Test Framework with hardware-in-the-loop (HIL) testing capabilities.

---

## Framework Requirements

### Primary Testing Framework

- **Foundation**: Unity Test Framework for ALL embedded testing
- **Platform**: PlatformIO's built-in testing infrastructure
- **Coverage**: Unit tests, integration tests, and hardware-in-the-loop testing

### Mocking Framework (Unit Tests ONLY)

- **Standard**: Unity's built-in mocking capabilities or CMock
- **Scope**: **MOCKING IS ONLY TO BE USED FOR UNIT TESTING**
- **Prohibition**: Integration tests and HIL tests must NOT use mocking
- **Rationale**: Hardware integration tests must validate real hardware behavior

### Specialized Frameworks

- **Hardware Testing**: Hardware-in-the-loop (HIL) testing for real hardware validation
- **Emulation Testing**: QEMU or Renode for hardware emulation when physical hardware unavailable
- **Performance Testing**: Real-time performance validation and timing analysis

---

## Testing Architecture

### Test Organization Structure

```markdown
test/
├── unit/                    # Unit tests (70% of test effort)
│   ├── drivers/            # Hardware driver unit tests
│   ├── protocols/          # Communication protocol tests
│   ├── algorithms/         # Algorithm and logic tests
│   └── utils/              # Utility function tests
├── integration/            # Integration tests (20% of test effort)
│   ├── hardware/           # Hardware component integration
│   ├── communication/      # Inter-component communication
│   └── system/             # System-level integration
├── hil/                    # Hardware-in-the-loop tests (10% of test effort)
│   ├── sensors/            # Sensor validation tests
│   ├── actuators/          # Actuator control tests
│   └── protocols/          # Communication protocol validation
└── support/                # Test support files
    ├── mocks/              # Mock definitions (unit tests only)
    ├── fixtures/           # Test data and configurations
    └── helpers/            # Common test utilities
```

### Testing Pyramid Distribution

- **Unit Tests**: 70% - Algorithms, drivers, protocols, utilities
- **Integration Tests**: 20% - Component interactions, system integration
- **HIL Tests**: 10% - Critical hardware validation, real-world scenarios

---

## Framework-Specific Standards

### 1. Unit Testing

- **Framework**: Unity Test Framework (mocking permitted)
- **Coverage Requirement**: ≥90% statement coverage
- **Scope**: Algorithms, drivers, protocols, utilities
- **Mocking**: Permitted for hardware abstractions and external dependencies
- **Location**: `test/unit/`

```c
// Example: Unit test with mocking
#include "unity.h"
#include "mock_gpio.h"
#include "led_driver.h"

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

void test_led_driver_should_turn_on_led(void) {
    // Arrange
    gpio_write_ExpectAndReturn(LED_PIN, HIGH, GPIO_OK);
    
    // Act
    led_result_t result = led_turn_on();
    
    // Assert
    TEST_ASSERT_EQUAL(LED_OK, result);
}

void test_led_driver_should_handle_gpio_failure(void) {
    // Arrange
    gpio_write_ExpectAndReturn(LED_PIN, HIGH, GPIO_ERROR);
    
    // Act
    led_result_t result = led_turn_on();
    
    // Assert
    TEST_ASSERT_EQUAL(LED_ERROR, result);
}
```

### 2. Integration Testing

- **Framework**: Unity Test Framework (NO mocking)
- **Coverage Requirement**: ≥80% integration coverage
- **Scope**: Component interactions, system integration
- **Mocking**: **PROHIBITED** - Test real component interactions
- **Location**: `test/integration/`

```c
// Example: Integration test without mocking
#include "unity.h"
#include "sensor_manager.h"
#include "data_processor.h"
#include "communication.h"

void test_sensor_to_communication_integration(void) {
    // Arrange
    sensor_config_t config = {
        .sample_rate = 100,
        .precision = SENSOR_HIGH_PRECISION
    };
    
    // Act
    sensor_manager_init(&config);
    sensor_data_t data = sensor_manager_read();
    processed_data_t processed = data_processor_process(&data);
    comm_result_t result = communication_send(&processed);
    
    // Assert
    TEST_ASSERT_EQUAL(COMM_SUCCESS, result);
    TEST_ASSERT_TRUE(data.timestamp > 0);
    TEST_ASSERT_TRUE(processed.checksum_valid);
}
```

### 3. Hardware-in-the-Loop (HIL) Testing

- **Framework**: Unity Test Framework with real hardware
- **Scope**: Hardware validation, sensor/actuator testing, real-world scenarios
- **Mocking**: **PROHIBITED** - Test actual hardware behavior
- **Location**: `test/hil/`

```c
// Example: HIL test
#include "unity.h"
#include "temperature_sensor.h"
#include "heater_control.h"

void test_temperature_control_loop_hil(void) {
    // Arrange - Real hardware setup
    temp_sensor_init();
    heater_init();
    
    float target_temp = 25.0;
    float tolerance = 0.5;
    
    // Act - Real hardware control loop
    heater_set_target(target_temp);
    
    // Wait for system to stabilize (real-time testing)
    delay_ms(5000);
    
    // Assert - Validate real sensor readings
    float actual_temp = temp_sensor_read();
    TEST_ASSERT_FLOAT_WITHIN(tolerance, target_temp, actual_temp);
    
    // Cleanup
    heater_disable();
}
```

### 4. Emulation Testing

- **Framework**: QEMU or Renode emulation
- **Scope**: Hardware emulation when physical hardware unavailable
- **Mocking**: **PROHIBITED** - Use hardware emulation instead
- **Location**: `test/emulation/`

```ini
; platformio.ini - Emulation test environment
[env:emulation_test]
platform = native
framework = unity
test_framework = unity
test_filter = test_emulation
build_flags = 
    -D EMULATION_MODE
    -D UNITY_INCLUDE_CONFIG_H
```

---

## Coverage Requirements

| Test Type | Minimum Coverage | Enforcement | Execution |
|-----------|------------------|-------------|-----------|
| Unit Tests | 90% | CI/CD Pipeline | Automated (CI) |
| Integration Tests | 80% | Manual Review | Manual Only |
| HIL Tests | Critical Hardware Paths | Manual Review | Manual Only |
| Overall Project | 85% | Release Review | Combined CI + Manual |

### Pipeline vs Manual Testing

**CI Pipeline (Automated):**
- Unit tests with 90% coverage requirement
- Executive report generation (unit tests only)
- Coverage validation and reporting
- BDD syntax validation (no execution)

**Manual Testing (Required for Release):**
- Acceptance tests with BDD scenario execution
- Hardware-in-the-loop (HIL) testing
- Integration testing with real hardware
- Executive report generation (full test suite)

---

## Make Targets for PlatformIO

PlatformIO projects must implement these standardized make targets:

```bash
# CI Pipeline (Unit Tests Only)
make ci
# - Executes: Unit tests + coverage + executive report generation
# - Coverage: ≥90% requirement enforced
# - Hardware: Not required (CI environment)
# - Reports: Executive report, coverage summary (unit tests only)

# Unit Testing (with mocking allowed)
make test-unit
# - Executes: pio test --filter test_unit
# - Coverage: ≥90% requirement
# - Mocking: Unity/CMock permitted

# Integration Testing (no mocking) - MANUAL ONLY
make test-integration
# - Executes: pio test --filter test_integration
# - Coverage: ≥80% requirement
# - Mocking: PROHIBITED
# - Execution: Manual only (not in CI pipeline)

# Acceptance Testing (BDD) - MANUAL ONLY
make test-acceptance
# - Executes: Behave BDD scenarios with HIL hardware
# - Scope: BDD scenario validation with real hardware
# - Mocking: PROHIBITED
# - Execution: Manual only (not in CI pipeline)

# Hardware-in-the-Loop Testing - MANUAL ONLY
make test-hil
# - Executes: pio test --filter test_hil --environment hil
# - Scope: Real hardware validation
# - Mocking: PROHIBITED
# - Execution: Manual only (not in CI pipeline)

# Complete Manual Test Suite (Release Testing)
make test
# - Executes: All test types with real hardware
# - Includes: Unit + Integration + Acceptance + HIL
# - Reports: Complete executive report + acceptance reports
# - Execution: Manual only (for release validation)
```

### CI vs Manual Testing Targets

**CI Pipeline Targets (Automated):**
- `make ci` - Complete CI pipeline (unit tests only)
- `make test-unit` - Individual unit test execution
- `make validate-config` - Configuration validation

**Manual Testing Targets (Hardware Required):**
- `make test-acceptance` - BDD acceptance testing
- `make test-integration` - Integration testing  
- `make test-hil` - Hardware-in-the-loop testing
- `make test` - Complete test suite for releases

---

## Hardware Detection and Fallback

### Automatic Hardware Detection

```c
// Example: Hardware detection in test setup
void setUp(void) {
    if (hardware_available()) {
        // Run HIL tests with real hardware
        hardware_init();
    } else {
        // Fall back to emulation
        emulation_init();
        TEST_MESSAGE("Hardware not available, using emulation");
    }
}
```

### CI/CD Integration

**CI Pipeline Configuration (Unit Tests Only):**

- **CI/CD Automated**: Run unit tests only (with 90% coverage validation)
- **CI/CD Automated**: BDD syntax validation (no execution)
- **Hardware Independence**: CI pipeline does not require physical hardware
- **Fast Feedback**: Quick validation for pull requests and development workflow

**Manual Testing (Hardware Required):**

- **Acceptance Tests**: Manual execution of BDD scenarios with HIL hardware
- **Hardware Integration**: Manual HIL test suite execution
- **Performance Validation**: Manual real-time performance testing
- **Release Validation**: Manual comprehensive test suite before releases

**Hardware Availability Strategy:**

- **CI Pipeline**: Unit tests + emulation (no hardware required)
- **Manual Execution**: Acceptance + HIL tests (hardware required)
- **Coverage Validation**: Unit test coverage enforced in CI, manual test results included in release reports

---

## Prohibited Practices

### Strictly Forbidden

- ❌ Mocking hardware in integration tests (use emulation instead)
- ❌ Mocking hardware in HIL tests (defeats purpose of hardware validation)
- ❌ Hardcoded timing values (use configurable test parameters)
- ❌ Tests that depend on specific hardware serial numbers
- ❌ Tests that modify production firmware without restoration

### Required Practices

- ✅ Use Unity Test Framework as foundation
- ✅ Mock only external dependencies in unit tests
- ✅ Use hardware emulation when physical hardware unavailable
- ✅ Implement proper test isolation and cleanup
- ✅ Validate real-time constraints in HIL tests

---

## Real-Time Testing Considerations

### Timing Validation

```c
// Example: Real-time constraint testing
void test_interrupt_response_time(void) {
    // Arrange
    uint32_t start_time, end_time, response_time;
    uint32_t max_response_time_us = 100; // 100 microseconds max
    
    // Act
    start_time = get_microseconds();
    trigger_interrupt();
    wait_for_interrupt_handled();
    end_time = get_microseconds();
    
    // Assert
    response_time = end_time - start_time;
    TEST_ASSERT_LESS_THAN(max_response_time_us, response_time);
}
```

### Performance Testing

- **Interrupt Latency**: Validate interrupt response times
- **Memory Usage**: Test memory allocation and deallocation
- **Power Consumption**: Validate power usage in different modes
- **Communication Timing**: Test protocol timing requirements

---

## Cross-Platform Considerations

### Multi-Target Testing

```ini
; platformio.ini - Multiple target environments
[env:esp32_test]
platform = espressif32
board = esp32dev
framework = arduino
test_framework = unity

[env:stm32_test]
platform = ststm32
board = nucleo_f401re
framework = stm32cube
test_framework = unity

[env:avr_test]
platform = atmelavr
board = uno
framework = arduino
test_framework = unity
```

### Platform-Specific Testing

- **ESP32**: WiFi, Bluetooth, dual-core testing
- **STM32**: Real-time performance, peripheral testing
- **AVR**: Memory constraints, timing-critical operations
- **ARM Cortex**: Advanced debugging, trace analysis

---

## Quality Gates for PlatformIO

All PlatformIO stories must pass these quality gates:

- [ ] Unit tests achieve ≥90% coverage (mocking permitted for external dependencies)
- [ ] Integration tests achieve ≥80% coverage (NO mocking)
- [ ] HIL tests validate critical hardware functionality (NO mocking)
- [ ] Real-time constraints validated in timing-critical tests
- [ ] Cross-platform compatibility verified on target hardware
- [ ] Memory usage within acceptable limits
- [ ] Power consumption meets requirements

---

✅ **This standard is authoritative for all PlatformIO embedded applications. Compliance is mandatory and enforced through CI/CD pipelines.**
