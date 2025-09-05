# Multi-Sonicator I/O Controller Testing Framework Guide

## Overview

This document provides comprehensive guidance for using the robust testing framework implemented for the Multi-Sonicator I/O Controller project. The framework follows industry best practices with three-stage testing: Unit → Acceptance → Integration.

## Framework Architecture

### 1. Unit Testing Framework (Unity)
- **Framework**: Unity Test Framework for embedded C/C++
- **Environment**: Native compilation with coverage analysis
- **Coverage Requirement**: ≥90% code coverage
- **Execution**: `make test-unit`

### 2. Acceptance Testing Framework (BDD)
- **Framework**: Behave (Python BDD framework)
- **Environment**: Hardware-in-the-Loop (HIL) with Arduino Test Wrapper
- **Architecture**: Arduino Uno → ATmega32A stimulation and monitoring
- **Coverage**: All PRD requirements mapped to BDD scenarios
- **Execution**: `make test-acceptance`
- **Documentation**: See [HIL Framework Architecture Guide](hil-framework-architecture.md)

### 3. Integration Testing
- **Framework**: HIL hardware validation with Arduino Test Wrapper
- **Environment**: Real hardware with automated setup and timing fixes
- **Coverage**: End-to-end system validation with 50% scenario success rate
- **Execution**: `make test-integration`
- **Setup Time**: ~6.4 seconds (improved from 41.5 seconds)

## Quick Start

### Run All Tests
```bash
make test
```

This executes the complete test suite:
1. Configuration validation
2. Unit tests with coverage analysis
3. Acceptance tests with HIL automation
4. Integration tests
5. Traceability report generation

### Run Individual Test Stages

#### Unit Tests Only
```bash
make test-unit                    # All modules
make test-unit-communication      # Communication module only
make test-unit-hal               # HAL module only
make test-unit-control           # Control module only
make test-unit-sonicator         # Sonicator module only
```

#### Acceptance Tests Only
```bash
make test-acceptance             # All BDD scenarios
make acceptance-test-gpio        # GPIO functionality only
make acceptance-test-adc         # ADC verification only
make acceptance-test-pwm         # PWM generation only
make acceptance-test-modbus      # MODBUS communication only
```

#### Configuration Validation
```bash
make validate-config             # Validate HIL configuration
```

#### Generate Reports
```bash
make generate-traceability-report  # Coverage and traceability reports
```

## Unit Testing Details

### Framework Configuration
- **Unity Version**: 2.5.2
- **Compilation**: GCC with coverage flags (`-fprofile-arcs -ftest-coverage`)
- **Mock System**: Comprehensive mocks for embedded dependencies
- **Coverage Tools**: gcov/lcov integration

### Test Structure
```
test/unit/
├── unity_config.h              # Unity framework configuration
├── communication/
│   └── test_communication.c    # MODBUS protocol tests
├── hal/
│   └── test_hal.c             # Hardware abstraction tests
├── control/
│   └── test_control.c         # Control logic tests
└── sonicator/
    └── test_sonicator.c       # Sonicator module tests
```

### Coverage Reports
- **JSON Report**: `coverage/coverage.json` (dashboard consumption)
- **HTML Report**: `coverage/coverage.html` (human readable)
- **Requirement**: ≥90% line coverage across all modules

## Acceptance Testing Details

### BDD Framework Configuration
- **Behave Version**: ≥1.2.6
- **HIL Configuration**: `test/acceptance/hil_framework/hil_config.yaml`
- **Arduino Harness**: Automated detection and setup
- **Feature Tags**: Selective test execution

### Test Structure
```
test/acceptance/
├── features/                   # BDD feature files
│   ├── hil_gpio_functionality.feature
│   ├── hil_adc_verification.feature
│   ├── hil_pwm_generation.feature
│   ├── hil_modbus_communication.feature
│   └── ...
├── steps/                      # Step definitions
│   ├── hil_connectivity_steps.py
│   ├── hil_gpio_steps.py
│   └── ...
└── hil_framework/             # HIL automation
    ├── hil_config.yaml        # Single source of truth
    ├── hil_controller.py      # Hardware interface
    └── ...
```

### Feature Tags
- `@hil`: Hardware-in-the-loop tests
- `@smoke`: Quick smoke tests
- `@pending`: Not yet implemented
- `@gpio`: GPIO-specific tests
- `@adc`: ADC-specific tests
- `@pwm`: PWM-specific tests
- `@modbus`: MODBUS-specific tests

### Selective Execution
```bash
behave --tags=@smoke           # Run only smoke tests
behave --tags=@gpio            # Run only GPIO tests
behave --tags="not @pending"   # Skip pending tests
```

## Configuration Management

### HIL Configuration File
The `hil_config.yaml` file serves as the single source of truth for all HIL testing parameters:

- **Hardware Settings**: MCU type, programmer, ports
- **Power Supply**: Voltage levels, safety limits
- **Pin Mapping**: Arduino harness pin assignments
- **MODBUS Configuration**: Slave ID, baud rate, register ranges
- **Testing Parameters**: Tolerances, timeouts, safety limits
- **Error Handling**: Recovery strategies, retry logic
- **Logging**: Output levels, file management

### Configuration Validation
```bash
python3 scripts/validate_hil_config.py
```

Validates:
- Required sections present
- Parameter ranges within safe limits
- Pin mapping conflicts
- MODBUS register ranges
- Safety limit reasonableness

## Reporting and Traceability

### Coverage Reports
- **Unit Test Coverage**: Line-by-line coverage analysis
- **Requirement Coverage**: BDD scenario mapping to PRD requirements
- **Traceability Matrix**: Complete test-to-requirement mapping

### Report Formats
- **JSON**: Machine-readable for CI/CD integration
- **HTML**: Human-readable with visual indicators
- **Dashboard**: Real-time metrics for project status

### Report Locations
```
coverage/
├── coverage.json              # Unit test coverage data
├── coverage.html              # Unit test coverage report
├── traceability_report.json   # Complete traceability data
└── traceability_report.html   # Complete traceability report
```

## Maintenance Procedures

### Adding New Unit Tests
1. Create test file in appropriate module directory
2. Follow Unity test naming conventions
3. Include comprehensive test cases for new functionality
4. Ensure coverage meets 90% requirement
5. Update mock implementations if needed

### Adding New BDD Scenarios
1. Create or update feature file in `test/acceptance/features/`
2. Add appropriate feature tags
3. Implement step definitions in `test/acceptance/steps/`
4. Update HIL configuration if new hardware interactions needed
5. Map scenario to PRD requirement for traceability

### Updating HIL Configuration
1. Modify `hil_config.yaml` with new parameters
2. Run configuration validation: `make validate-config`
3. Test changes with relevant acceptance tests
4. Update documentation if configuration structure changes

### Periodic Reviews
- **Weekly**: Review test results and coverage metrics
- **Monthly**: Update traceability matrix with new requirements
- **Release**: Comprehensive test suite execution and report generation

## Troubleshooting

### Common Issues

#### Unit Test Compilation Errors
- Check Unity library installation: `pio pkg install --library "throwtheswitch/Unity@^2.5.2"`
- Verify mock implementations are complete
- Check include paths in coverage runner

#### HIL Connection Issues
- Verify Arduino harness connection
- Check port permissions and availability
- Validate HIL configuration with `make validate-config`
- Review hardware detection logs

#### Coverage Below 90%
- Identify uncovered code paths in coverage report
- Add test cases for missing scenarios
- Review mock implementations for completeness
- Check for unreachable code

#### BDD Test Failures
- Check HIL hardware setup and connections
- Verify Arduino harness programming
- Review test logs for specific failure points
- Validate test parameters against hardware capabilities

### Getting Help
- Review test logs in `test/acceptance/logs/`
- Check coverage reports for detailed analysis
- Validate configuration with built-in validator
- Consult traceability report for requirement mapping

## HIL Framework Improvements (2025)

### Architecture Enhancements
- **Arduino Test Wrapper Architecture**: Implemented Arduino Uno → ATmega32A HIL approach
- **Timing Fixes**: Reduced setup time from 41.5s to 6.4s (85% improvement)
- **Communication Protocol**: Standardized command/response protocol with Arduino wrapper
- **Buffer Management**: Automatic startup message clearing and buffer management
- **Error Handling**: Robust connection retry mechanism with graceful fallbacks

### Performance Metrics
- **Setup Time**: 6.4 seconds average (previously 41.5 seconds)
- **Success Rate**: 50% scenario pass rate for core HIL functionality
- **Communication**: <100ms command response time
- **Reliability**: Consistent performance across multiple test runs

### Validated Scenarios
- ✅ HIL Smoke Test: Basic connectivity and PING/PONG communication
- ✅ HIL GPIO Functionality: Pin control and voltage verification
- ⚠️ HIL Basic Connectivity: Power supply verification (needs refinement)
- ⚠️ HIL ADC Verification: Analog reading implementation (in progress)

For detailed information, see [HIL Framework Architecture Guide](hil-framework-architecture.md).

## Best Practices

1. **Run tests frequently** during development
2. **Maintain 90% coverage** for all new code
3. **Update BDD scenarios** when requirements change
4. **Validate configuration** before major test runs
5. **Review traceability reports** regularly
6. **Keep HIL hardware** properly maintained and calibrated
7. **Document test failures** and resolution steps
8. **Use feature tags** for efficient test execution
9. **Follow Arduino wrapper architecture** for HIL scenarios
10. **Allow proper initialization time** for Arduino Test Wrapper
