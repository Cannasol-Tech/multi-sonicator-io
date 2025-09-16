Feature: DB9 Hardware Mapping Consistency
  As a hardware validation engineer
  I want the DB9 connector mappings to match the single source of truth
  So that the Arduino Test Wrapper and DUT PCB remain consistent and testable

  Background:
    Given the hardware configuration is loaded

  @hil @hardware @trace:S04-AC1
  Scenario: Pin assignments and DB9 mappings are consistent with hardware-config.yaml
    Then DB9 connectors for sonicators 1..4 shall map required signals per channel
    And a MODBUS connector (DB9-0 or DB9-5) shall be present with RX/TX pins defined
    And all referenced DUT pins shall exist in the DUT pin matrix

