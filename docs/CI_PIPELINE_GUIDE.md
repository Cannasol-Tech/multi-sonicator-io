# CI Pipeline Usage Guide

## Overview

The Multi-Sonicator I/O Controller project now has a solidified CI pipeline that focuses on **unit tests only** for fast feedback and hardware independence. This document explains how to use the new CI pipeline and when to run manual tests.

## Quick Start

### CI Pipeline (Automated)
```bash
# Run complete CI pipeline (unit tests only)
make ci
```

### Manual Testing (Release Preparation)
```bash
# Run complete test suite including acceptance tests
make test-acceptance    # Manual with hardware
make generate-complete-executive-report
```

## CI Pipeline Details

### What the CI Pipeline Does
1. **Configuration Validation**: Validates HIL configuration integrity
2. **Unit Tests**: Runs all unit tests with Unity Test Framework
3. **Coverage Validation**: Enforces 90% coverage requirement
4. **Executive Report**: Generates unit test executive report
5. **Coverage Report**: Generates coverage summary report

### What the CI Pipeline Generates
- `final/executive-report.json` - Machine-readable unit test results
- `final/executive-report.md` - Human-readable unit test summary
- `final/unit-test-summary.json` - Detailed unit test results
- `final/coverage-summary.json` - Coverage metrics
- `coverage/coverage.html` - HTML coverage report

### CI Pipeline Benefits
- ✅ **Fast Feedback**: Completes in ~2-3 minutes
- ✅ **Hardware Independent**: No physical hardware required
- ✅ **PR Validation**: Runs on every pull request
- ✅ **Coverage Enforcement**: Validates 90% unit test coverage
- ✅ **Standardized Reports**: Machine and human-readable formats

## Manual Testing (Release Process)

### When to Run Manual Tests
- Before creating releases
- When testing hardware-specific functionality
- For acceptance criteria validation
- For complete system validation

### Manual Testing Process
```bash
# 1. Ensure hardware is connected and configured
# 2. Run acceptance tests
make test-acceptance

# 3. Generate complete executive report
make generate-complete-executive-report

# 4. Review final/executive-report-complete.md for release readiness
```

### Manual Testing Outputs
- `final/acceptance-report.json` - Acceptance test results
- `final/executive-report-complete.json` - Combined unit + acceptance results
- `final/executive-report-complete.md` - Release readiness assessment

## Testing Strategy Summary

| Test Type | Execution | Frequency | Hardware Required | Purpose |
|-----------|-----------|-----------|------------------|---------|
| **Unit Tests** | Automated (CI) | Every PR/commit | No | Fast feedback, regression detection |
| **Acceptance Tests** | Manual | Pre-release | Yes | Feature validation, user scenarios |
| **Integration Tests** | Manual | Pre-release | Yes | System integration validation |
| **HIL Tests** | Manual | Pre-release | Yes | Hardware-specific validation |

## GitHub Workflow Integration

### PR Workflow
1. Developer pushes changes
2. GitHub Actions runs `make ci`
3. Reports uploaded as artifacts
4. Coverage and test results validated
5. PR can be merged if CI passes

### Release Workflow
1. Manual testing performed with hardware
2. Complete executive report generated
3. Release readiness assessed
4. GitHub release created with all reports

## Quality Gates

### CI Pipeline Quality Gates
- ✅ Unit test coverage ≥ 90%
- ✅ All unit tests pass
- ✅ Configuration validation passes
- ✅ Reports generated successfully

### Release Quality Gates
- ✅ CI pipeline passes
- ✅ Acceptance tests pass
- ✅ Integration tests pass
- ✅ HIL tests pass
- ✅ Overall test pass rate ≥ 95%

## Troubleshooting

### CI Pipeline Issues
```bash
# Check dependencies
make check-deps

# Run individual stages
make validate-config
make test-unit
make generate-executive-report
```

### Manual Testing Issues
```bash
# Check hardware connectivity
python3 scripts/detect_hardware.py

# Validate HIL configuration
make validate-config

# Run specific acceptance tests
cd test/acceptance && behave --tags="@smoke"
```

## Migration from Previous Approach

### What Changed
- ❌ **Removed**: Acceptance tests from CI pipeline
- ❌ **Removed**: Hardware dependency in CI
- ✅ **Added**: `make ci` target for CI pipeline
- ✅ **Added**: Unit test-only executive reports
- ✅ **Added**: Clear separation of CI vs manual testing

### What Stayed the Same
- Unit test framework (Unity)
- Acceptance test framework (Behave BDD)
- Coverage requirements (90%)
- Report formats and standards
- Make targets for manual testing

### Developer Impact
- **Faster CI**: No more waiting for hardware-dependent tests
- **Clear Process**: Obvious separation between CI and release testing
- **Better Reports**: Separate reports for CI and release processes
- **Same Tooling**: Existing make targets still work for manual testing

## Standards Compliance

This CI pipeline complies with:
- **Issue #18 Requirements**: All 8 requirements addressed
- **Software Testing Standard**: Three-stage testing approach
- **PlatformIO Testing Standard**: Unity framework with coverage
- **Cannasol Reports Standard**: Standardized JSON/MD formats

---

For questions or issues, refer to:
- `docs/standards/sw-testing-standard.md`
- `docs/standards/platformio-specific/platformio-testing-standard.md`
- `test/acceptance/README.md`
- `final/README.md`