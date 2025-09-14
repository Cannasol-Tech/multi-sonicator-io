@web-ui @test-automation @critical
Feature: Web UI Test Automation Interface
  As a test engineer
  I want to execute acceptance tests through the web interface
  So that I can validate system functionality and generate reports

  Background:
    Given the web UI is loaded and accessible
    And the Test Automation panel is visible
    And the backend test automation service is running

  @smoke @ui-basic
  Scenario: TAU-001 Load test scenarios from backend
    When I navigate to the Test Automation tab
    Then I should see available test scenarios loaded
    And each scenario should display its name and description
    And each scenario should show associated tags
    And the scenario count should be greater than 0

  @ui-controls @scenario-selection
  Scenario: TAU-002 Select individual test scenarios
    Given test scenarios are loaded in the interface
    When I click on a scenario checkbox
    Then the scenario should be marked as selected
    And the selection count should update
    When I click the checkbox again
    Then the scenario should be deselected

  @ui-controls @bulk-selection
  Scenario: TAU-003 Select all scenarios at once
    Given test scenarios are loaded in the interface
    When I click the "Select All" button
    Then all scenarios should be marked as selected
    And the selection count should equal total scenarios
    When I click "Clear Selection"
    Then no scenarios should be selected

  @test-execution @individual
  Scenario: TAU-004 Execute individual test scenario
    Given test scenarios are loaded
    And I have selected a single test scenario
    When I click the "Run Selected" button
    Then the test execution should start
    And I should see a progress indicator
    And the execution status should show "running"
    And I should see real-time progress updates

  @test-execution @full-suite
  Scenario: TAU-005 Execute full acceptance test suite
    Given test scenarios are loaded
    When I click the "Run Full Suite" button
    Then all available scenarios should be executed
    And I should see an iridescent progress bar
    And the progress should update in real-time
    And I should see passed/failed scenario counts

  @test-execution @stop-execution
  Scenario: TAU-006 Stop running test execution
    Given a test execution is in progress
    When I click the "Stop Execution" button
    Then the test execution should be terminated
    And the status should change to "stopped"
    And I should see a summary of completed tests

  @test-results @modal
  Scenario: TAU-007 View detailed test results
    Given a test execution has completed
    When I click "View Results"
    Then a test results modal should open
    And I should see execution summary statistics
    And I should see individual scenario results
    And I should see failed test details if any
    And I should see execution duration

  @test-results @export
  Scenario: TAU-008 Export test execution results
    Given test results are displayed
    When I click the "Export Results" button
    Then I should be able to download results as JSON
    And the export should include all scenario details
    And the export should include execution metadata
    And the export should include timing information
