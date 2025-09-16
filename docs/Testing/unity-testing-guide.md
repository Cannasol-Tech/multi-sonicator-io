# Complete Guide: ATMEGA32A Unit Testing with Unity Framework and PlatformIO

## Table of Contents
1. [Overview](#overview)
2. [ATMEGA32A Specifications](#atmega32a-specifications)
3. [Project Structure](#project-structure)
4. [PlatformIO Configuration](#platformio-configuration)
5. [Unity Framework Setup](#unity-framework-setup)
6. [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal)
7. [Test Organization](#test-organization)
8. [Mock Objects and Test Doubles](#mock-objects-and-test-doubles)
9. [Memory Management](#memory-management)
10. [CI/CD Integration](#cicd-integration)
11. [Best Practices](#best-practices)
12. [Example Implementation](#example-implementation)

## Overview

The Unity Test Framework is a lightweight, pure C unit testing framework specifically designed for embedded systems. It's ideal for the ATMEGA32A due to its minimal memory footprint and compatibility with 8-bit microcontrollers. PlatformIO provides seamless integration with Unity, making it the perfect development environment for test-driven embedded development.

### Key Benefits
- **Minimal Memory Footprint**: Unity works even on 8-bit AVR MCUs with limited resources
- **Cross-Platform Testing**: Run tests both on native host and target hardware
- **Arduino Framework Compatibility**: Full support for Arduino-based projects
- **Automated Testing**: Integration with CI/CD pipelines

## ATMEGA32A Specifications

### Memory Constraints
- **Flash Memory**: 32KB (for program storage)
- **SRAM**: 2KB (for runtime data)
- **EEPROM**: 1KB (for persistent data storage)

These constraints require careful test organization and potentially splitting tests into multiple builds to fit within memory limits.

## Project Structure

```
project_root/
├── platformio.ini
├── src/
│   ├── main.cpp
│   └── modules/
│       ├── led_controller.cpp
│       ├── led_controller.h
│       ├── sensor_reader.cpp
│       └── sensor_reader.h
├── lib/
│   └── hal/
│       ├── hardware_abstraction.h
│       └── hardware_abstraction.cpp
├── test/
│   ├── test_native/
│   │   ├── test_algorithms.cpp
│   │   └── unity_system_config.h
│   └── test_embedded/
│       ├── test_hardware/
│       │   ├── test_led_controller.cpp
│       │   └── test_sensor_reader.cpp
│       └── unity_system_config.h
└── include/
    └── project_system_config.h
```

## PlatformIO Configuration

### platformio.ini

```ini
; PlatformIO Project Configuration File

[platformio]
default_envs = atmega32_dev

; Shared configuration
[env]
platform = atmelavr
framework = arduino
test_framework = unity
monitor_speed = 115200

; Development environment for ATMEGA32A
[env:atmega32_dev]
board = ATmega32
board_build.f_cpu = 16000000L
upload_protocol = usbasp
upload_flags = -Pusb
build_flags = 
    -D DEBUG_MODE
    -D F_CPU=16000000L
    -Wall
    -Wextra
test_build_src = yes

; Test environment for target hardware
[env:atmega32_test]
board = ATmega32
board_build.f_cpu = 16000000L
upload_protocol = usbasp
build_flags = 
    -D UNIT_TEST
    -D F_CPU=16000000L
    -Os  ; Optimize for size
test_port = /dev/ttyUSB0
test_speed = 115200

; Native test environment for host machine
[env:native_test]
platform = native
build_flags = 
    -D UNIT_TEST
    -D NATIVE_TEST
    -std=c++11
test_build_src = no
lib_compat_mode = off

; Memory-constrained test environment (splits tests)
[env:atmega32_test_suite1]
extends = env:atmega32_test
test_filter = test_embedded/test_hardware/test_led*
build_flags = 
    ${env:atmega32_test.build_flags}
    -D TEST_SUITE_1

[env:atmega32_test_suite2]
extends = env:atmega32_test
test_filter = test_embedded/test_hardware/test_sensor*
build_flags = 
    ${env:atmega32_test.build_flags}
    -D TEST_SUITE_2
```

## Unity Framework Setup

### Custom Unity Configuration (test/unity_system_config.h)

```c
#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

// Memory optimization for ATMEGA32A
#define UNITY_EXCLUDE_FLOAT
#define UNITY_EXCLUDE_DOUBLE
#define UNITY_EXCLUDE_FLOAT_PRINT

// Use 16-bit integers for line numbers to save memory
#define UNITY_LINE_TYPE uint16_t

// Custom output for Arduino Serial
#ifdef ARDUINO
    #include <Arduino.h>
    #define UNITY_OUTPUT_START()    Serial.begin(115200)
    #define UNITY_OUTPUT_CHAR(a)    Serial.write(a)
    #define UNITY_OUTPUT_FLUSH()    Serial.flush()
    #define UNITY_OUTPUT_COMPLETE() Serial.end()
#else
    // Native platform uses stdio
    #include <stdio.h>
    #define UNITY_OUTPUT_CHAR(a)    putchar(a)
    #define UNITY_OUTPUT_FLUSH()    fflush(stdout)
#endif

// Memory constraints
#define UNITY_MAX_ARRAY_LENGTH 32
#define UNITY_SUPPORT_64 0

// Custom assertions for embedded
#define TEST_ASSERT_BITS(mask, expected, actual) \
    TEST_ASSERT_EQUAL_HEX((expected) & (mask), (actual) & (mask))

#endif // UNITY_CONFIG_H
```

## Hardware Abstraction Layer (HAL)

### Base HAL Interface (lib/hal/hardware_abstraction.h)

```cpp
#ifndef HARDWARE_ABSTRACTION_H
#define HARDWARE_ABSTRACTION_H

#include <stdint.h>
#include <stdbool.h>

// GPIO abstraction
class IGpioPort {
public:
    virtual ~IGpioPort() = default;
    virtual void setPinMode(uint8_t pin, uint8_t mode) = 0;
    virtual void digitalWrite(uint8_t pin, uint8_t value) = 0;
    virtual uint8_t digitalRead(uint8_t pin) = 0;
};

// ADC abstraction
class IAdcReader {
public:
    virtual ~IAdcReader() = default;
    virtual void init() = 0;
    virtual uint16_t read(uint8_t channel) = 0;
};

// Timer abstraction
class ITimer {
public:
    virtual ~ITimer() = default;
    virtual void start(uint32_t periodMs) = 0;
    virtual void stop() = 0;
    virtual void attachInterrupt(void (*callback)()) = 0;
};

// Factory for creating HAL instances
class HardwareFactory {
public:
    static IGpioPort* createGpioPort();
    static IAdcReader* createAdcReader();
    static ITimer* createTimer();
};

#endif // HARDWARE_ABSTRACTION_H
```

### Mock Implementation for Testing

```cpp
#ifndef MOCK_HARDWARE_H
#define MOCK_HARDWARE_H

#include "hardware_abstraction.h"
#include <map>
#include <vector>

class MockGpioPort : public IGpioPort {
private:
    std::map<uint8_t, uint8_t> pinModes;
    std::map<uint8_t, uint8_t> pinStates;
    std::vector<std::pair<uint8_t, uint8_t>> writeHistory;

public:
    void setPinMode(uint8_t pin, uint8_t mode) override {
        pinModes[pin] = mode;
    }
    
    void digitalWrite(uint8_t pin, uint8_t value) override {
        pinStates[pin] = value;
        writeHistory.push_back({pin, value});
    }
    
    uint8_t digitalRead(uint8_t pin) override {
        return pinStates[pin];
    }
    
    // Test helpers
    void setReadValue(uint8_t pin, uint8_t value) {
        pinStates[pin] = value;
    }
    
    bool wasWritten(uint8_t pin, uint8_t value) {
        for (auto& write : writeHistory) {
            if (write.first == pin && write.second == value) {
                return true;
            }
        }
        return false;
    }
    
    void reset() {
        pinModes.clear();
        pinStates.clear();
        writeHistory.clear();
    }
};

class MockAdcReader : public IAdcReader {
private:
    std::map<uint8_t, uint16_t> channelValues;
    bool initialized = false;

public:
    void init() override {
        initialized = true;
    }
    
    uint16_t read(uint8_t channel) override {
        if (!initialized) return 0;
        return channelValues[channel];
    }
    
    // Test helpers
    void setChannelValue(uint8_t channel, uint16_t value) {
        channelValues[channel] = value;
    }
    
    bool isInitialized() { return initialized; }
};

#endif // MOCK_HARDWARE_H
```

## Test Organization

### Module Test Example (test/test_embedded/test_hardware/test_led_controller.cpp)

```cpp
#include <unity.h>
#include "led_controller.h"
#include "mock_hardware.h"

// Test fixtures
static MockGpioPort* mockGpio;
static LedController* controller;

void setUp(void) {
    mockGpio = new MockGpioPort();
    controller = new LedController(mockGpio);
}

void tearDown(void) {
    delete controller;
    delete mockGpio;
}

void test_led_init_sets_pin_mode() {
    controller->init(13);
    TEST_ASSERT_TRUE(mockGpio->wasWritten(13, OUTPUT));
}

void test_led_on_writes_high() {
    controller->init(13);
    controller->turnOn();
    TEST_ASSERT_TRUE(mockGpio->wasWritten(13, HIGH));
}

void test_led_off_writes_low() {
    controller->init(13);
    controller->turnOff();
    TEST_ASSERT_TRUE(mockGpio->wasWritten(13, LOW));
}

void test_led_toggle_changes_state() {
    controller->init(13);
    controller->turnOn();
    controller->toggle();
    TEST_ASSERT_TRUE(mockGpio->wasWritten(13, LOW));
}

void test_led_blink_pattern() {
    controller->init(13);
    uint8_t pattern[] = {1, 0, 1, 1, 0};
    
    for (int i = 0; i < 5; i++) {
        controller->setState(pattern[i]);
        TEST_ASSERT_TRUE(mockGpio->wasWritten(13, pattern[i]));
    }
}

// Arduino framework entry point
#ifdef ARDUINO
void setup() {
    delay(2000);  // Wait for serial connection
    UNITY_BEGIN();
    
    RUN_TEST(test_led_init_sets_pin_mode);
    RUN_TEST(test_led_on_writes_high);
    RUN_TEST(test_led_off_writes_low);
    RUN_TEST(test_led_toggle_changes_state);
    RUN_TEST(test_led_blink_pattern);
    
    UNITY_END();
}

void loop() {
    // Empty loop for Arduino
}

// Native platform entry point
#else
int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_led_init_sets_pin_mode);
    RUN_TEST(test_led_on_writes_high);
    RUN_TEST(test_led_off_writes_low);
    RUN_TEST(test_led_toggle_changes_state);
    RUN_TEST(test_led_blink_pattern);
    
    return UNITY_END();
}
#endif
```

## Memory Management

### Memory-Aware Test Strategy

```cpp
// test/test_memory_aware.cpp
#include <unity.h>
#include <avr/pgmspace.h>

// Store test data in program memory to save RAM
const char testString[] PROGMEM = "Test Data";

class MemoryOptimizedTest {
private:
    // Use bit fields to save memory
    struct Flags {
        uint8_t initialized : 1;
        uint8_t error : 1;
        uint8_t state : 3;
        uint8_t reserved : 3;
    } flags;
    
    // Reuse buffers for different tests
    static uint8_t sharedBuffer[32];
    
public:
    void setUp() {
        memset(&flags, 0, sizeof(flags));
        memset(sharedBuffer, 0, sizeof(sharedBuffer));
    }
    
    void runCompactTest() {
        // Use stack allocation for temporary data
        uint8_t localData[8];
        
        // Process data in chunks to minimize memory usage
        for (uint8_t i = 0; i < 8; i++) {
            localData[i] = processChunk(i);
        }
        
        TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, localData, 8);
    }
    
    uint8_t processChunk(uint8_t index) {
        // Process one chunk at a time
        return sharedBuffer[index] * 2;
    }
};

uint8_t MemoryOptimizedTest::sharedBuffer[32];
```

## CI/CD Integration

### GitHub Actions Workflow (.github/workflows/test.yml)

```yaml
name: PlatformIO CI

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  test-native:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    
    - name: Cache PlatformIO
      uses: actions/cache@v3
      with:
        path: |
          ~/.cache/pip
          ~/.platformio/.cache
        key: ${{ runner.os }}-pio
    
    - name: Setup Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.11'
    
    - name: Install PlatformIO
      run: |
        pip install --upgrade platformio
        pio pkg update
    
    - name: Run Native Tests
      run: pio test -e native_test
    
    - name: Generate Coverage Report
      run: |
        pio test -e native_test --coverage
        
  build-embedded:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        environment: [atmega32_dev, atmega32_test]
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Cache PlatformIO
      uses: actions/cache@v3
      with:
        path: |
          ~/.cache/pip
          ~/.platformio/.cache
        key: ${{ runner.os }}-pio
    
    - name: Setup Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.11'
    
    - name: Install PlatformIO
      run: pip install --upgrade platformio
    
    - name: Build ${{ matrix.environment }}
      run: pio run -e ${{ matrix.environment }}
    
    - name: Check Code Quality
      run: pio check -e ${{ matrix.environment }}
    
    - name: Upload Firmware
      uses: actions/upload-artifact@v3
      with:
        name: firmware-${{ matrix.environment }}
        path: .pio/build/${{ matrix.environment }}/firmware.hex
```

## Best Practices

### 1. Test Organization Patterns

```cpp
// Use descriptive test names following a pattern
void test_ModuleName_FunctionName_Condition_ExpectedResult() {
    // Arrange
    setupTestConditions();
    
    // Act
    performAction();
    
    // Assert
    verifyExpectedOutcome();
}
```

### 2. Memory-Efficient Testing

```cpp
class TestSuite {
public:
    // Run tests sequentially to minimize memory usage
    static void runMemoryConstrainedTests() {
        #ifdef TEST_SUITE_1
        runLedTests();
        #endif
        
        #ifdef TEST_SUITE_2
        runSensorTests();
        #endif
        
        #ifdef TEST_SUITE_3
        runCommunicationTests();
        #endif
    }
    
private:
    // Group related tests
    static void runLedTests() {
        TestLedController tests;
        tests.runAll();
    }
};
```

### 3. Dependency Injection Pattern

```cpp
class SensorManager {
private:
    IAdcReader* adcReader;
    ITimer* timer;
    
public:
    // Constructor injection for testability
    SensorManager(IAdcReader* adc, ITimer* tmr) 
        : adcReader(adc), timer(tmr) {}
    
    uint16_t readSensor(uint8_t channel) {
        return adcReader->read(channel);
    }
};
```

### 4. Test Data Management

```cpp
// Store test data efficiently
namespace TestData {
    // Use PROGMEM for constant data on AVR
    const uint8_t expectedPattern[] PROGMEM = {
        0xAA, 0x55, 0xFF, 0x00
    };
    
    // Compact test vectors
    struct TestVector {
        uint8_t input;
        uint8_t expected;
    };
    
    const TestVector testCases[] PROGMEM = {
        {0, 0},
        {1, 2},
        {2, 4},
        {3, 6}
    };
}
```

## Example Implementation

### Complete LED Controller Module

```cpp
// src/modules/led_controller.h
#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "hardware_abstraction.h"

class LedController {
private:
    IGpioPort* gpio;
    uint8_t pin;
    bool currentState;
    
public:
    explicit LedController(IGpioPort* gpioPort);
    
    void init(uint8_t ledPin);
    void turnOn();
    void turnOff();
    void toggle();
    void setState(bool state);
    bool getState() const;
    
    // Pattern support
    void blink(uint16_t onTime, uint16_t offTime, uint8_t count);
    void pulse(uint8_t intensity);
};

#endif
```

```cpp
// src/modules/led_controller.cpp
#include "led_controller.h"

LedController::LedController(IGpioPort* gpioPort) 
    : gpio(gpioPort), pin(0), currentState(false) {
}

void LedController::init(uint8_t ledPin) {
    pin = ledPin;
    gpio->setPinMode(pin, OUTPUT);
    turnOff();
}

void LedController::turnOn() {
    gpio->digitalWrite(pin, HIGH);
    currentState = true;
}

void LedController::turnOff() {
    gpio->digitalWrite(pin, LOW);
    currentState = false;
}

void LedController::toggle() {
    if (currentState) {
        turnOff();
    } else {
        turnOn();
    }
}

void LedController::setState(bool state) {
    if (state) {
        turnOn();
    } else {
        turnOff();
    }
}

bool LedController::getState() const {
    return currentState;
}

void LedController::blink(uint16_t onTime, uint16_t offTime, uint8_t count) {
    for (uint8_t i = 0; i < count; i++) {
        turnOn();
        delay(onTime);
        turnOff();
        delay(offTime);
    }
}

void LedController::pulse(uint8_t intensity) {
    // PWM simulation for non-PWM pins
    uint8_t onTime = (intensity * 10) / 255;
    uint8_t offTime = 10 - onTime;
    
    for (uint8_t i = 0; i < 100; i++) {
        if (onTime > 0) {
            turnOn();
            delayMicroseconds(onTime);
        }
        if (offTime > 0) {
            turnOff();
            delayMicroseconds(offTime);
        }
    }
}
```

## Running Tests

### Command Line Usage

```bash
# Run all tests
pio test

# Run tests on specific environment
pio test -e atmega32_test

# Run native tests only
pio test -e native_test

# Run specific test suite
pio test -e atmega32_test_suite1

# Run with verbose output
pio test -v

# Run tests matching pattern
pio test -f "test_led*"

# Generate coverage report (native only)
pio test -e native_test --coverage
```

### VS Code Integration

The PlatformIO IDE extension provides a Testing tab with:
- Visual test explorer
- One-click test execution
- Test results visualization
- Coverage reporting
- Debug test capability

## Troubleshooting

### Common Issues and Solutions

1. **Memory Overflow**
   - Split tests into multiple suites
   - Use PROGMEM for constant data
   - Optimize Unity configuration
   - Reduce test complexity

2. **Serial Output Issues**
   - Ensure correct baud rate (115200)
   - Add delay(2000) in setup()
   - Check USB cable and drivers
   - Verify test_port in platformio.ini

3. **Linking Errors**
   - Check for duplicate Unity inclusion
   - Verify mock implementations
   - Ensure proper HAL abstraction

4. **Test Discovery Problems**
   - Follow naming convention (test_*)
   - Check test directory structure
   - Verify platformio.ini configuration

## Conclusion

This comprehensive setup provides a robust, modular, and memory-efficient unit testing framework for the ATMEGA32A using Unity and PlatformIO. The combination of hardware abstraction, mock objects, and proper test organization enables thorough testing of embedded code both on native platforms and target hardware, while the CI/CD integration ensures continuous quality assurance throughout the development process.