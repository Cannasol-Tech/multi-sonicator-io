Feature: MODBUS protocol behaviors
  # PRD §10
  @pending

  @req-fc16 @simulavr @hil
  Scenario: SC-020 FC16 multiple write boundary cases
    Given the system is initialized
    When I write holding register 40001 with value 50
    Then holding register 40001 should equal 50 within 100 ms

  @req-illegal-address @simulavr
  Scenario: SC-021 Illegal address handling
    Given the system is initialized
    When I write holding register 49999 with value 1
    Then holding register 49999 should equal 0 within 100 ms

  @req-slave-id @simulavr @hil
  Scenario: SC-022 Slave ID configuration
    Given the system is initialized
    When I write holding register 40002 with value 2
    Then holding register 40002 should equal 2 within 100 ms

  @req-crc @simulavr
  Scenario: SC-023 CRC error handling
    Given the system is initialized
    Then corrupted frames are rejected
