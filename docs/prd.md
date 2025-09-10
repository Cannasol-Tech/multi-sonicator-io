# Sonicator Multiplexer Product Requirements Document (PRD)

## 1. Goals and Background Context

### 1.1. Goals

* Increase throughput per PLC cell by at least 3x compared to the single-channel baseline.
* Reduce operator intervention time per batch by 50% or more.
* Decrease unplanned downtime from overload events by at least 30% through a standardized reset flow.
* Ensure the PLC receives per-unit telemetry at the required cadence with less than 1% packet loss.
* Enable the HMI to start, stop, or reset any connected sonicator unit within a 200ms command latency.
* Maintain amplitude control accuracy within ±2% of the setpoint during steady-state operation.

### 1.2. Background Context

Currently, operations depend on manual or single-channel control of CT2000 sonicator units. This approach creates significant production bottlenecks, leads to inconsistent processing outcomes, and relies on fragmented, ad-hoc telemetry. The lack of a standardized, automated overload reset procedure further contributes to preventable downtime. These inefficiencies cap throughput, increase operator workload, and introduce quality assurance variability.

The proposed solution is a microcontroller-based multiplexer designed to manage up to four CT2000 units concurrently. It will expose a stable, well-defined MODBUS RTU interface for seamless integration with the primary PLC/HMI system. This system will provide granular, per-unit control (amplitude, start/stop, reset) and real-time monitoring (power, frequency, status), all validated through a comprehensive hardware-in-the-loop (HIL) testing framework to guarantee reliability before deployment.

### 1.3. Change Log

| Date       | Version | Description                             | Author    |
| :--------- | :------ | :-------------------------------------- | :-------- |
| 2025-09-01 | 1.0     | Initial draft based on Project Brief.   | John (PM) |
| 2025-09-01 | 1.1     | Updated requirements after brainstorming. | John (PM) |
| 2025-09-09 | 1.2     | Resolved all open questions, added FR12 for connected sonicators count, updated CT2000 specifications. | John (PM) |

## 2. Requirements

### 2.1. Functional Requirements

1. **FR1**: The system shall control up to four CT2000 sonicator units concurrently via a single interface.
2. **FR2**: The system shall provide individual start and stop control for each connected sonicator unit. The `START` command being active is the strict definition of a unit being "active".
3. **FR3**: The system shall provide individual amplitude control for each sonicator unit. If a value outside the 20-100% range is commanded, the system shall clamp it to the nearest valid limit (20% or 100%).
4. **FR4**: The system shall provide an individual overload reset command for each sonicator unit.
5. **FR5**: The system shall monitor and report the real-time power consumption (in Watts) for each sonicator unit.
6. **FR6**: The system shall monitor and report the real-time operating frequency (in Hz) for each sonicator unit.
7. **FR7**: The system shall monitor and report the status for each sonicator unit, including `Overload`, `Frequency Lock`, and a `COMM_FAULT` flag if a unit becomes physically disconnected.
8. **FR8**: The system shall expose all control and monitoring capabilities through a MODBUS RTU slave interface.
9. **FR9**: The system shall report the total number of currently active sonicators and a bitmask representing their individual running states.
10. **FR10**: On power-up, all sonicator units shall default to an 'off' state and require an explicit 'start' command to begin operation.
11. **FR11**: If communication with the master PLC/HMI is lost for more than one second, all sonicator units shall automatically enter a safe, non-operational state.
12. **FR12**: The system shall provide a `CONNECTED_SONICATORS_COUNT` MODBUS register that automatically detects and reports the number of physically connected and responsive sonicator units (0-4) by polling for signals from each sonicator module.

### 2.2. Non-Functional Requirements

1. **NFR1**: The end-to-end latency for any command (e.g., start, stop, reset) shall not exceed 200ms.
2. **NFR2**: Any change in a sonicator's state must be reflected in its corresponding MODBUS register within 100ms.
3. **NFR3**: The firmware must be implemented in C with a modular architecture that separates hardware abstraction, communication protocols, and control logic.
4. **NFR4**: All public functions and data structures in header files must be documented using Doxygen-style comments.
5. **NFR5**: The system must implement a watchdog timer to ensure it resets to a safe, non-operational state in the event of a software failure.
6. **NFR6**: The project must achieve a minimum of 90% unit test coverage using Unity Test Framework.
7. **NFR7**: All functional requirements must be verifiable through automated acceptance tests (simulation and/or Hardware-in-the-Loop).
8. **NFR8**: The MODBUS RTU communication must operate at 115200 baud with an 8N1 frame format.
9. **NFR9**: Telemetry data (power, frequency) for each unit shall be updated in the MODBUS registers at a minimum frequency of 10Hz.
10. **NFR10**: The system shall provide a non-volatile flag or dedicated register to indicate if the last system reset was caused by the watchdog timer.
11. **NFR11**: Firmware updates will be performed via a physical ISP header; no over-the-air update mechanism is required for the MVP.

## 3. Technical Assumptions

### 3.1. Repository Structure: Monorepo

The project will be contained within a single repository that includes firmware source code, testing suites, documentation, and CI/CD configurations.

### 3.2. Service Architecture: Monolith

The firmware will be a single, monolithic application running on the ATmega32A microcontroller. It will feature a layered internal architecture (HAL, Communication, Control Logic) but will be deployed as a single binary.

### 3.3. Testing Requirements: Full Testing Pyramid

The project will employ a comprehensive testing strategy that includes:

* **Unit Testing**: Unity Test Framework (C/C++) for individual modules and functions per organizational embedded standards.
* **Integration/Acceptance Testing**: Behavior-Driven Development (BDD) tests using pytest + Behave to validate requirements against HIL hardware.
* **Hardware-in-the-Loop (HIL) Testing**: BDD tests executed against physical hardware using a dedicated test harness to validate end-to-end functionality.

### 3.4. Additional Technical Assumptions and Requests

* **Firmware Language**: The primary language for the firmware will be C.
* **Development Framework**: The project will utilize the Arduino framework managed via PlatformIO for core libraries and toolchain management.
* **Scheduling**: The system will use a lightweight, non-blocking cooperative scheduler (`taskLoop`) and will not use a real-time operating system (RTOS).
* **CI/CD**: Continuous integration and deployment will be managed using GitHub Actions.
* **Frequency Measurement**: Implementation will use tick counting over set time periods for all four FREQ_DIV10_X pins to monitor 1.8kHz - 2.05kHz range simultaneously.
* **Sonicator Base Class**: Agnostic frequency monitoring implementation for 1.8kHz - 2.05kHz range to support CT2000 specifications.

## 4. Out of Scope

The following items are explicitly excluded from the scope of this project:

* **PLC/HMI Development**: The design, development, and testing of the PLC logic and HMI screens that will consume the MODBUS interface are the responsibility of the integration team and are not included in this project.
* **Hardware Design and Manufacturing**: This project covers the firmware for the specified ATmega32A-based multiplexer. The design, fabrication, and sourcing of the physical hardware are considered complete and outside the scope of this firmware development effort.
* **Cloud Connectivity and Data Logging**: While future enhancements may include cloud features, the initial version of this project will not implement any form of internet connectivity, cloud data streaming, or remote telemetry logging.
* **Advanced Power Analytics**: The system will report real-time power consumption (Watts). Any further analysis, such as historical trending, efficiency calculations, or predictive maintenance analytics, is not in scope.
* **Dynamic Reconfiguration**: The system is designed to control up to four pre-defined sonicator units. It will not support dynamic addition or removal of units at runtime.

## 5. Epic List

### Epic 1: Foundational Control & Communication

**Goal:** Establish the core firmware structure, basic sonicator control, and the MODBUS communication layer, enabling control and monitoring of a single sonicator.

* Implement the basic project structure, CI/CD pipeline, and build system.
* Develop the MODBUS RTU slave interface with the register map.
* Implement control logic for a single sonicator (Start/Stop, Amplitude).
* Implement telemetry for a single sonicator (Power, Frequency, Status).
* Write unit tests for the initial modules.

### Epic 2: Multi-Sonicator Management & Safety Features

**Goal:** Scale the system to manage all four sonicators concurrently and implement critical safety and reliability features.

* Extend control and telemetry logic to handle four sonicators.
* Implement the overload reset functionality.
* Integrate the watchdog timer for system stability.
* Implement the communication loss failsafe mechanism.
* Develop acceptance tests for multi-sonicator scenarios.

### Epic 3: HIL-Validation and Finalization

**Goal:** Validate the complete system against physical hardware and finalize documentation and testing requirements for release.

* Set up the Hardware-in-the-Loop (HIL) test harness.
* Execute the full suite of acceptance tests on the physical hardware.
* Achieve the 85% unit test coverage target.
* Generate and review the final Doxygen documentation.
* Perform a 24-hour soak test to ensure system reliability.

## 6. Success Metrics

The success of this project will be measured against the following key performance indicators (KPIs), which are directly tied to the project goals.

### 6.1. Primary Metrics

* **Throughput Increase**: The system must demonstrate control of at least three sonicator units simultaneously in a production-like environment, validating the 3x throughput goal.
* **Command Latency**: End-to-end command latency (from PLC command issuance to sonicator action) must be verified to be under 200ms for all control actions (start, stop, reset).
* **Telemetry Accuracy & Cadence**:
  * Power and frequency telemetry must update in MODBUS registers at a rate of at least 10Hz.
  * Amplitude control must be maintained within ±2% of the setpoint.
* **System Reliability**:
  * The system must pass all HIL acceptance tests without failure for a continuous 24-hour soak test.
  * The watchdog timer must correctly reset the system into a safe state upon a forced software hang.

### 6.2. Secondary Metrics

* **Test Coverage**: Unit test coverage must meet or exceed the 85% target.
* **Documentation Quality**: All public APIs in the firmware must be fully documented with Doxygen comments, and the generated documentation must be complete and free of errors.
* **Integrator Satisfaction**: The integration team must confirm that the provided MODBUS register map and documentation are clear and sufficient for PLC/HMI integration.

## 7. Timeline and Milestones

This project is divided into three main epics, each concluding with a significant milestone.

* **Epic 1: Foundational Control & Communication (Estimated Duration: 2 Weeks)**
  * **Milestone:** A single sonicator can be fully controlled and monitored via the MODBUS interface. The core firmware architecture and CI/CD pipeline are operational.

* **Epic 2: Multi-Sonicator Management & Safety Features (Estimated Duration: 2 Weeks)**
  * **Milestone:** The system can reliably control and monitor all four sonicators concurrently. All safety features (overload reset, watchdog, failsafe) are implemented and pass initial tests.

* **Epic 3: HIL-Validation and Finalization (Estimated Duration: 1 Week)**
  * **Milestone:** The firmware is fully validated on physical hardware, meets all performance and reliability requirements, and is ready for deployment. All documentation and test coverage targets are met.

## 8. Risks and Mitigations

* **Risk:** Hardware integration with the CT2000 units reveals unexpected behavior not covered in the datasheet.
  * **Mitigation:** Allocate buffer time in the schedule for an initial hardware discovery phase. Develop a flexible hardware abstraction layer (HAL) to quickly adapt to minor hardware variations.

* **Risk:** MODBUS RTU implementation proves to be unreliable under heavy load or in noisy electrical environments.
  * **Mitigation:** Implement robust error checking (CRC) and a retry mechanism in the communication protocol. Conduct extensive HIL testing under simulated high-load and noisy conditions.

* **Risk:** The ATmega32A microcontroller lacks the processing power to handle four sonicators at the required 10Hz telemetry update rate while managing the communication stack.
  * **Mitigation:** Profile the application early and continuously. Optimize critical code paths. If performance is still an issue, evaluate reducing telemetry frequency as a trade-off with the stakeholder.

* **Risk:** Delays in receiving the physical hardware for HIL testing could impact the project timeline.
  * **Mitigation:** Rely heavily on the simulation environment for initial development and validation. Maintain open communication with the hardware team to get the earliest possible access to a prototype.

## 9. Open Questions

*All open questions have been resolved as of September 9, 2025.*

### 9.1. Resolved Questions

#### ✅ CT2000 Specifications (Resolved 2025-09-09)

* **Frequency Range:** 18kHz - 20.5kHz operating range
* **FREQ_DIV10_X Output:** 1.8kHz - 2.05kHz (frequency divided by 10 for digital monitoring)
* **Power Output:** 0-8.160 VDC representing 0-100% power operation
* **Power Scaling:** 5.44mV = 1 WATT (maximum ~1,500 Watts at 8.160V)

#### ✅ Hardware Verification (Resolved 2025-09-09)

* **Overload Reset Pin 2:** Confirmed available and functional
* **Internal Amplitude Control Switch:** Verified existence and functionality

#### ✅ ATmega32A Pin Allocation (Resolved 2025-09-09)

* **Hardware Configuration:** Documented as SOLE SOURCE OF TRUTH in `config/hardware-config.yaml`
* **Pin Capacity:** 40 pins available vs 39 pins required - sufficient capacity confirmed

#### ✅ Frequency Measurement Implementation (Resolved 2025-09-09)

* **Approach:** Monitor ticks over set time period for all four FREQ_DIV10_X pins
* **Base Class:** Agnostic monitoring for 1.8kHz - 2.05kHz range
* **Implementation:** Tick counting method for simultaneous 4-channel measurement

#### ✅ System Redundancy (Resolved 2025-09-09)

* **Hardware Redundancy:** Not implemented in current design
* **Safety Mechanisms:** Comprehensive failsafe operations already defined (Watchdog timer, communication loss failsafe, safe power-up state)
* **Conclusion:** Existing safety mechanisms provide adequate protection

#### ✅ CE Marking (Resolved 2025-09-09)

* **Requirement:** Not applicable - US Market only deployment
* **European Conformity:** Not required for this project scope

#### ✅ Backward Compatibility (Resolved 2025-09-09)

* **Approach:** `CONNECTED_SONICATORS_COUNT` MODBUS register for auto-detection
* **Implementation:** ATmega32A polls signals from four sonicator modules
* **Testing:** Not required - multiplexer serves as source of truth for connected units

### 9.2. Testing Limitations

* **Power Verification Testing:** DUT can only verify low-end power output until upgraded power supply capable of full 8.160V output is implemented for complete testing capability.

## 10. Glossary

* **CT2000:** The specific model of the sonicator unit being controlled by the multiplexer. Operating frequency range: 18kHz - 20.5kHz. Power output: 0-8.160 VDC (0-100% operation). Power scaling: 5.44mV = 1 WATT.
* **FREQ_DIV10_X:** Digital frequency output signal from CT2000 sonicators, representing the operating frequency divided by 10 (1.8kHz - 2.05kHz range).
* **Firmware:** Software embedded within the microcontroller, responsible for all control logic and communication.
* **HAL (Hardware Abstraction Layer):** A software layer that isolates the core application logic from the specific microcontroller hardware, improving portability.
* **HIL (Hardware-in-the-Loop):** A testing methodology where the final firmware is run on the actual microcontroller while simulating the external hardware (sonicators, PLC) to validate real-world interactions.
* **HMI (Human-Machine Interface):** The graphical interface on the PLC system used by operators to control and monitor the sonicators.
* **MODBUS RTU:** A master-slave serial communication protocol used for industrial control systems. The multiplexer acts as the slave device.
* **Multiplexer:** The hardware device, controlled by the firmware, that manages communication and control signals for up to four sonicator units.
* **PLC (Programmable Logic Controller):** The industrial computer that acts as the master controller, sending commands to and receiving data from the multiplexer via MODBUS.
* **Sonicator:** A device that uses ultrasonic energy to process materials.
* **Telemetry:** Data related to the status and performance of the sonicators (e.g., power, frequency, status flags) transmitted from the multiplexer to the PLC.
* **Watchdog Timer:** A hardware safety feature that resets the microcontroller if the main application freezes or becomes unresponsive.

## 11. Appendix

* **[ATmega32A Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-8155-8-bit-AVR-Microcontroller-ATmega32A_Datasheet.pdf):** Official datasheet for the selected microcontroller.
* **[MODBUS Protocol Specification](https://modbus.org/specs.php):** Official documentation for the MODBUS communication protocol.
* **CT2000 Sonicator Documentation:** (Placeholder for link to the specific technical manual for the sonicator units).
