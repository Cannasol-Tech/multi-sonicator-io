Feature: Complete System Integration Testing
  As a system integrator
  I want to verify the complete Multi-Sonicator I/O Controller system functionality
  So that I can ensure all requirements are met for production deployment

  Background:
    Given the Multi-Sonicator I/O Controller is powered and initialized
    And all 4 sonicator units are connected and ready
    And the MODBUS RTU communication is established at 115200 baud
    And the system is in a safe default state

  @system @integration @critical
  Scenario: Complete system startup and initialization
    Given the system has been powered off
    When I power on the Multi-Sonicator I/O Controller
    Then within 2 seconds the system status register should indicate "READY"
    And all sonicator units should be in "STOPPED" state
    And the active sonicator count should be 0
    And the active sonicator mask should be 0x0000
    And the watchdog status should indicate "HEALTHY"
    And all amplitude setpoints should default to 50%

  @system @integration @critical
  Scenario: Four-sonicator simultaneous operation
    Given all sonicator units are connected and ready
    When I start all 4 sonicators simultaneously
    And I set sonicator 1 amplitude to 25%
    And I set sonicator 2 amplitude to 50%
    And I set sonicator 3 amplitude to 75%
    And I set sonicator 4 amplitude to 100%
    Then within 200ms all sonicators should be running
    And the active sonicator count should be 4
    And the active sonicator mask should be 0x000F
    And each sonicator should report its correct amplitude
    And each sonicator should report power consumption > 0 watts
    And each sonicator should report frequency within operational range

  @system @integration @critical
  Scenario: Sequential sonicator control with monitoring
    Given the system is initialized and ready
    When I start sonicator 1 with 30% amplitude
    Then within 100ms sonicator 1 should be running at 30% amplitude
    And the active count should be 1 and mask should be 0x0001
    When I start sonicator 2 with 60% amplitude
    Then within 100ms sonicator 2 should be running at 60% amplitude
    And the active count should be 2 and mask should be 0x0003
    When I start sonicator 3 with 90% amplitude
    Then within 100ms sonicator 3 should be running at 90% amplitude
    And the active count should be 3 and mask should be 0x0007
    When I stop sonicator 2
    Then within 100ms sonicator 2 should be stopped
    And the active count should be 2 and mask should be 0x0005

  @system @integration @safety
  Scenario: Emergency stop functionality
    Given all 4 sonicators are running at various amplitudes
    When I trigger the emergency stop command
    Then within 50ms all sonicators should be stopped
    And the emergency stop flag should be set in system status
    And the active sonicator count should be 0
    And the active sonicator mask should be 0x0000
    And no sonicator should accept start commands until emergency stop is cleared
    When I clear the emergency stop condition
    Then the system should return to normal operation mode
    And sonicators should accept start commands again

  @system @integration @fault-handling
  Scenario: Overload detection and recovery
    Given sonicator 1 is running at 75% amplitude
    When sonicator 1 experiences an overload condition
    Then within 100ms the overload flag should be set for sonicator 1
    And sonicator 1 should automatically stop
    And the active count should decrease by 1
    And the system should log the overload event
    When I send an overload reset command for sonicator 1
    Then the overload flag should be cleared
    And sonicator 1 should be ready to start again
    When I restart sonicator 1
    Then sonicator 1 should operate normally

  @system @integration @communication
  Scenario: MODBUS communication reliability under load
    Given all 4 sonicators are running
    When I perform 100 consecutive MODBUS read operations
    Then all reads should complete successfully within 100ms each
    And the CRC should be correct for all responses
    And no communication errors should be logged
    When I perform 50 consecutive MODBUS write operations
    Then all writes should complete successfully within 100ms each
    And all register values should be updated correctly
    And the system should maintain stable operation throughout

  @system @integration @performance
  Scenario: System performance under maximum load
    Given all 4 sonicators are connected
    When I start all sonicators at maximum amplitude (100%)
    Then each sonicator should reach full power within 500ms
    And the total system power consumption should be within specifications
    And all telemetry data should update at minimum 10Hz
    And the system temperature should remain within safe limits
    And no performance degradation should occur over 10 minutes of operation

  @system @integration @timing
  Scenario: Response time requirements validation
    Given the system is in normal operation
    When I send a start command to any sonicator
    Then the command should be acknowledged within 50ms
    And the sonicator should begin operation within 200ms
    And the status should be reflected in MODBUS registers within 100ms
    When I send a stop command to any running sonicator
    Then the sonicator should stop within 200ms
    And the status change should be reflected within 100ms

  @system @integration @data-integrity
  Scenario: Telemetry data accuracy and consistency
    Given sonicator 1 is running at a stable 60% amplitude
    When I monitor telemetry data for 30 seconds
    Then the power readings should be consistent within ±5%
    And the frequency readings should be stable within ±1%
    And the amplitude readings should match the setpoint within ±2%
    And no data corruption or invalid values should occur
    And the timestamp accuracy should be maintained

  @system @integration @configuration
  Scenario: System configuration persistence
    Given the system is configured with specific settings
    When I set custom amplitude values for all sonicators
    And I configure system parameters
    And I power cycle the system
    Then the system should retain all configuration settings
    And sonicators should return to their configured default states
    And no configuration drift should occur

  @pending @system @integration @advanced
  Scenario: Advanced diagnostics and health monitoring
    Given the system has been running for extended periods
    When I request system diagnostics
    Then the system should report component health status
    And provide runtime statistics for each sonicator
    And report any maintenance recommendations
    And log historical performance data
    And identify any potential issues before they become critical

  @pending @system @integration @networking
  Scenario: Multi-controller network operation
    Given multiple Multi-Sonicator I/O Controllers are networked
    When I configure unique MODBUS slave IDs for each controller
    Then each controller should respond only to its assigned ID
    And network communication should remain stable
    And no address conflicts should occur
    And centralized monitoring should work correctly

  @pending @system @integration @calibration
  Scenario: Automated system calibration
    Given the system requires calibration
    When I initiate the automated calibration sequence
    Then the system should calibrate all measurement channels
    And verify amplitude control accuracy
    And adjust power measurement scaling
    And validate frequency measurement precision
    And store calibration data persistently
