Feature: Executive reporting
  # PRD §1, §14
  @pending

  @req-exec-report @hil
  Scenario: SC-011 Executive report artifact
    Given the system is initialized
    Then the CI pipeline generates a valid executive report
