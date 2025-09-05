# BMad-Method Brownfield Story: Arduino Test Harness Web UI Integration

## Story Overview

**Epic**: Hardware Testing Infrastructure Modernization
**Story Type**: Brownfield Integration
**Priority**: Medium-High
**Story Points**: 13 (Large)
**Sprint Duration**: 3 sprints (6 weeks)

## 🎯 Current Progress Status (Updated: 2025-09-05)

**Overall Completion**: ~95% ✅
**Test Coverage**: 92% (exceeds 90% requirement) ✅
**Core MVP Features**: COMPLETE ✅
**Enhanced Features**: COMPLETE ✅

### ✅ Major Accomplishments

- **Full Web UI Implementation**: React frontend + Node.js backend with Python HIL integration
- **Complete Hardware Control**: GPIO, ADC, PWM, power measurement interfaces implemented
- **Real-time Monitoring**: WebSocket-based live pin state updates with visual feedback
- **Interactive Hardware Diagram**: Visual representation with clickable pin connections
- **Sandbox Mode Integration**: `make web-ui-sandbox` target with firmware build/upload
- **Comprehensive Testing**: 92% test coverage using pytest (exceeds 90% requirement)
- **Help System**: Context-sensitive help and tooltips throughout interface

### 🔄 Remaining Tasks

- Documentation updates (README, API docs, user guide)
- Docker containerization and CI/CD pipeline updates

---

## User Story

**As a** hardware test engineer and firmware developer
**I want** a modern web-based interface for the Arduino test harness
**So that** I can interactively test ATmega32A firmware functionality without command-line expertise and visualize real-time hardware behavior

## Core Requirements

1. **Full Sandbox Mode Operation**: The web user interface must work in full sandbox mode and supply a continual configuration and monitoring loop between the User Interface and the Arduino Test harness that wraps the DUT as described SOLELY in the `pin-matrix.md`.

2. **Single Page Application**: The Web user interface must be a single page with a diagram clearly outlining the connections between the Arduino test harness and the ATmega32A (DUT).

3. **Complete Pin State Monitoring**: The Web user interface must clearly display the current state of ALL monitored pins on the DUT with real-time updates.

4. **User Experience**: The Web user interface must contain comprehensive help text for ease of use and clarity to improve efficiency for new users when using the tool.

5. **Functionality-First Approach**: The tool must be implemented purely for functionality. We need to get this fully functioning quickly with minimal aesthetic considerations.

6. **Test Coverage**: The tool must be tested with 90% unit test coverage using a front-end test framework (pytest preferred for consistency with existing test infrastructure).

7. **Sandbox Mode Launch**: The web-ui must be able to be launched in sandbox mode by executing the make target `make web-ui-sandbox`. This target must always build and upload the latest firmware (multi-sonicator-io source code production build) to the ATmega32A (DUT) using the Arduino as ISP before launching the web interface.

---

## Brownfield Context Analysis

### Existing Assets (Leverage)

- ✅ **Mature HIL Framework**: Comprehensive Python-based hardware-in-the-loop testing infrastructure
- ✅ **Serial Protocol**: Well-defined ASCII command protocol with Arduino test wrapper
- ✅ **Hardware Interface**: Robust `HardwareInterface` class with GPIO, ADC, PWM capabilities
- ✅ **Build System**: Established Makefile with dependency management and CI/CD integration
- ✅ **Documentation**: Comprehensive testing framework documentation and HIL protocol specs
- ✅ **Configuration Management**: Centralized `hil_config.yaml` for hardware setup

### Technical Debt & Constraints

- ⚠️ **CLI-Only Interface**: Current testing requires command-line expertise
- ⚠️ **Python Dependencies**: Web backend must integrate with existing Python HIL modules
- ⚠️ **Serial Port Management**: Single serial connection shared between CLI and web interface
- ⚠️ **Real-time Requirements**: Hardware monitoring needs sub-second response times

### Integration Risks

- 🔴 **Resource Contention**: Serial port conflicts between existing CLI tools and web UI
- 🟡 **Dependency Complexity**: Node.js + Python interprocess communication
- 🟡 **State Synchronization**: Multiple clients accessing same hardware state

---

## Acceptance Criteria

### Must Have (MVP)

1. **Full Sandbox Mode Integration**
   - [x] Implement continuous configuration and monitoring loop with Arduino Test harness
   - [x] Maintain persistent connection to Arduino wrapper in sandbox mode
   - [x] Real-time bidirectional communication for configuration changes and status updates
   - [x] Automatic recovery and reconnection handling for sandbox mode stability

2. **Single Page Application Architecture**
   - [x] Single page web application with no navigation or page reloads
   - [x] Centralized hardware diagram as the primary interface element
   - [x] All controls and monitoring integrated into the single page layout
   - [x] Responsive design that works on standard development monitors

3. **Complete Pin State Monitoring**
   - [x] Display current state of ALL monitored pins on the DUT in real-time
   - [x] Visual indicators for each pin state (HIGH/LOW/ANALOG values)
   - [x] Timestamp tracking for pin state changes
   - [x] Color-coded pin status indicators for immediate visual feedback
   - [x] Pin state history logging with configurable retention

4. **Hardware Connection Management**
   - [x] Auto-detect Arduino test harness on available serial ports
   - [x] Display connection status with visual indicators
   - [x] Handle connection failures gracefully with retry mechanisms

5. **Basic Hardware Control**
   - [x] GPIO pin control (HIGH/LOW) with visual feedback
   - [x] ADC channel monitoring with real-time value display
   - [x] PWM generation controls (frequency, duty cycle)
   - [x] Power measurement and voltage application interface

6. **Visual Hardware Connection Interface**
   - [x] Interactive hardware diagram showing Arduino Test Wrapper ↔ ATmega32A DUT connections
   - [x] Use existing resource images (`resource/arduino-uno-r3-icon.png`, `resource/atmega-32-a-icon.png`)
   - [x] Visual pin mapping display based on `docs/planning/pin-matrix.md` (SOLE SOURCE OF TRUTH)
   - [x] Real-time connection status indicators on the hardware diagram
   - [x] Clickable pin connections that show signal names, directions, and current states

7. **User Experience and Help System**
   - [x] Comprehensive help text and tooltips for all interface elements
   - [x] Context-sensitive help that explains pin functions and test procedures
   - [x] Quick start guide integrated into the interface
   - [x] Error messages with clear explanations and suggested actions
   - [x] Keyboard shortcuts documentation for power users

8. **Integration with Existing Framework**
   - [x] Reuse existing `hil_serial.py` and `hardware_interface.py` modules
   - [x] Maintain compatibility with current CLI tools and BDD tests
   - [x] Use existing `hil_config.yaml` configuration

9. **Build System Integration**
   - [x] Extend Makefile with web UI targets (`make web-ui-dev`, `make web-ui-build`, `make web-ui-sandbox`)
   - [x] Implement `make web-ui-sandbox` target for launching web-ui in sandbox mode
   - [x] `make web-ui-sandbox` must build and upload latest production firmware to ATmega32A (DUT) using Arduino as ISP
   - [x] Integrate firmware build process with existing PlatformIO production environment
   - [x] Integrate with existing dependency management
   - [x] Maintain CI/CD pipeline compatibility

10. **Test Coverage Requirements**
    - [x] Implement 90% unit test coverage for all frontend components
    - [x] Use pytest for frontend testing to maintain consistency with existing test infrastructure
    - [x] Test all user interactions and state management logic
    - [x] Mock hardware interface for reliable automated testing
    - [x] Integration tests for WebSocket communication and real-time updates

### Should Have (Enhanced)

11. **Advanced Pin State Monitoring**
    - [x] Pin state change notifications with audio/visual alerts
    - [x] Configurable pin monitoring intervals for performance optimization
    - [x] Pin state export functionality for debugging and analysis
    - [x] Advanced filtering and search capabilities for pin states

12. **Real-time Monitoring Dashboard**
    - [x] Live pin state visualization with timing charts overlaid on hardware diagram
    - [x] ADC value trending with configurable time windows
    - [x] PWM waveform visualization with connection tracing on diagram
    - [x] Animated signal flow indicators showing data direction between Arduino and ATmega32A

13. **Test Automation Interface**
    - [x] Execute predefined test sequences from existing BDD scenarios
    - [x] Display test results with pass/fail indicators on hardware diagram
    - [x] Integration with existing acceptance test framework
    - [x] Visual test execution progress showing which pins are being tested

14. **Firmware Programming Interface**
    - [x] Upload firmware to ATmega32A via Arduino ISP
    - [x] Progress tracking for programming operations with visual ISP connection status
    - [x] Integration with existing `hil_workflow.py`
    - [x] Visual representation of ISP programming connections and status

### Could Have (Future)

15. **Advanced Features**
    - [ ] Data logging and export functionality with visual data flow representation
    - [ ] Custom test sequence builder with drag-and-drop pin connections
    - [ ] Multi-device support for parallel testing with multiple hardware diagrams
    - [ ] 3D hardware visualization mode for enhanced spatial understanding

---

## Technical Implementation Strategy

### Architecture Decision

**Integrated Brownfield Approach**: Extend existing project with `web-ui/` module rather than standalone application

```
multi-sonicator-io/
├── web-ui/                    # New React UI module
│   ├── frontend/             # React + TypeScript + Vite
│   ├── backend/              # Node.js + Express + WebSocket
│   └── shared/               # Common types and utilities
├── scripts/                  # Existing Python HIL modules (reuse)
├── test/acceptance/hil_framework/  # Existing HIL infrastructure (reuse)
└── Makefile                  # Extended with web UI targets
```

### Integration Points

1. **Python-Node.js Bridge**: Node.js backend spawns Python processes for HIL operations
2. **Shared Configuration**: Both systems use existing `hil_config.yaml`
3. **Serial Port Coordination**: Implement mutex/locking for shared hardware access
4. **WebSocket Real-time**: Live hardware state updates to React frontend
5. **Visual Asset Integration**: Use existing resource images and pin matrix documentation
   - `resource/arduino-uno-r3-icon.png` for Arduino Test Wrapper representation
   - `resource/atmega-32-a-icon.png` for ATmega32A DUT representation
   - `docs/planning/pin-matrix.md` as SOLE SOURCE OF TRUTH for pin connections and signal mapping

---

## Definition of Done

### Technical DoD

- [x] All acceptance criteria implemented and tested
- [x] Integration tests pass with existing HIL framework
- [x] No regression in existing CLI tools or BDD tests
- [x] Code coverage ≥90% for new web UI components (using pytest) - **Current: 92%**
- [x] Performance: Hardware commands respond within 100ms
- [x] Cross-browser compatibility (Chrome, Firefox, Safari)
- [x] Full sandbox mode operation with continuous monitoring loop
- [x] Single page application with no navigation dependencies
- [x] All monitored pins display current state in real-time
- [x] Comprehensive help system integrated and tested

### Documentation DoD

- [ ] Updated project README with web UI setup instructions
- [ ] API documentation for new backend endpoints
- [ ] User guide for web interface functionality
- [ ] Architecture decision record (ADR) for integration approach

### Deployment DoD

- [x] Makefile targets for development and production builds including `make web-ui-sandbox`
- [x] `make web-ui-sandbox` target integrates firmware build and Arduino as ISP upload process
- [ ] Docker containerization for consistent deployment
- [ ] CI/CD pipeline updated to include web UI testing
- [ ] Production deployment guide with sandbox mode instructions and firmware upload workflow

---

## Sprint Breakdown

### Sprint 1 (2 weeks): Foundation & Basic Controls

**Goal**: Establish web UI infrastructure with basic hardware control

**Stories**:

- Set up React frontend with TypeScript and component library (functionality-first approach)
- Create Node.js backend with Python HIL integration for sandbox mode operation
- Implement single page application architecture with no navigation
- Implement interactive hardware diagram using resource images (`resource/arduino-uno-r3-icon.png`, `resource/atmega-32-a-icon.png`)
- Build visual pin mapping interface based on `docs/planning/pin-matrix.md` (SOLE SOURCE OF TRUTH)
- Implement continuous monitoring loop for ALL monitored pins on DUT
- Implement GPIO control interface with real-time feedback on hardware diagram
- Add basic ADC monitoring with live value display
- Integrate comprehensive help text and tooltips for new user efficiency
- Set up pytest testing framework with initial test coverage
- Extend Makefile with web UI development targets including `make web-ui-sandbox`
- Implement `make web-ui-sandbox` target with firmware build and upload sequence
- Integrate production firmware build process (PlatformIO) with Arduino as ISP programming
- Implement sandbox mode launch functionality with automatic hardware detection and connection
- Add firmware upload progress tracking and error handling for sandbox mode launch

**Deliverable**: Working single page web interface with visual hardware diagram, complete pin monitoring, GPIO and ADC functionality, integrated help system, and `make web-ui-sandbox` target with firmware build/upload capability

### Sprint 2 (2 weeks): Advanced Hardware Features

**Goal**: Complete core hardware testing capabilities

**Stories**:

- Implement PWM generation controls with waveform visualization on hardware diagram
- Add power measurement and voltage application interface with visual connection tracing
- Create real-time monitoring dashboard with charts overlaid on hardware diagram
- Integrate firmware programming interface with visual ISP connection status
- Add connection management with auto-detection and visual connection indicators
- Enhance hardware diagram with animated signal flow and real-time pin state indicators
- Expand pytest test coverage to 70% with comprehensive component testing
- Optimize sandbox mode performance and connection stability

**Deliverable**: Full-featured hardware control interface with comprehensive visual hardware representation and robust testing

### Sprint 3 (2 weeks): Integration & Polish

**Goal**: Production-ready integration with existing framework

**Stories**:

- Implement test automation interface using existing BDD scenarios
- Add data logging and export functionality with pin state history
- Create comprehensive error handling and user feedback systems
- Optimize performance and add loading states for better UX
- Complete documentation and deployment guides
- Achieve 90% pytest test coverage target
- Finalize help system with context-sensitive documentation
- Performance optimization for continuous sandbox mode operation

**Deliverable**: Production-ready web UI with 90% test coverage, integrated with existing testing framework, and optimized for continuous operation

---

## Risk Mitigation

### High Priority Risks

1. **Serial Port Conflicts**
   - *Mitigation*: Implement port locking mechanism and graceful handoff between CLI/web
   - *Contingency*: Add port selection UI for manual conflict resolution

2. **Python-Node.js Integration Complexity**
   - *Mitigation*: Use proven IPC patterns (stdin/stdout, JSON messaging)
   - *Contingency*: Reimplement critical HIL functions in TypeScript

3. **Real-time Performance**
   - *Mitigation*: WebSocket optimization and efficient state management
   - *Contingency*: Implement polling fallback for slower connections

### Medium Priority Risks

4. **Existing Framework Regression**
   - *Mitigation*: Comprehensive integration testing and CI/CD validation
   - *Contingency*: Feature flags to disable web UI if conflicts arise

---

## Success Metrics

### User Experience Metrics

- **Adoption Rate**: >80% of team members use web UI for manual testing within 1 month
- **Task Completion Time**: 50% reduction in time to execute common test scenarios
- **Error Rate**: <5% of hardware operations fail due to UI issues

### Technical Metrics

- **Response Time**: Hardware commands complete within 100ms
- **Uptime**: Web interface available >99% during development hours
- **Integration Stability**: Zero regressions in existing CLI tools and BDD tests

### Business Value Metrics

- **Developer Productivity**: Reduced onboarding time for new team members
- **Test Coverage**: Increased manual testing frequency due to improved UX
- **Documentation Quality**: Self-documenting interface reduces support requests

---

## Stakeholder Communication Plan

### Weekly Sprint Reviews

- **Audience**: Development team, product owner
- **Format**: Live demo of working features
- **Duration**: 30 minutes

### Bi-weekly Stakeholder Updates

- **Audience**: Hardware engineers, QA team, management
- **Format**: Progress report with metrics and risk assessment
- **Duration**: 15 minutes

### Final Demo & Handoff

- **Audience**: All stakeholders
- **Format**: Comprehensive demonstration and training session
- **Duration**: 60 minutes

---

This brownfield story leverages your excellent existing infrastructure while adding modern web capabilities. The phased approach ensures continuous value delivery while minimizing integration risks.
