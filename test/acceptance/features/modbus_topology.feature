Feature: MODBUS Topology and Link Verification
  As a PLC integration engineer
  I want the MODBUS link and configuration to be validated against the SSoT
  So that the Main Automation System communicates reliably via DB9-0/DB9-5

  Background:
    Given the hardware configuration is loaded

  @hil @modbus @hardware @trace:S04-AC3
  Scenario: MODBUS connector and config alignment
    Then a MODBUS connector (DB9-0 or DB9-5) shall be present with RX/TX pins defined
    And the configured MODBUS slave id and baud rate shall match the hardware configuration
    And the HIL profile shall use the same serial port as the hardware configuration when available

  @hil @modbus @hardware @trace:S04-AC3
  Scenario: Establish MODBUS link on configured settings
    Given the ATmega32A is programmed with MODBUS firmware
    And the MODBUS slave ID is set to 2
    And communication is established at 115200 baud
    Then the MODBUS communication is established

