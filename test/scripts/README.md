# HIL Test Scripts

This directory contains automation scripts for HIL (Hardware-in-the-Loop) testing.

## Overview

These scripts provide automated execution, monitoring, and reporting for HIL tests. They integrate with the CI/CD pipeline and provide both interactive and batch execution modes.

## Directory Structure

```
test/scripts/
├── README.md                    # This file
├── hil_automation.py           # Main HIL automation controller
├── hardware_setup.py           # Hardware initialization and validation
├── test_scheduler.py           # Automated test scheduling
├── safety_monitor.py           # Safety system monitoring
├── report_generator.py         # HIL test report generation
├── emergency_stop.py           # Emergency stop procedures
└── utilities/                  # Utility scripts
    ├── port_scanner.py         # Serial port detection
    ├── hardware_probe.py       # Hardware capability probing
    └── config_validator.py     # Configuration validation
```

## Main Scripts

### hil_automation.py
Main automation controller that orchestrates HIL test execution:
- Hardware initialization and validation
- Test case scheduling and execution
- Safety monitoring and emergency procedures
- Result collection and reporting
- Integration with CI/CD pipeline

### hardware_setup.py
Hardware initialization and configuration:
- Arduino wrapper firmware upload
- ATmega32A DUT programming
- Hardware connectivity validation
- Pin matrix verification
- Power system validation

### test_scheduler.py
Automated test scheduling and execution:
- Test case prioritization
- Resource allocation and management
- Parallel test execution
- Timeout and retry handling
- Test result aggregation

### safety_monitor.py
Continuous safety system monitoring:
- Hardware fault detection
- Emergency stop procedures
- Overload protection monitoring
- Safety interlock validation
- Incident reporting and logging

## Utility Scripts

### port_scanner.py
Serial port detection and management:
- Automatic Arduino detection
- Port availability checking
- Device identification
- Connection validation

### hardware_probe.py
Hardware capability probing:
- Available test channels detection
- Hardware feature validation
- Performance benchmarking
- Capability reporting

### config_validator.py
Configuration validation:
- Hardware config validation against actual hardware
- Pin matrix verification
- Safety system validation
- Compliance checking

## Integration Points

### CI/CD Pipeline
- Automated HIL test execution
- Hardware availability checking
- Test result reporting
- Build gate integration

### Web UI
- Real-time test monitoring
- Interactive test execution
- Hardware status display
- Test result visualization

### Acceptance Testing
- Behave BDD integration
- Test case automation
- Result validation
- Traceability reporting

## Usage Examples

### Basic HIL Test Execution
```bash
python test/scripts/hil_automation.py --test-suite basic
```

### Hardware Setup and Validation
```bash
python test/scripts/hardware_setup.py --validate --upload-wrapper
```

### Continuous Safety Monitoring
```bash
python test/scripts/safety_monitor.py --continuous --alert-threshold high
```

### Test Scheduling
```bash
python test/scripts/test_scheduler.py --schedule daily --priority critical
```

## Configuration

Scripts use configuration from:
- `config/hardware-config.yaml` - Hardware pin matrix and capabilities
- `test/acceptance/hil_framework/hil_config.yaml` - HIL framework settings
- Environment variables for CI/CD integration

## Safety Features

- Emergency stop procedures (< 100ms response)
- Hardware fault detection and isolation
- Safe state restoration
- Incident logging and reporting
- Automated safety system validation

## Monitoring and Reporting

- Real-time hardware status monitoring
- Test execution progress tracking
- Performance metrics collection
- Automated report generation
- Integration with project reporting systems
