# Multi-Sonicator CI/CD Pipeline

This directory contains the CI/CD pipeline configuration and supporting scripts for the Multi-Sonicator I/O Controller project.

## Overview

The CI/CD pipeline provides:
- Automated testing on every commit and pull request
- Hardware simulation for testing without physical hardware
- Build artifact generation and versioning
- Release management with GitHub releases
- Comprehensive error reporting and debugging information

## Workflows

### Main CI Pipeline (`.github/workflows/ci.yml`)
- **Triggers**: Push to main/develop, pull requests, manual dispatch
- **Jobs**:
  - `detect-hardware`: Checks if physical hardware is available
  - `unit-tests`: Runs PlatformIO unit tests
  - `integration-tests`: Runs integration tests (hardware or simulated)
  - `build-firmware`: Builds firmware for development and production
  - `stage-release`: Stages builds for release (main branch only)

### Release Management (`.github/workflows/release.yml`)
- **Triggers**: Git tags (v*), manual dispatch
- **Features**:
  - Builds production firmware
  - Generates release documentation
  - Creates GitHub releases with artifacts
  - Automated changelog generation

### Nightly Testing (`.github/workflows/nightly.yml`)
- **Triggers**: Daily at 2 AM UTC, manual dispatch
- **Features**:
  - Extended test suites
  - Memory usage analysis
  - Performance benchmarks
  - Comprehensive reporting

## Hardware Simulation

The pipeline includes a hardware simulation framework for testing without physical hardware:

### Virtual Sonicator (`test/simulation/virtual_sonicator.cpp`)
- Simulates CT2000 sonicator behavior
- Realistic frequency and power responses
- Overload condition simulation
- Fault injection for edge case testing

### Hardware Detection (`scripts/detect_hardware.py`)
- Automatically detects available hardware
- Falls back to simulation when hardware is unavailable
- Supports Arduino programmers and target hardware

## Scripts

### CI Scripts (`scripts/ci/`)
- `setup_environment.sh`: Sets up the CI build environment
- `generate_reports.py`: Generates HTML test reports and build summaries

### Release Scripts (`scripts/release/`)
- `version_bump.py`: Handles semantic versioning
- `changelog_generator.py`: Generates release changelogs (TODO)

## Error Reporting

The pipeline provides comprehensive error reporting:

### Test Failures
- JUnit XML output with detailed failure information
- HTML reports with test results and timing
- Failure context and debugging suggestions

### Build Failures
- Detailed compilation output
- Memory usage analysis
- Size optimization recommendations

### Example Error Output
```
❌ TEST FAILED: test_sonicator_amplitude_control

File: test/unit/test_sonicator.cpp:45
Function: test_amplitude_validation()

Expected: Amplitude = 50%
Actual: Amplitude = 0%

Possible causes:
1. PWM initialization failed - Check Timer2 configuration
2. Op-amp circuit not functioning - Verify 0-10V scaling
3. MODBUS register mapping incorrect - Check REG_SONICATOR_1_AMPLITUDE

Debug steps:
1. Run: pio test -e test_desktop -f test_sonicator --verbose
2. Check hardware connections on pins PB3 (PWM output)
3. Verify register 40001 is properly mapped

Related documentation: docs/troubleshooting.md#amplitude-control
```

## Artifacts

### Build Artifacts
- `firmware.hex`: Production firmware binary
- `manifest.json`: Build metadata and configuration
- `reports/`: Test reports and build summaries

### Release Artifacts
- `multi-sonicator-vX.X.X.tar.gz`: Complete release package
- Release notes and changelog
- Documentation updates

## Configuration

### Environment Variables
- `PLATFORMIO_CORE_DIR`: PlatformIO installation directory
- `ARTIFACT_NAME`: Base name for build artifacts

### Secrets (if needed)
- `GITHUB_TOKEN`: Automatically provided by GitHub Actions
- `AZURE_CREDENTIALS`: For Azure DevOps integration (future)

## Usage

### Running Tests Locally
```bash
# Run unit tests
pio test -e test_desktop

# Run with hardware simulation
pio test -e test_desktop --filter test_virtual_sonicator

# Generate test reports
python scripts/ci/generate_reports.py --test-results test_results
```

### Building Firmware
```bash
# Development build
pio run -e development

# Production build
pio run -e production

# Generate build manifest
python scripts/release/version_bump.py --current
```

### Release Process
1. Create a git tag: `git tag v1.0.0`
2. Push tag: `git push origin v1.0.0`
3. Release workflow automatically creates GitHub release
4. Artifacts are uploaded and made available

## Future Enhancements

### Planned Features
- [ ] Azure DevOps integration for release approval
- [ ] Slack/Teams notifications for build results
- [ ] Advanced hardware-in-the-loop testing
- [ ] Performance regression testing
- [ ] Security scanning for firmware
- [ ] Automated rollback procedures

### Performance Targets
- Pipeline execution time: < 10 minutes
- Test coverage: > 80%
- Build success rate: > 95%

## Troubleshooting

### Common Issues
1. **PlatformIO platform installation fails**: Usually network connectivity
2. **Tests timeout**: Increase timeout values in workflow
3. **Build artifacts missing**: Check build output for compilation errors
4. **Hardware detection fails**: Verify hardware connections and drivers

### Debug Mode
Enable verbose output by adding `--verbose` to PlatformIO commands or setting debug environment variables.

## Contributing

When adding new tests or build steps:
1. Follow the existing patterns in workflows
2. Add appropriate error handling and reporting
3. Update documentation and comments
4. Test changes with both hardware and simulation
5. Ensure artifacts are properly uploaded

## Support

For issues with the CI/CD pipeline:
1. Check the GitHub Actions logs for detailed error messages
2. Review the generated test reports in artifacts
3. Use the issue template for bug reports
4. Contact the development team for assistance