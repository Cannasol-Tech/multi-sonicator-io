Feature: CI drift checks
  # PRD §17 Risk

  @req-ci-drift @simulavr
  Scenario: SC-025 PRD vs include/config.h drift detection
    Given the system is initialized
    Then the CI drift check passes
