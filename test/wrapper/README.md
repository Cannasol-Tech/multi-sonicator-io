# Arduino Test Wrapper

This directory contains the Arduino test wrapper firmware for HIL testing.

## Overview

The Arduino Test Wrapper provides the hardware interface between the test framework and the ATmega32A DUT (Device Under Test). It acts as both an ISP programmer and a test harness controller.

## Directory Structure

```
test/wrapper/
├── README.md                           # This file
├── arduino_test_wrapper/              # Arduino firmware project
│   ├── arduino_test_wrapper.ino       # Main wrapper firmware
│   ├── pin_definitions.h               # Pin mapping definitions
│   ├── safety_systems.h                # Safety interlock definitions
│   └── protocol_handlers.h             # Command protocol handlers
├── platformio.ini                      # PlatformIO configuration
├── upload_wrapper.py                   # Wrapper firmware upload script
└── test_wrapper_protocol.py           # Protocol testing utilities
```

## Hardware Configuration

The wrapper firmware implements the pin mapping defined in `config/hardware-config.yaml`:

### Current Single-Channel Prototype (Sonicator 4)
- **MODBUS Communication**: D2 (TX to DUT), D3 (RX from DUT)
- **System Status**: D4 (Status LED monitoring)
- **Sonicator 4 Interface**:
  - D7: FREQ_DIV10_4 (frequency ÷10 signal generation)
  - D8: FREQ_LOCK_4 (frequency lock signal)
  - A2: OVERLOAD_4 (overload condition simulation)
  - A3: START_4 (start signal monitoring)
  - A4: RESET_4 (reset signal monitoring)
  - A1: POWER_SENSE_4 (power measurement ADC)
  - D9: AMPLITUDE_ALL (PWM amplitude control monitoring)

## Communication Protocol

The wrapper uses a simple ASCII command protocol over serial (115200 baud):

### Basic Commands
- `PING` → `OK PONG` (connectivity test)
- `INFO` → `OK HIL-Wrapper/0.1.0 S4-ONLY` (version info)
- `DEBUG` → `OK SIM=ON/OFF VOLT=x.x` (debug status)

### Control Commands (Sonicator 4)
- `SET OVERLOAD 4 <0|1>` - Set/clear overload condition
- `SET FREQ 4 <hz>` - Set frequency generation (placeholder)
- `SET LOCK 4 <0|1>` - Set/clear frequency lock signal

### Monitoring Commands
- `READ STATUS 4` → `OK RUN=<0|1> OVL=<0|1> LOCK=<0|1>`
- `READ POWER 4` → `OK POWER=<adc_value>`
- `READ ADC <channel>` → `OK ADC=<value>`
- `READ PWM <channel>` → `OK PWM=<duty_cycle>%`

### Simulation Commands
- `SET VOLTAGE <channel> <voltage>` - Simulate voltage for testing

## Safety Features

- Safe default states on startup (all outputs LOW/neutral)
- Emergency stop capability (immediate output disable)
- Hardware fault detection and reporting
- Overload protection monitoring
- Timeout-based safety resets

## ISP Programming

The wrapper can function as an Arduino ISP programmer for the ATmega32A DUT:
- Uses Arduino as ISP sketch for firmware programming
- Supports standard AVR programming protocols
- Integrated with automated testing workflows

## Testing and Validation

- Protocol compliance testing
- Hardware interface validation
- Safety system verification
- Performance benchmarking
- Integration with HIL test framework

## Future Expansion

- Multi-channel support (Sonicators 1-3)
- Enhanced safety systems
- Real-time signal generation
- Advanced protocol features
- Remote programming capabilities
