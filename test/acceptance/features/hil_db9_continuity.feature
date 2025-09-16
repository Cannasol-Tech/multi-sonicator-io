Feature: DB9 Continuity and Signal-Level HIL Checks
  As a hardware validation engineer
  I want to verify that DB9 pins toggle/read correctly via HIL
  So that physical wiring and signal semantics are validated end-to-end

  Background:
    Given the ATmega32A is programmed with MODBUS firmware
    And the HIL framework is connected

  @hil @hardware @trace:S04-AC2 @trace:S04-AC6
  Scenario: START_4 control toggles via MODBUS reflects on DB9-4 START_4
    When I write value 1 to MODBUS register 0x0160 (sonicator 4 start)
    Then pin "START_4" should be HIGH
    When I write value 0 to MODBUS register 0x0160
    Then pin "START_4" should be LOW

  @hil @hardware @trace:S04-AC2 @trace:S04-AC6
  Scenario: Simulated OVERLOAD and FREQ_LOCK reflect on inputs
    When I simulate OVERLOAD for channel 4 ON
    Then pin "OVERLOAD_4" should be HIGH
    When I simulate OVERLOAD for channel 4 OFF
    Then pin "OVERLOAD_4" should be LOW
    When I simulate FREQ_LOCK for channel 4 ON
    Then pin "FREQ_LOCK_4" should be HIGH
    When I simulate FREQ_LOCK for channel 4 OFF
    Then pin "FREQ_LOCK_4" should be LOW

  @hil @hardware @trace:S04-AC6
  Scenario: POWER_SENSE_4 ADC range sanity
    When I read ADC "POWER_SENSE_4"
    Then ADC "POWER_SENSE_4" should be between 0.0 and 5000.0 mV

  @hil @hardware @trace:S04-AC2 @trace:S04-AC6
  Scenario: All connected channels — START control toggles via MODBUS
    Then START pins for all connected channels shall toggle via MODBUS

  @hil @hardware @trace:S04-AC2
  Scenario: All connected channels — OVERLOAD and FREQ_LOCK reflect on inputs
    Then OVERLOAD and FREQ_LOCK signals shall reflect for all connected channels

  @hil @hardware @trace:S04-AC6
  Scenario: All connected channels — POWER_SENSE ADC range sanity
    Then POWER_SENSE ADC ranges shall be sane for all connected channels

  @hil @hardware @trace:S04-AC2
  Scenario: All connected channels — RESET pulses via MODBUS command
    Then RESET pins for all connected channels shall pulse via MODBUS reset command

  @hil @hardware @trace:S04-AC2
  Scenario: All connected channels — AMPLITUDE setpoints are writable/readable
    Then AMPLITUDE setpoints for all connected channels shall be writable and readable

  @hil @hardware @trace:S04-AC2
  Scenario: All connected channels — MODBUS status flags reflect simulated inputs
    Then MODBUS status flags shall reflect OVERLOAD and FREQ_LOCK for all connected channels
