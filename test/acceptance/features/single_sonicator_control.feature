Feature: Single Sonicator Control (Story 1.4)
  As a firmware engineer,
  I want to implement basic control and monitoring of a single sonicator unit via the ATmega32A,
  so that the system can start/stop and modulate amplitude while reporting status through the standardized interface.

  Background:
    Given the HIL hardware is initialized
    And the ATmega32A firmware is loaded
    And MODBUS RTU communication is established at 115200 baud

  @acceptance @hil @story-1.4 @critical
  Scenario: AC-1 Start/Stop Command Response Time
    Given Sonicator 4 is in IDLE state
    When I write MODBUS register 0x0160 (REG_SON4_START_STOP) with value 1
    Then the sonicator START signal should be asserted within 50ms
    And the sonicator status should indicate RUNNING state
    When I write MODBUS register 0x0160 (REG_SON4_START_STOP) with value 0  
    Then the sonicator START signal should be deasserted within 100ms
    And the sonicator status should indicate IDLE state

  @acceptance @hil @story-1.4 @critical
  Scenario: AC-2 Amplitude Control Within Tolerance
    Given Sonicator 4 is in IDLE state
    When I write MODBUS register 0x0161 (REG_SON4_AMPLITUDE_SP) with value 75
    Then the amplitude PWM output should be 75% ± 2% within 200ms
    When I write MODBUS register 0x0161 (REG_SON4_AMPLITUDE_SP) with value 50
    Then the amplitude PWM output should be 50% ± 2% within 200ms
    When I write MODBUS register 0x0161 (REG_SON4_AMPLITUDE_SP) with value 100
    Then the amplitude PWM output should be 100% ± 2% within 200ms

  @acceptance @hil @story-1.4 @critical
  Scenario: AC-3 Telemetry Registers Accuracy
    Given Sonicator 4 is running at 50% amplitude
    When I read MODBUS register 0x0170 (REG_SON4_POWER_WATTS)
    Then the power reading should be > 0 watts
    When I read MODBUS register 0x0171 (REG_SON4_FREQUENCY_HZ)
    Then the frequency reading should be between 19800Hz and 20200Hz
    When I read MODBUS register 0x0172 (REG_SON4_STATUS_FLAGS)
    Then the RUNNING flag should be set (bit 0)
    And the FREQ_LOCK flag should be set (bit 2)

  @acceptance @hil @story-1.4 @critical
  Scenario: AC-4 MODBUS Response Time Compliance
    Given Sonicator 4 is in any valid state
    When I send any MODBUS read command to Sonicator 4 registers
    Then the response should arrive within 100ms
    When I send any MODBUS write command to Sonicator 4 registers
    Then the response should arrive within 100ms
    And the control action should be executed within 100ms

  @acceptance @hil @story-1.4 @critical @frequency
  Scenario: AC-5 HIL Frequency Generation and MODBUS Reading
    Given the HIL wrapper is connected and ready
    And the DUT is powered and at safe defaults
    When unit 4 receives a frequency of 2000 Hz on the ÷10 input
    Then the HIL wrapper reports frequency 2000 Hz for unit 4
    And within 500 ms holding register 40370 is approximately 20000 Hz
    When unit 4 receives a frequency of 1800 Hz on the ÷10 input
    Then the HIL wrapper reports frequency 1800 Hz for unit 4
    And within 500 ms holding register 40370 is approximately 18000 Hz
    When unit 4 receives a frequency of 0 Hz on the ÷10 input
    Then the HIL wrapper reports frequency 0 Hz for unit 4
    And within 500 ms holding register 40370 is approximately 0 Hz

  @acceptance @hil @story-1.4 @critical
  Scenario: AC-5 Complete Control Loop Validation
    Given the system is in a clean initial state
    # Full control sequence validation
    When I write REG_SON4_AMPLITUDE_SP with value 60
    And I write REG_SON4_START_STOP with value 1
    Then within 200ms the following should be true:
      | Register              | Expected Value | Tolerance |
      | REG_SON4_STATUS_FLAGS | RUNNING + FREQ_LOCK | exact |
      | REG_SON4_POWER_WATTS  | > 0            | n/a   |
      | REG_SON4_FREQUENCY_HZ | 19800-20200    | range |
      | REG_SON4_AMPLITUDE_ACT| 60             | ± 2%  |
    # Stop sequence validation  
    When I write REG_SON4_START_STOP with value 0
    Then within 200ms the following should be true:
      | Register              | Expected Value | Tolerance |
      | REG_SON4_STATUS_FLAGS | 0 (IDLE)       | exact |
      | REG_SON4_POWER_WATTS  | 0              | exact |

  @acceptance @hil @story-1.4 @edge-case
  Scenario: Amplitude Clamping Validation
    Given Sonicator 4 is in IDLE state
    # Test minimum clamping
    When I write REG_SON4_AMPLITUDE_SP with value 10
    Then the register should read back as 20 (clamped minimum)
    # Test maximum clamping  
    When I write REG_SON4_AMPLITUDE_SP with value 150
    Then the register should read back as 100 (clamped maximum)
    # Test valid range
    When I write REG_SON4_AMPLITUDE_SP with value 75
    Then the register should read back as 75 (unchanged)

  @acceptance @hil @story-1.4 @fault-handling
  Scenario: Overload Reset Command
    Given Sonicator 4 is in FAULT state due to overload
    When I write REG_SON4_OVERLOAD_RESET with value 1
    Then within 50ms:
      | Action | Expected Result |
      | RESET pulse sent | PC1 (RESET_4) asserted for 20ms |
      | Register auto-clear | REG_SON4_OVERLOAD_RESET reads 0 |
      | Status update | OVERLOAD flag cleared if fault resolved |

  @acceptance @hil @story-1.4 @timing
  Scenario: State Transition Timing Validation
    Given Sonicator 4 is in IDLE state
    # Start sequence timing
    When I command start at time T0
    Then at T0 + 0ms: state should be STARTING
    And at T0 + 50ms: state should be RUNNING  
    And at T0 + 60ms: START signal should be asserted
    # Stop sequence timing
    When I command stop at time T1
    Then at T1 + 0ms: state should be STOPPING
    And at T1 + 100ms: state should be IDLE
    And at T1 + 110ms: START signal should be deasserted

  @acceptance @hil @story-1.4 @comprehensive
  Scenario: HIL End-to-End Validation
    # This scenario validates complete hardware integration
    Given the Arduino Test Harness is connected per pin matrix
    And Sonicator 4 hardware signals are mapped correctly:
      | ATmega32A Pin | Signal Name    | Test Harness Pin | Function |
      | PC0 (22)      | START_4        | A3               | Output   |
      | PC1 (23)      | RESET_4        | A4               | Output   |
      | PB4 (5)       | FREQ_LOCK_4    | D8               | Input    |
      | PB0 (1)       | FREQ_DIV10_4   | D7               | Input    |
    When I execute the complete sonicator control sequence
    Then all hardware signals should respond correctly
    And all telemetry should be accurate within specifications
    And MODBUS response times should be < 100ms throughout
