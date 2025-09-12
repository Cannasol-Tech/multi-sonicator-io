@web-ui @progress-monitoring @real-time @critical
Feature: Real-time Test Progress Monitoring
  As a test engineer
  I want to monitor test execution progress in real-time
  So that I can track execution status and identify issues quickly

  Background:
    Given the web UI Test Automation panel is loaded
    And test scenarios are available for execution
    And the progress monitoring interface is ready

  @smoke @basic-progress
  Scenario: TPM-001 Display basic progress information
    Given a test execution is started with 10 scenarios
    When the execution begins
    Then I should see a progress bar
    And I should see "0 of 10 scenarios completed"
    And I should see the execution ID
    And I should see the start time

  @progress-bar @iridescent
  Scenario: TPM-002 Show iridescent progress bar animation
    Given a test execution is in progress
    When scenarios are being executed
    Then I should see the iridescent progress bar
    And the progress should animate smoothly
    And the shimmer effect should be visible
    And the progress percentage should update in real-time

  @step-tracking @detailed-progress
  Scenario: TPM-003 Track individual test step progress
    Given a test scenario with 5 steps is executing
    When the scenario runs
    Then I should see "Step 1 of 5" progress indicator
    And each completed step should be marked
    When the scenario progresses to step 3
    Then I should see "Step 3 of 5"
    And steps 1-2 should show as completed

  @scenario-status @live-updates
  Scenario: TPM-004 Show live scenario execution status
    Given multiple scenarios are queued for execution
    When execution starts
    Then I should see scenario statuses:
      | Scenario         | Status   |
      | HIL-001         | Running  |
      | HIL-002         | Pending  |
      | HIL-003         | Pending  |
    When HIL-001 completes successfully
    Then I should see:
      | Scenario         | Status   |
      | HIL-001         | Passed   |
      | HIL-002         | Running  |
      | HIL-003         | Pending  |

  @statistics @counters
  Scenario: TPM-005 Display execution statistics
    Given a test execution with mixed results is in progress
    When some scenarios pass and some fail
    Then I should see accurate counters:
      | Metric           | Value |
      | Total Scenarios  | 10    |
      | Passed          | 6     |
      | Failed          | 2     |
      | Running         | 1     |
      | Pending         | 1     |

  @timing @duration-tracking
  Scenario: TPM-006 Track execution timing and duration
    Given a test execution starts at 10:00:00
    When the execution progresses
    Then I should see "Duration: 00:02:30" after 2.5 minutes
    And I should see estimated time remaining
    And I should see average time per scenario
    When a scenario takes longer than expected
    Then I should see a "slow execution" indicator

  @error-handling @failure-details
  Scenario: TPM-007 Display detailed failure information
    Given a test scenario fails during execution
    When I view the progress monitoring
    Then I should see the failed scenario highlighted in red
    And I should see the error message
    And I should see the step where it failed
    When I click on the failed scenario
    Then I should see detailed failure information

  @performance @resource-monitoring
  Scenario: TPM-008 Monitor system resource usage
    Given a large test suite is executing
    When I view the system monitoring section
    Then I should see CPU usage percentage
    And I should see memory consumption
    And I should see network activity indicators
    When resources exceed thresholds
    Then I should see warning indicators

  @websocket @connectivity
  Scenario: TPM-009 Handle WebSocket connection issues
    Given a test execution is in progress
    When the WebSocket connection is lost
    Then I should see a "Connection Lost" indicator
    And progress updates should pause
    When the connection is restored
    Then I should see "Connection Restored"
    And progress updates should resume

  @notifications @alerts
  Scenario: TPM-010 Show execution notifications and alerts
    Given test execution monitoring is active
    When a critical test fails
    Then I should see a high-priority alert notification
    When execution completes successfully
    Then I should see a success notification
    When execution is stopped by user
    Then I should see a "Execution Stopped" notification

  @history @execution-log
  Scenario: TPM-011 Maintain execution history log
    Given multiple test executions have occurred
    When I view the execution history
    Then I should see a chronological list of executions
    And each entry should show:
      | Field           | Example                    |
      | Execution ID    | TEST-EXEC-20250312-001     |
      | Start Time      | 2025-03-12 10:30:00        |
      | Duration        | 00:15:30                   |
      | Status          | Completed                  |
      | Pass/Fail       | 8/2                        |
      | Tags Used       | @smoke @hil                |

  @export @progress-data
  Scenario: TPM-012 Export progress monitoring data
    Given test execution progress data is available
    When I click "Export Progress Data"
    Then I should be able to download progress metrics as CSV
    And the export should include timing data
    And the export should include resource usage data
    And the export should include step-by-step progress log
