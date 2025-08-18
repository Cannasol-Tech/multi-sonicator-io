@smoke @simulavr
Feature: Smoke
  Verify BDD harness and emulator runner can execute a passing test.

  Scenario: SMK-001 Harness runs a passing step
    Given the emulation profile is simulavr
    Then the harness sanity check passes
    And the serial device is available for the harness
