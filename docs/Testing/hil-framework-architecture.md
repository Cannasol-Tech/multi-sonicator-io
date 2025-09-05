# HIL Framework Architecture Guide

## Overview

The Hardware-in-the-Loop (HIL) testing framework for the Multi-Sonicator-IO system uses an **Arduino Test Wrapper → ATmega32A** architecture to provide comprehensive hardware validation. This document describes the architecture, usage patterns, and best practices for writing HIL test scenarios.

## Architecture Components

### 1. Arduino Test Wrapper (Hardware Interface)
- **Purpose**: Acts as a test harness to stimulate and monitor the ATmega32A target
- **Hardware**: Arduino Uno connected to ATmega32A via GPIO pins
- **Communication**: Serial communication at 115200 baud (USB)
- **Role**: External test equipment that can:
  - Stimulate ATmega32A inputs (OVERLOAD, FREQ_LOCK)
  - Monitor ATmega32A outputs (START, RESET)
  - Read analog values (POWER_SENSE)
  - Control test conditions

### 2. ATmega32A Target (Device Under Test)
- **Purpose**: The actual sonicator control firmware being tested
- **Hardware**: ATmega32A microcontroller with sonicator interface pins
- **Firmware**: Production firmware or test firmware
- **Role**: Responds to stimuli and produces outputs based on control logic

### 3. HIL Framework (Test Controller)
- **Purpose**: Orchestrates test execution and communicates with Arduino wrapper
- **Language**: Python with Behave BDD framework
- **Communication**: Serial commands to Arduino Test Wrapper
- **Role**: Test execution engine that:
  - Sends commands to Arduino wrapper
  - Interprets responses and validates behavior
  - Manages test scenarios and assertions

## Communication Protocol

### Arduino Test Wrapper Commands

The Arduino Test Wrapper accepts ASCII commands and responds with status messages:

```
Command Format: <COMMAND> <PARAMETERS>\n
Response Format: OK <DATA> | ERR <ERROR_MESSAGE>
```

#### Core Commands

| Command | Purpose | Example | Response |
|---------|---------|---------|----------|
| `PING` | Connectivity test | `PING` | `OK PONG` |
| `INFO` | System information | `INFO` | `OK HIL-Wrapper/0.1.0 S4-ONLY` |
| `SET OVERLOAD <unit> <0\|1>` | Stimulate overload input | `SET OVERLOAD 4 1` | `OK` |
| `SET LOCK <unit> <0\|1>` | Stimulate frequency lock | `SET LOCK 4 1` | `OK` |
| `READ STATUS <unit>` | Read pin states | `READ STATUS 4` | `OK RUN=0 OVL=0 LOCK=0` |
| `READ POWER <unit>` | Read power analog | `READ POWER 4` | `OK POWER=428` |

#### Pin Mapping (Sonicator 4 Only)

| Signal | ATmega32A Pin | Arduino Pin | Direction | Purpose |
|--------|---------------|-------------|-----------|---------|
| START_4 | PC0 | D2 | Output | Start signal from ATmega32A |
| RESET_4 | PC1 | D3 | Output | Reset signal from ATmega32A |
| OVERLOAD_4 | PC2 | D4 | Input | Overload stimulation to ATmega32A |
| FREQ_LOCK_4 | PC3 | D5 | Input | Frequency lock stimulation |
| POWER_SENSE_4 | ADC0 | A0 | Analog | Power measurement |

## HIL Test Scenario Patterns

### 1. System Initialization Pattern

```gherkin
Given the HIL framework is connected to ATmega32A
And the GPIO HAL is loaded on the target
```

**Implementation**: Verifies Arduino wrapper communication and system readiness.

### 2. Input Stimulation Pattern

```gherkin
When I set pin "OVERLOAD_4" to HIGH via HIL framework
```

**Implementation**: Uses `SET OVERLOAD 4 1` command to stimulate ATmega32A input.

### 3. Output Monitoring Pattern

```gherkin
Then the pin voltage should be approximately 5.0V
```

**Implementation**: Uses `READ STATUS 4` to verify ATmega32A output states.

### 4. External Voltage Application Pattern

```gherkin
When the HIL framework applies 0V to pin "OVERLOAD_4" externally
```

**Implementation**: Maps voltage levels to digital stimulation commands.

## Writing HIL Test Scenarios

### Best Practices

1. **Use Arduino Wrapper Commands**: Don't expect direct ATmega32A control
2. **Understand Pin Directions**: Inputs can be stimulated, outputs can be monitored
3. **Use Appropriate Timeouts**: Allow time for ATmega32A firmware to respond
4. **Verify System Status**: Use `READ STATUS` to confirm system state

### Example Scenario

```gherkin
@hil @hardware @gpio
Scenario: Overload input stimulation
  Given the HIL framework is connected to ATmega32A
  And the GPIO HAL is loaded on the target
  When I set pin "OVERLOAD_4" to HIGH via HIL framework
  Then the system should respond to overload condition
  When I set pin "OVERLOAD_4" to LOW via HIL framework
  Then the system should clear overload condition
```

### Step Definition Mapping

| Gherkin Step | Arduino Command | Purpose |
|--------------|-----------------|---------|
| `I set pin "OVERLOAD_4" to HIGH` | `SET OVERLOAD 4 1` | Stimulate overload input |
| `I set pin "FREQ_LOCK_4" to LOW` | `SET LOCK 4 0` | Clear frequency lock |
| `pin voltage should be 5.0V` | `READ STATUS 4` | Verify output state |
| `reading pin "START_4" should return HIGH` | `READ STATUS 4` | Monitor output pin |

## Hardware Setup

### Physical Connections

1. **Arduino Uno**: Connected via USB for serial communication
2. **ATmega32A**: Connected to Arduino via GPIO pins (see pin mapping)
3. **Power Supply**: Shared 5V power between Arduino and ATmega32A
4. **Ground**: Common ground connection

### Software Setup

1. **Arduino Firmware**: Upload `arduino_test_wrapper.ino` to Arduino Uno
2. **ATmega32A Firmware**: Program with test or production firmware
3. **HIL Framework**: Configure `hil_config.yaml` with correct serial port

## Troubleshooting

### Common Issues

1. **Connection Timeout**: Ensure Arduino is programmed and connected
2. **Command Not Responding**: Check baud rate (115200) and serial port
3. **Pin State Mismatch**: Verify pin mapping and firmware behavior
4. **Startup Messages**: Framework automatically clears Arduino startup output

### Debug Commands

```bash
# Test Arduino wrapper directly
python3 -c "
import serial
ser = serial.Serial('/dev/cu.usbmodem2101', 115200, timeout=2)
ser.write(b'PING\n')
print(ser.readline().decode())
"

# Test HIL framework connection
cd test/acceptance
python3 -m behave features/hil_smoke.feature -v
```

## Performance Characteristics

- **Setup Time**: ~6.4 seconds (includes 3-second Arduino initialization)
- **Command Response**: <100ms typical
- **Test Execution**: ~6.5 seconds per scenario
- **Reliability**: 50% scenario pass rate (core functionality working)

## Future Enhancements

1. **Multi-Unit Support**: Extend to Sonicators 1-3
2. **Advanced ADC Testing**: Implement precise analog measurements
3. **Timing Analysis**: Add precise timing measurements
4. **Error Recovery**: Enhanced error handling and recovery mechanisms
