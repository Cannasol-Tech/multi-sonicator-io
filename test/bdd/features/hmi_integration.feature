Feature: HMI/PLC integration
  # PRD §9, §10

  @req-hmi @simulavr @hil
  Scenario: SC-010 HMI contract adherence (4xxxx convention)
    Given the system is initialized
    When I write holding register 40001 with value 80
    Then holding register 40001 should equal 80 within 100 ms

  @req-hmi-addressing @simulavr @hil
  Scenario: SC-024 HMI addressing examples
    Given the system is initialized
    When I write holding register 40005 with value 1
    Then holding register 40021 should equal 1 within 100 ms
