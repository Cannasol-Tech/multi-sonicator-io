@hil @hardware_abstraction
Feature: HIL Hardware Abstraction Layer
  As a test automation engineer
  I want a hardware abstraction layer for test automation
  So that I can write tests independent of specific hardware implementations

  Background:
    Given the HIL hardware is operational
    And the hardware abstraction layer is initialized
    And the test wrapper communication protocol is active

  @hil @ac4 @communication_protocol
  Scenario: AC4 - Test wrapper communication protocol implementation
    Given the Arduino Test Wrapper is connected
    When I send a "PING" command via the communication protocol
    Then I should receive "OK PONG" response within 100ms
    When I send an "INFO" command
    Then I should receive version and capability information
    When I send a "DEBUG" command
    Then I should receive current system status information
    When I send an invalid command "INVALID_CMD"
    Then I should receive an error response
    And the wrapper should remain stable and responsive

  @hil @ac4 @hardware_abstraction
  Scenario: Hardware abstraction for test automation
    Given the hardware abstraction layer is active
    When I request to control "OVERLOAD_4" signal
    Then the abstraction layer should map to the correct Arduino pin
    And the command should be sent via the communication protocol
    When I request to read "POWER_SENSE_4" value
    Then the abstraction layer should read from the correct ADC channel
    And return a calibrated voltage value
    When I request to generate a PWM signal on "AMPLITUDE_ALL"
    Then the abstraction layer should configure the correct PWM channel
    And set the requested duty cycle

  @hil @ac4 @signal_abstraction
  Scenario: Test signal generation abstraction
    Given the hardware abstraction layer is ready
    When I request to generate a digital HIGH signal on "FREQ_LOCK_4"
    Then the abstraction layer should execute "SET LOCK 4 1"
    And verify the signal was set correctly
    When I request to generate a digital LOW signal on "FREQ_LOCK_4"
    Then the abstraction layer should execute "SET LOCK 4 0"
    And verify the signal was cleared correctly
    When I request to simulate 100W power consumption
    Then the abstraction layer should calculate the correct voltage
    And apply it to the power sensing channel
    And verify the simulation is active

  @hil @ac4 @monitoring_abstraction
  Scenario: Hardware monitoring and data collection abstraction
    Given the hardware abstraction layer is operational
    When I request to monitor "START_4" signal status
    Then the abstraction layer should read the digital input
    And return the current signal state
    When I request to monitor "POWER_SENSE_4" continuously
    Then the abstraction layer should start continuous ADC readings
    And provide real-time power measurement data
    When I request to stop monitoring "POWER_SENSE_4"
    Then the continuous monitoring should stop
    And resources should be released properly

  @hil @ac4 @control_interface
  Scenario: Test execution control interface
    Given the hardware abstraction layer is initialized
    When I create a test execution plan with multiple signal operations
    Then the abstraction layer should queue the operations
    When I execute the test plan
    Then each operation should be executed in sequence
    And the results should be collected automatically
    And any failures should be reported with context
    When I request to abort the test execution
    Then all pending operations should be cancelled safely
    And the hardware should return to a safe state

  @hil @ac4 @error_handling
  Scenario: Hardware abstraction error handling
    Given the hardware abstraction layer is active
    When the underlying hardware communication fails
    Then the abstraction layer should detect the failure
    And attempt automatic recovery
    And report the error to the test framework
    When hardware is disconnected during operation
    Then the abstraction layer should handle the disconnection gracefully
    And provide meaningful error messages
    And maintain system stability

  @hil @ac4 @configuration_management
  Scenario: Hardware configuration management through abstraction
    Given the hardware abstraction layer is loaded
    When I request the current hardware configuration
    Then the abstraction layer should return the pin matrix mapping
    And the available test capabilities
    And the current hardware status
    When I validate the configuration against "config/hardware-config.yaml"
    Then all pin mappings should match the configuration file
    And all declared capabilities should be available
    And any configuration mismatches should be reported
