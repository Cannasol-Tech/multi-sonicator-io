@hil @hardware_setup
Feature: HIL Hardware Setup
  As a test engineer
  I want to verify the HIL hardware test harness is properly set up
  So that I can run automated tests on the ATmega32A Sonicator Multiplexer

  Background:
    Given the HIL test environment is initialized
    And the hardware configuration is loaded from "config/hardware-config.yaml"

  @hil @ac1 @physical_harness
  Scenario: AC1 - Physical hardware test harness operational with Arduino Test Wrapper
    Given the Arduino Uno R4 WiFi is connected via USB
    When I detect available Arduino ports
    Then an Arduino should be found on a USB port
    And the Arduino should respond to basic communication
    When I upload the Arduino Test Wrapper firmware
    Then the wrapper firmware should be uploaded successfully
    And the wrapper should respond to "PING" with "OK PONG"
    And the wrapper should report version information with "INFO"

  @hil @ac2 @dut_connection
  Scenario: AC2 - ATmega32A Sonicator Multiplexer DUT properly connected and powered
    Given the Arduino Test Wrapper is operational
    When I verify the DUT power connections
    Then the 24V to 12V power regulation should be functional
    And the 12V to 5V power regulation should be functional
    And the DUT should receive stable 5V power supply
    When I check the DUT pin connections via the pin matrix
    Then all DB9 connector pins should be properly mapped
    And the MODBUS communication pins should be connected
    And the sonicator control pins should be connected
    And the power sensing pins should be connected

  @hil @ac3 @isp_programming
  Scenario: AC3 - ISP programming interface functional for firmware deployment
    Given the Arduino Test Wrapper is operational
    And the ATmega32A DUT is connected via ISP pins
    When I configure the Arduino as an ISP programmer
    Then the ISP programming interface should be ready
    When I attempt to read the ATmega32A device signature
    Then the device signature should be "0x1E9502" for ATmega32A
    When I program a test firmware to the DUT
    Then the firmware programming should complete successfully
    And the DUT should respond to basic MODBUS commands
    When I verify the programmed firmware
    Then the firmware verification should pass

  @hil @ac2 @ac3 @power_validation
  Scenario: Power supply validation and ISP functionality
    Given the HIL hardware is connected
    When I measure the power supply voltages
    Then the 24V input should be within ±5% tolerance
    And the 12V regulated output should be within ±2% tolerance  
    And the 5V regulated output should be within ±1% tolerance
    When I test the ISP programming under different power conditions
    Then ISP programming should work reliably at all voltage levels
    And the DUT should maintain stable operation during programming

  @hil @ac1 @ac2 @ac3 @integration
  Scenario: Complete hardware setup integration test
    Given no HIL hardware is currently configured
    When I run the complete HIL hardware setup procedure
    Then the Arduino Test Wrapper should be detected and configured
    And the DUT power supply should be validated
    And the ISP programming interface should be functional
    And all pin matrix connections should be verified
    And the system should be ready for automated testing
    When I run a basic end-to-end connectivity test
    Then all hardware components should respond correctly
    And the setup should be marked as "operational"
