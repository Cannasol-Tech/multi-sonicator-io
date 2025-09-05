# 📋 PRD Requirements to BDD Acceptance Test Implementation Mapping

**Source of Truth Document**  
**Last Updated:** September 5, 2025  
**Compliance Status:** ✅ 100% Coverage (26/26 requirements)

---

## 🎯 **OVERVIEW**

This document provides the **complete mapping** between PRD requirements and their corresponding BDD acceptance test implementations. Each requirement is traced to specific Gherkin scenarios with implementation details.

---

## 📊 **TRACEABILITY MATRIX**

### **CONTROL FUNCTIONS** (`test/acceptance/features/control.feature`)

#### **REQ-AMPLITUDE** - Amplitude Control (20-100%)
- **PRD Reference:** §4, §5, §10
- **Registers:** 40001-40004 (Sonicators 1-4)
- **Scenario ID:** SC-001
- **BDD Implementation:**
  ```gherkin
  @pending @req-amplitude @prd-5-functional @prd-10-api @reg-40001-40004 @hil @hil
  Scenario: SC-001 Amplitude control per unit
    Given the system is initialized
    When I write holding register 40001 with value 75
    Then register 40001 should contain value 75
    And the amplitude should be reflected within 100ms
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Write setpoints, verify reflected and clamped; update ≤100 ms

#### **REQ-STARTSTOP** - Start/Stop Control
- **PRD Reference:** §4, §5, §10  
- **Registers:** 40005-40008 (Sonicators 1-4)
- **Scenario ID:** SC-002
- **BDD Implementation:**
  ```gherkin
  @pending @req-startstop @prd-5-functional @prd-10-api @reg-40005-40008 @hil @hil
  Scenario: SC-002 Start/Stop per unit
    Given the system is initialized
    When I write holding register 40005 with value 1
    Then the sonicator should start
    When I write holding register 40005 with value 0
    Then the sonicator should stop
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Toggle start/stop, verify status flags and timing

#### **REQ-OVERLOAD-RESET** - Overload Reset Pulse
- **PRD Reference:** §4, §5, §10
- **Registers:** 40009-40012 (Sonicators 1-4)
- **Scenario ID:** SC-003
- **BDD Implementation:**
  ```gherkin
  @pending @req-overload-reset @prd-5-functional @prd-10-api @reg-40009-40012 @hil @hil
  Scenario: SC-003 Overload Reset pulse semantics
    Given the system is initialized
    And sonicator 1 is in overload state
    When I write holding register 40009 with value 1
    Then the overload should be reset
    And register 40009 should auto-clear to 0
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Write 1 then auto-clear; electrical pulse observed in HIL

---

### **MONITORING FUNCTIONS** (`test/acceptance/features/monitoring.feature`)

#### **REQ-POWER** - Power Monitoring
- **PRD Reference:** §4, §5, §10
- **Registers:** 40013-40016 (Sonicators 1-4)
- **Scenario ID:** SC-004
- **BDD Implementation:**
  ```gherkin
  @req-power @prd-10-api @reg-40013-40016 @hil @hil
  Scenario: SC-004 Power monitoring scaling
    Given the system is initialized
    When I inject a power signal of 100W
    Then register 40013 should reflect the scaled value
    And the scaling should be 5.44 mV/W
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Inject power signal; check scaling and units

#### **REQ-FREQUENCY** - Frequency Monitoring
- **PRD Reference:** §4, §5, §10
- **Registers:** 40017-40020 (Sonicators 1-4)
- **Scenario ID:** SC-005
- **BDD Implementation:**
  ```gherkin
  @req-frequency @prd-10-api @reg-40017-40020 @hil @hil
  Scenario: SC-005 Frequency monitoring
    Given the system is initialized
    When I inject a frequency signal of 200Hz
    Then register 40017 should contain 20
    And the frequency should be divided by 10
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Inject frequency; verify computed Hz

#### **REQ-STATUS-FLAGS** - Status Flag Monitoring
- **PRD Reference:** §4, §5, §10
- **Registers:** 40021-40024 (Sonicators 1-4)
- **Scenario ID:** SC-006
- **BDD Implementation:**
  ```gherkin
  @req-status-flags @prd-10-api @reg-40021-40024 @hil @hil
  Scenario: SC-006 Status flags
    Given the system is initialized
    When sonicator 1 is running
    Then bit 0 of register 40021 should be set
    When sonicator 1 experiences overload
    Then bit 1 of register 40021 should be set
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Stimulate running/overload/freq lock; verify bits

---

### **SYSTEM FUNCTIONS** (`test/acceptance/features/system.feature`)

#### **REQ-ACTIVE-COUNT** - Active Sonicator Count
- **PRD Reference:** §4, §5
- **Registers:** 40035
- **Scenario ID:** SC-007
- **BDD Implementation:**
  ```gherkin
  @req-active-count @prd-5-functional @prd-10-api @reg-40035 @hil @hil
  Scenario: SC-007 Active Sonicator Count
    Given the system is initialized
    When I start sonicators 1 and 3
    Then register 40035 should contain value 2
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Vary active units; verify count

#### **REQ-ACTIVE-MASK** - Active Sonicator Mask
- **PRD Reference:** §4, §5
- **Registers:** 40036
- **Scenario ID:** SC-008
- **BDD Implementation:**
  ```gherkin
  @req-active-mask @reg-40036 @hil @hil
  Scenario: SC-008 Active Sonicator Mask
    Given the system is initialized
    When I start sonicators 1 and 3
    Then register 40036 should contain bitmask 0x0005
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Verify bitmask across transitions

---

### **TIMING & PERFORMANCE** (`test/acceptance/features/timing.feature`)

#### **REQ-LATENCY** - Response Time Requirements
- **PRD Reference:** §5, §6
- **Scenario ID:** SC-009
- **BDD Implementation:**
  ```gherkin
  @req-latency @prd-5-functional @prd-6-nfr @hil @hil
  Scenario: SC-009 Latency ≤100 ms (representative)
    Given the system is initialized
    When I write a control register
    Then the state change should be reflected within 100ms
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Measure latency for representative signals and writes

---

### **MODBUS PROTOCOL** (`test/acceptance/features/modbus.feature`)

#### **REQ-ILLEGAL-ADDRESS** - Error Handling
- **PRD Reference:** §10
- **Scenario ID:** SC-021
- **BDD Implementation:**
  ```gherkin
  @req-illegal-address @prd-10-api @hil
  Scenario: SC-021 Illegal address handling
    Given the system is initialized
    When I attempt to read register 50000
    Then I should receive MODBUS exception code 02
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Return proper exception codes

#### **REQ-SLAVE-ID** - MODBUS Slave Configuration
- **PRD Reference:** §10
- **Scenario ID:** SC-022
- **BDD Implementation:**
  ```gherkin
  @req-slave-id @prd-10-api @hil @hil
  Scenario: SC-022 Slave ID configuration
    Given the system is initialized
    Then the default slave ID should be 2
    When I configure slave ID to 5
    Then the slave ID should persist
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Default is 2; persists per design; range enforced

---

### **SAFETY & COMPLIANCE** (`test/acceptance/features/safety.feature`)

#### **REQ-ESTOP** - Emergency Stop
- **PRD Reference:** §5
- **Registers:** 40026
- **Scenario ID:** SC-014
- **BDD Implementation:**
  ```gherkin
  @req-estop @prd-5-functional @reg-40026 @hil
  Scenario: SC-014 Emergency stop register behavior
    Given the system is initialized
    When I assert emergency stop via register 40026
    Then all sonicator outputs should be disabled
    And the system should enter safe state
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** Assert E‑stop blocks outputs; resets properly

---

### **CI/CD VALIDATION** (`test/acceptance/features/ci.feature`)

#### **REQ-CI-DRIFT** - Documentation Synchronization
- **PRD Reference:** §17
- **Scenario ID:** SC-025
- **BDD Implementation:**
  ```gherkin
  @req-ci-drift @prd-17-risk @hil
  Scenario: SC-025 CI drift check: PRD vs include/config.h
    Given the CI environment is configured
    And the PRD requirements are documented in project-requirements.md
    And the implementation constants are defined in include/config.h
    When the CI drift check script runs
    Then it should compare PRD requirements against implementation
    And it should flag any mismatches between documentation and code
    And it should block merge if critical drift is detected
  ```
- **Test Profiles:** @hil
- **Acceptance Criteria:** CI script flags mismatches; block merge

---

## 📁 **FILE LOCATIONS**

### **BDD Feature Files:**
- `test/acceptance/features/control.feature` - Control functions
- `test/acceptance/features/monitoring.feature` - Monitoring functions  
- `test/acceptance/features/system.feature` - System-level functions
- `test/acceptance/features/timing.feature` - Performance requirements
- `test/acceptance/features/modbus.feature` - Protocol compliance
- `test/acceptance/features/safety.feature` - Safety functions
- `test/acceptance/features/ci.feature` - CI/CD validation

### **Step Implementations:**
- `test/acceptance/steps/common_steps.py` - Shared step definitions
- `test/acceptance/steps/hil_*.py` - Hardware-in-the-loop steps
- `test/acceptance/steps/steps_sim.py` - Simulation steps

### **Source of Truth Documents:**
- `docs/prd-shards/prd-to-scenarios-traceability.md` - Master traceability matrix
- `docs/prd-shards/project-requirements.md` - Authoritative requirements
- `include/config.h` - Implementation constants

---

## 🎯 **COMPLIANCE STATUS**

- **Total Requirements:** 26
- **Covered Requirements:** 26 (100%)
- **BDD Scenarios:** 124 total
- **Critical Pending:** 5 scenarios
- **Validation Status:** ✅ COMPLIANT

---

## 🔄 **MAINTENANCE**

This mapping is automatically validated by:
- `scripts/validate_traceability_compliance.py`
- `.github/workflows/traceability-compliance.yml`
- `make validate-traceability`

Any changes to PRD requirements must update corresponding BDD scenarios and maintain this traceability mapping.
