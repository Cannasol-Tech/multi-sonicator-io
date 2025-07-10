# Multi-Sonicator I/O Controller

**Industrial automation controller for simultaneous control of up to 4 CT2000 sonicators**

![Project Status](https://img.shields.io/badge/Status-Development-yellow)
![Version](https://img.shields.io/badge/Version-0.1.0-blue)
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
3. Configure ISP programmer (see [Environment Setup Guide](docs/Guides/environment-setup.md))
4. Build and upload firmware

```bash
# Build project
pio run

# Upload to target (via Arduino as ISP)
pio run --target upload
```

## Documentation

### Project Documentation
- **[Detailed Project Requirements](docs/Requirements/project-requirements.md)** - Complete technical specifications
- **[Architecture & Design Decisions](docs/Architecture/design-decisions.md)** - Hardware and software design rationale
- **[Implementation Plan](docs/Planning/implementation-plan.md)** - Development phases and timeline

### Development Guides
- **[Environment Setup](docs/Guides/environment-setup.md)** - Development environment configuration
- **[Component Selection](docs/Digikey/component-selection.md)** - Hardware components and sourcing
- **[Budget Analysis](docs/Budget/budget-analysis.md)** - Cost breakdown and optimization

### Features & Specifications
- **[Feature Specifications](docs/Features/Specifications/)** - Detailed feature implementations
- **[Installation Guide](docs/Features/Specifications/18-installation-and-commissioning.md)** - Deployment and commissioning

## Project Structure

```
├── src/                    # Source code
├── include/               # Header files
├── lib/                   # Libraries
├── test/                  # Unit tests
├── docs/                  # Documentation
│   ├── Architecture/      # System architecture
│   ├── Requirements/      # Project requirements
│   ├── Features/          # Feature specifications
│   ├── Guides/           # Development guides
│   └── Digikey/          # Component information
└── platformio.ini        # PlatformIO configuration
```

## Technical Specifications

- **Microcontroller**: ATmega32A @ 16MHz (external crystal)
- **Communication**: MODBUS RTU over RS-485/RS-232
- **Power**: 24VDC input with local regulation (12V/5V stages)
- **Interfaces**: 4x DB9 connectors for sonicator control
- **Programming**: Arduino as ISP (Uno R4 WiFi)

## Development Status

- [x] Initial project setup and documentation
- [x] Hardware architecture design
- [x] Component selection and budgeting
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