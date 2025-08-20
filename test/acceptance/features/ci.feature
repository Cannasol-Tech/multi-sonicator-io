Feature: CI drift checks
  # PRD §17 Risk

  @req-ci-drift @simulavr
  Scenario: SC-025 PRD vs include/config.h drift detection
    Given the system is initialized
    Then the CI drift check passes

  # @req-ci-coverage @pending @simulavr
  # Scenario: SC-026 Coverage gate >= 85%
  #   Given acceptance and unit tests have executed in CI
  #   Then the aggregated coverage report meets or exceeds 85%
  #   And the CI job passes the coverage gate
