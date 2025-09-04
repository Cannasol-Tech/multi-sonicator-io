Feature: Communication Acceptance
  @communication @implemented
  Scenario: Communication basic acceptance
    Given the hardware is initialized
    When the communication feature is exercised
    Then the expected result is observed

  @pending
  Scenario: Communication advanced acceptance (pending implementation)
    Given the hardware is initialized
    When the advanced communication feature is exercised
    Then the expected advanced result is observed
