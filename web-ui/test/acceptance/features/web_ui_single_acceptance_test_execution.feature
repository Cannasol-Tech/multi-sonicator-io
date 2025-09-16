@web-ui @acceptance-testing @single-execution
Feature: Single Acceptance Test Web-UI Execution and Results Verification
  As a test engineer
  I want to run a single acceptance test from the web-ui and verify the results
  So that I can validate individual test scenarios and view detailed results

  Background:
    Given the web-ui is running in simulation mode
    And the Test Automation panel is loaded
    And acceptance test scenarios are loaded from the backend
    And the hardware mock is enabled

  @smoke @single-test @execution
  Scenario: Execute single acceptance test and wait for completion
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I select a single test scenario "Hardware Basic Connectivity"
    And I click the "Run Selected" button
    Then the test execution should start
    And I should see a progress indicator showing test execution
    And the execution status should show "running"
    And I should wait for the test to finish
    And the final status should be either "passed" or "failed"

  @single-test @results-display
  Scenario: Verify single test results are displayed correctly
    Given I have executed a single acceptance test
    And the test has completed
    When I view the test results
    Then I should see the detailed test results displayed
    And the results should show the scenario name and description
    And the results should display the execution time
    And the results should show the final status (passed/failed)
    And if failed, error details should be visible
    And all test steps should be listed with their individual status

  @single-test @real-time-updates
  Scenario: Verify real-time progress updates during single test execution
    Given I am on the Test Automation panel
    And I have selected a single test scenario
    When I start the test execution
    Then I should see real-time progress updates
    And the current step being executed should be highlighted
    And step completion should be indicated as it happens
    And the overall progress percentage should update
    And any step failures should be immediately visible

  @single-test @results-export
  Scenario: Export single test results
    Given I have completed a single acceptance test
    And the results are displayed
    When I click the "Export Results" button
    And I select "JSON" format
    Then the test results should be exported as a JSON file
    And the exported file should contain all test details
    And the file should include scenario information, steps, and outcomes
    And the export should be downloadable

  @single-test @error-handling
  Scenario: Handle single test execution errors gracefully
    Given I am on the Test Automation panel
    And I select a test scenario that will fail
    When I run the selected test
    And the test fails during execution
    Then I should see a clear error message
    And the error details should be displayed
    And the failed step should be highlighted
    And there should be an option to retry the test
    And the interface should remain stable after the error

  @single-test @test-interruption
  Scenario: Stop single test execution mid-run
    Given I am running a single acceptance test
    And the test is currently executing
    When I click the "Stop Execution" button
    Then the test execution should stop immediately
    And the status should show "stopped" or "cancelled"
    And any partial results should be preserved
    And I should be able to start a new test execution
    And the interface should return to the ready state

  @single-test @detailed-step-view
  Scenario: View detailed information for individual test steps
    Given I have completed a single acceptance test
    And the results are displayed
    When I click on a specific test step in the results
    Then I should see detailed step information
    And the step should show its type (Given/When/Then)
    And the step description should be fully visible  
    And any pin interactions should be listed
    And step execution time should be displayed
    And for failed steps, detailed error information should be shown