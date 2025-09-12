# HIL (Hardware-in-the-Loop) Test Framework

This directory contains the HIL test framework for validating firmware behavior against actual ATmega32A hardware.

## Directory Structure

```
test/hil/
├── README.md                    # This file
├── hil_test_runner.py          # Main HIL test execution engine
├── hardware_validation.py      # Hardware configuration validation
├── safety_interlocks.py        # Safety systems and emergency stop
├── signal_generators.py        # Test signal generation utilities
├── test_cases/                 # HIL-specific test cases
│   ├── basic_connectivity.py   # Basic hardware connectivity tests
│   ├── modbus_communication.py # MODBUS protocol validation
│   ├── sonicator_control.py    # Sonicator control validation
│   └── power_monitoring.py     # Power sensing validation
└── reports/                    # HIL test reports and logs
    ├── hardware_status.json    # Hardware status reports
    └── test_results.json       # Test execution results
```

## Hardware Configuration

The HIL framework validates against the hardware configuration defined in:
- **Source of Truth**: `config/hardware-config.yaml`
- **Pin Matrix**: ATmega32A to Arduino Uno R4 WiFi connections
- **Test Capabilities**: Currently supports Sonicator 4 (single-channel prototype)

## Test Execution

HIL tests are executed through:
1. **Direct execution**: `python test/hil/hil_test_runner.py`
2. **CI/CD integration**: Automated execution via GitHub Actions
3. **Manual validation**: Interactive testing via HIL CLI

## Safety Features

- Emergency stop procedures (< 100ms response time)
- Hardware overload protection
- Safe default states on startup and error conditions
- Hardware fault detection and reporting

## Integration Points

- **Acceptance Tests**: Integration with Behave BDD framework
- **Unit Tests**: Hardware abstraction layer validation
- **CI/CD Pipeline**: Automated HIL testing in build process
- **Web UI**: Real-time hardware monitoring and control

## Hardware Requirements

- Arduino Uno R4 WiFi (ISP programmer and test wrapper)
- ATmega32A Sonicator Multiplexer DUT
- Power supply: 24V → 12V → 5V regulation cascade
- DB9 connectors: 4x sonicator interfaces + 1x MODBUS communication
- Safety interlocks and emergency stop systems

## Current Limitations

- Single-channel prototype (Sonicator 4 only)
- Channels 1-3 require additional harness connections
- Hardware availability dependent on physical setup

## Future Expansion

- Multi-channel HIL harness (all 4 sonicators)
- Enhanced safety systems
- Automated hardware provisioning
- Remote HIL testing capabilities
