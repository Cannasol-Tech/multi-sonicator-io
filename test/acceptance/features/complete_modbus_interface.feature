Feature: Complete MODBUS Interface Testing
  As a PLC programmer
  I want to test the complete MODBUS register interface of the Multi-Sonicator I/O Controller
  So that I can ensure full compatibility with industrial automation systems

  Background:
    Given the Multi-Sonicator I/O Controller is connected via MODBUS RTU
    And the communication is established at 115200 baud, 8N1 format
    And the MODBUS slave ID is configured correctly
    And all 4 sonicators are connected and ready

  @modbus @interface @critical
  Scenario: Complete system status register testing
    Given the system is initialized and operational
    When I read the system status registers (0x0000-0x000F)
    Then register 0x0000 should contain the overall system status
    And register 0x0001 should contain the active sonicator count
    And register 0x0002 should contain the active sonicator bitmask
    And register 0x0003 should contain the watchdog status
    And register 0x0004 should contain the communication error count
    And all reserved registers should return zero
    And all reads should complete within 100ms

  @modbus @interface @critical
  Scenario: Global control register testing
    Given the system is ready for control operations
    When I write to the global control registers (0x0010-0x001F)
    Then writing 1 to register 0x0010 should enable the global system
    And writing 1 to register 0x0011 should trigger emergency stop
    And writing 1 to register 0x0012 should initiate system reset
    And all control actions should be reflected within 100ms
    And reading back the registers should confirm the written values
    And reserved registers should reject writes gracefully

  @modbus @interface @comprehensive
  Scenario: Complete sonicator 1 register interface testing
    Given sonicator 1 is connected and ready
    When I test all sonicator 1 registers (0x0100-0x011F)
    Then writing to register 0x0100 should control start/stop
    And writing to register 0x0101 should set amplitude (20-100%)
    And writing to register 0x0102 should trigger overload reset
    And reading register 0x0110 should return power consumption
    And reading register 0x0111 should return operating frequency
    And reading register 0x0112 should return status flags
    And reading register 0x0113 should return actual amplitude
    And all operations should complete within 100ms

  @modbus @interface @comprehensive
  Scenario: Complete sonicator 2 register interface testing
    Given sonicator 2 is connected and ready
    When I test all sonicator 2 registers (0x0120-0x013F)
    Then writing to register 0x0120 should control start/stop
    And writing to register 0x0121 should set amplitude (20-100%)
    And writing to register 0x0122 should trigger overload reset
    And reading register 0x0130 should return power consumption
    And reading register 0x0131 should return operating frequency
    And reading register 0x0132 should return status flags
    And reading register 0x0133 should return actual amplitude
    And all operations should complete within 100ms

  @modbus @interface @comprehensive
  Scenario: Complete sonicator 3 register interface testing
    Given sonicator 3 is connected and ready
    When I test all sonicator 3 registers (0x0140-0x015F)
    Then writing to register 0x0140 should control start/stop
    And writing to register 0x0141 should set amplitude (20-100%)
    And writing to register 0x0142 should trigger overload reset
    And reading register 0x0150 should return power consumption
    And reading register 0x0151 should return operating frequency
    And reading register 0x0152 should return status flags
    And reading register 0x0153 should return actual amplitude
    And all operations should complete within 100ms

  @modbus @interface @comprehensive
  Scenario: Complete sonicator 4 register interface testing
    Given sonicator 4 is connected and ready
    When I test all sonicator 4 registers (0x0160-0x017F)
    Then writing to register 0x0160 should control start/stop
    And writing to register 0x0161 should set amplitude (20-100%)
    And writing to register 0x0162 should trigger overload reset
    And reading register 0x0170 should return power consumption
    And reading register 0x0171 should return operating frequency
    And reading register 0x0172 should return status flags
    And reading register 0x0173 should return actual amplitude
    And all operations should complete within 100ms

  @modbus @interface @validation
  Scenario: MODBUS function code 03 (Read Holding Registers) testing
    Given the system has various register values set
    When I use function code 03 to read single registers
    Then all readable registers should return correct values
    When I use function code 03 to read multiple consecutive registers
    Then all values should be returned in the correct order
    And the response should include the correct byte count
    And the CRC should be calculated correctly

  @modbus @interface @validation
  Scenario: MODBUS function code 06 (Write Single Register) testing
    Given the system is ready for write operations
    When I use function code 06 to write to writable registers
    Then the register values should be updated correctly
    And the response should echo the written address and value
    And the CRC should be calculated correctly
    When I attempt to write to read-only registers
    Then the system should return an illegal address exception

  @modbus @interface @validation
  Scenario: MODBUS function code 16 (Write Multiple Registers) testing
    Given the system is ready for multiple write operations
    When I use function code 16 to write multiple consecutive registers
    Then all register values should be updated correctly
    And the response should confirm the starting address and quantity
    And the CRC should be calculated correctly
    When I attempt to write across read-only boundaries
    Then the system should return an appropriate exception

  @modbus @interface @error-handling
  Scenario: MODBUS error handling and exception responses
    Given the system is operational
    When I attempt to read from an illegal address
    Then the system should return exception code 02 (Illegal Data Address)
    When I attempt to write an illegal data value
    Then the system should return exception code 03 (Illegal Data Value)
    When I send a request with invalid CRC
    Then the system should not respond (silent error handling)
    When I send a request to the wrong slave ID
    Then the system should not respond

  @modbus @interface @performance
  Scenario: MODBUS communication performance testing
    Given the system is under normal operating conditions
    When I perform 100 consecutive read operations
    Then each operation should complete within 100ms
    And no communication errors should occur
    And all responses should have correct CRC values
    When I perform 50 consecutive write operations
    Then each operation should complete within 100ms
    And all register values should be updated correctly
    And the system should maintain stable operation

  @modbus @interface @stress-testing
  Scenario: MODBUS communication stress testing
    Given the system is configured for stress testing
    When I perform rapid sequential read/write operations
    Then the system should handle the load without errors
    And response times should remain within specifications
    And no data corruption should occur
    When I perform operations while sonicators are running
    Then MODBUS communication should not interfere with sonicator operation
    And sonicator operation should not affect MODBUS timing

  @modbus @interface @legacy-compatibility
  Scenario: Legacy MODBUS register mapping compatibility
    Given the system supports legacy register addresses
    When I access registers using the 40001-based addressing
    Then register 40001 should map to sonicator 1 amplitude
    And register 40005 should map to sonicator 1 start/stop
    And register 40009 should map to sonicator 1 overload reset
    And all legacy addresses should work correctly
    And the mapping should be consistent with documentation

  @modbus @interface @concurrent-access
  Scenario: Concurrent MODBUS access testing
    Given multiple MODBUS masters may access the system
    When I simulate concurrent access from multiple sources
    Then the system should handle requests in order
    And no data corruption should occur
    And all responses should be sent to the correct master
    And the system should maintain data consistency
    And performance should degrade gracefully under load

  @pending @modbus @interface @advanced-functions
  Scenario: Advanced MODBUS function codes
    Given the system supports extended MODBUS functions
    When I use function code 23 (Read/Write Multiple Registers)
    Then the system should perform atomic read/write operations
    When I use function code 43 (Read Device Identification)
    Then the system should return device identification information
    And vendor name, product code, and version should be included
    And all extended functions should follow MODBUS specifications
