@hil @smoke
Feature: HIL Smoke Test
  Verify HIL wrapper connectivity and basic communication.
  
  Background:
    Given the HIL wrapper is connected and ready
  
  Scenario: HIL-001 Wrapper responds to PING
    Then the harness sanity check passes
    And the serial device is available for the harness
