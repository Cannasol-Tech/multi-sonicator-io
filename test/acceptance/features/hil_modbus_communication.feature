Feature: MODBUS RTU Communication Hardware Validation
  As a PLC integration engineer
  I want to verify MODBUS RTU communication with real hardware
  So that I can ensure reliable industrial automation integration

  Background:
    Given the ATmega32A is programmed with MODBUS firmware
    And the HIL framework is connected
    And the MODBUS slave ID is set to 2
    And communication is established at 115200 baud
    
  @hil @modbus @hardware
  Scenario: Read System Status Registers
    When I read MODBUS register 0x0000 (system status)
    Then the register value should be valid
    And the response time should be less than 100ms
    And the CRC should be correct
    
  @hil @modbus @hardware
  Scenario: Write and Verify Control Register
    When I write value 1 to MODBUS register 0x0010 (global enable)
    Then reading register 0x0010 should return value 1
    And the register change should be reflected within 100ms
    And the system status should update accordingly
    
  @hil @modbus @hardware  
  Scenario: Communication Fault Detection
    Given the MODBUS communication is established
    When I disconnect the communication cable
    And wait for 2 seconds
    Then the system should detect communication fault
    And enter failsafe mode per requirements

  @hil @modbus @hardware
  Scenario: Multiple register read operation
    When I read MODBUS registers 0x0000 to 0x0003 in sequence
    Then all reads should succeed
    And each response time should be less than 100ms
    And all CRC values should be correct

  @hil @modbus @hardware
  Scenario: Register write and immediate readback
    When I write value 0x1234 to MODBUS register 0x0100
    And immediately read MODBUS register 0x0100
    Then the read value should be 0x1234
    And the total operation time should be less than 200ms

  @hil @modbus @hardware
  Scenario: Invalid register access handling
    When I attempt to read MODBUS register 0xFFFF (invalid address)
    Then I should receive a MODBUS exception response
    And the exception code should indicate illegal address

  @hil @modbus @hardware
  Scenario: CRC error handling
    When I send a MODBUS request with invalid CRC
    Then the slave should not respond
    And no register values should change
    When I send the same request with valid CRC
    Then the slave should respond correctly

  @hil @modbus @hardware
  Scenario: Communication timeout handling
    When I stop sending MODBUS requests for 5 seconds
    Then the communication fault flag should be set
    When I resume MODBUS communication
    Then the communication fault flag should clear within 1 second

  @hil @modbus @hardware
  Scenario: Sonicator control via MODBUS
    When I write value 1 to MODBUS register 0x0160 (sonicator 4 start)
    Then pin "START_4" should go HIGH
    When I write value 0 to MODBUS register 0x0160
    Then pin "START_4" should go LOW
    And the operation should complete within 50ms
