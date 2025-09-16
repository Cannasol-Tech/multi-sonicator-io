@web-ui @acceptance-testing @loading
Feature: Web-UI Acceptance Test Loading
  As a test engineer
  I want to verify that acceptance tests are loaded into the web-ui
  So that I can run acceptance tests through the web interface with hardware mock

  Background:
    Given the web-ui is running in simulation mode
    And the backend test automation service is available
    And the hardware mock is enabled

  @smoke @ui-loading
  Scenario: Load acceptance test scenarios into web-ui interface
    Given I navigate to the Test Automation panel
    When I request the list of available test scenarios
    Then I should see acceptance test scenarios loaded from the backend
    And each scenario should display its name and description
    And each scenario should show its associated tags
    And the total scenario count should be greater than 0

  @ui-loading @scenario-details
  Scenario: Verify acceptance test scenario details are displayed
    Given I navigate to the Test Automation panel
    And acceptance test scenarios are loaded
    When I select a specific acceptance test scenario
    Then I should see detailed scenario information
    And the scenario should show its feature file name
    And the scenario should display all associated tags
    And the scenario should show the number of steps
    And the scenario description should be visible

  @ui-loading @hardware-mock
  Scenario: Verify acceptance tests load with hardware mock enabled
    Given the hardware mock is enabled in the backend
    And I navigate to the Test Automation panel
    When I load acceptance test scenarios
    Then all acceptance test scenarios should be available
    And no hardware connectivity errors should be displayed
    And the interface should indicate simulation mode is active
    And all test scenarios should be executable in mock mode

  @ui-loading @refresh-scenarios
  Scenario: Refresh acceptance test scenarios
    Given I am on the Test Automation panel
    And acceptance test scenarios are loaded
    When I click the refresh scenarios button
    Then the scenarios should be reloaded from the backend
    And the scenario count should remain consistent
    And any new scenarios should appear in the list
    And the loading indicator should be displayed during refresh

  @ui-loading @error-handling
  Scenario: Handle backend connection errors gracefully
    Given the backend test automation service is unavailable
    When I navigate to the Test Automation panel
    And I attempt to load acceptance test scenarios
    Then I should see a user-friendly error message
    And the interface should indicate the backend is not available
    And there should be a retry button available
    And the error should not crash the web interface