# Story 0.3: Testing Infrastructure & Make Targets

**Story ID**: S-0.3
**Feature**: F-001: CI/CD Pipeline & Release Standard Adherence
**Priority**: Critical
**Owner**: Product Owner (Sarah)
**Created**: 2025-09-10
**Updated**: 2025-09-10

## Status

Draft

## Story

**As a** firmware developer,
**I want** a comprehensive three-stage testing infrastructure with standardized Make targets,
**so that** I can ensure firmware quality through unit tests, acceptance tests, and integration validation with automated coverage enforcement.

## Feature Context

**Feature F-001: CI/CD Pipeline & Release Standard Adherence**

- Goal: Establish comprehensive CI/CD pipeline and ensure complete adherence to company release standards
- Creates foundational infrastructure for reliable firmware development and quality assurance

## Story Context

**Three-Stage Testing Pyramid:**

- **Unit Tests**: Unity Test Framework with ≥90% coverage requirement
- **Acceptance Tests**: BDD scenarios with pytest + Behave
- **Integration Tests**: Hardware-in-the-Loop (HIL) validation
- Standardized Make targets for consistent testing workflows

**Integration Points:**

- Unity Test Framework for embedded unit testing
- pytest + Behave for BDD acceptance testing
- HIL testing framework for hardware validation
- Coverage reporting and enforcement

## Acceptance Criteria

**Testing Framework Requirements:**

1. Unity Test Framework integrated with ≥90% code coverage enforcement per company standards
2. pytest + Behave framework configured for BDD acceptance testing scenarios
3. HIL testing infrastructure prepared for hardware validation integration
4. Coverage reports generated in HTML and XML formats with detailed metrics

**Make Target Implementation:**

5. `make test-unit` executes Unity tests with coverage analysis and reporting
6. `make test-acceptance` runs BDD acceptance tests via pytest + Behave
7. `make test-integration` executes HIL hardware validation tests (when hardware available)
8. `make test` runs complete three-stage testing pyramid with consolidated reporting

**Quality Enforcement:**

9. Unit test coverage below 90% fails the testing pipeline with clear reporting
10. Test failures provide detailed diagnostic information and debugging guidance
11. Test execution timing and resource utilization reported for performance monitoring
12. All tests execute in clean, isolated environments without cross-test dependencies

## Technical Implementation Details

### Three-Stage Testing Architecture

```bash
# Testing Make Targets
make test-unit          # Unity unit tests with coverage (≥90% required)
make test-acceptance    # BDD acceptance tests (pytest + Behave)
make test-integration   # HIL hardware validation tests
make test               # Complete testing pipeline
make ci-local          # Local CI simulation with all tests
```

### Unit Testing Infrastructure

```cpp
// test/unit/ structure
test/unit/
├── test_main.cpp           # Unity test runner
├── test_hal_gpio.cpp       # HAL GPIO unit tests
├── test_modbus_rtu.cpp     # MODBUS RTU unit tests
├── test_sonicator_ctrl.cpp # Sonicator control unit tests
└── unity_config.h          # Unity configuration
```

### BDD Acceptance Testing

```gherkin
# test/acceptance/features/
Feature: Sonicator Control
  Scenario: Basic sonicator startup
    Given the system is initialized
    When sonicator 1 is commanded to start
    Then sonicator 1 status indicates running
```

### Coverage Requirements

- **Minimum Coverage**: 90% statement coverage (company standard)
- **Coverage Reporting**: HTML dashboard and XML for CI integration
- **Coverage Exclusions**: Documented exceptions for hardware-dependent code
- **Quality Gates**: Coverage below threshold fails pipeline

## Definition of Done

- [ ] Unity Test Framework configured with 90% coverage enforcement
- [ ] pytest + Behave framework operational for acceptance testing
- [ ] HIL testing infrastructure prepared and validated
- [ ] All required Make targets functional and tested
- [ ] Coverage reports generated in HTML and XML formats
- [ ] Test failures provide clear diagnostic information
- [ ] Testing pipeline integrated with CI/CD workflow
- [ ] Test execution performance meets requirements (<10 minutes total)
- [ ] Clean test environments with no cross-test dependencies
- [ ] Ready for documentation and release standards (Story 0.4)

## Tasks / Subtasks

- [ ] **Task 1: Unity Test Framework Setup** (AC: 1, 4, 9)
  - [ ] Configure Unity Test Framework with ATmega32A embedded target
  - [ ] Set up coverage analysis with 90% threshold enforcement
  - [ ] Implement coverage reporting in HTML and XML formats

- [ ] **Task 2: BDD Acceptance Testing Framework** (AC: 2)
  - [ ] Configure pytest + Behave for BDD scenario execution
  - [ ] Create initial acceptance test structure and examples
  - [ ] Integrate acceptance testing with project build system

- [ ] **Task 3: HIL Testing Preparation** (AC: 3)
  - [ ] Prepare HIL testing infrastructure integration points
  - [ ] Configure hardware validation test framework
  - [ ] Set up HIL testing environment and dependencies

- [ ] **Task 4: Make Target Implementation** (AC: 5, 6, 7, 8)
  - [ ] Implement `make test-unit` with coverage analysis
  - [ ] Implement `make test-acceptance` with BDD execution
  - [ ] Implement `make test-integration` for HIL validation
  - [ ] Implement `make test` for complete testing pipeline

- [ ] **Task 5: Quality Gates and Reporting** (AC: 10, 11, 12)
  - [ ] Configure detailed test failure diagnostics and reporting
  - [ ] Implement test execution timing and performance monitoring
  - [ ] Ensure clean, isolated test execution environments

- [ ] **Task 6: CI Integration and Local Simulation** (AC: 12)
  - [ ] Integrate testing pipeline with CI/CD workflow
  - [ ] Implement `make ci-local` for local CI simulation
  - [ ] Validate CI testing behavior matches local execution

## Dev Notes

### Testing Infrastructure Architecture

**Unit Testing Framework**: Unity Test Framework for C/C++ embedded systems
**Location**: `test/unit/`

**BDD Framework**: pytest + Behave for acceptance criteria validation
**Location**: `test/acceptance/`

**HIL Framework**: Hardware-in-the-Loop testing for physical validation
**Location**: `test/integration/`

**Coverage Tools**: gcov + lcov for coverage analysis and reporting

### Testing Standards

**Test File Location**: 
- Unit: `test/unit/test_*.cpp`
- Acceptance: `test/acceptance/features/*.feature`
- Integration: `test/integration/test_hil_*.py`

**Testing Framework**: Unity Test Framework with company standards compliance

**Testing Requirements:**
- Unit tests: ≥90% statement coverage (company standard)
- Acceptance tests: BDD scenarios mapping to PRD requirements
- Integration tests: Hardware validation via HIL framework
- Performance requirements: <10 minutes total execution time

**Specific Testing Patterns:**
- Embedded unit testing with mocks for hardware dependencies
- BDD scenario-driven acceptance testing
- Hardware-in-the-loop integration validation
- Isolated test execution with clean environments

### Coverage Standards

**Coverage Targets:**
- **Statement Coverage**: ≥90% (company standard)
- **Branch Coverage**: ≥85% (recommended)
- **Function Coverage**: 100% (public functions)

**Coverage Exclusions:**
- Hardware initialization sequences (documented)
- Debug-only code paths
- Error handling for impossible conditions (documented)

## Dependencies

**Prerequisites:**
- Story 0.1: Project Structure & Standards Compliance completed
- Story 0.2: CI/CD Pipeline Implementation completed
- Unity Test Framework available
- pytest + Behave framework installed
- HIL testing hardware access prepared

**Enables Next Stories:**
- Story 0.4: Documentation & Release Standards

## Change Log

| Date | Version | Description | Author |
|------|---------|-------------|---------|
| 2025-09-10 | 1.0 | Initial story creation for F-001 | Product Owner (Sarah) |
