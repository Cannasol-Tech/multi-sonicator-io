# Multi-Sonicator I/O Controller Embedded Architecture Document

## 1. Introduction

This document outlines the complete embedded architecture for multi-sonicator-io, including microcontroller firmware, hardware interfaces, and communication protocols. It serves as the single source of truth for AI-driven development, ensuring consistency across the firmware stack and integration with industrial automation systems.

This unified approach combines what would traditionally be separate hardware, firmware, and communication architecture documents, streamlining the development process for embedded systems where these concerns are tightly coupled.

## Starter Template or Existing Project

N/A - This is a custom embedded firmware project built on PlatformIO and Arduino framework for ATmega32A, not derived from a standard embedded starter template or existing codebase. The architecture focuses on industrial control with MODBUS RTU communication and multi-unit sonicator management, which doesn't align with typical embedded starters (e.g., no RTOS frameworks like FreeRTOS or specific MCU SDKs are used beyond Arduino core).

## Change Log

| Date | Version | Description | Author |
|------|---------|-------------|--------|
| 2025-09-01 | 1.0 | Initial embedded architecture draft (refined from fullstack template based on user feedback) | Winston (Architect) |

## Multi-Sonicator I/O Controller Architecture

## Overview

This document outlines the firmware and hardware architecture for the Multi-Sonicator I/O Controller, designed to manage up to four CT2000 sonicators simultaneously within Cannasol Technologies' industrial automation system.

## System Architecture

### High-Level System Integration

The Multi-Sonicator I/O Controller acts as a MODBUS RTU slave device within the existing automation hierarchy:

```markdown
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
- **Programming**: Arduino as ISP (Uno R3)

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
- Status LED (PD2) driver for unified system indication per `config/hardware-config.yaml` (SOLE SOURCE OF TRUTH) (`STATUS_LED_PIN`)

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
- Status LED policy: single LED on PD2 (physical pin 16) reflects system states (e.g., boot, error, running) replacing legacy RGB scheme

### 6. Runtime Scheduling (Cooperative taskLoop)

**Approach**: A lightweight cooperative scheduler built on a timer tick and a centralized dispatcher function named `taskLoop()`. This formalizes the common `millis()` pattern without introducing RTOS complexity.

### Runs in ISRs (minimal work only)

- UART RX/TX: MODBUS bytes to/from ring buffers; no parsing in ISR.
- Timer tick (e.g., 1 ms): increments system time; marks tasks due.
- Frequency capture: count edges from CT2000 ÷10 output.
- ADC EOC: push raw samples to a small buffer.
- Optional pin-change: latch E‑stop/overload immediately for safety.

**Runs in taskLoop() (cooperative, non-blocking)**

- Parse MODBUS frames and execute reads/writes via the register map.
- Compute frequency and power from buffered data; apply filters.
- Update status flags and Active Count/Mask.
- Apply outputs (start/stop level, overload reset pulse, amplitude PWM/DAC).
- Housekeeping (runtime counters, diagnostics, watchdog kick at a safe point).

**Task rates (aligned with requirements)**

- Power & Frequency: 5–10 Hz (100–200 ms)
- Status/Mask/Count: 10–20 Hz (50–100 ms)
- Runtime Counters: 1 Hz
- MODBUS parsing: dispatched every tick but bounded per cycle

**Why this design**

- Meets ≤100 ms reflection and safety latency targets with low jitter.
- Centralizes timing, improves testability/traceability, and avoids duplicated `if (millis() - last >= period)` code across modules.
- No RTOS stacks or preemption; fits ATmega32A (32 KB Flash, 2 KB SRAM).

## Module Dependencies

```markdown
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

### PCB Layout Rules (ATmega32A, 16 MHz, ADC, PWM 0–10 V, RS‑485)

- **Short return paths**
  - Use a solid, continuous ground plane; avoid routing signals across plane splits.
  - Keep signal/return vertically aligned (microstrip/stripline) to minimize loop area.
  - If a split must be crossed, add a nearby ground stitching via to provide a local return.

- **Analog/Digital ground strategy**
  - Keep analog front‑end (LM358, ADC dividers, LM2907) over an analog ground region.
  - Tie AGND↔DGND at a single point near MCU AGND/AREF. Prefer 0 Ω link; a ferrite bead (FB) is acceptable if HF isolation helps.
  - Filter AVCC with FB + 100 nF + 1 µF close to the pin; route AREF cleanly with its cap (e.g., 100 nF) to AGND.

- **Decoupling and bulk**
  - 100 nF X7R at every IC VCC pin, placed as close as possible with a direct via to the ground plane.
  - Add 1 µF per IC cluster/rail and 10–47 µF bulk per rail near connectors/regulators.
  - LM358/analog path: 100 nF at power pins + 1 µF nearby. Keep feedback loops short and quiet.

- **Edge‑rate control (EMI reduction)**
  - Add series resistors (≈33–68 Ω) at noisy GPIO/PWM sources to damp ringing and slow edges.
  - Keep PWM switching node compact; place RC filter and op‑amp (0–10 V path) close to the source.
  - Add RC snubbers only where scope shows persistent overshoot/ringing.

- **Clock and crystal routing**
  - Keep crystal loop tiny and symmetric; place load caps next to pins; avoid long traces and vias.
  - Keep high‑speed digital away from the crystal/AREF area.

- **RS‑485/MODBUS physical layer**
  - Place TVS, common‑mode choke, and 120 Ω termination near the connector; keep the differential pair tightly coupled.
  - Provide bias resistors per RS‑485 segment rules; observe cable length and topology constraints.

- **Connector I/O and cabling**
  - Use 360° shield termination where applicable; consider ferrite beads on cable egress.
  - Keep high‑di/dt paths (drivers, switching regulators) away from analog inputs and connectors.

- **Ground stitching and zones**
  - Fence noisy regions with stitching vias; stitch grounds around board edges and near connector transitions.
  - Avoid creating unintended current loops with multiple AGND↔DGND ties.

- **Verification checklist**
  - Probe VCC at IC pins under load; confirm low ripple and minimal ringing.
  - Measure ADC noise floor with inputs shorted; verify improvement with AVCC/AREF filtering.
  - Scope PWM/GPIO rise/fall and overshoot at the load; tune series R/snubbers as needed.
  - Perform a pre‑scan for radiated EMI (30 MHz–1 GHz) to catch dominant harmonics early.

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

- **ISP Programming**: Arduino Uno R3 as in-system programmer
- **Serial Debug**: UART-based debugging and diagnostics
- **Hardware Testing**: Built-in self-test and diagnostic routines

### HIL Test Harness (arduino_test_wrapper.ino)

The HIL harness validates end-to-end behavior by electrically interfacing a USB‑connected Arduino wrapper to the ATmega32A DUT headers.

```
Host (Behave BDD + pytest infrastructure) ⇄ USB Serial ⇄ Arduino Wrapper (arduino_test_wrapper.ino)
                                             ⇅ Harness (GPIO/ADC/Counter)
                                        ATmega32A DUT (Sonicator Controller)
```

**Pin mapping (conceptual; see `config/hardware-config.yaml` as SOLE SOURCE OF TRUTH; `include/system_config.h` mirrors the YAML)**

| Function | DUT Signal | Wrapper Role |
|----------|------------|--------------|
| Overload (per unit) | Digital input from CT2000 | Drive to DUT or sense, per test |
| Frequency out ÷10 (per unit) | Digital pulse | Generate pulse trains at target Hz |
| Frequency Lock (per unit) | Digital | Drive/sense lock state |
| Start (per unit) | Digital output to CT2000 | Sense DUT output (level) |
| Overload Reset (per unit) | Digital output to CT2000 | Sense pulse one-shot |
| Power (per unit) | Analog (≈5.44 mV/W) | Provide calibrated analog level; readback optional |
| Amplitude (per unit) | Analog/PWM to CT2000 | Measure PWM/DAC via wrapper ADC |

Notes:

- Wrapper exposes a simple serial command protocol (documented in the wrapper source) to stimulate inputs and assert observed outputs.
- Safety-first: default lines to safe states; E‑stop/overload cannot be bypassed.

#### Optional Smart I/O Extender

To increase I/O headroom and improve timing fidelity, an auxiliary ATmega can act as a smart extender controlled by the Uno R4 wrapper.

- **Link**: UART between Uno R4 (host) and Extender (DUT‑side controller). Simple framed text or compact binary protocol.
- **Extender roles**:
  - Generate 4× frequency ÷10 waveforms (timer‑driven) to stimulate DUT.
  - Drive Overload and Freq‑Lock lines per unit.
  - Source Power analog levels (PWM+RC or onboard DAC if available).
  - Sense Start/Reset lines with pulse latching and report state.
- **Minimal host↔extender protocol** (example):
  - `X SET OVERLOAD <unit> <0|1>`
  - `X SET FLOCK <unit> <0|1>`
  - `X SET FREQ <unit> <Hz>`
  - `X SET POWER <unit> <watts>`
  - `X READ START <unit>` → `X RESP START <unit> <0|1>`
  - `X READ RESET_PULSE <unit>` → `X RESP RESET_PULSE <unit> <0|1>`
  - `X INFO` → `X RESP INFO <version> <mapping_checksum>`
- **Pin groups on extender (conceptual mapping to DUT signals; see `include/system_config.h`)**
  - Outputs → DUT inputs: Overload[4], FreqLock[4], Freq[4]
  - Inputs ← DUT outputs: Start[4], Reset[4]
  - Analog: Power[4] sources, Amplitude monitor (1) via divider

Notes:

- Keeps Uno focused on host protocol and CLI; extender ensures accurate timers and ample GPIO.
- If not needed, tests can serialize frequency checks on Uno‑only setup.

##### Concrete Pin Maps

###### Profile A — Uno‑only, per‑unit harness

Test one sonicator channel at a time by moving a small 6–7 wire harness between S1..S4.

| Role | Uno R3 Pin | Direction | Notes |
|------|------------|-----------|-------|
| Overload (selected unit) | D6 | Out | Drive DUT input (simulate CT2000 overload) |
| Freq Lock (selected unit) | D7 | Out | Drive DUT input |
| Frequency ÷10 (selected unit) | D3 | Out | Timer‑based square wave generator |
| Power analog source (selected unit) | D5 (PWM) | Out | PWM+RC to analog; calibrate to W via PRD scale |
| Start (selected unit) | D8 | In | Sense DUT output (level) |
| Reset (selected unit) | D9 | In | Sense pulse; latch in firmware |
| Amplitude 0–10 V (shared) | A0 | In (ADC) | Measure DUT amplitude output via divider |
| GND reference | GND | — | Common ground |

Notes:

- Move this harness to headers for S1, S2, S3, S4 to run identical tests per unit.
- See `include/system_config.h` for authoritative DUT pin roles; this mapping is wrapper‑side only.

###### Profile B — Uno + Smart I/O Extender (full parallel)

Uno focuses on host protocol; extender provides all DUT‑side signals.

| Role | Uno R4 Pin | Direction | Notes |
|------|------------|-----------|-------|
| Host↔Extender UART TX/RX | (e.g.) D10/D11 | TX/RX | Dedicated UART link (choose per wiring); native USB used for host |
| Optional extender reset | D12 | Out | Reset line to extender MCU |

Extender (conceptual groups; exact pins tbd on extender):

- Outputs → DUT inputs: Overload[4], FreqLock[4], Freq[4], Power[4]
- Inputs ← DUT outputs: Start[4], Reset[4]
- Analog: Amplitude monitor (1) via divider

The extender mirrors the same command set (SET/READ/INFO) over the UART link.

---

## 2. Existing Project Analysis

### Current Project State

- **Primary Purpose:** Industrial automation controller managing CT2000 sonicator units via MODBUS RTU
- **Current Tech Stack:** ATmega32A microcontroller, Arduino framework, PlatformIO build system
- **Architecture Style:** Layered architecture (HAL → Communication → Control Logic → Application)
- **Deployment Method:** ISP programming via Arduino Uno, embedded firmware deployment
- **Key Constraints:** 32KB Flash, 2KB SRAM, 16MHz clock, cooperative scheduling (no RTOS)

### Available Documentation

- Comprehensive architecture document with detailed layer descriptions
- MODBUS RTU register map specification
- PCB layout rules and design principles
- HIL test harness documentation with pin mappings
- Power architecture and safety monitoring details

### Identified Constraints

- **Memory Limits:** 32KB Flash and 2KB SRAM constrain multi-unit state management
- **Processing Power:** 16MHz ATmega32A must handle 4 concurrent units with 10Hz telemetry
- **Communication:** MODBUS RTU at 115200 baud with strict timing requirements
- **Safety:** Real-time overload detection and emergency shutdown capabilities
- **Testing:** HIL harness must simulate 4 sonicator units concurrently

**Architecture Version**: 0.1.0  
**Last Updated**: December 2024  
**Review Status**: Approved for implementation

## 3. Enhancement Scope and Integration Strategy

### Enhancement Overview

- **Enhancement Type:** Multi-unit firmware enhancement with safety features
- **Scope:** Extend single-sonicator control to 4 concurrent units with advanced safety monitoring
- **Integration Impact:** Medium - requires coordination with existing HAL and communication layers
- **Technical Approach:** Maintain existing cooperative scheduling while adding multi-unit orchestration

### Integration Approach

- **Code Integration Strategy:** Extend existing Sonicator Control Logic layer with multi-unit orchestration
- **Database Integration:** No database changes required (embedded system)
- **API Integration:** Extend existing MODBUS RTU register map with per-unit registers
- **UI Integration:** No UI changes (industrial PLC/HMI integration remains unchanged)

### Compatibility Requirements

- **Existing API Compatibility:** Maintain current MODBUS RTU protocol and register structure
- **Database Schema Compatibility:** N/A (no database)
- **UI/UX Consistency:** Maintain existing PLC/HMI interface patterns
- **Performance Impact:** Must not exceed existing 200ms command latency requirements
