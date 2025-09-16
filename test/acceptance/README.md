# Acceptance & Integration Testing

Location: `test/acceptance/`

## Overview

This directory contains the unified testing framework for all Acceptance and Integration tests. As defined by the `docs/sop/sw-testing.md`, these tests run as:

- **Hardware-in-the-Loop (HIL) tests** when physical hardware is available.
- **Emulation tests** when hardware is unavailable (e.g., in a CI pipeline).

**Important**: As of the CI pipeline solidification (Issue #18), acceptance tests are **MANUAL EXECUTION ONLY**. They are not run automatically in CI pipelines to ensure hardware independence and faster CI feedback.

It uses the Behave BDD framework to run the same scenarios against either a real or a simulated hardware target.

Always adhere to the standards outlined in `docs/sop/sw-testing.md`.

## Purpose

- Run Behave BDD scenarios against either a real (HIL) or simulated (emulation) hardware target.
- Stimulate DUT inputs and observe outputs to validate MODBUS contract end-to-end.
- Validate timing and performance requirements.
- Validate pin-level interactions and timing.
- Validate PRD Functionality Requirements through BDD Scenarios outlined in the features and steps directories

## Testing Strategy

### CI Pipeline (Automated)
- **Unit Tests Only**: CI runs only unidates BDD scenario syntax but does not execute tests
- **Fast Feedback**: Quick validation for pull requests and development workflow
- **Hardware Independence**: No physical hardware required for CI

### Manual Testing (Required for Release)
- **Acceptance Tests**: Manual execution of BDD scenarios with HIL hardware
- **Complete Coverage**: Full test suite execution including HIL scenarios
- **Release Validation**: Comprehensive testing before releases
- **Executive Reports**: Generated from manual test execution results

## Directory Structure

- `hil_framework/`: Contains the HIL framework for hardware testing.
- `features/`: Contains `.feature` files written in Gherkin syntax. Each file describes a feature and its scenarios.
- `steps/`: Contains Python scripts that implement the steps defined in the `.feature` files.
- `sketches/`: Contains Arduino sketches for the test environment, including the test wrapper for HIL.
- `environment.py`: Behave environment file for test setup and teardown.

Components:

- `sketches/ArduinoISP.ino`: ArduinoISP sketch for programming the ATmega32A DUT.
- `sketches/arduino_test_wrapper.ino`: Arduino sketch exposing a simple serial command protocol.

Protocol (draft):

- ASCII line protocol over USB CDC. Examples:
  - `PING` -> `OK PONG`
  - `SET FREQ <unit> <hz>`
  - `SET OVERLOAD <unit> <0|1>`
  - `SET FREQ_LOCK <unit> <0|1>`
  - `READ STATUS <unit>`
  - `PULSE RESET <unit> <ms>`
- Finalize command set alongside DUT harness and keep in sync with `include/system_config.h`.

Getting Started:

1. Open the sketch in Arduino IDE/CLI and flash to the wrapper board.
2. Connect the wrapper to the DUT harness (pins per harness doc).
3. From a terminal, verify with a serial monitor: send `PING`, expect `OK PONG`.
4. Point Behave HIL profile to the wrapper serial port (to be defined in `behave.ini`/env var).

Notes:

- Wrapper must default to safe states on boot and protocol errors.
- Keep harness mapping documented and versioned with this folder.
- Current harness profile: Arduino Uno R4 WiFi, single-channel (S1 only). Untested prototype; do not enable in CI.
- See `config/hardware-config.yaml` for the authoritative mapping (SOLE SOURCE OF TRUTH; wrapper_pin and test_point fields).
