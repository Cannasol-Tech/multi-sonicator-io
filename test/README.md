# Test Directory

This directory contains the complete testing framework for the Multi-Sonicator I/O Controller firmware, implementing three distinct testing levels for comprehensive validation.

## Testing Architecture

The project implements a three-tier testing strategy:

1. **Unit Tests** - Fast, isolated component testing using Unity framework
2. **Acceptance Tests** - BDD scenarios running in simulavr emulator
3. **Hardware-in-the-Loop (HIL) Tests** - Same BDD scenarios on real hardware

## Directory Structure

```markdown
test/
├── README.md              # This file
├── test_main.cpp          # Unity test runner
├── test_config/           # Unit tests for configuration
├── test_sonicator/        # Unit tests for sonicator interface
├── test_system/           # Unit tests for system integration
├── acceptance/            # BDD acceptance tests (simulavr + HIL)
│   ├── environment.py     # Behave environment configuration
│   ├── features/          # Gherkin feature files
│   └── steps/             # Python step implementations
└── hardware/              # HIL-specific test configurations
```

## 1. Unit Tests (Native PlatformIO + Unity)

Fast, isolated tests for individual components and business logic.

### Running Unit Tests

```bash
# Run all unit tests
pio test -e test_desktop

# Run specific test suite
pio test -e test_desktop -f test_config
pio test -e test_desktop -f test_sonicator
pio test -e test_desktop -f test_system

# Run with verbose output
pio test -e test_desktop -v
```

### Unit Test Structure

- **`test_config/`** - Configuration validation tests
- **`test_sonicator/`** - Sonicator interface component tests
- **`test_system/`** - System integration and orchestration tests
- **`test_main.cpp`** - Unity test runner and harness

## 2. Acceptance Tests (BDD + simulavr Emulator)

Behavior-driven development tests running complete scenarios in the simulavr AVR emulator within Docker containers.

### Running Acceptance Tests

```bash
# Run all acceptance tests in simulavr emulator
behave test/acceptance -D profile=simulavr

# Run specific feature
behave test/acceptance/features/control.feature -D profile=simulavr

# Run with Docker (recommended)
make test-emulation
```

### Acceptance Test Features

- **`control.feature`** - Sonicator start/stop/amplitude control
- **`monitoring.feature`** - Power and frequency monitoring
- **`safety.feature`** - Emergency stop and overload protection
- **`modbus.feature`** - MODBUS RTU communication protocol
- **`timing.feature`** - Real-time performance requirements
- **`smoke.feature`** - Basic system functionality validation

## 3. Hardware-in-the-Loop (HIL) Tests

Same BDD scenarios as acceptance tests, but executed on real ATmega32A hardware when available.

### Running HIL Tests

```bash
# Run all tests on hardware (requires connected ATmega32A)
behave test/acceptance -D profile=hil

# Run specific feature on hardware
behave test/acceptance/features/safety.feature -D profile=hil

# Detect and test with hardware
make test-hil
```

### HIL Requirements

- ATmega32A controller connected via USB-to-serial adapter
- Proper wiring per hardware documentation
- Hardware detection script: `scripts/detect_hardware.py`

## Test Execution Profiles

The BDD framework automatically switches between emulation and hardware based on the profile:

- **`simulavr`** (default) - Uses simulavr emulator in Docker
- **`hil`** - Uses real hardware when available

Profile selection is handled in `test/acceptance/environment.py`.

## Integration with Build System

All testing levels are integrated into the project Makefile:

```bash
# Run all test levels
make test-all

# Individual test levels
make test-unit          # PlatformIO Unity tests
make test-emulation     # BDD with simulavr
make test-hil           # BDD with hardware

# Continuous integration
make ci-test            # Full test suite for CI/CD
```

## Test Environments

- **`test_desktop`** - Native desktop environment for unit tests
- **`simulavr`** - AVR emulator environment for acceptance tests
- **`hil`** - Hardware-in-the-loop environment for real hardware validation

## Dependencies

- **Unity** - C unit testing framework (via PlatformIO)
- **Behave** - Python BDD framework for acceptance tests
- **simulavr** - AVR emulator for hardware simulation
- **Docker** - Containerized emulation environment