@story_0_3_AC3 @story_0_3_AC4 @story_0_3_AC6
Feature: Frequency measurement and lock status per unit
  As an operator I can see frequency and lock status accurately per unit.

  Background:
    Given the HIL wrapper is connected and ready
    And the DUT is powered and at safe defaults

  @us-004 @requirement_FR6 @requirement_FR7
  Scenario Outline: Frequency and lock reflected within 100 ms
    Given unit <unit> receives a frequency of <hz> Hz on the ÷10 input
    And unit <unit> lock input is set to <lock>
    Then within 100 ms holding register <freq_addr> is approximately <hz> Hz
    And status flag bit2 for unit <unit> equals <lock>

    Examples:
      | unit | freq_addr | hz   | lock |
      | 1    | 40017     | 2000 | 1    |
      | 1    | 40017     | 0    | 0    |
