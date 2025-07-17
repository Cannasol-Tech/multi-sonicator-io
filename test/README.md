# Test Directory

This directory contains unit tests for the Multi-Sonicator I/O Controller firmware.

## Test Framework

The project uses the Unity testing framework for embedded C++ unit tests, integrated with PlatformIO's test runner.

## Structure

- `test_main.cpp` - Main test runner
- `test_config/` - Configuration validation tests
- `test_sonicator/` - Sonicator interface tests
- `test_system/` - System integration tests

## Running Tests

```bash
# Run all tests
pio test

# Run specific test
pio test -e test_desktop -f test_config

# Run tests with verbose output
pio test -v
```

## Test Environments

- `test_desktop` - Native desktop tests for business logic
- `test_embedded` - Embedded target tests (requires hardware)