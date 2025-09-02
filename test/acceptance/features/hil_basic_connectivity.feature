Feature: Hardware Basic Connectivity
  As a firmware developer
  I want to verify basic hardware connectivity  
  So that I can ensure the HIL framework can communicate with the ATmega32A

  Background:
    Given the Arduino test harness is connected
    And the ATmega32A target is powered
    And the HIL framework is initialized

  @hil @hardware @connectivity
  Scenario: Power supply verification
    When I measure the power supply voltages
    Then the 24V input should be within tolerance
    And the 5V rail should be 5.0V ± 0.1V
    And the 3.3V rail should be 3.3V ± 0.1V

  @hil @hardware @connectivity
  Scenario: Programming interface verification  
    When I attempt to program the ATmega32A via Arduino as ISP
    Then the programming should succeed
    And the firmware version should be readable
    And the target should respond to commands

  @hil @hardware @connectivity
  Scenario: Serial communication establishment
    When I establish serial communication with the target
    Then I should receive a valid response
    And the communication should be stable for 10 seconds
    
  @hil @hardware @connectivity
  Scenario: Arduino Test Harness ping test
    When I send a ping command to the Arduino Test Harness
    Then I should receive a PONG response
    And the response time should be less than 100ms
