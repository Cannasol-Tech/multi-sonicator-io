Feature: Module 1 Acceptance
  @module_1 @implemented
  Scenario: Module 1 basic acceptance
    Given the hardware is initialized
    When the module 1 feature is exercised
    Then the expected result is observed

  @pending
  Scenario: Module 1 advanced acceptance (pending implementation)
    Given the hardware is initialized
    When the advanced feature is exercised
    Then the expected advanced result is observed
