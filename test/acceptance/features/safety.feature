@requirement_FR11 @story_0_3_AC3 @story_0_3_AC4 @story_0_3_AC8
Feature: Safety
  # PRD §11

  @req-watchdog-docs @pending
  Scenario: SC-012 Watchdog policy is documented
    Given the system is initialized
    Then the watchdog policy documentation is present

  @req-estop @hil @pending
  Scenario: SC-014 Emergency stop register behavior
    Given the system is initialized
    When I trigger emergency stop
    Then holding register 40026 should equal 1 within 50 ms

  @requirement_FR11 @pending
  Scenario: SC-021 Loss of communication triggers safe state
    Given the system is initialized
    When communication with the PLC/HMI is lost for more than 1 second
    Then all sonicators enter a safe non-operational state within 200 ms
