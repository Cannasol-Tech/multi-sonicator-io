# **Multi Sonicator I/O Controller - Product Requirements Document (PRD)**

**Date:** 2025-06-26
**Author:** Stephen Boyett
**Industry:** Software & Industrial Automation

---

## Overview

The Multi Sonicator I/O Controller is an industrial automation module developed for Cannasol Technologies to extend the control capabilities of their CT2000 sonicators. This system enables a single controller, based on the ATmega32A microcontroller, to simultaneously monitor and control up to four sonicators over isolated DB9 serial interfaces, integrating with an existing MODBUS RTU infrastructure and remote mobile app via Firebase.

---

## Objectives

* Replace single-sonicator control with scalable multi-unit management (up to 4 sonicators).
* Leverage existing HMI and PLC infrastructure (Velocio 1630c) via MODBUS RTU.
* Extend existing mobile app and Firebase architecture for real-time remote control and monitoring.
* Ensure industrial-grade reliability, EMI resistance, and modular expandability.
* Maintain full safety and FDA/pharma-compliant logging and control integrity.

---

## Scope

* **Hardware Design**: PCB design, power regulation, signal isolation, DB9 interfaces, enclosure.
* **Firmware Development**: MODBUS RTU implementation, signal capture and actuation, system diagnostics.
* **System Integration**: HMI and PLC ladder logic updates, Firebase schema updates, app UI expansion.
* **Testing and Validation**: Full HIL (hardware-in-loop), regression, EMI and thermal stress testing.

---

## Functional Requirements

### Hardware

* ATmega32A-PU (DIP-40) with external 16MHz crystal.
* Four isolated DB9 female ports to interface with CT2000 sonicators:

  * Inputs: Overload indication, frequency lock, frequency output, power output
  * Outputs: Amplitude control (0-10V via DAC), sonics start, overload reset
* Industrial power supply: 24V input, cascaded 12V and 5V local regulation.
* Professional 4-layer PCB (100x80mm) with DIN rail mounting in IP65 aluminum enclosure.
* Cable and connector design supporting EMI-resilient deployment with liquid-tight conduit.

### Software

* Real-time embedded firmware using PlatformIO (Arduino framework) with AVR-GCC.
* Object-oriented software modules:

  * `SonicatorInterface` (1 per unit): monitor/control
  * `SystemController`: orchestration, MODBUS register mapping
  * `CommunicationManager`: MODBUS RTU slave handler
  * `SafetyMonitor`: overload detection, emergency stop
  * `DiagnosticsSystem`: runtime stats, error reporting
* Support for accurate 50us response time for safety-critical interrupts.
* MODBUS RTU via UART (115200 baud) with CRC-16 and slave ID configuration.

---

## Non-Functional Requirements

* EMI shielding and thermal dissipation suitable for sonicator environment.
* CE, FCC Part 15, and optional UL/FDA documentation compliance.
* Modular and extensible for future 8/16-sonicator versions.
* Remote firmware update readiness via ISP programming header.
* Diagnostic LED array for all status indicators (power, run, error, communication, per-sonicator LEDs).

---

## User Stories

### Operators and Technicians

* As an **automation engineer**, I want to control all sonicators from the HMI, so I can automate production.
* As a **technician**, I want to reset sonicators remotely after an overload, so I don’t need to enter cleanrooms.
* As a **line operator**, I want to see each sonicator’s run/stop status on the HMI, so I can confirm they're active without walking the floor.
* As a **process technician**, I want real-time alarms for frequency lock loss or overloads, so I can take immediate corrective action.

### App and Supervisory Roles

* As a **manager**, I want to monitor power output trends from the mobile app, so I can track production efficiency.
* As a **remote supervisor**, I want to get push notifications for overload events, so I can alert the team even when off-site.
* As a **quality engineer**, I want to view historical sonicator runtime logs, so I can validate process consistency.

### Engineering and Integration

* As a **maintenance engineer**, I want to access system diagnostics via MODBUS registers, so I can troubleshoot issues without disassembling the panel.
* As a **test engineer**, I want to simulate sonicator signals via a test harness, so I can validate firmware without live equipment.
* As a **systems integrator**, I want to add new units by editing MODBUS register maps, so I can expand without rewriting code.
* As a **developer**, I want modular firmware classes, so I can reuse logic for future expansion.
* As a **developer**, I want to configure slave ID via firmware or DIP switch, so the controller works in multi-device environments.

### Firmware Development and Safety

* As a **firmware developer**, I want the `SonicatorInterface` class to expose clean APIs for diagnostics and commands, so I can maintain code easily.
* As a **developer**, I want interrupt-safe design for emergency shutdowns, so I can ensure system safety under all conditions.
* As a **debugger**, I want an LED status panel that reflects each sonicator’s state, so I can visually confirm operation without a PC.
* As a **compliance engineer**, I want audit trails for each safety event, so I can validate the system against FDA process control requirements.

---

## Milestones & Timeline

| Milestone                           | Target Date |
| ----------------------------------- | ----------- |
| Finalize PCB and enclosure design   | 2025-06-30  |
| Complete firmware MVP (single unit) | 2025-07-05  |
| Full 4-unit firmware integration    | 2025-07-12  |
| HMI & PLC ladder logic update       | 2025-07-14  |
| Firebase and mobile app updates     | 2025-07-16  |
| System integration and test         | 2025-07-20  |
| Customer validation and install     | 2025-07-25  |

---

## Risks & Mitigations

| Risk                                | Likelihood | Impact | Mitigation                                 |
| ----------------------------------- | ---------- | ------ | ------------------------------------------ |
| MODBUS sync issues                  | Medium     | High   | Early testing with PLC and MODBUS slave    |
| EMI-induced errors                  | Medium     | High   | Shielded cables, opto-isolation, filtering |
| Supply chain delay (DACs, PCB fabs) | Low        | Medium | Pre-order parts, maintain alternates       |
| Firmware bugs in safety logic       | Medium     | High   | HIL testing with sonicator simulators      |

---

## Assumptions

* CT2000 sonicators conform exactly to pinout and voltage specs described.
* Main automation box supports 24V delivery and has spare DB9 port.
* ESP32 and Firebase infrastructure are already deployed and functioning.
* Arduino UNO R4 WiFi will only be used during prototyping; USBasp or similar for production.

---
