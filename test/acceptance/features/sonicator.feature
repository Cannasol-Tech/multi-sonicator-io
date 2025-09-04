Feature: Sonicator Acceptance
  @sonicator @implemented
  Scenario: Sonicator basic acceptance
    Given the hardware is initialized
    When the sonicator feature is exercised
    Then the expected result is observed

  @pending
  Scenario: Sonicator advanced acceptance (pending implementation)
    Given the hardware is initialized
    When the advanced sonicator feature is exercised
    Then the expected advanced result is observed
