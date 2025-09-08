# AI Coding Agent Instructionsfor Multi-Sonicator-IO

## Purpose

Ensure Auggie (Augment Agent) consistently follows our house rules and repository standards for the multi-sonicator-io project.

## Source of Truth Hierarchy

1. **Primary standards**: `docs/standards/` (single source of truth)
2. **Architecture docs**: `docs/architecture/` and PRD documents
3. **Legacy standards**: `.bmad-core/*` (only when not in conflict)

> **Note**: If standards are duplicated elsewhere, treat `docs/standards/` as canonical. Flag any divergence and propose sync PRs.

## Decision Hierarchy (highest to lowest priority)

1. This instructions file (`auggie.md`)
2. `docs/standards/*` in this repo
3. `docs/architecture/*` and PRD docs
4. `.bmad-core/*` (only when not conflicting)

## Coding & Architecture Standards

### Code Quality

- **Modular Design**: Write modular, object-oriented code with single responsibility per class/function
- **DRY Principle**: Avoid duplication; centralize shared logic in reusable modules/services
- **Single Source of Truth**: Maintain one authoritative source for configuration, constants, and policies
- **Documentation**: Use doxygen-style comments for public interfaces and important modules
- **Architecture Changes**: Before making architecture-level changes (new modules/services, cross-cutting patterns), pause and request user approval with a brief plan/diagram

### Code Structure

```markdown
- One clear, distinct purpose per function
- Centralized services/modules for shared functionality
- Clean architecture with easily accessible components
- Reusable functions and classes
```

## Testing & Quality Assurance

### Definition of Done

- Unit Tests must be completed for all newly implemented features
- Unit Test Coverage must exceed 85% statement coverage
- Unit Test Coverage must be neatly displayed in the test report after running make test-unit
- The test report must follow the standards listed in `docs/standards/sw-testing-standard.md` and `docs/standards/release-report.md`
- The feature must be verified through Acceptance BDD Tests with scenarios that are neatly mapped to PRD Requirements.
- The Acceptance BDD Tests must be run through the make target: make test-acceptance
- The Acceptance BDD Test Report must follow the standards listed in `docs/standards/sw-testing-standard.md` and `docs/standards/release-report.md
- The new functionality must be triple-checked for any code duplication or integration issues with existing systems.
- Regression: All previously written tests should pass after the new functionality is implemented.
- Previously written tests that are failing after the new feature has been implemented should be elevated to Project Management's attention by creating a file in the repository called `REGRESSION-ISSUES.md` which should clearly outline which tests are now failing and why they are failing.  The file should also outline potential solutions to fix the issues with pro's and cons for each solution clearly listed.

### Testing Standards

- All tests should be run solely by make targets as defined in the Company's Software Testing Standards!
- Follow testing guidance in `docs/standards/sw-testing-standard.md`
- Enforce coverage thresholds as defined in canonical standards
- For conflicts between testing standards, prefer `docs/standards/` and report deltas
- **Verification Requirement**: For every code change, add/update tests until behavior is verified
- Prefer fast, deterministic tests
- Provide test plans and execution commands

### Quality Gates

- All code must be tested before delivery
- Never deliver untested functionality
- Always verify added code works through comprehensive testing
- Unit Test Coverage must exceed 85% for all newly implemented features

## Tooling & Safety

- Always use your task manager for all tasks - even when a single task is only required, use the task manager.
- When working, create a task list using your task manager for all tasks to be completed at all times. 
- Be consistent with updating your task list using your task manager tool. 
- The task manager should be utilized and kept up to date whether working on a single test or a large feature with many tasks.  Using the task manager helps increase visibility for project management and alignment between Agents and Project Managers.

### Dependency Management

- **Use Package Managers**: npm/yarn/pnpm, pip/poetry, cargo, go mod, etc.
- **Never Hand-Edit**: Do not manually edit lockfiles or package configuration files
- Let package managers handle version resolution and dependency conflicts

### Safety Practices

- Show diffs before writing files
- Ask permission before executing commands or modifying files
- Prefer the smallest, safest change that satisfies requirements
- Use appropriate tools for external resource access
- Use GitHub tool for all GitHub interactions

## Documentation & Process

### Documentation Updates

- Update implementation-plan documents when tasks complete
- Keep README/Usage examples current when CLI or developer UX changes
- Maintain doxygen comments for documentation generation

### Communication

- **Ask Questions**: Never hesitate to ask for clarification
- Ensure alignment on project vision and approach
- **Suggest Improvements**: Propose better solutions when identified
- **Include URLs**: When requesting items from humans, include step-by-step instruction URLs

## Decision Making

### When Unsure

- Ask for clarification with 1-3 options and a recommendation
- Ensure we're on the same page before proceeding
- Get architecture decisions approved before implementation

### Improvement Mindset

- Suggest more efficient, cheaper, or better solutions
- Engage in conversations about approaches
- Challenge approaches constructively when beneficial

## Project-Specific Guidelines -- Multi Sonicator IO (a.k.a Sonicator Multiplexer)

### Multi-Sonicator-IO Focus

- Always referenct `docs/planning/pin-matrix.md` as the single SOLE SOURCE OF TRUTH for the pin assignments and connections between the Arduino Test Wrapper and the ATMEGA32A Sonicator Multiplexer DUT.
- Understand the sonicator control and monitoring requirements
- Follow embedded systems best practices where applicable
- Maintain real-time performance considerations
- Ensure hardware interface reliability
- Web User Interface should be implemented for functionality and be as simple as possible while being an effective tool for testing the ATMEGA32A Sonicator Multiplexer DUT.

## Metadata

- **File**: `auggie.md` (repository root)
- **Created**: 2025-09-05
- **Last Updated**: 2025-09-05
- **Project**: Multi-Sonicator-IO
- **Repository**: cannasol-technologies/multi-sonicator-io

---

*This file serves as the primary instruction set for Auggie when working on the multi-sonicator-io project. All development work should align with these guidelines.*
