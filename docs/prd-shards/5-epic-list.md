# 5. Epic List

## Epic 1: Foundational Control & Communication

**Goal:** Establish the core firmware structure, basic sonicator control, and the MODBUS communication layer, enabling control and monitoring of a single sonicator.

* Implement the basic project structure, CI/CD pipeline, and build system.
* Develop the MODBUS RTU slave interface with the register map.
* Implement control logic for a single sonicator (Start/Stop, Amplitude).
* Implement telemetry for a single sonicator (Power, Frequency, Status).
* Write unit tests for the initial modules.

## Epic 2: Multi-Sonicator Management & Safety Features

**Goal:** Scale the system to manage all four sonicators concurrently and implement critical safety and reliability features.

* Extend control and telemetry logic to handle four sonicators.
* Implement the overload reset functionality.
* Integrate the watchdog timer for system stability.
* Implement the communication loss failsafe mechanism.
* Develop acceptance tests for multi-sonicator scenarios.

## Epic 3: HIL-Validation and Finalization

**Goal:** Validate the complete system against physical hardware and finalize documentation and testing requirements for release.

* Set up the Hardware-in-the-Loop (HIL) test harness.
* Execute the full suite of acceptance tests on the physical hardware.
* Achieve the 85% unit test coverage target.
* Generate and review the final Doxygen documentation.
* Perform a 24-hour soak test to ensure system reliability.
