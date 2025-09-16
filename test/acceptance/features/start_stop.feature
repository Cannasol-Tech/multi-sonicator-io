@story_0_3_AC3 @story_0_3_AC4 @story_0_3_AC5
Feature: Start and stop a single sonicator
  As an operator I can start/stop any unit and see status reflected promptly.

  Background:
    Given the HIL wrapper is connected and ready
    And the DUT is powered and at safe defaults

  @smoke @us-001 @requirement_FR2 @requirement_FR9
  Scenario Outline: Start/Stop reflects in status and system count/mask within 100 ms
    When I write <cmd> to holding register <addr>
    Then within 100 ms the status flag bit0 for unit <unit> equals <expected>
    And Active Sonicator Count and Mask reflect the running units

    Examples:
      | unit | addr  | cmd | expected |
      | 1    | 40005 | 1   | 1        |
      | 1    | 40005 | 0   | 0        |
      | 2    | 40006 | 1   | 1        |
      | 2    | 40006 | 0   | 0        |
