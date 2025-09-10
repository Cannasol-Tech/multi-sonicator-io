# Epic 1: Foundational Control & Communication

**Epic ID**: E-1  
**Priority**: Critical  
**Owner**: Product Owner  
**Created**: 2025-01-09  
**Updated**: 2025-01-09  

## Status

InProgress

## Epic Summary

Establish core firmware structure, basic sonicator control, and MODBUS communication layer for the Multi-Sonicator I/O Controller. This epic provides the foundational infrastructure required for all subsequent development.

## Scope Definition

### EXACTLY What Will Be Implemented

- Complete embedded project structure with PlatformIO build system
- Hardware Abstraction Layer (HAL) for ATmega32A peripherals
- MODBUS RTU slave communication interface (115200 baud, 8N1)
- Single sonicator control and monitoring capabilities
- Hardware-in-the-Loop (HIL) testing framework with BDD acceptance tests
- Pin assignments per docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)

### EXACTLY What Will NOT Be Implemented

- Multi-sonicator coordination (Epic 2)
- Advanced safety features beyond basic overload detection
- Web UI interface (Epic 3)
- Production deployment automation
- Advanced analytics or data logging

## Business Value

**Primary Value**: Enables basic sonicator control through standardized MODBUS interface
**Secondary Value**: Establishes testing infrastructure for reliable development
**Risk Mitigation**: Validates hardware interface and communication protocols early

## User Stories

1. **Story 1.0**: HIL Testing Framework - Hardware-in-the-Loop testing with BDD acceptance tests
2. **Story 1.1**: Project Structure & Build System - PlatformIO embedded project setup
3. **Story 1.2**: Hardware Abstraction Layer (HAL) - Consistent peripheral interfaces
4. **Story 1.3**: MODBUS RTU Communication Framework - Industrial automation interface
5. **Story 1.4**: Single Sonicator Control Implementation - Basic control and monitoring

## Acceptance Criteria

1. **Build System**: PlatformIO project builds successfully for ATmega32A target
2. **HAL Implementation**: All required peripherals (GPIO, ADC, PWM, UART) abstracted
3. **MODBUS Communication**: RTU slave responds to read/write operations within 100ms
4. **Sonicator Control**: Single unit start/stop/amplitude control operational
5. **Hardware Testing**: HIL framework validates all functionality with real hardware
6. **Code Quality**: ≥90% unit test coverage, all acceptance tests passing
7. **Documentation**: Complete setup and usage documentation provided

## Dependencies

### Prerequisites
- Arduino Uno R4 WiFi configured as ISP programmer
- ATmega32A target hardware assembled and powered
- CT2000 sonicator unit for testing
- Pin matrix documentation (docs/planning/pin-matrix.md)

### Enables
- Epic 2: Multi-Sonicator Coordination
- Epic 3: Web UI Development
- Epic 4: Production Deployment

## Risks & Mitigations

**Risk**: Hardware setup complexity may delay development
**Mitigation**: Comprehensive HIL framework with simulation capabilities

**Risk**: MODBUS protocol implementation complexity
**Mitigation**: Start with basic register operations, expand incrementally

**Risk**: Pin matrix changes affecting multiple stories
**Mitigation**: Pin matrix as SOLE SOURCE OF TRUTH, all changes require epic review

## Definition of Done

- [ ] All 5 user stories completed and accepted
- [ ] ≥90% unit test coverage achieved
- [ ] All HIL acceptance tests passing
- [ ] MODBUS communication validated with external master
- [ ] Single sonicator control demonstrated end-to-end
- [ ] Documentation complete and reviewed
- [ ] Code review completed by senior developer
- [ ] Epic retrospective conducted

## Change Log

| Date | Version | Description | Author |
|------|---------|-------------|--------|
| 2025-01-09 | 1.0 | Created epic following Axovia Flow standards | Scrum Master |
