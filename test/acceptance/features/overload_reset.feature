Feature: Overload detection and reset per unit
  As an operator I can see overload per unit and clear it safely.

  Background:
    Given the HIL wrapper is connected and ready
    And the DUT is powered and at safe defaults

  @us-002
  Scenario Outline: Overload flag reflects and reset behavior is correct
    Given unit <unit> overload input is set to <ovl>
    When I write 1 to holding register <reset_addr> to request overload reset
    Then within 100 ms the overload flag bit1 for unit <unit> equals <expected_flag>
    And starting the unit via register <start_addr> yields behavior per spec when overload=<ovl>

    Examples:
      | unit | reset_addr | start_addr | ovl | expected_flag |
      | 1    | 40009      | 40005      | 1   | 1             |
      | 1    | 40009      | 40005      | 0   | 0             |
