Feature: Hardware MODBUS Frequency Verification
  As a firmware engineer,
  I want to verify frequency generation and measurement capabilities by reading MODBUS registers,
  so that I can confirm the complete frequency pipeline works with actual ATmega32A hardware.

  Background:
    Given the Arduino HIL wrapper is connected to ATmega32A
    And the MODBUS RTU connection is established at 115200 baud
    And the DUT is powered and at safe defaults

  @acceptance @hil @hardware @frequency @critical
  Scenario: AC-6 Hardware MODBUS Frequency Verification - Nominal Frequency
    Given the HIL wrapper is connected and ready
    When I generate 2000 Hz frequency on FREQ_DIV10_4 input via HIL
    And I wait 1000 ms for ATmega32A frequency measurement to stabilize
    Then MODBUS register 40370 should contain approximately 20000 Hz within 5% tolerance
    And the frequency reading should be stable over multiple readings

  @acceptance @hil @hardware @frequency @critical
  Scenario: AC-7 Hardware MODBUS Frequency Verification - Low Frequency Range  
    Given the HIL wrapper is connected and ready
    When I generate 1800 Hz frequency on FREQ_DIV10_4 input via HIL
    And I wait 1000 ms for ATmega32A frequency measurement to stabilize
    Then MODBUS register 40370 should contain approximately 18000 Hz within 5% tolerance
    And the frequency reading should be stable over multiple readings

  @acceptance @hil @hardware @frequency @critical
  Scenario: AC-8 Hardware MODBUS Frequency Verification - High Frequency Range
    Given the HIL wrapper is connected and ready  
    When I generate 2200 Hz frequency on FREQ_DIV10_4 input via HIL
    And I wait 1000 ms for ATmega32A frequency measurement to stabilize
    Then MODBUS register 40370 should contain approximately 22000 Hz within 5% tolerance
    And the frequency reading should be stable over multiple readings

  @acceptance @hil @hardware @frequency @critical
  Scenario: AC-9 Hardware MODBUS Frequency Verification - Zero Frequency
    Given the HIL wrapper is connected and ready
    When I generate 0 Hz frequency on FREQ_DIV10_4 input via HIL
    And I wait 1000 ms for ATmega32A frequency measurement to stabilize  
    Then MODBUS register 40370 should contain approximately 0 Hz within 10 Hz tolerance
    And the frequency reading should be stable over multiple readings

  @acceptance @hil @hardware @frequency @critical
  Scenario: AC-10 Hardware MODBUS Frequency Verification - Frequency Change Detection
    Given the HIL wrapper is connected and ready
    When I generate 2000 Hz frequency on FREQ_DIV10_4 input via HIL
    And I wait 1000 ms for ATmega32A frequency measurement to stabilize
    Then MODBUS register 40370 should contain approximately 20000 Hz within 5% tolerance
    When I change frequency to 1900 Hz on FREQ_DIV10_4 input via HIL
    And I wait 1000 ms for ATmega32A frequency measurement to stabilize
    Then MODBUS register 40370 should contain approximately 19000 Hz within 5% tolerance
    And the frequency change should be detected within acceptable time