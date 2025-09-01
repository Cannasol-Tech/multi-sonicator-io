# 3. Technical Assumptions

## 3.1. Repository Structure: Monorepo

The project will be contained within a single repository that includes firmware source code, testing suites, documentation, and CI/CD configurations.

## 3.2. Service Architecture: Monolith

The firmware will be a single, monolithic application running on the ATmega32A microcontroller. It will feature a layered internal architecture (HAL, Communication, Control Logic) but will be deployed as a single binary.

## 3.3. Testing Requirements: Full Testing Pyramid

The project will employ a comprehensive testing strategy that includes:

* **Unit Testing**: C++ based tests for individual modules and functions.
* **Integration/Acceptance Testing**: Behavior-Driven Development (BDD) tests using Behave to validate requirements against a simulated environment.
* **Hardware-in-the-Loop (HIL) Testing**: BDD tests executed against physical hardware using a dedicated test harness to validate end-to-end functionality.

## 3.4. Additional Technical Assumptions and Requests

* **Firmware Language**: The primary language for the firmware will be C.
* **Development Framework**: The project will utilize the Arduino framework managed via PlatformIO for core libraries and toolchain management.
* **Scheduling**: The system will use a lightweight, non-blocking cooperative scheduler (`taskLoop`) and will not use a real-time operating system (RTOS).
* **CI/CD**: Continuous integration and deployment will be managed using GitHub Actions.
