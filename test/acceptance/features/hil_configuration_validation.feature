@hil @configuration_validation
Feature: HIL Configuration Validation
  As a test engineer
  I want hardware configuration validation against the pin matrix
  So that I can ensure the physical setup matches the expected configuration

  Background:
    Given the HIL system is initialized
    And the hardware configuration file "config/hardware-config.yaml" is loaded
    And the pin matrix validation system is ready

  @hil @ac9 @pin_matrix_validation
  Scenario: AC9 - Hardware configuration validation against pin matrix
    Given the pin matrix configuration is loaded
    When I validate the DUT pin assignments
    Then all ATmega32A pins should match the configuration
    And all signal directions should be correctly defined
    And all pin functions should be properly assigned
    When I validate the harness pin mappings
    Then all Arduino Uno R4 pins should match the configuration
    And the DUT-to-Arduino connections should be verified
    And all signal routing should be validated

  @hil @ac9 @db9_connector_validation
  Scenario: DB9 connector mapping validation
    Given the DB9 connector configuration is loaded
    When I validate DB9-0 (MODBUS communication)
    Then pin 2 should map to MODBUS TX signal
    And pin 3 should map to MODBUS RX signal
    And pin 5 should map to ground reference
    When I validate DB9-1 through DB9-4 (Sonicator interfaces)
    Then each connector should have proper signal mapping
    And power pins should be correctly assigned
    And control signals should be properly routed
    And sensing signals should be correctly connected

  @hil @ac9 @signal_integrity_validation
  Scenario: Signal integrity and electrical validation
    Given the hardware configuration is validated
    When I test signal integrity on all connections
    Then digital signals should have clean transitions
    And analog signals should have proper voltage levels
    And PWM signals should have correct duty cycles
    And communication signals should have proper timing
    When I measure signal crosstalk and interference
    Then crosstalk should be within acceptable limits
    And power supply noise should be minimal
    And signal isolation should be maintained

  @hil @ac9 @power_distribution_validation
  Scenario: Power distribution validation
    Given the power system configuration is loaded
    When I validate the 24V input power
    Then the voltage should be 24V ±5%
    And the current capacity should meet requirements
    When I validate the 12V regulated power
    Then the voltage should be 12V ±2%
    And the regulation should be stable under load
    When I validate the 5V regulated power
    Then the voltage should be 5V ±1%
    And the ripple should be less than 50mV
    And the load regulation should be within specification

  @hil @ac9 @test_capability_validation
  Scenario: Test capability validation against configuration
    Given the test capabilities are defined in configuration
    When I validate MODBUS communication capability
    Then UART communication should be functional
    And MODBUS protocol should be properly implemented
    And communication timing should meet specifications
    When I validate Sonicator 4 control capability
    Then all control signals should be operational
    And all monitoring signals should be functional
    And power measurement should be accurate
    When I validate safety system capabilities
    Then emergency stop should be functional
    And overload protection should be operational
    And all safety interlocks should be working

  @hil @ac9 @configuration_consistency
  Scenario: Configuration consistency validation
    Given multiple configuration sources exist
    When I compare the hardware configuration file
    Then it should match the physical pin connections
    And it should match the firmware pin assignments
    And it should match the test framework expectations
    When I detect configuration inconsistencies
    Then specific mismatches should be identified
    And recommendations for correction should be provided
    And the validation should fail with clear error messages

  @hil @ac9 @dynamic_configuration_validation
  Scenario: Dynamic configuration validation during operation
    Given the HIL system is operational
    When I perform runtime configuration validation
    Then the current hardware state should match configuration
    And all configured capabilities should be available
    And any configuration drift should be detected
    When hardware is modified during operation
    Then configuration changes should be detected automatically
    And validation should be re-run automatically
    And any new mismatches should be reported immediately

  @hil @ac9 @configuration_version_control
  Scenario: Configuration version control and tracking
    Given the hardware configuration has version information
    When I validate the configuration version
    Then the version should match the expected hardware revision
    And the configuration should be compatible with current firmware
    And the configuration should be compatible with test framework
    When configuration versions are mismatched
    Then specific version conflicts should be identified
    And upgrade/downgrade paths should be recommended
    And compatibility issues should be clearly explained

  @hil @ac9 @automated_configuration_validation
  Scenario: Automated configuration validation in CI/CD
    Given the configuration validation is integrated with CI/CD
    When automated tests run in the CI/CD pipeline
    Then configuration validation should run automatically
    And any configuration issues should fail the build
    And detailed validation reports should be generated
    And configuration compliance should be verified
    When configuration validation fails in CI/CD
    Then the specific failures should be reported
    And the build should be marked as failed
    And notifications should be sent to relevant teams
