@hil @automated_testing
Feature: HIL Automated Testing and Reporting
  As a test automation engineer
  I want automated test execution with hardware validation and comprehensive reporting
  So that I can run unattended tests and collect detailed results

  Background:
    Given the HIL hardware is operational
    And the automated test framework is initialized
    And test result collection is enabled

  @hil @ac6 @automated_execution
  Scenario: AC6 - Automated test execution with hardware validation
    Given a test suite "basic_sonicator_validation" is defined
    When I execute the automated test suite
    Then the test framework should initialize hardware automatically
    And validate hardware connectivity before starting tests
    And execute all test cases in the defined sequence
    And collect results from each test case
    And generate a comprehensive test report
    When hardware validation fails during execution
    Then the test suite should abort safely
    And report the hardware validation failure
    And preserve any partial results collected

  @hil @ac6 @hardware_validation
  Scenario: Hardware validation during automated testing
    Given the automated test execution is starting
    When the framework performs pre-test hardware validation
    Then it should verify Arduino wrapper connectivity
    And validate DUT power supply stability
    And confirm all required signal channels are functional
    And check safety system responsiveness
    When any hardware validation check fails
    Then the specific failure should be identified and reported
    And the test execution should not proceed
    And the hardware should be returned to a safe state

  @hil @ac7 @result_collection
  Scenario: AC7 - Test result collection and reporting from physical hardware
    Given automated tests are running on physical hardware
    When each test case executes
    Then real-time hardware measurements should be collected
    And test execution timestamps should be recorded
    And pass/fail status should be determined from actual hardware responses
    And any hardware anomalies should be captured
    When a test case fails
    Then the failure should include hardware state information
    And diagnostic data should be collected automatically
    And the failure context should be preserved for analysis

  @hil @ac7 @comprehensive_reporting
  Scenario: Comprehensive test result reporting
    Given automated tests have completed execution
    When I generate the test report
    Then the report should include overall test summary
    And individual test case results with hardware data
    And execution timeline with timestamps
    And hardware performance metrics
    And any safety incidents or warnings
    And recommendations for failed tests
    When I export the test results
    Then results should be available in JSON format
    And results should be available in HTML format
    And results should be available in JUnit XML format
    And all formats should contain complete hardware data

  @hil @ac6 @ac7 @test_scheduling
  Scenario: Automated test scheduling and execution
    Given the test scheduling system is configured
    When I schedule a test suite to run every hour
    Then the scheduler should execute tests automatically
    And collect results without human intervention
    And store results with unique identifiers
    And alert on any failures or anomalies
    When scheduled tests encounter hardware issues
    Then the scheduler should attempt automatic recovery
    And escalate persistent issues appropriately
    And maintain a log of all scheduling events

  @hil @ac6 @parallel_testing
  Scenario: Parallel test execution capabilities
    Given multiple test channels are available
    When I execute tests that can run in parallel
    Then the framework should utilize available channels efficiently
    And prevent resource conflicts between parallel tests
    And collect results from all parallel executions
    And synchronize results into a unified report
    When parallel tests have dependencies
    Then the framework should respect execution order constraints
    And ensure dependent tests wait for prerequisites

  @hil @ac7 @real_time_monitoring
  Scenario: Real-time test monitoring and data collection
    Given automated tests are executing
    When I monitor test execution in real-time
    Then I should see current test progress
    And live hardware measurements
    And immediate notification of any failures
    And estimated completion time
    When hardware parameters exceed normal ranges
    Then real-time alerts should be generated
    And the test execution should adapt appropriately
    And all anomalies should be logged for analysis

  @hil @ac6 @ac7 @regression_testing
  Scenario: Automated regression testing with historical comparison
    Given a baseline set of test results exists
    When I run automated regression tests
    Then current results should be compared to baseline
    And any performance degradation should be identified
    And new failures should be highlighted
    And improvements should be noted
    When regression tests reveal issues
    Then detailed comparison data should be provided
    And recommendations for investigation should be included
    And the regression report should be automatically generated
