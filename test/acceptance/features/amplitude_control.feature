Feature: Amplitude control (common)
  As an operator I can set amplitude within range and see it reflected on hardware.

  Background:
    Given the HIL wrapper is connected and ready
    And the DUT is powered and at safe defaults

  @us-003
  Scenario Outline: Set amplitude within range and verify mapping
    When I write <value> to holding register <addr>
    Then within 100 ms the amplitude output maps to <value> percent within tolerance 2%

    Examples:
      | addr  | value |
      | 40001 | 20    |
      | 40001 | 60    |
      | 40001 | 100   |

  @us-003 @invalid
  Scenario: Reject out-of-range amplitude
    When I write 5 to holding register 40001
    Then the write is rejected or the register value is clamped per spec
