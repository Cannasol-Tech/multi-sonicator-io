Feature: Executive reporting
  # PRD §1, §14

  @req-exec-report @simulavr
  Scenario: SC-011 Executive report artifact
    Given the system is initialized
    Then the CI pipeline generates a valid executive report
