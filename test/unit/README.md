# Unit Tests for Multi-Sonicator IO

## Overview

Unit tests for Multi-Sonicator IO are located in the `test/unit` directory.

## Unit Testing Framework Defined

- These tests follow completely and explicitly the procedure defined in the docs/sop/sw-testing-standard.md file.
- Unit tests are meant to provide a fast and reliable way to test behavior of the code at a functional level.
- The tests should be written so that they can test the code in isolation from other components.
- Mocks and stubs should be used to replace the other components and simulate their behavior.
- The tests should be written so that the state of the code (databases / agents / etc. ) is the same after the test as it was before (clean up).
- The tests should be written so that they are easy to read and understand.
- The tests should achieve 85% code coverage for the codebase.  
- The unit tests should strive to achieve 100% coverage with well-written tests that are verifying actual functionality of the small slices of code.
- The unit tests should be run as part of the CI pipeline and act as the first gate. **1 unit test failure == 1 pipeline failure!**
- Parameterized tests should be used to test many scenarios to ensure good branch and edge case coverage.  **Test as much as we can!**

## Unit Testing Tech-Stack Defined (Per Embedded/Hardware Project Standards)

- The unit tests are located in the `test/unit` directory.
- The unit tests are named according to the `test_{component}_{scenario}` convention.
- The unit tests are run using the `make test-unit` command.
- The unit tests use the **Unity Test Framework** and are written in **C/C++**.
- The unit tests are run using the **PlatformIO** framework for embedded C/C++ testing.
- This follows the organizational standard for embedded/hardware projects as defined in sw-testing-standard.md.
