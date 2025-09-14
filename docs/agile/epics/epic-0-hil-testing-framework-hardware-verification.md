# Epic 0: HIL Testing Framework & Hardware Verification

**Epic ID**: E-0  
**Priority**: Critical  
**Owner**: Product Owner  
**Created**: 2025-09-10  
**Updated**: 2025-09-10  

## Status

NotStarted

## Epic Summary

Establish a comprehensive Hardware-in-the-Loop (HIL) testing framework to validate firmware behavior against actual hardware, ensuring reliable integration before deployment. This epic focuses on creating the foundational testing infrastructure, project structure, and hardware verification systems required for all subsequent development.

## Scope Definition

### EXACTLY What Will Be Implemented

- Complete project structure, CI/CD pipeline, and build system according to Company Standards
- Hardware-in-the-Loop (HIL) test harness with physical hardware integration
- BDD acceptance testing framework with HIL integration and Gherkin scenarios
- Hardware connection verification against approved pin matrix (config/hardware-config.yaml)
- Arduino Test Wrapper and ISP programming interface validation
- Sandbox CLI for manual HIL testing and debugging capabilities
- MODBUS communication layer validation with physical hardware
- Web User Interface for signals and MODBUS register verification
- Web UI continuous manual configuration and monitoring of single Sonicator Module
- Web UI acceptance testing interface with individual/tagged/complete BDD scenario execution
- Complete acceptance test suite execution on physical hardware
- 85% unit test coverage achievement and validation
- Comprehensive Doxygen documentation generation and review

### EXACTLY What Will NOT Be Implemented

- Multi-sonicator coordination logic (Epic 1+)
- Production deployment automation beyond CI/CD framework
- Advanced analytics or data logging features
- Complex safety features beyond basic hardware verification
- Performance optimization beyond functional validation

## Business Value

**Primary Value**: Establishes reliable testing infrastructure ensuring hardware-firmware integration quality
**Secondary Value**: Enables continuous validation and reduces integration risks
**Risk Mitigation**: Validates all hardware interfaces and communication protocols before feature development

## User Stories

1. **Story 0.1**: Project Structure & CI/CD Pipeline - Standard build system and automation
2. **Story 0.2**: HIL Test Harness Setup - Physical hardware testing infrastructure
3. **Story 0.3**: BDD Acceptance Testing Framework - Behavior-driven testing with HIL integration
4. **Story 0.4**: Hardware Connection Verification - Pin matrix validation and interface testing
5. **Story 0.5**: Arduino Test Wrapper Validation - ISP programming and test interface
6. **Story 0.6**: Sandbox CLI Implementation - Manual testing and debugging tools
7. **Story 0.7**: MODBUS Hardware Validation - Communication layer verification
8. **Story 0.8**: Web UI Testing Interface - Signal verification and acceptance testing UI
9. **Story 0.9**: Test Coverage & Documentation - Quality assurance and documentation

## Acceptance Criteria

1. **Project Structure**: Complete build system following company standards with CI/CD automation
2. **HIL Framework**: Physical hardware testing harness operational with automated test execution
3. **BDD Testing**: Comprehensive acceptance tests with Gherkin scenarios covering all hardware interfaces
4. **Hardware Verification**: All pin assignments validated against config/hardware-config.yaml
5. **Arduino Interface**: ISP programming and test wrapper fully functional
6. **Sandbox Tools**: CLI interface enables manual testing and debugging of all hardware functions
7. **MODBUS Validation**: Communication layer verified with physical hardware integration
8. **Web UI Testing**: Interface provides signal verification and BDD scenario execution capabilities
9. **Quality Gates**: ≥85% unit test coverage, all acceptance tests passing on hardware
10. **Documentation**: Complete Doxygen documentation generated and reviewed

## Dependencies

### Prerequisites

- Arduino Uno R4 WiFi configured as ISP programmer and test wrapper
- ATmega32A Sonicator Multiplexer DUT hardware assembled and powered
- CT2000 sonicator unit(s) for physical testing
- Hardware configuration documented in config/hardware-config.yaml (SOLE SOURCE OF TRUTH)
- Company standards documentation loaded (.axovia-flow/sop/)

### Enables

- Epic 1: Foundational Control & Communication
- Epic 2: Multi-Sonicator Coordination  
- Epic 3: Web UI Development
- All subsequent feature development with validated testing infrastructure

## Risks & Mitigations

**Risk**: Hardware setup complexity may delay testing framework development
**Mitigation**: Incremental HIL setup with simulation fallbacks and comprehensive documentation

**Risk**: BDD framework integration complexity with physical hardware
**Mitigation**: Start with basic scenarios, expand incrementally with clear test isolation

**Risk**: Pin matrix changes affecting multiple hardware interfaces
**Mitigation**: config/hardware-config.yaml as SOLE SOURCE OF TRUTH, automated validation

**Risk**: CI/CD pipeline complexity with hardware dependencies
**Mitigation**: Hybrid approach with software-only CI and scheduled hardware validation

## Definition of Done

- [ ] All 9 user stories completed and accepted
- [ ] ≥85% unit test coverage achieved and validated
- [ ] All HIL acceptance tests passing on physical hardware
- [ ] Hardware connections verified against pin matrix
- [ ] Arduino Test Wrapper and ISP interface operational
- [ ] Sandbox CLI functional for manual testing
- [ ] MODBUS communication validated with hardware
- [ ] Web UI testing interface operational
- [ ] BDD scenarios executable individually, by tag, and complete suite
- [ ] CI/CD pipeline operational following company standards
- [ ] Doxygen documentation complete and reviewed
- [ ] Epic retrospective conducted

## Feature Requests (Gating Features)

1. **FEATURE: CI/CD Pipeline Automation and SOP Release Adherence**
2. **FEATURE: Standard of Procedures Test Framework Adherence**  
3. **FEATURE: Web-UI Sandbox Mode**
4. **FEATURE: Web-UI Acceptance Testing Display**
5. **FEATURE: HIL Testing Framework**

## Change Log

| Date | Version | Description | Author |
|------|---------|-------------|--------|
| 2025-09-10 | 1.0 | Created epic following Axovia Flow standards for HIL testing framework | Project Manager |
