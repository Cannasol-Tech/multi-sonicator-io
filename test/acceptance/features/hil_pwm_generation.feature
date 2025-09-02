Feature: PWM Generation Hardware Verification
  As a firmware developer
  I want to verify PWM output generation and measurement
  So that I can ensure amplitude control signals work correctly

  Background:
    Given the HIL framework is connected to ATmega32A
    And the PWM subsystem is initialized
    And pin "AMPLITUDE_ALL" is configured for PWM output

  @hil @hardware @pwm
  Scenario: Basic PWM output generation
    When I set PWM on pin "AMPLITUDE_ALL" to 50% duty cycle at 1kHz
    Then the HIL framework should measure approximately 50% duty cycle
    And the frequency should be approximately 1000Hz ± 10Hz

  @hil @hardware @pwm
  Scenario: PWM duty cycle variation
    When I set PWM duty cycle to 25% on pin "AMPLITUDE_ALL"
    Then the measured duty cycle should be 25% ± 2%
    When I set PWM duty cycle to 75% on pin "AMPLITUDE_ALL"
    Then the measured duty cycle should be 75% ± 2%

  @hil @hardware @pwm
  Scenario: PWM frequency accuracy
    When I set PWM frequency to 500Hz on pin "AMPLITUDE_ALL"
    Then the measured frequency should be 500Hz ± 5Hz
    When I set PWM frequency to 2000Hz on pin "AMPLITUDE_ALL"
    Then the measured frequency should be 2000Hz ± 20Hz

  @hil @hardware @pwm
  Scenario: PWM voltage levels
    When I set PWM to 0% duty cycle on pin "AMPLITUDE_ALL"
    Then the average voltage should be approximately 0V
    When I set PWM to 100% duty cycle on pin "AMPLITUDE_ALL"
    Then the average voltage should be approximately 5V
    When I set PWM to 50% duty cycle on pin "AMPLITUDE_ALL"
    Then the average voltage should be approximately 2.5V ± 0.2V

  @hil @hardware @pwm
  Scenario: PWM signal stability
    When I set PWM to 60% duty cycle at 1kHz on pin "AMPLITUDE_ALL"
    And I measure PWM for 10 seconds continuously
    Then the duty cycle should remain stable within ±1%
    And the frequency should remain stable within ±5Hz

  @hil @hardware @pwm
  Scenario: PWM disable and enable
    When PWM is enabled on pin "AMPLITUDE_ALL" with 40% duty cycle
    Then I should measure PWM signal
    When I disable PWM on pin "AMPLITUDE_ALL"
    Then the pin should be in a static state
    When I re-enable PWM with the same settings
    Then I should measure the same PWM signal again
