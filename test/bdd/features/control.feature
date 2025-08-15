Feature: Control functions via MODBUS
  # PRD §4, §5, §10

  @req-amplitude @prd-5-functional @prd-10-api @reg-40001-40004 @simulavr @hil
  Scenario: SC-001 Amplitude control per unit
    Given the system is initialized
    When I write holding register 40001 with value 75
    Then holding register 40001 should equal 75 within 100 ms

  @req-startstop @prd-5-functional @prd-10-api @reg-40005-40008 @simulavr @hil
  Scenario: SC-002 Start/Stop per unit
    Given the system is initialized
    When I write holding register 40005 with value 1
    Then holding register 40021 should equal 1 within 100 ms

  @req-overload-reset @prd-5-functional @prd-10-api @reg-40009-40012 @simulavr @hil
  Scenario: SC-003 Overload Reset pulse semantics
    Given the system is initialized
    When I write holding register 40009 with value 1
    Then holding register 40009 should equal 0 within 100 ms

  @req-amplitude-clamp @prd-6-nfr @reg-40001-40004 @simulavr @hil
  Scenario Outline: SC-018 Amplitude clamping
    Given the system is initialized
    When I write holding register <address> with value <value>
    Then holding register <address> should equal <expected> within 100 ms

    Examples:
      | address | value | expected |
      | 40001   | 10    | 20       |
      | 40001   | 105   | 100      |
