Feature: System metrics
  # PRD §4, §5

  @req-active-count @reg-40035 @simulavr @hil
  Scenario: SC-007 Active Sonicator Count
    Given the system is initialized
    When I stimulate running to 1
    Then holding register 40035 should equal 1 within 100 ms

  @req-active-mask @reg-40036 @simulavr @hil
  Scenario: SC-008 Active Sonicator Mask
    Given the system is initialized
    When I stimulate running to 1
    Then holding register 40036 should equal 1 within 100 ms

  @req-system-status @reg-40025 @simulavr @hil
  Scenario: SC-013 System status bitfield
    Given the system is initialized
    When I stimulate estop to 1
    Then holding register 40025 should equal 1 within 50 ms

  @req-active-transitions @reg-40035 @reg-40036 @simulavr @hil
  Scenario: SC-017 Active transitions stability
    Given the system is initialized
    When I stimulate running to 0
    Then holding register 40035 should equal 0 within 100 ms
