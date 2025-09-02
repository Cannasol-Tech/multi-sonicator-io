Feature: Power Supply and Electrical Verification
  As a hardware engineer
  I want to verify power supply voltages and electrical characteristics
  So that I can ensure the system operates within specifications

  Background:
    Given the HIL framework is connected
    And the power supply is enabled
    And the ATmega32A target is powered

  @hil @hardware @power
  Scenario: Input power supply verification
    When I measure the input voltage
    Then the 24V input should be 24.0V ± 0.5V
    And the input current should be within expected range

  @hil @hardware @power
  Scenario: Regulated power rail verification
    When I measure the regulated power rails
    Then the 5V rail should be 5.0V ± 0.1V
    And the 3.3V rail should be 3.3V ± 0.1V if present
    And the ripple voltage should be less than 50mV peak-to-peak

  @hil @hardware @power
  Scenario: Power supply load regulation
    When I apply varying loads to the 5V rail
    Then the voltage should remain 5.0V ± 0.1V
    And the regulation should be maintained under all load conditions

  @hil @hardware @power
  Scenario: Crystal oscillator verification
    When I measure the crystal oscillator frequency
    Then the frequency should be 16.000MHz ± 0.01%
    And the crystal should be stable and running

  @hil @hardware @power
  Scenario: Power-on sequence verification
    When I power cycle the system
    Then the power rails should come up in correct sequence
    And the ATmega32A should start executing within 2 seconds
    And all GPIO pins should be in their default states

  @hil @hardware @power
  Scenario: Brown-out detection verification
    When I gradually reduce the input voltage
    Then the brown-out detector should trigger at the specified threshold
    And the system should reset cleanly
    When the voltage is restored
    Then the system should restart normally
