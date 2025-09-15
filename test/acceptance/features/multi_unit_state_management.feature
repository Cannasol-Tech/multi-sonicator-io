@pending @hil @story-4-1 @epic-4 @state-management
Feature: Multi-Unit State Management (Story 4.1)
  As a Control Systems Engineer
  I want coordinated state machines for up to 4 units
  So that multi-unit operation is safe, synchronized, and efficient

  Background:
    Given all units are in normal operating condition

  @ac-1 @trace:S-4.1
  Scenario: Individual state machines exist for 4 units
    When the system initializes
    Then all units should be in stopped state

  @ac-2 @trace:S-4.1
  Scenario: Master coordination state machine exists
    When the system initializes
    Then a master coordination state should be available

  @ac-3 @trace:S-4.1
  Scenario: State synchronization during coordinated start
    When a coordinated start is requested for units 1-4
    Then within 50 ms the system count should be 4
    And within 50 ms the system mask should be 0x000F

  @ac-4 @trace:S-4.1 @safety
  Scenario: Safe transitions and interlocks
    Given all units are in normal operating condition
    And a coordinated start is requested for units 1-4
    When a conflicting command is issued during coordinated operation
    Then the master should reject unsafe transitions

  @ac-5 @trace:S-4.1
  Scenario: State persistence and safe recovery without auto-resume
    Given unit 2 was running with amplitude 70% and the system state is persisted
    When the system restarts
    Then all units should be in stopped state
    And the master coordination state should be IDLE
    And the previous state for unit 2 should be recorded as "RUNNING"
    And the persisted amplitude for unit 2 should be 70%

  @ac-6 @trace:S-4.1 @modbus
  Scenario: Real-time state reporting via MODBUS
    When a coordinated start is requested for units 1-4
    Then within 50 ms the system count should be 4
    And within 50 ms the system mask should be 0x000F

  @ac-7 @trace:S-4.1 @safety
  Scenario: Emergency stop handling
    When an emergency stop is issued
    Then the system should enter emergency stop mode
    And all units should be in safe state

  @ac-8 @trace:S-4.1
  Scenario: Transition coverage completeness
    Given transition matrices are defined for master and unit state machines
    Then tests should cover all valid transitions

  @ac-9 @trace:S-4.1 @performance
  Scenario: Real-time performance under 4-unit coordination
    When a coordinated start is requested for units 1-4
    Then within 10 ms the system count should be 4
    And within 10 ms the system mask should be 0x000F

  @ac-10 @trace:S-4.1 @safety
  Scenario: Safety integration and overload protection
    Given unit 4 overload input is set to 1
    When I write 1 to holding register 352
    Then starting the unit via register 352 yields behavior per spec when overload=1
    And within 100 ms the overload flag should be set

  @ac-11 @trace:S-4.1 @modbus @read-only
  Scenario Outline: System previous-state registers are read-only
    Given all units are in normal operating condition
    When I write <value> to holding register <addr>
    Then the write is rejected or the register value is clamped per spec

    Examples:
      | addr | value |
      | 5    | 0x000F |
      | 6    | 3      |

  @ac-11 @trace:S-4.1 @modbus @read-only
  Scenario Outline: Per-unit previous-state registers are read-only
    Given all units are in normal operating condition
    When I write <value> to holding register <addr>
    Then the write is rejected or the register value is clamped per spec

    Examples:
      | addr | value |
      | 276  | 2     |   # 0x0114 Unit1 PREV_STATE
      | 277  | 80    |   # 0x0115 Unit1 PERSISTED_AMPLITUDE
      | 278  | 1     |   # 0x0116 Unit1 LAST_FAULT_CODE
      | 279  | 1234  |   # 0x0117 Unit1 LAST_STATE_TIMESTAMP_LO
      | 308  | 2     |   # 0x0134 Unit2 PREV_STATE
      | 309  | 70    |   # 0x0135 Unit2 PERSISTED_AMPLITUDE
      | 310  | 0     |   # 0x0136 Unit2 LAST_FAULT_CODE
      | 311  | 5678  |   # 0x0137 Unit2 LAST_STATE_TIMESTAMP_LO
      | 340  | 2     |   # 0x0154 Unit3 PREV_STATE
      | 341  | 60    |   # 0x0155 Unit3 PERSISTED_AMPLITUDE
      | 342  | 0     |   # 0x0156 Unit3 LAST_FAULT_CODE
      | 343  | 42    |   # 0x0157 Unit3 LAST_STATE_TIMESTAMP_LO
      | 372  | 2     |   # 0x0174 Unit4 PREV_STATE
      | 373  | 50    |   # 0x0175 Unit4 PERSISTED_AMPLITUDE
      | 374  | 0     |   # 0x0176 Unit4 LAST_FAULT_CODE
      | 375  | 9999  |   # 0x0177 Unit4 LAST_STATE_TIMESTAMP_LO

  @ac-11 @trace:S-4.1 @modbus
  Scenario Outline: Address mapping for previous-state per-unit registers
    When the system initializes
    Then calculated register address for "<name>" unit <unit> should be <addr>

    Examples:
      | unit | name                  | addr |
      | 1    | PREV_STATE           | 276  |
      | 1    | PERSISTED_AMPLITUDE  | 277  |
      | 1    | LAST_FAULT_CODE      | 278  |
      | 1    | LAST_STATE_TIMESTAMP_LO | 279 |
      | 2    | PREV_STATE           | 308  |
      | 2    | PERSISTED_AMPLITUDE  | 309  |
      | 2    | LAST_FAULT_CODE      | 310  |
      | 2    | LAST_STATE_TIMESTAMP_LO | 311 |
      | 3    | PREV_STATE           | 340  |
      | 3    | PERSISTED_AMPLITUDE  | 341  |
      | 3    | LAST_FAULT_CODE      | 342  |
      | 3    | LAST_STATE_TIMESTAMP_LO | 343 |
      | 4    | PREV_STATE           | 372  |
      | 4    | PERSISTED_AMPLITUDE  | 373  |
      | 4    | LAST_FAULT_CODE      | 374  |
      | 4    | LAST_STATE_TIMESTAMP_LO | 375 |

  @ac-11 @trace:S-4.1 @modbus
  Scenario: System previous-active mask reflects last known running units
    Given units 1,2,3 were previously running and the system state is persisted
    When the system restarts
    Then all units should be in stopped state
    And the master coordination state should be IDLE
    And the previous active mask should be 0x0007

  @ac-11 @trace:S-4.1 @modbus
  Scenario Outline: Shutdown reason codes are exposed and stable
    Given the last shutdown reason was "<reason>"
    When the system restarts
    Then reading LAST_SHUTDOWN_REASON returns <code>

    Examples:
      | reason      | code |
      | normal      | 0    |
      | watchdog    | 1    |
      | power-loss  | 2    |
      | e-stop      | 3    |
      | fault       | 4    |

  @ac-11 @trace:S-4.1 @modbus
  Scenario Outline: Previous state values are from the allowed state enumeration
    Given the system had previous state recorded as <prev_state> for unit <unit>
    When the system restarts
    Then the previous state for unit <unit> should be recorded as "<prev_state_str>"

    Examples:
      | unit | prev_state | prev_state_str |
      | 1    | 0          | STOPPED        |
      | 1    | 1          | STARTING       |
      | 1    | 2          | RUNNING        |
      | 1    | 3          | STOPPING       |
      | 1    | 4          | OVERLOAD       |
      | 1    | 5          | FAULT          |
      | 1    | 6          | UNKNOWN        |

  @ac-3 @trace:S-4.1 @sync
  Scenario: Coordinated start uses an all-or-none barrier
    Given all units are in normal operating condition
    When a coordinated start is requested for units 1-4
    Then within 100 ms the system count should be 4
    And within 100 ms all units 1-4 should be in "RUNNING" state
    And the master coordination state should be RUNNING

  @ac-3 @trace:S-4.1 @sync
  Scenario: Partial mask coordinated start (units 1 and 3)
    Given all units are in normal operating condition
    When a coordinated start is requested for units 1 and 3
    Then within 50 ms the system count should be 2
    And within 50 ms the system mask should be 0x0005
    And units 1 and 3 should be in "RUNNING" state
    And units 2 and 4 should be in "STOPPED" state

  @ac-4 @trace:S-4.1 @conflict
  Scenario: Concurrent stop command during coordinated start is rejected
    Given all units are in normal operating condition
    When a coordinated start is requested for units 1-4
    And a stop command is issued for unit 1 during coordination
    Then the master should reject unsafe transitions
    And within 100 ms all units 1-4 should be in "RUNNING" state

  @ac-4 @trace:S-4.1 @conflict @fault-gating
  Scenario: Faulted unit blocks coordinated start per safety policy
    Given unit 3 is in FAULT condition
    When a coordinated start is requested for units 1-4
    Then the coordinated start should be rejected
    And the master coordination state should be FAULT_ISOLATION
    And all units should be in safe state

  @ac-4 @trace:S-4.1 @conflict
  Scenario: Emergency stop overrides any in-flight coordination
    Given a coordinated start is in progress for units 1-4
    When an emergency stop is issued
    Then the system should enter emergency stop mode
    And all units should be in safe state

  @ac-3 @trace:S-4.1 @sync
  Scenario: Coordinated stop brings all active units down together
    Given units 1-4 are RUNNING under coordinated control
    When a coordinated stop is requested for units 1-4
    Then within 100 ms the system count should be 0
    And all units should be in stopped state
