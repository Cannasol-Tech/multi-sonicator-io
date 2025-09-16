@modbus @documentation
@trace:modbus-interface
@requirement_FR8 @story_0_3_AC3 @story_0_3_AC4 @story_0_3_AC6
Feature: MODBUS RTU Conformance & Performance
  As a PLC integration engineer
  I need the device to conform to MODBUS RTU and meet performance targets
  So that integration is seamless and reliable under industrial conditions

  # Traceability
  # Source: docs/features/modbus-communication-interface.md
  # PRD: docs/prd.md §2 (FR8), §2.2 (NFR2, NFR8), §6.1

  @pending @hil @TODO:modbus-rtu-conformance @trace:modbus-conformance
  Scenario: MODBUS protocol conformance (FC03/FC06/FC16 basics)
    Given the device is configured at 115200 baud, 8N1 with a valid slave ID
    When standard read and write operations are executed across defined registers
    Then responses shall conform to MODBUS RTU framing and semantics

  @pending @hil @TODO:modbus-performance @trace:modbus-performance
  Scenario: Communication performance under load
    Given the system is under nominal operating conditions
    When 100 consecutive read transactions are performed
    Then each response time shall be under 100 ms and CRCs valid

  @pending @hil @TODO:modbus-error-handling @trace:modbus-errors
  Scenario: Illegal address and CRC error handling
    Given the system is operational
    When requests target illegal addresses or include bad CRCs
    Then the device shall respond with appropriate exceptions or remain silent

  @pending @hil @TODO:modbus-legacy-addressing @trace:modbus-legacy
  Scenario: Legacy 4xxxx addressing examples for HMI
    Given an HMI using 4xxxx addressing convention
    When addresses are mapped to device registers per documentation
    Then reads and writes shall operate correctly with documented scaling
