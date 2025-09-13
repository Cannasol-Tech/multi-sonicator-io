@hil @hardware @documentation
@trace:hil-testing-framework
Feature: HIL Testing Framework & Hardware Verification
  As a test engineer
  I want a verified HIL framework and hardware connections
  So that firmware can be developed and validated reliably against real hardware

  # Traceability
  # Source: docs/features/FEATURE-002-hil-testing-framework-hardware-verification.md
  # PRD: docs/prd.md §2 (FR1–FR12), §3.3, §6.1, §6.2
  # Hardware: config/hardware-config.yaml (SOLE SOURCE OF TRUTH)

  @pending @hil @TODO:pin-matrix-verification @trace:hil-pin-matrix
  Scenario: Pin Matrix Verification against authoritative mapping
    Given the HIL framework is initialized
    And the ATmega32A DUT is connected to the Arduino Test Wrapper
    When the pin verification process executes against config/hardware-config.yaml
    Then all declared pins shall be detected and validated with 100% accuracy
    And a human-readable report is produced with mismatches and tolerances

  @pending @hil @TODO:hil-bdd-execution @trace:hil-bdd-execution
  Scenario: Execute BDD scenarios on physical hardware
    Given the HIL framework is operational
    When the acceptance suite is executed with profile "hil"
    Then scenarios shall run against the DUT and produce pass/fail results
    And results shall be archived for executive reporting

  @pending @hil @web-ui @TODO:web-ui-modbus-monitoring @trace:web-ui-modbus
  Scenario: Web UI displays real-time MODBUS values
    Given the Web UI testing interface is connected to the DUT
    When the user monitors MODBUS holding registers
    Then real-time values shall be displayed with under 1 second latency
    And register updates shall reflect hardware state changes

  @pending @hil @cli @TODO:sandbox-cli-debug @trace:sandbox-cli
  Scenario: Sandbox CLI provides manual hardware control
    Given the Sandbox CLI is connected to the DUT via the HIL framework
    When the engineer issues manual control commands
    Then the DUT shall respond and log actions with timestamps

  @pending @hil @programming @TODO:isp-programming @trace:isp-interface
  Scenario: ISP programming interface functional
    Given the Arduino as ISP is prepared per project instructions
    When the test firmware is uploaded to the DUT via ISP
    Then the upload shall succeed and the DUT shall boot the new image

  @pending @hil @ct2000 @TODO:ct2000-interface-validation @trace:ct2000-signals
  Scenario: CT2000 interface signals are validated end-to-end
    Given CT2000 interface harnesses are connected (all 4 channels)
    When frequency, power, start/reset, and frequency-lock signals are exercised
    Then measurements shall match expected ranges and semantics per PRD
