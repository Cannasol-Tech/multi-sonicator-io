# WARP.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Project Overview

The Multi-Sonicator I/O Controller is an industrial automation system built on an ATmega32A microcontroller running the Arduino Framework. It controls up to 4 CT2000 sonicators simultaneously via MODBUS RTU communication, integrating with existing Velocio PLC and HMI systems.

**Key Technologies:**
- ATmega32A microcontroller @ 16MHz (external crystal)
- Arduino Framework with PlatformIO build system
- MODBUS RTU slave interface (115200 baud, 8N1)
- Hardware-in-the-Loop (HIL) testing framework (Web UI Test Runner)
- Multi-framework testing: Unity (unit), pytest (integration), Behave (acceptance)

## Essential Development Commands

### Building and Flashing

```bash
# Install all dependencies and sync company standards
make install-deps

# Build ATmega32A firmware
make build

# Setup Arduino as ISP (auto-upload ArduinoISP sketch if needed)
make setup-arduino-isp

# Upload firmware to ATmega32A via Arduino as ISP
make upload-to-device

# Clean build artifacts
make clean
```

### Testing Framework

```bash
# Run complete test suite (Unit → Acceptance → Integration)
make test

# Run unit tests only (Unity Test Framework with coverage)
make test-unit

# Run acceptance tests (BDD with Behave + HIL hardware)
make test-acceptance

# Run integration tests (HIL hardware validation)
make test-integration

# CI pipeline simulation (unit tests + reports)
make ci
```

### Web UI Test Runner (HIL Testing)

```bash
# Check test runner status
make web-ui-status

# Validate HIL configuration
make web-ui-validate

# Start testing interface (simulation mode - no hardware required)
make web-ui-simulate

# Start testing interface (hardware mode - requires physical setup)
make web-ui-start

# Run all HIL tests
make web-ui-test

# Generate executive report
make web-ui-report
```

### Hardware-in-the-Loop Development

```bash
# Setup complete HIL sandbox environment
make hardware-sandbox

# Monitor serial output from Arduino Test Harness
make monitor-device

# Upload Arduino Test Harness firmware
make upload-harness

# Individual HIL test components
make acceptance-test-gpio
make acceptance-test-adc
make acceptance-test-pwm
make acceptance-test-modbus
```

### Web UI Development

```bash
# Install Web UI dependencies
make web-ui-install

# Start development servers (frontend + backend)
make web-ui-dev

# Build for production
make web-ui-build

# Launch full sandbox mode (firmware + web UI)
make web-ui-sandbox
```

## Code Architecture

### Layered Architecture

The firmware follows a strict layered architecture:

1. **Hardware Abstraction Layer (HAL)** (`src/modules/hal/`)
   - GPIO control for sonicator interfaces
   - ADC for power monitoring (5.44mV/W scaling)
   - PWM for amplitude control (0-10V output)
   - UART for MODBUS RTU communication

2. **Communication Layer** (`src/modules/communication/`)
   - MODBUS RTU slave implementation
   - Register map management (0x0000-0x041F ranges)
   - Error handling and timeout management

3. **Control Layer** (`src/modules/control/`)
   - Sonicator state management
   - Safety systems and watchdog integration
   - Multi-unit coordination

### Key Components

- **Main Application** (`src/main.cpp`): Unified main loop with MODBUS processing and sonicator service calls
- **Configuration** (`include/config.h`): Complete pin definitions and hardware specifications
- **Register Map** (`include/register_map.h`): MODBUS register structures and addressing
- **Hardware Configuration** (`config/hardware-config.yaml`): **SOLE SOURCE OF TRUTH** for hardware pin assignments

### Hardware Interface Design

**Pin Assignments (ATmega32A DIP-40):**
- **Sonicator Control**: PC0-PC7 for START/RESET signals (ULN2003A drivers)
- **Status Monitoring**: PD3-PD6 for OVERLOAD inputs (6N137 optocouplers)
- **Frequency Monitoring**: PB0-PB7 for FREQ_LOCK and FREQ_DIV10 inputs
- **Power Monitoring**: PA4-PA7 ADC channels (10kΩ voltage divider, 0-10.88V range)
- **Amplitude Control**: PD7 PWM output (shared across all 4 sonicators, 0-10V)
- **MODBUS RTU**: PD0/PD1 UART (115200 baud)

## Testing Strategy

### Three-Stage Testing Pyramid

1. **Unit Tests** (Unity Framework)
- Target: 85% code coverage minimum
   - Native C/C++ testing with mocks
   - Individual module validation

2. **Acceptance Tests** (Behave BDD + pytest)
   - Behavior-driven development scenarios
   - HIL hardware integration
   - Real sonicator interface validation

3. **Integration Tests** (HIL Hardware)
   - End-to-end system validation
   - 24-hour soak testing capability
   - Performance benchmarking

### HIL Test Harness

The project includes a comprehensive Hardware-in-the-Loop testing framework:
- **DUT**: ATmega32A target device
- **Test Harness**: Arduino Uno R4 WiFi with custom test wrapper
- **Automation**: Python-based test controller with serial communication
- **Coverage**: GPIO, ADC, PWM, MODBUS, and power monitoring validation

## Project Structure Patterns

```
src/
├── main.cpp                 # Main application entry point
├── modules/                 # Modular architecture
│   ├── hal/                # Hardware abstraction layer
│   ├── communication/      # MODBUS RTU implementation
│   └── control/           # Sonicator control logic
├── compat/                # Arduino framework compatibility layer

test/
├── unit/                  # Unity unit tests
├── acceptance/           # BDD acceptance tests
│   ├── features/         # Gherkin feature files
│   ├── hil_framework/    # HIL test automation
│   └── arduino_harness/  # Test harness firmware

docs/
├── planning/
├── inactive/
│   └── pin-matrix.md     # Historical pin assignments (superseded)
├── prd.md               # Product requirements document
└── standards/           # Company development standards

config/
└── hardware-config.yaml # SOLE SOURCE OF TRUTH for hardware configuration
```

## Development Guidelines

### Hardware Pin Assignments

**CRITICAL**: All pin assignments must reference `config/hardware-config.yaml` as the **SOLE SOURCE OF TRUTH**. Never modify pin mappings in `include/config.h` without updating the hardware configuration first.

### MODBUS Register Map

The system implements a structured register map:
- **0x0000-0x000F**: System Status (Read Only)
- **0x0010-0x001F**: Global Control (Read/Write)
- **0x0100-0x041F**: Per-Sonicator Registers (32 sonicators max, 4 implemented)

### Arduino Framework Integration

This project uses Arduino Framework with PlatformIO for:
- Rapid development with proven libraries
- UART, I2C, PWM, and ADC operations
- MODBUS RTU implementation
- Industrial reliability with mature toolchain

### Safety Requirements

- **Watchdog Timer**: 1000ms timeout with automatic safe-state recovery
- **Emergency Stop**: <50μs response time for safety shutdown
- **Communication Loss**: 1-second timeout triggers safe state
- **Power-on Default**: All sonicators default to 'off' state

## Company Standards Integration

This project follows Agile Flow company standards:
- **Coding Style Standards**: `docs/sop/coding-style.md`
- **Project Structure Standards**: `docs/sop/project-structure.md`
- **Software Testing Standards**: `docs/sop/sw-testing.md`

Standards are automatically synchronized:
```bash
make update-standards    # Sync latest standards (remote or local fallback)
make check-standards     # Check standards status
```

## Troubleshooting Common Issues

### Arduino as ISP Setup

If upload fails:
```bash
make check-arduino-isp   # Check ISP status
make setup-arduino-isp   # Auto-setup ISP
```

### HIL Testing Issues

If HIL tests fail to connect:
```bash
make hardware-sandbox    # Complete HIL setup with guided hardware changes
python3 scripts/detect_hardware.py --check-arduino  # Hardware detection
```

### Build Issues

For PlatformIO build problems:
```bash
make check-deps          # Verify dependencies
make check-pio          # Install PlatformIO if missing
pio platform update     # Update platform packages
```

## Performance Characteristics

- **Command Latency**: <200ms end-to-end (PLC to sonicator action)
- **Telemetry Update Rate**: 10Hz minimum for power/frequency data
- **Amplitude Control Accuracy**: ±2% of setpoint
- **MODBUS Response Time**: <100ms for register updates
- **Memory Usage**: ~24KB flash, ~1.5KB RAM (ATmega32A: 32KB/2KB available)

## CI/CD Integration

The project includes comprehensive CI/CD with GitHub Actions:
- Automated unit testing with coverage reporting
- HIL test execution (when hardware available)
- Executive report generation per company standards
- Release artifact packaging
- Documentation generation (Doxygen)

Local CI simulation:
```bash
make ci-local    # Run complete local CI pipeline
```
