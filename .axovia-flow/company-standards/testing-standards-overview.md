# Software Testing Standards Overview

> **Note:** This document provides the organizational overview of testing standards across all project types.

## Purpose

This document serves as the central hub for all testing standards within the organization, ensuring consistent quality and structured practices across different project types. It provides the framework that applies to all BMad-Core workflows while directing teams to project-specific testing standards.

---

## Project-Specific Testing Standards

### By Project Type

| Project Type | Testing Standard Document | Primary Frameworks |
|--------------|---------------------------|-------------------|
| **Flutter Applications** | [flutter-testing-standard.md](flutter-testing-standard.md) | `flutter_test` + `mocktail` |
| **React UI Applications** | [react-testing-standard.md](react-testing-standard.md) | `Jest` + `React Testing Library` |
| **PlatformIO Embedded** | [platformio-testing-standard.md](platformio-testing-standard.md) | `Unity Test Framework` |

### Universal Principles

All project types must adhere to these core principles:

1. **Mocking Policy**: Mocks are reserved for unit testing only, unless explicitly approved
2. **Coverage Requirements**: Minimum coverage thresholds defined per project type
3. **Test Organization**: Structured test directories and naming conventions
4. **Quality Gates**: Automated validation before story completion
5. **BMad-Core Integration**: Compliance with agent workflows and tasks

---

## Universal Testing Strategy

### Three-Stage Foundation

All project types follow this foundational approach, adapted to their specific needs:

1. **Unit Testing**
   - **Focus**: Individual functions, modules, and components in isolation
   - **Coverage**: High coverage requirements (varies by project type)
   - **Mocking**: Permitted and encouraged for external dependencies

2. **Integration Testing**
   - **Focus**: System workflows and component interactions
   - **Coverage**: Critical paths and external integrations
   - **Mocking**: Generally prohibited (test real interactions)

3. **End-to-End Testing**
   - **Focus**: Complete user workflows and business scenarios
   - **Coverage**: Primary user journeys and acceptance criteria
   - **Mocking**: Prohibited (test complete system)

---

## Standardized Make Targets

All projects must implement standardized make targets for consistent execution:

### Required Targets (Project-Specific Implementation)

```bash
# Unit Testing
make test-unit
# - Project-specific unit test execution
# - Coverage reporting and validation
# - Framework varies by project type

# Integration Testing
make test-integration
# - Component and system integration testing
# - External dependency validation
# - Real environment testing

# End-to-End Testing
make test-e2e
# - Complete workflow validation
# - User acceptance scenario testing
# - Production-like environment testing

# Complete Test Suite
make test
# - Executes all test stages in sequence
# - Primary target for CI/CD pipelines
# - Equivalent to: make test-unit && make test-integration && make test-e2e
```

---

## Quality Gates

### Universal Requirements

All stories must pass these quality gates regardless of project type:

- [ ] Unit tests achieve project-specific coverage requirements
- [ ] Integration tests validate external system interactions
- [ ] End-to-end tests cover critical user workflows
- [ ] Test reports generated in standardized format
- [ ] No prohibited mocking in integration/e2e tests

### Project-Specific Gates

Additional quality gates are defined in each project-specific testing standard:

- **Flutter**: Widget tests, golden tests, cross-platform validation
- **React**: Component tests, accessibility tests, browser compatibility
- **PlatformIO**: Hardware-in-the-loop tests, emulation validation

---

## BMad-Core Integration

### Agent Workflow Integration

- **QA Agent**: Implements project-specific standards through specialized commands
- **Dev Agent**: Achieves coverage requirements before story completion
- **PM Agent**: Ensures acceptance criteria align with testing capabilities

### Task Integration

- **test-design.md**: Generates test scenarios following project-specific patterns
- **trace-requirements.md**: Maps tests to requirements across all project types
- **qa-gate.md**: Validates compliance with appropriate testing standard
- **review-story.md**: Comprehensive review including project-specific testing

---

## Implementation Guidance

### Choosing the Right Standard

1. **Identify Project Type**: Determine primary technology stack
2. **Select Testing Standard**: Use the appropriate project-specific document
3. **Follow Framework Requirements**: Implement required tools and patterns
4. **Validate Compliance**: Ensure quality gates are met

### Cross-Project Considerations

- **Monorepos**: May require multiple testing standards
- **Microservices**: Each service follows its appropriate standard
- **Full-Stack Projects**: Frontend and backend may use different standards

---

## Continuous Improvement

- **Quarterly Reviews**: Assess effectiveness of project-specific standards
- **Tool Evolution**: Update standards as frameworks and tools evolve
- **Best Practice Sharing**: Cross-pollinate learnings between project types
- **Industry Alignment**: Maintain compliance with relevant industry standards

---

## Quick Reference

### Getting Started

1. Identify your project type
2. Read the appropriate project-specific testing standard
3. Implement required make targets
4. Set up CI/CD integration
5. Begin following the prescribed testing patterns

### Support Resources

- Project-specific testing standard documents
- BMad-Core task documentation
- Framework-specific implementation guides
- Quality gate validation tools

---

✅ **This overview provides the organizational framework. Always refer to your project-specific testing standard for detailed implementation guidance.**
