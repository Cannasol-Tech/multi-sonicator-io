@hil @signal_testing
Feature: HIL Signal Testing
  As a test engineer
  I want comprehensive signal generation and monitoring capabilities
  So that I can validate sonicator control and monitoring functions

  Background:
    Given the HIL hardware is operational
    And the signal generation system is initialized
    And Sonicator 4 is available for testing

  @hil @ac5 @digital_signals
  Scenario: AC5 - Digital signal generation and monitoring
    Given the digital signal interface is ready
    When I generate a HIGH signal on "FREQ_LOCK_4"
    Then the digital output should be set to HIGH
    And I should be able to read back the HIGH state
    When I generate a LOW signal on "FREQ_LOCK_4"
    Then the digital output should be set to LOW
    And I should be able to read back the LOW state
    When I generate a HIGH signal on "OVERLOAD_4"
    Then the overload condition should be simulated
    And the DUT should detect the overload condition
    When I clear the "OVERLOAD_4" signal
    Then the overload condition should be removed
    And the DUT should return to normal operation

  @hil @ac5 @analog_signals
  Scenario: AC5 - Analog signal generation and ADC monitoring
    Given the analog signal interface is ready
    When I apply 2.5V to the "POWER_SENSE_4" channel
    Then the ADC should read approximately 2.5V ±0.1V
    When I apply 1.0V to the "POWER_SENSE_4" channel
    Then the ADC should read approximately 1.0V ±0.1V
    When I apply 4.0V to the "POWER_SENSE_4" channel
    Then the ADC should read approximately 4.0V ±0.1V
    When I simulate 50W power consumption
    Then the power sensing channel should show the equivalent voltage
    And the calculated power should be approximately 50W ±5W

  @hil @ac5 @pwm_signals
  Scenario: AC5 - PWM signal generation and monitoring
    Given the PWM signal interface is ready
    When I generate a 50% duty cycle PWM on "AMPLITUDE_ALL"
    Then the PWM output should have 50% ±2% duty cycle
    When I generate a 25% duty cycle PWM on "AMPLITUDE_ALL"
    Then the PWM output should have 25% ±2% duty cycle
    When I generate a 75% duty cycle PWM on "AMPLITUDE_ALL"
    Then the PWM output should have 75% ±2% duty cycle
    When I monitor the PWM signal for 5 seconds
    Then the duty cycle should remain stable within ±1%
    And the frequency should be within specification

  @hil @ac5 @frequency_signals
  Scenario: AC5 - Frequency signal generation
    Given the frequency signal interface is ready
    When I generate a 1000Hz signal on "FREQ_DIV10_4"
    Then the frequency output should be 1000Hz ±10Hz
    When I generate a 500Hz signal on "FREQ_DIV10_4"
    Then the frequency output should be 500Hz ±5Hz
    When I generate a 2000Hz signal on "FREQ_DIV10_4"
    Then the frequency output should be 2000Hz ±20Hz
    When I stop the frequency generation
    Then the frequency output should return to 0Hz

  @hil @ac5 @uart_communication
  Scenario: AC5 - UART communication testing
    Given the UART communication interface is ready
    When I send a MODBUS read request to register 0x0001
    Then I should receive a valid MODBUS response
    And the response time should be less than 50ms
    When I send a MODBUS write request to register 0x0002 with value 100
    Then I should receive a MODBUS acknowledgment
    And the write operation should complete successfully
    When I read back register 0x0002
    Then the value should be 100
    And the communication should be error-free

  @hil @ac5 @signal_stability
  Scenario: Signal generation stability and accuracy
    Given all signal interfaces are operational
    When I generate multiple signals simultaneously
      | Signal        | Type    | Value  |
      | FREQ_LOCK_4   | Digital | HIGH   |
      | OVERLOAD_4    | Digital | LOW    |
      | POWER_SENSE_4 | Analog  | 3.3V   |
      | AMPLITUDE_ALL | PWM     | 60%    |
    Then all signals should be generated correctly
    And there should be no signal interference
    When I maintain these signals for 60 seconds
    Then all signals should remain stable
    And the accuracy should be within specification
    When I change all signals simultaneously
    Then the transitions should be clean and synchronized

  @hil @ac5 @signal_edge_cases
  Scenario: Signal generation edge cases and limits
    Given the signal generation system is ready
    When I attempt to generate signals at maximum limits
    Then the system should handle maximum voltage (5.0V)
    And the system should handle minimum voltage (0.0V)
    And the system should handle maximum PWM duty cycle (100%)
    And the system should handle minimum PWM duty cycle (0%)
    When I attempt to generate invalid signal values
    Then the system should reject out-of-range values gracefully
    And provide meaningful error messages
    And maintain system stability
