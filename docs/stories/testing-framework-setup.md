# Story: Robust Multi-Sonicator Test Framework Setup — Unit & Acceptance Alignment

**Date:** 2025-09-04  
**Author:** Scrum Master (Bob)  
**Status:** Draft

---

## Background

To guarantee product robustness and traceability, our test infrastructure must support both modular unit testing and hardware-in-the-loop (HIL) acceptance testing. The folder structure and configuration must align with current project conventions and PRD traceability.

---

## Unit Test Framework

- **Execution:** All unit tests are executed via `make test-unit`.
- **Coverage:** Achieve ≥90% code coverage across the entire source code base.
- **Environment:** Tests run in the Unity Native Environment.
- **Structure:**
  - Modular folder structure:

    ```markdown
    test/
      unit/
        <module-name-1>/
        <module-name-2>/
        unity_config.h
        ...
    ```

  - Each module is independently testable and can be run in isolation or as part of the full suite.
- **Reporting:** Every test run generates a coverage report that is consumable by our functionality reports dashboard as outlined in our release standards.

---

## Acceptance Test Framework

- **Execution:** Acceptance tests are run via `make test-acceptance`.
- **Scenario Format:** Use BDD scenarios for all acceptance tests.
- **Traceability:** Scenarios are mapped directly to PRD requirements via a single source of truth document.
- **Automated Flow:**
  1. Auto-detect Arduino connection port and verify if the Arduino ISP Sketch is loaded.
  2. Load the sketch if not present.
  3. Build the latest ATMEGA32A source code.
  4. Upload the build to the ATMEGA32A chip using Arduino as ISP.
  5. Upload the Arduino harness (`#file:arduino_harness`) to the Arduino.
  6. Run acceptance tests by communicating with the Arduino harness, interacting with the ATMEGA32A chip per pin assignments in `#file:pin-matrix.md`.
- **Configuration:** Use `#file:hil_config.yaml` as the sole source of truth for all test configuration.
- **Scenario Tagging:** Acceptance test scenarios are tagged per feature and can be executed individually or as a complete suite.
- **Structure:**
  - Modular folder structure:

    ```markdown
    test/
      acceptance/
        features/
        steps/
        hil-framework/          # HIL Framework supporting acceptance tests
            arduino_harness/    # Arduino Test Wrapper/Harness firmware
        logs/
        sketches/
        environment.py
        conftest.py
        ...
    ```

---

## Acceptance Criteria

- The unit test framework is fully functioning using Unity Native tests with a modular file structure
- The acceptance test framework is fully functioning using Behave BDD tests with a modular file structure
- The acceptance tests are easily executable using the make target `make test-acceptance`
- The unit tests are easily executable using the make target `make test-unit`
- The acceptance test make target automatically detects and configures the HIL interface
- The unit test make target automatically runs all unit tests with coverage reporting
- The acceptance test scenarios are clearly defined and separated by feature tags
- The acceptance tests can be run per feature or all together
- Both frameworks follow the specified folder structure
- All requirements above are implemented and verified
- Coverage and traceability are demonstrable via reports and mapping documents
- All flows are automated and require minimal manual intervention
- Acceptance test scenarios have been written for all requirements listed in the PRD
- Acceptance test scenarios that are written for unimplemented functionality are skipped until the functionality has been implemented

---

## QA Results

**Reviewer:** Quinn — Test Architect  
**Date:** 2025-09-04

- **Gate Decision:** PASS
- **Rationale:**
  - Requirements are explicit, traceable, and mapped to PRD.
  - Folder structure and automation flows are well-defined and align with project conventions.
  - Coverage and reporting requirements are clear and measurable (≥90% coverage, coverage reports).
  - Acceptance test automation flow is robust, minimizing manual intervention and ensuring hardware-in-the-loop traceability.
  - All referenced configuration and mapping files are present or planned, supporting testability and maintainability.
- **Risk Profile:**
  - Technical risk is low due to modular structure and automation.
  - Traceability risk is mitigated by mapping scenarios to PRD and using a single source of truth for configuration.
  - Minor risk: Ensure ongoing maintenance of mapping documents and configuration files as requirements evolve.
- **Testability Assessment:**
  - Controllability: High (automated flows, modular test execution).
  - Observability: High (coverage reports, scenario tagging, reporting).
  - Debuggability: Sufficient (modular isolation, clear automation steps).
- **Improvement Suggestions:**
  - Consider periodic review of mapping documents to ensure continued PRD alignment.
  - Automate validation of configuration file integrity (e.g., hil_config.yaml) as part of CI.
- **Quality Gate:** PASS — Ready for implementation and further QA validation during development.

---

## Story Draft Checklist (Scrum Master Review)

- [x] Title is clear and actionable
- [x] Background provides context and rationale
- [x] All requirements are listed and traceable to PRD
- [x] Folder structure and conventions are specified
- [x] Execution flows are described for both unit and acceptance tests
- [x] Automation requirements are explicit
- [x] Coverage and reporting requirements are defined
- [x] Acceptance criteria are clear and testable
- [x] All referenced files (e.g., pin-matrix.md, hil_config.yaml) exist or are planned
- [x] Status is set to Draft or Ready

---

## Implementation Checklist

### Unit Test Framework

- [ ] Create modular folder structure under `test/unit/` for each source module
- [ ] Configure Unity Native Environment for unit tests
- [ ] Implement `make test-unit` target to run all unit tests
- [ ] Integrate coverage reporting (≥90%) and output to functionality reports dashboard
- [ ] Ensure each module is independently testable and can run in isolation

### Acceptance Test Framework

- [ ] Create modular folder structure for acceptance tests using BDD scenarios
- [ ] Implement `make test-acceptance` target to run all acceptance tests
- [ ] Develop automated flow:
  - [ ] Auto-detect Arduino connection port
  - [ ] Verify/load Arduino ISP Sketch
  - [ ] Build and upload ATMEGA32A source code
  - [ ] Upload Arduino harness
  - [ ] Run acceptance tests via Arduino harness, interacting with ATMEGA32A per pin assignments
- [ ] Use `hil_config.yaml` as the sole source of truth for test configuration
- [ ] Tag acceptance test scenarios per feature for selective execution
- [ ] Map scenarios to PRD requirements via a single source of truth document

### Reporting & Traceability

- [ ] Ensure coverage and traceability are demonstrable via reports and mapping documents
- [ ] Automate skipping of acceptance scenarios for unimplemented features

### Automation & CI

- [ ] Automate validation of configuration file integrity (e.g., `hil_config.yaml`) in CI
- [ ] Minimize manual intervention in all test flows

### Documentation & Maintenance

- [ ] Maintain mapping documents and configuration files as requirements evolve
- [ ] Periodically review mapping documents for PRD alignment
