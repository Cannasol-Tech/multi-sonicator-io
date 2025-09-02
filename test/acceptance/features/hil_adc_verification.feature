Feature: ADC Hardware Verification
  As a firmware developer
  I want to verify ADC functionality with known reference voltages
  So that I can ensure power sensing and analog measurements work correctly

  Background:
    Given the HIL framework is connected to ATmega32A
    And the ADC subsystem is initialized
    And the ADC reference is set to 5.0V

  @hil @hardware @adc
  Scenario: POWER_SENSE_4 ADC channel verification
    When I apply 0V to ADC channel "POWER_SENSE_4"
    Then the ADC reading should be approximately 0
    And the voltage should be approximately 0.0V

  @hil @hardware @adc
  Scenario: POWER_SENSE_4 mid-scale verification  
    When I apply 2.5V to ADC channel "POWER_SENSE_4"
    Then the ADC reading should be approximately 512
    And the voltage should be approximately 2.5V ± 0.1V

  @hil @hardware @adc
  Scenario: POWER_SENSE_4 full-scale verification
    When I apply 5.0V to ADC channel "POWER_SENSE_4"
    Then the ADC reading should be approximately 1023
    And the voltage should be approximately 5.0V ± 0.1V

  @hil @hardware @adc
  Scenario: Power measurement scaling verification
    Given the power measurement scaling is 5.44 mV/W
    When I simulate 100W power consumption
    Then the ADC should read a voltage of approximately 0.544V
    And the calculated power should be approximately 100W ± 5W

  @hil @hardware @adc
  Scenario: ADC noise and stability
    When I apply a stable 2.5V to ADC channel "POWER_SENSE_4"
    And I take 100 ADC readings over 10 seconds
    Then the standard deviation should be less than 5 ADC counts
    And all readings should be within ±20 ADC counts of the mean

  @hil @hardware @adc  
  Scenario: Multiple ADC channel independence
    When I apply 1.0V to ADC channel "POWER_SENSE_4"
    And I apply 3.0V to another available ADC channel
    Then "POWER_SENSE_4" should read approximately 1.0V
    And the other channel should read approximately 3.0V
    And neither channel should affect the other
