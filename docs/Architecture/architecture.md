# Multi-Sonicator I/O Controller Architecture

## Overview

This document outlines the firmware and hardware architecture for the Multi-Sonicator I/O Controller, designed to manage up to four CT2000 sonicators simultaneously within Cannasol Technologies' industrial automation system.

## System Architecture

### High-Level System Integration

The Multi-Sonicator I/O Controller acts as a MODBUS RTU slave device within the existing automation hierarchy:

```
HMI System (MODBUS Master)
    ├── Velocio 1630c PLC (Slave ID = 1)
    └── Multi-Sonicator Controller (Slave ID = 2)
            ├── CT2000 Sonicator #1 (DB9 Interface)
            ├── CT2000 Sonicator #2 (DB9 Interface)
            ├── CT2000 Sonicator #3 (DB9 Interface)
            └── CT2000 Sonicator #4 (DB9 Interface)
```

### Hardware Architecture

#### Core Processing Unit
- **Microcontroller**: ATmega32A @ 16MHz (external crystal)
- **Power Supply**: 24VDC → 12V → 5V cascaded regulation
- **Communication**: MODBUS RTU over isolated RS-485/RS-232
- **Programming**: Arduino as ISP (Uno R4 WiFi)

#### Interface Design
- **DB9 Power Delivery**: Single cable solution from main automation system
- **Sonicator Interfaces**: 4x isolated DB9 connectors with individual control
- **Signal Isolation**: Galvanic isolation between communication and control circuits
- **Safety Monitoring**: Real-time overload detection and protection

## Firmware Architecture

### 1. Hardware Abstraction Layer (HAL)
**Purpose**: Provides consistent interface to hardware peripherals

**Components**:
- GPIO management for control signals
- ADC interface for analog feedback
- PWM generation for amplitude control
- UART communication handling
- Timer management for precise timing

### 2. Communication Layer
**Purpose**: Handles MODBUS RTU protocol implementation

**Components**:
- MODBUS RTU slave implementation
- Register mapping and validation
- CRC-16 error detection
- Timeout and retry handling
- Isolated communication driver

### 3. Sonicator Control Logic
**Purpose**: Manages individual sonicator operation and coordination

**Components**:
- Sonicator interface classes (4 instances)
- Multi-unit orchestration and synchronization
- Amplitude control via PWM + RC filtering
- Frequency generation and monitoring
- State machine management per unit

### 4. Safety and Monitoring System
**Purpose**: Ensures safe operation and fault detection

**Components**:
- Real-time overload detection
- Emergency shutdown procedures
- Safety interlock management
- Fault logging and reporting
- Watchdog timer implementation

### 5. System Services
**Purpose**: Provides diagnostic and maintenance capabilities

**Components**:
- System diagnostics and self-test
- Performance monitoring and metrics
- Event logging and alarm management
- Configuration management
- Firmware update support

## Module Dependencies

```
┌─────────────────────────┐
│   Application Layer     │  ← Multi-unit orchestration, safety logic
├─────────────────────────┤
│   Control Logic         │  ← Sonicator control, state machines
├─────────────────────────┤
│   Communication Layer   │  ← MODBUS RTU, protocol handling
├─────────────────────────┤
│   Hardware Abstraction  │  ← GPIO, ADC, PWM, UART drivers
├─────────────────────────┤
│   Arduino Framework     │  ← Core libraries, timing, interrupts
└─────────────────────────┘
```

## Communication Architecture

### MODBUS RTU Register Map

| Register Range | Purpose | Access |
|---------------|---------|--------|
| 0x0000-0x000F | System Status | Read |
| 0x0010-0x001F | Global Control | Read/Write |
| 0x0100-0x010F | Sonicator #1 Control | Read/Write |
| 0x0110-0x011F | Sonicator #1 Status | Read |
| 0x0200-0x020F | Sonicator #2 Control | Read/Write |
| 0x0210-0x021F | Sonicator #2 Status | Read |
| 0x0300-0x030F | Sonicator #3 Control | Read/Write |
| 0x0310-0x031F | Sonicator #3 Status | Read |
| 0x0400-0x040F | Sonicator #4 Control | Read/Write |
| 0x0410-0x041F | Sonicator #4 Status | Read |

### Power Architecture

**Input Power**: 24VDC via DB9 Pin 1 from main automation system
- Conservative power budget: 42mA @ 24VDC (≈1W)
- Local regulation cascade: 24V → 12V (LM2596) → 5V (LM7805)
- Separate analog and digital supplies for noise immunity
- EMI filtering and protection at power input

## Design Principles

### Safety-First Architecture
- **Fail-Safe Operation**: System defaults to safe state on any fault
- **Real-Time Monitoring**: Continuous monitoring of all critical parameters
- **Emergency Shutdown**: Immediate shutdown capability for safety events
- **Isolation**: Galvanic isolation between systems prevents fault propagation

### Industrial Reliability
- **Robust Communication**: MODBUS RTU with error detection and recovery
- **Temperature Stability**: External crystal for ±0.002% frequency accuracy
- **EMI Immunity**: Shielded interfaces and proper grounding
- **Watchdog Protection**: Hardware watchdog prevents system lockup

### Modular Design
- **Scalable Architecture**: Support for 1-4 sonicators with identical interfaces
- **Reusable Components**: Common code base for all sonicator interfaces
- **Testable Modules**: Independent testing of each system component
- **Maintainable Code**: Clear separation of concerns and well-documented APIs

## Development Framework

### Build System
- **PlatformIO**: Cross-platform build system and library manager
- **Arduino Framework**: Familiar development environment with AVR support
- **MightyCore**: ATmega32A board package for Arduino compatibility

### Programming and Debug
- **ISP Programming**: Arduino Uno R4 WiFi as in-system programmer
- **Serial Debug**: UART-based debugging and diagnostics
- **Hardware Testing**: Built-in self-test and diagnostic routines

---

**Architecture Version**: 0.1.0  
**Last Updated**: December 2024  
**Review Status**: Approved for implementation
