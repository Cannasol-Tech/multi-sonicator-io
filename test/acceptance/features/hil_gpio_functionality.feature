Feature: GPIO Hardware Functionality
  As a firmware developer
  I want to verify GPIO pin control and monitoring
  So that I can ensure sonicator interface pins work correctly

  Background:
    Given the HIL framework is connected to ATmega32A
    And the GPIO HAL is loaded on the target

  @hil @hardware @gpio
  Scenario: Digital output control - START_4 pin
    When I set pin "START_4" to HIGH via HIL framework
    Then the pin voltage should be approximately 5.0V
    When I set pin "START_4" to LOW via HIL framework  
    Then the pin voltage should be approximately 0.0V

  @hil @hardware @gpio
  Scenario: Digital output control - RESET_4 pin
    When I set pin "RESET_4" to HIGH via HIL framework
    Then the pin voltage should be approximately 5.0V
    When I set pin "RESET_4" to LOW via HIL framework
    Then the pin voltage should be approximately 0.0V

  @hil @hardware @gpio
  Scenario: Digital input reading - OVERLOAD_4 pin
    Given pin "OVERLOAD_4" is configured as input with pull-up
    When the HIL framework applies 0V to pin "OVERLOAD_4" externally
    Then reading pin "OVERLOAD_4" should return LOW
    When the HIL framework disconnects the external connection
    Then reading pin "OVERLOAD_4" should return HIGH

  @hil @hardware @gpio
  Scenario: Digital input reading - FREQ_LOCK_4 pin
    Given pin "FREQ_LOCK_4" is configured as input
    When the HIL framework applies 5V to pin "FREQ_LOCK_4"
    Then reading pin "FREQ_LOCK_4" should return HIGH
    When the HIL framework applies 0V to pin "FREQ_LOCK_4"
    Then reading pin "FREQ_LOCK_4" should return LOW

  @hil @hardware @gpio
  Scenario: Pin state persistence
    When I set pin "START_4" to HIGH
    And I set pin "RESET_4" to LOW
    And I wait for 5 seconds
    Then pin "START_4" should still be HIGH
    And pin "RESET_4" should still be LOW

  @hil @hardware @gpio
  Scenario: Status LED control
    When I set pin "STATUS_LED" to HIGH
    Then the status LED should be visible on the hardware
    When I set pin "STATUS_LED" to LOW
    Then the status LED should be off
