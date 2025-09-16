@web-ui @acceptance-testing @multiple-execution
Feature: Multiple Acceptance Test Web-UI Execution and Results Verification
  As a test engineer
  I want to run multiple acceptance tests from the web-ui and verify the results
  So that I can execute test suites and view comprehensive results

  Background:
    Given the web-ui is running in simulation mode
    And the Test Automation panel is loaded
    And acceptance test scenarios are loaded from the backend
    And the hardware mock is enabled

  @smoke @multiple-tests @execution
  Scenario: Execute multiple acceptance tests and wait for completion
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I select multiple test scenarios: "Hardware Basic Connectivity" and "GPIO Functionality"
    And I click the "Run Selected" button
    Then the test execution should start
    And I should see a progress indicator for the overall execution
    And the execution status should show "running"
    And I should see which scenario is currently executing
    And I should wait for all tests to finish
    And the final status should show overall results (passed/failed/mixed)

  @multiple-tests @batch-execution
  Scenario: Execute full test suite (all scenarios)
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I click the "Select All" button
    And I click the "Run Selected" button
    Then all acceptance test scenarios should start executing
    And I should see the total number of scenarios to be executed
    And the progress should show "X of Y scenarios completed"
    And each scenario should execute in sequence
    And I should wait for the entire suite to complete
    And the final results should show aggregate statistics

  @multiple-tests @results-summary
  Scenario: Verify multiple test results summary is displayed
    Given I have executed multiple acceptance tests
    And all tests have completed
    When I view the test results
    Then I should see a comprehensive results summary
    And the summary should show total scenarios executed
    And the summary should display passed/failed/error counts
    And the summary should show total execution time
    And individual scenario results should be listed below
    And each scenario should show its name, status, and duration

  @multiple-tests @progress-tracking
  Scenario: Track progress during multiple test execution
    Given I am running multiple acceptance tests
    And the execution is in progress
    When I monitor the test progress
    Then I should see the current scenario being executed
    And the overall progress percentage should update
    And the number of completed vs total scenarios should be shown
    And the estimated remaining time should be displayed (if available)
    And previously completed scenarios should show their final status

  @multiple-tests @mixed-results
  Scenario: Handle mixed results (some pass, some fail) in multiple test execution
    Given I have executed multiple acceptance tests
    And some tests passed while others failed
    When I view the results
    Then the overall status should indicate "mixed" or "partial success"
    And passed scenarios should be clearly marked as successful
    And failed scenarios should be highlighted with error indicators
    And I should be able to filter results by status (passed/failed)
    And there should be an option to re-run only failed tests

  @multiple-tests @execution-interruption
  Scenario: Stop multiple test execution mid-run
    Given I am running multiple acceptance tests
    And several scenarios have completed
    And at least one scenario is still executing
    When I click the "Stop Execution" button
    Then the current scenario execution should stop
    And any remaining queued scenarios should be cancelled
    And completed scenarios should retain their results
    And the overall status should show "stopped" or "cancelled"
    And I should be able to start a new execution

  @multiple-tests @tag-based-execution
  Scenario: Execute multiple tests filtered by tags
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I filter scenarios by the "smoke" tag
    And I select all filtered scenarios
    And I click the "Run Selected" button
    Then only the scenarios with "smoke" tags should execute
    And the progress should reflect the filtered scenario count
    And the results should only show the executed scenarios
    And tag information should be preserved in the results

  @multiple-tests @results-export
  Scenario: Export multiple test results
    Given I have completed multiple acceptance tests
    And the results are displayed
    When I click the "Export Results" button
    And I select "HTML" format
    Then the test results should be exported as an HTML report
    And the report should include the summary statistics
    And the report should list all individual scenario results
    And the report should be formatted for easy reading
    And the export should be downloadable

  @multiple-tests @retry-failed
  Scenario: Re-run only failed scenarios from multiple test execution
    Given I have executed multiple acceptance tests
    And some tests have failed
    When I view the results summary
    And I click the "Retry Failed Tests" button
    Then only the previously failed scenarios should be queued for execution
    And the execution should start with just the failed tests
    And the progress should show the count of scenarios being retried
    And upon completion, the results should be updated with new outcomes
    And the retry attempt should be recorded in the test history