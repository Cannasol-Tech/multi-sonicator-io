# Multi-Sonicator I/O Controller

**Industrial automation controller for simultaneous control of up to 4 CT2000 sonicators**

![Project Status](https://img.shields.io/badge/Status-Development-yellow)
![Version](https://img.shields.io/badge/Version-1.0.0-blue)
![License](https://img.shields.io/badge/License-Proprietary-red)

## Overview

The Multi-Sonicator I/O Controller extends Cannasol Technologies' proprietary automation system to control multiple CT2000 sonicators simultaneously. This ATmega32A-based industrial controller integrates with existing Velocio PLC and HMI systems via MODBUS RTU communication.

### Key Features

- **Multi-Unit Control**: Simultaneous control of up to 4 CT2000 sonicators
- **Industrial Integration**: MODBUS RTU communication with existing automation systems
- **DB9 Interfaces**: Individual DB9 connections for each sonicator unit
- **Cloud Connectivity**: Firebase integration via ESP32 for remote monitoring
- **Safety Systems**: Comprehensive monitoring and alarm management

## Quick Start

### Hardware Requirements

- ATmega32A microcontroller (16MHz external crystal)
- Arduino Uno R4 WiFi (as ISP programmer)
- 24VDC power supply (via DB9 from main automation system)

### Development Setup

1. Install PlatformIO with VSCode
2. Clone this repository
3. Configure ISP programmer (see [Arduino ISP Setup Guide](docs/setup/arduino-isp-setup.md))
4. Build and upload firmware

```bash
# Install dependencies and sync company standards
make install-deps

# Build project
make build

# Upload to target (via Arduino as ISP)
make upload-to-device

# Update company standards (when needed)
make update-standards
```

### Web UI and HIL Testing

This project includes a Web UI with backend APIs and tests that support hardware-in-the-loop (HIL) workflows.

#### Quick Start

```bash
# Install frontend/backend and Python test dependencies
make web-ui-install

# Start development servers (frontend + backend)
make web-ui-dev

# Run Web UI tests with coverage (HTML report in web-ui/htmlcov)
make web-ui-test

# Optional: Full firmware + Web UI sandbox workflow
make web-ui-sandbox

# Generate overall coverage summary for the repo (final/coverage-summary.json)
make coverage
```

#### Features

- **Hardware Configuration**: Pre-configured for ATmega32A DUT and Arduino Uno R3 test harness
- **Multi-Suite Testing**: Integrates Unity (unit), pytest (integration), and Behave (acceptance) tests
- **Real-time Monitoring**: Web-based interface for test execution and monitoring
- **Coverage Reporting**: Coverage summary and HTML reports for Web UI tests
- **Sandbox Mode**: Optional end-to-end run including firmware build and upload



## Documentation

### Company Standards

This project follows Axovia Flow company standards. Standards are automatically synchronized from the central repository (with local fallback):

- **[Coding Style Standards](.axovia-flow/company-standards/coding-style.md)** - Code formatting and quality guidelines
- **[Project Structure Standards](.axovia-flow/company-standards/project-structure.md)** - Directory organization and file placement
- **[Software Testing Standards](.axovia-flow/company-standards/sw-testing-standard.md)** - Testing methodology and requirements

**Standards Management:**

```bash
make update-standards    # Sync latest standards (remote or local fallback)
make check-standards     # Check standards status
```

**Features:**

- ✅ **Standards-Only Import**: Downloads only `docs/standards/` (not entire repository)
- ✅ **Local Fallback**: Uses local standards when remote repository unavailable
- ✅ **Automatic Sync**: Standards updated during `make install-deps`
- ✅ **Flexible Configuration**: Easy to change source repository in `scripts/sync_company_standards.py`

### Project Documentation

- **[Product Requirements Document](docs/prd.md)** - Complete technical specifications and requirements
- **[Project Requirements (Detailed)](docs/prd-shards/project-requirements.md)** - Comprehensive project requirements
- **[Architecture Overview](docs/architecture.md)** - System architecture and design decisions
- **[Hardware Configuration (SOLE SOURCE OF TRUTH)](config/hardware-config.yaml)** - Hardware pin assignments and interface mapping

### Hardware Documentation

- **[ATmega32A Information](docs/ATMEGA32A/)** - Microcontroller specifications and pin definitions
- **[Arduino ISP Setup](docs/ArduinoISP/uno-r3-isp-setup.md)** - Programming setup guide
- **[Component Selection](docs/Digikey/component-list.md)** - Hardware components and sourcing

### Development Standards & Testing

- **[Coding Standards](docs/standards/)** - Development guidelines and best practices
- **[Testing Framework](docs/Testing/hil-serial-protocol.md)** - Hardware-in-the-loop testing protocol
- **[Simulation Framework](docs/simulation_framework.md)** - Testing without hardware

### CI/CD Generated Documentation & Reports

The CI/CD pipeline automatically generates comprehensive documentation and test reports. Access the latest results:

#### 📊 **Pipeline Results & Reports**

- **[Latest CI Pipeline Results](../../actions/workflows/ci.yml)** - Build status, test results, and artifacts
- **[Nightly Test Reports](../../actions/workflows/nightly.yml)** - Extended testing, performance benchmarks, and memory analysis
- **[Release Pipeline](../../actions/workflows/release.yml)** - Release builds and automated documentation

#### 📚 **Generated Documentation** (Available in CI Artifacts)

- **API Reference Documentation** - Auto-generated from source code comments
- **User Manual** - Operating procedures and safety guidelines
- **Simulation Framework Guide** - Testing without hardware
- **Implementation Summary** - Technical architecture details

#### 📈 **Performance & Quality Reports** (Available in Nightly Artifacts)

- **Performance Benchmarks** (`performance_report.json`) - Build times and execution metrics
- **Memory Analysis** (`memory_report.json`) - Memory usage and leak detection
- **Test Coverage Reports** - Unit and integration test coverage
- **Build Output Logs** (`build_output.txt`) - Detailed compilation results

#### 🔗 **How to Access Generated Results**

1. Go to **[Actions tab](../../actions)** in this repository
2. Click on the latest workflow run (CI, Nightly, or Release)
3. Scroll to **Artifacts** section at the bottom
4. Download the artifact package (e.g., `nightly-test-results-YYYYMMDD`)
5. Extract and view the generated documentation and reports

> **Note**: Generated documentation is updated automatically on every commit and available as downloadable artifacts from the GitHub Actions workflows.

## Project Structure

```
├── src/                    # Source code (Arduino Framework)
│   ├── main.cpp           # Main Arduino application (placeholder)
│   └── modules/           # Modular components
├── include/               # Header files
│   ├── config.h           # System configuration
│   └── types.h            # Type definitions
├── test/                  # Testing framework
│   ├── unit/              # Unit tests
│   └── acceptance/        # Hardware-in-the-loop tests
├── docs/                  # Documentation
│   ├── prd.md             # Product Requirements Document
│   ├── architecture.md    # System architecture
│   ├── planning/          # Pin matrix and planning docs
│   ├── prd-shards/        # Detailed requirements
│   ├── standards/         # Development standards
│   ├── ATMEGA32A/         # Microcontroller documentation
│   ├── ArduinoISP/        # Programming setup
│   ├── Digikey/           # Component information
│   └── Testing/           # Test protocols
├── scripts/               # Build and utility scripts
├── tools/                 # Development tools
└── platformio.ini         # PlatformIO configuration
```

## Technical Specifications

- **Microcontroller**: ATmega32A @ 16MHz (external crystal)
- **Development Framework**: Arduino Framework (chosen for rapid development and proven libraries)
- **Communication**: MODBUS RTU over RS-485/RS-232
- **Power**: 24VDC input with local regulation (12V/5V stages)
- **Interfaces**: 4x DB9 connectors for sonicator control
- **Programming**: Arduino as ISP (Uno R4 WiFi)

### Arduino Framework Rationale

This project uses the **Arduino Framework** rather than bare metal programming for the following reasons:

- **Rapid Development**: Proven libraries for UART, I2C, PWM, and ADC operations
- **MODBUS Integration**: Easier implementation of MODBUS RTU communication protocols
- **Industrial Reliability**: Arduino framework is mature and well-tested in industrial applications
- **Resource Efficiency**: ATmega32A (32KB flash, 2KB RAM) has sufficient resources for framework overhead
- **Debugging & Testing**: Better toolchain support and debugging capabilities
- **Maintainability**: More readable code and easier future modifications

## Development Status

- [x] Initial project setup and documentation
- [x] Hardware architecture design
- [x] Component selection and budgeting
- [x] Arduino Framework integration and configuration
- [ ] Firmware development (in progress)
- [ ] Hardware testing and validation
- [ ] System integration testing

## License

Proprietary software developed by Cannasol Technologies. All rights reserved.

---

**Project Lead**: Stephen Boyett  
**Hardware Advisor**: Josh (CEO)  
**Company**: Cannasol Technologies  
**Last Updated**: December 2024

   Identified Open Questions Requiring Resolution:

     1. CT2000 Specifications Research (Lines 26-27 in Q&A)
     • Frequency ranges need to be researched and
       documented
     • Power levels need to be researched and documented

     2. Hardware Verification Requirements (Lines 125-128 in
        Q&A)
     • Confirm Overload Reset Pin 2 availability and
       functionality
     • Verify internal amplitude control switch existence
       and functionality

     3. ATmega32A Pin Allocation Diagram (Lines 142-143 in
        Q&A)
     • Need detailed pin diagram with all 40 pins labeled
       to confirm capabilities
     • Verify 39 pins required vs 40 pins available

     4. Frequency Measurement Implementation Details (Lines
        147-154 in Q&A)
     • Implementation approach for 4 simultaneous frequency
       measurements
     • Decision on external frequency-to-voltage converters

     5. System Redundancy Implementation (Lines 177-178 in
        Q&A)
     • Ideas needed for redundancy implementation
     • Fallback options if one chip fails

     6. CE Marking Clarification (Line 81 in Q&A)
     • Understanding of CE Marking requirements

     7. Backward Compatibility Testing Plan (Line 176 in
        Q&A)
     • Detailed testing plan for single vs multiple
       sonicator configurations
