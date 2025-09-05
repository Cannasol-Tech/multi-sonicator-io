@smoke @hil
Feature: Smoke
  Verify BDD harness and Arduino Test Wrapper can execute a passing test.

  Scenario: SMK-001 Harness runs a passing step
    Given the HIL test harness is connected
    Then the harness sanity check passes
    And the Arduino Test Wrapper is available for testing
