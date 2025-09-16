---
story_id: 1.5
epic_id: 2
title: Refactor to Object-Oriented Structure
status: "To Do"
priority: High
creation_date: 2025-09-16
---

# Story 1.5: Refactor to Object-Oriented Structure

## User Story

As a developer,
I want to refactor the existing procedural code into an object-oriented structure,
So that the codebase is more modular, maintainable, and aligned with the project's architectural goals.

## Story Context

**Existing System Integration:**

- **Integrates with:** The main application logic, hardware abstraction layer.
- **Technology:** C++ on ATmega32A (Arduino Framework).
- **Follows pattern:** This story will establish the primary OOP pattern for sonicator control.
- **Touch points:** `main.cpp`, and any files responsible for direct sonicator control and monitoring.

## Acceptance Criteria

**Functional Requirements:**

1.  A `CT2000Sonicator` abstract class (or equivalent) shall be created to define the contract for all sonicator objects.
2.  A concrete `CT2000Sonicator` class shall be implemented, inheriting from `CT2000Sonicator`.
3.  All existing functionality for controlling a sonicator (start, stop, set amplitude, reset) must be encapsulated within the `CT2000Sonicator` class.
4.  All existing telemetry for a sonicator (power, frequency, status) must be encapsulated within the `CT2000Sonicator` class.

**Integration Requirements:**

5.  The main application loop in `main.cpp` shall be updated to use an array of `CT2000Sonicator` pointers to manage the four sonicator units.
6.  Existing functionality for all four sonicators must continue to work as before the refactoring.
7.  The MODBUS communication layer must correctly interact with the new `CT2000Sonicator` objects to get and set data.

**Quality Requirements:**

8.  The new classes shall be covered by unit tests, maintaining or increasing the project's overall test coverage.
9.  The public interface of the new classes shall be documented using Doxygen-style comments.
10. The refactoring must not introduce any memory leaks or performance regressions.

## Technical Notes

- **Integration Approach:** The existing procedural code for managing sonicators will be moved into the new `CT2000Sonicator` class. The `main.cpp` file will be updated to instantiate and manage four instances of this class.
- **Existing Pattern Reference:** This will establish a new, foundational OOP pattern for the project, as described in PRD sections NFR3 and 3.4.
- **Key Constraints:** The solution must be mindful of the memory and processing limitations of the ATmega32A microcontroller.

## Definition of Done

- [ ] `CT2000Sonicator` and `CT2000Sonicator` classes are implemented.
- [ ] All sonicator control and telemetry logic is encapsulated in the `CT2000Sonicator` class.
- [ ] `main.cpp` is refactored to use the new classes.
- [ ] All functional and integration requirements are met and verified.
- [ ] Unit tests for the new classes are written and passing.
- [ ] Doxygen documentation is created for the new classes.
- [ ] A code review of the refactored structure has been completed.
