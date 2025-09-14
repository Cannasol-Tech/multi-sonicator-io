@hil @safety_systems
Feature: HIL Safety Systems
  As a test engineer
  I want comprehensive safety interlocks and emergency stop procedures
  So that I can ensure safe operation during all HIL testing activities

  Background:
    Given the HIL hardware is operational
    And the safety systems are initialized
    And all safety interlocks are armed

  @hil @ac8 @emergency_stop
  Scenario: AC8 - Emergency stop procedures and response time
    Given the HIL system is running normally
    When I trigger an emergency stop
    Then all hardware outputs should be disabled within 100ms
    And the system should enter emergency stop state
    And all ongoing operations should be terminated immediately
    And the emergency stop status should be clearly indicated
    When I attempt to perform any operations during emergency stop
    Then all operations should be blocked
    And appropriate error messages should be displayed
    When I reset the emergency stop
    Then the system should return to safe operational state
    And all hardware should be re-initialized properly

  @hil @ac8 @safety_interlocks
  Scenario: Safety interlocks and fault detection
    Given the safety interlock system is active
    When a hardware fault is detected
    Then the safety system should activate automatically
    And isolate the faulty hardware component
    And prevent further operations that could cause damage
    And log the safety incident with full details
    When communication with hardware is lost
    Then the safety system should assume worst-case scenario
    And activate protective measures within 500ms
    And attempt automatic recovery procedures
    And escalate if recovery fails

  @hil @ac8 @overload_protection
  Scenario: Overload protection and monitoring
    Given the overload protection system is enabled
    When I simulate an overload condition on Sonicator 4
    Then the overload should be detected within 100ms
    And protective measures should be activated
    And the affected channel should be isolated
    And other channels should continue operating safely
    When the overload condition is cleared
    Then the system should verify safe conditions
    And allow the channel to be re-enabled
    And log the overload event for analysis

  @hil @ac8 @power_safety
  Scenario: Power system safety monitoring
    Given the power monitoring system is active
    When the input voltage exceeds safe limits
    Then the power safety system should activate
    And disconnect power to sensitive components
    And preserve system state information
    And alert operators of the power fault
    When power returns to safe levels
    Then the system should perform safety checks
    And allow controlled restart procedures
    And verify all components are functional

  @hil @ac8 @communication_safety
  Scenario: Communication safety and timeout handling
    Given the communication safety system is monitoring
    When communication with the Arduino wrapper times out
    Then the safety system should detect the timeout within 5 seconds
    And assume hardware is in an unknown state
    And activate safe shutdown procedures
    And attempt to re-establish communication
    When communication is restored
    Then the system should verify hardware state
    And perform safety validation before resuming
    And log the communication interruption

  @hil @ac8 @thermal_protection
  Scenario: Thermal protection and monitoring
    Given thermal monitoring is enabled
    When I simulate elevated temperature conditions
    Then the thermal protection should activate
    And reduce power to heat-generating components
    And increase cooling measures if available
    And warn operators of thermal conditions
    When temperature returns to safe levels
    Then normal operation should be restored gradually
    And thermal event should be logged for analysis

  @hil @ac8 @safety_system_testing
  Scenario: Safety system self-testing and validation
    Given the HIL system is operational
    When I initiate safety system self-test
    Then all safety interlocks should be tested
    And emergency stop response time should be measured
    And all safety sensors should be validated
    And safety communication paths should be verified
    And a comprehensive safety test report should be generated
    When any safety system test fails
    Then the system should not allow normal operation
    And the specific safety failure should be identified
    And corrective actions should be recommended

  @hil @ac8 @safety_logging
  Scenario: Safety incident logging and reporting
    Given the safety logging system is active
    When any safety event occurs
    Then the event should be logged with precise timestamp
    And complete system state should be captured
    And the event severity should be classified
    And appropriate notifications should be sent
    When I review safety logs
    Then I should see chronological safety events
    And detailed context for each incident
    And trends and patterns in safety events
    And recommendations for safety improvements

  @hil @ac8 @fail_safe_behavior
  Scenario: Fail-safe behavior under various fault conditions
    Given the HIL system is operating normally
    When I simulate various fault conditions
      | Fault Type           | Expected Response                    |
      | Power failure        | Safe shutdown within 1 second       |
      | Communication loss   | Emergency stop within 5 seconds     |
      | Hardware overload    | Channel isolation within 100ms      |
      | Software crash       | Hardware watchdog reset within 10s  |
      | Sensor failure       | Conservative safe mode activation    |
    Then each fault should trigger the expected safety response
    And the system should maintain safety throughout
    And recovery should only occur after validation
    And all fault events should be properly logged
