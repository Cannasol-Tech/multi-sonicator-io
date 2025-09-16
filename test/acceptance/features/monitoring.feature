@story_0_3_AC3 @story_0_3_AC4 @story_0_3_AC7
Feature: Monitoring via MODBUS
  # PRD §4, §5, §10

  @req-power @prd-10-api @reg-40013-40016 @hil @requirement_FR5
  Scenario: SC-004 Power monitoring scaling
    Given the system is initialized
    When I simulate power reading of 100
    Then holding register 40013 should equal 100 within 100 ms

  @req-frequency @prd-10-api @reg-40017-40020 @hil
  Scenario: SC-005 Frequency monitoring
    Given the system is initialized
    When I simulate frequency reading of 20000
    Then holding register 40017 should equal 20000 within 100 ms

  @req-status-flags @prd-10-api @reg-40021-40024 @hil
  Scenario: SC-006 Status flags
    Given the system is initialized
    When I simulate starting sonicator 1
    Then status flag bit 0 for unit 1 is 1

  @req-runtimes @reg-40027-40030 @hil @pending
  Scenario: SC-015 Per-unit runtimes
    Given the system is initialized
    When I simulate starting sonicator 1
    Then holding register 40027 should equal 5 within 1000 ms

  @req-overload-counts @reg-40031-40034 @hil @pending
  Scenario: SC-016 Overload counts
    Given the system is initialized
    When I simulate overload on sonicator 1
    Then holding register 40031 should equal 1 within 200 ms

  @req-freq-lock @hil
  Scenario: SC-019 Frequency lock behavior
    Given the system is initialized
    When I simulate frequency lock on sonicator 1
    Then status flag bit 2 for unit 1 is 1
