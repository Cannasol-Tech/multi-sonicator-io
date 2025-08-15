Feature: Safety
  # PRD §11

  @req-watchdog-docs
  Scenario: SC-012 Watchdog policy is documented
    Given the system is initialized
    Then the watchdog policy documentation is present

  @req-estop @hil
  Scenario: SC-014 Emergency stop register behavior
    Given the system is initialized
    When I stimulate estop to 1
    Then holding register 40026 should equal 1 within 50 ms
