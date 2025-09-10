# 5. Epic List

## Epic 0: HIL Testing Framework & Hardware Verification

**Goal:** Establish a Hardware-in-the-Loop (HIL) testing framework to validate firmware behavior against actual hardware, ensuring reliable integration before deployment.

* Implement the basic project structure, CI/CD pipeline, and build system according to Company Standards 
* Set up the Hardware-in-the-Loop (HIL) test harness.
* Implement the BDD acceptance testing framework with HIL integration.
* Verify all hardware connections and pin assignments against the approved pin matrix.
* Validate the Arduino Test Wrapper and ISP programming interface.
* Implement the sandbox CLI for manual HIL testing and debugging.
* Validate the MODBUS communication layer with hardware.
* Validate the HIL testing framework with hardware.
* Verify that the Web User interface is functioning correctly for Signals and MODBUS register verification
* Verify that the Web User interface allows for continuous manual configuration and monitoring of a single Sonicator Module in the ATmega32a
* Verify that the Web User interface allows for Acceptance Testing of the ATmega32a
* Verify that the Web User interface allows for running BDD scenarios individually, per tag, and all together
* Execute the full suite of acceptance tests on the physical hardware.
* Achieve the 85% unit test coverage target.
* Generate and review the final Doxygen documentation.
* Perform a 24-hour soak test to ensure system reliability.

## Epic 1: Foundational Control & Communication

**Goal:** Establish the core firmware structure, basic sonicator control, and the MODBUS communication layer, enabling control and monitoring of a single sonicator.

* Develop the MODBUS RTU slave interface with the register map.
* Implement a single source of truth enumeration for the MODBUS register map in `include/register_map.h` file
* Develop a Single Sonicator Module class in `src/modules//sonicator.h`
* Develop a Modular HAL for the ATmega32a in `src/modules/hal/`

  *HAL Requirement:*
  * HAL should be written as modular as possible containing class for (at minimum):
        1. GPIO pin Interface
        2. ADC Interface
        3. PWM Interface
        4. UART Interface
        5. Timer Interface

* Implement control logic for a single sonicator (Start/Stop, Amplitude).
* Implement telemetry for a single sonicator (Power, Frequency, Status).
* Write unit tests for the initial modules.
* Include Function Requirements for configuring and monitoring a single sonicator
* Include Funcional Requirements for all safety features for a single sonicator
* Create Acceptance Test BDD Scenarios for all Functinal Requirements
* Verify all MODBUS register access for the single sonicator using Web User interface
* Verify all MODBUS register access for the single sonicator using Web User interface

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
