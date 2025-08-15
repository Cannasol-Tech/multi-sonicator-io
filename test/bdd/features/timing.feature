Feature: Timing
  # PRD §5, §6
  @pending

  @req-latency @simulavr @hil
  Scenario: SC-009 Latency ≤100 ms (representative)
    Given the system is initialized
    When I stimulate running to 1
    Then holding register 40021 should equal 1 within 100 ms

  @req-latency @simulavr @hil
  Scenario: SC-009B Latency matrix (overload reset, frequency)
    Given the system is initialized
    When I write holding register 40009 with value 1
    Then holding register 40009 should equal 0 within 100 ms
