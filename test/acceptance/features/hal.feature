Feature: HAL Acceptance
  @hal @implemented
  Scenario: HAL basic acceptance
    Given the hardware is initialized
    When the HAL feature is exercised
    Then the expected result is observed

  @pending
  Scenario: HAL advanced acceptance (pending implementation)
    Given the hardware is initialized
    When the advanced HAL feature is exercised
    Then the expected advanced result is observed
