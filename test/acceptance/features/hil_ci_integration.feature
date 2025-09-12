@hil @ci_integration
Feature: HIL CI/CD Integration
  As a DevOps engineer
  I want HIL testing integrated with the CI/CD pipeline
  So that automated HIL tests run as part of the build and deployment process

  Background:
    Given the CI/CD pipeline is configured
    And the HIL integration scripts are available
    And the build environment has access to HIL hardware

  @hil @ac10 @ci_pipeline_integration
  Scenario: AC10 - Integration with CI/CD pipeline for automated HIL testing
    Given a code change is pushed to the repository
    When the CI/CD pipeline is triggered
    Then the pipeline should detect HIL hardware availability
    And include HIL tests in the build process
    And execute HIL tests automatically
    And collect HIL test results
    And integrate results with the overall build status
    When HIL tests pass
    Then the build should continue to the next stage
    When HIL tests fail
    Then the build should be marked as failed
    And detailed HIL failure information should be provided

  @hil @ac10 @hardware_availability_check
  Scenario: Hardware availability checking in CI/CD
    Given the CI/CD pipeline is starting
    When the HIL integration checks for hardware availability
    Then it should detect connected Arduino hardware
    And verify HIL wrapper functionality
    And confirm DUT connectivity
    And validate safety systems
    When HIL hardware is not available
    Then the pipeline should skip HIL tests gracefully
    And mark HIL tests as "skipped" not "failed"
    And continue with other tests that don't require hardware
    And log the hardware unavailability for investigation

  @hil @ac10 @automated_test_execution
  Scenario: Automated HIL test execution in CI/CD
    Given HIL hardware is available in the CI/CD environment
    When the automated HIL tests are executed
    Then the test framework should initialize hardware automatically
    And run the complete HIL test suite
    And collect detailed test results and metrics
    And generate comprehensive test reports
    And clean up hardware state after testing
    When HIL tests encounter hardware issues
    Then the tests should attempt automatic recovery
    And escalate persistent issues appropriately
    And preserve diagnostic information for analysis

  @hil @ac10 @test_result_integration
  Scenario: HIL test result integration with CI/CD reporting
    Given HIL tests have completed in the CI/CD pipeline
    When test results are processed
    Then results should be integrated with standard CI/CD reporting
    And HIL-specific metrics should be included
    And test artifacts should be preserved
    And results should be available in multiple formats (JUnit, HTML, JSON)
    When HIL test results indicate failures
    Then specific failure details should be included in build reports
    And hardware diagnostic information should be attached
    And recommendations for resolution should be provided

  @hil @ac10 @parallel_pipeline_execution
  Scenario: Parallel HIL testing in CI/CD pipelines
    Given multiple CI/CD pipelines are running simultaneously
    When HIL hardware resources are limited
    Then the CI/CD system should manage hardware access
    And queue HIL tests when hardware is busy
    And provide estimated wait times
    And optimize hardware utilization
    When multiple HIL test requests conflict
    Then the system should prioritize based on configured rules
    And ensure fair access to hardware resources
    And prevent resource deadlocks

  @hil @ac10 @build_gate_integration
  Scenario: HIL testing as a build gate
    Given HIL testing is configured as a build gate
    When a build reaches the HIL testing stage
    Then all HIL tests must pass for the build to proceed
    And HIL test quality gates must be met
    And HIL performance benchmarks must be satisfied
    When HIL tests fail the build gate
    Then the build should be blocked from proceeding
    And detailed failure analysis should be provided
    And the development team should be notified
    And the failure should be tracked for metrics

  @hil @ac10 @deployment_validation
  Scenario: HIL testing for deployment validation
    Given a deployment candidate is ready
    When HIL validation tests are executed
    Then the deployment should be tested against actual hardware
    And all critical functionality should be validated
    And performance should meet deployment criteria
    And safety systems should be verified
    When HIL validation passes
    Then the deployment should be approved for release
    When HIL validation fails
    Then the deployment should be blocked
    And rollback procedures should be initiated if necessary

  @hil @ac10 @continuous_monitoring
  Scenario: Continuous HIL monitoring in production pipeline
    Given the production pipeline includes HIL monitoring
    When production deployments are active
    Then HIL tests should run continuously
    And monitor system health and performance
    And detect degradation or failures early
    And alert operations teams of issues
    When continuous HIL monitoring detects problems
    Then automatic incident response should be triggered
    And relevant teams should be notified immediately
    And diagnostic data should be collected automatically

  @hil @ac10 @metrics_and_reporting
  Scenario: HIL testing metrics and CI/CD reporting
    Given HIL testing is integrated with CI/CD
    When HIL tests execute over time
    Then test execution metrics should be collected
    And hardware utilization should be tracked
    And test reliability should be monitored
    And performance trends should be analyzed
    When generating CI/CD reports
    Then HIL testing metrics should be included
    And hardware health status should be reported
    And test coverage and quality metrics should be provided
    And recommendations for improvement should be generated
