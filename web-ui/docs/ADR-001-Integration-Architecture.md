# ADR-001: Web UI Integration Architecture

## Status

Accepted

## Context

The Multi-Sonicator-IO project required a modern web-based interface for the existing Arduino Test Harness system. The project already had a mature Hardware-in-the-Loop (HIL) testing framework built in Python, comprehensive BDD test scenarios, and a well-established command-line interface.

### Existing Assets
- Mature Python HIL framework with `hardware_interface.py`
- ASCII command protocol with Arduino test wrapper
- Comprehensive BDD test scenarios using Behave
- Established build system with Makefile integration
- Centralized configuration in `hil_config.yaml`
- Pin matrix documentation as single source of truth

### Requirements
- Single Page Application with hardware diagram interface
- Real-time pin state monitoring for all DUT pins
- Full sandbox mode operation with continuous monitoring
- Integration with existing HIL framework (no breaking changes)
- 85% test coverage requirement
- Sub-100ms hardware command response times

### Constraints
- Cannot break existing CLI tools or BDD tests
- Must reuse existing Python HIL modules
- Serial port shared between CLI and web interface
- Limited development timeline (3 sprints)

## Decision

We decided to implement an **Integrated Brownfield Approach** with the following architecture:

### Architecture Overview

```
multi-sonicator-io/
├── web-ui/                    # New React UI module
│   ├── frontend/             # React + TypeScript + Vite
│   ├── backend/              # Node.js + Express + WebSocket
│   └── shared/               # Common types and utilities
├── scripts/                  # Existing Python HIL modules (reused)
├── test/acceptance/hil_framework/  # Existing HIL infrastructure (reused)
└── Makefile                  # Extended with web UI targets
```

### Key Architectural Decisions

#### 1. Technology Stack
- **Frontend**: React + TypeScript + Vite
  - Rationale: Modern, fast development with strong typing
  - Single Page Application requirement
  - Excellent WebSocket support for real-time updates

- **Backend**: Node.js + Express + WebSocket
  - Rationale: JavaScript ecosystem consistency
  - Excellent WebSocket support for real-time communication
  - Easy integration with existing Python processes

- **Integration Layer**: Python subprocess communication
  - Rationale: Reuse existing HIL framework without modification
  - JSON-based IPC for reliable communication
  - Maintains compatibility with existing CLI tools

#### 2. Integration Strategy

**Python-Node.js Bridge Pattern**:
- Node.js backend spawns Python processes for HIL operations
- JSON messaging over stdin/stdout for communication
- Shared configuration using existing `hil_config.yaml`
- Serial port coordination through process management

**Benefits**:
- Zero changes to existing Python HIL framework
- Maintains CLI tool compatibility
- Leverages proven HIL infrastructure
- Rapid development timeline

**Trade-offs**:
- Additional process management complexity
- IPC overhead (acceptable for 100ms requirement)
- Dependency on both Node.js and Python runtimes

#### 3. Real-time Communication

**WebSocket Architecture**:
- Single WebSocket connection per client
- Server-side event broadcasting for pin state changes
- Client-side state management with React hooks
- Automatic reconnection handling

**Message Types**:
- Pin state updates
- Test execution progress
- Hardware connection status
- Arduino command logging

#### 4. Hardware Interface Design

**Visual Hardware Diagram**:
- SVG-based interactive diagram
- Real-time pin state visualization
- Clickable pin connections with detailed information
- Uses existing resource images (Arduino, ATmega32A icons)

**Pin Matrix Integration**:
- `docs/planning/pin-matrix.md` as SOLE SOURCE OF TRUTH
- Dynamic pin mapping from configuration
- Real-time state updates for all monitored pins

#### 5. Test Automation Integration

**BDD Framework Reuse**:
- Existing Behave scenarios executed via Python subprocess
- Web interface provides scenario selection and execution
- Real-time progress updates via WebSocket
- Results parsing and display in web interface

**Test Organization**:
- Tag-based filtering system
- Preset combinations for common test suites
- Search functionality across scenarios
- Export capabilities for results

## Alternatives Considered

### 1. Standalone Web Application
**Approach**: Separate web application with its own hardware interface
**Rejected because**:
- Would require duplicating HIL framework logic
- Risk of divergence between CLI and web interfaces
- Longer development timeline
- Potential for breaking existing workflows

### 2. Python Web Framework (Flask/Django)
**Approach**: Python-based web interface using Flask or Django
**Rejected because**:
- Team expertise primarily in JavaScript/TypeScript
- Less optimal WebSocket support compared to Node.js
- Would still require frontend framework (React/Vue)
- No significant advantage over Node.js approach

### 3. Direct Serial Communication in Browser
**Approach**: Use Web Serial API for direct hardware communication
**Rejected because**:
- Limited browser support for Web Serial API
- Would bypass existing HIL framework
- Security concerns with direct hardware access
- Loss of existing command protocol and error handling

### 4. Electron Desktop Application
**Approach**: Desktop application using Electron
**Rejected because**:
- Requirement was specifically for web-based interface
- Additional deployment complexity
- No significant technical advantages
- Team preference for browser-based solution

## Consequences

### Positive
- **Rapid Development**: Leveraged existing infrastructure for fast delivery
- **Zero Breaking Changes**: Existing CLI tools and tests continue to work
- **Proven Reliability**: Reused battle-tested HIL framework
- **Modern UX**: React-based interface with real-time updates
- **Comprehensive Testing**: Achieved 90.31% test coverage
- **Maintainability**: Clear separation of concerns between web UI and HIL logic

### Negative
- **Runtime Dependencies**: Requires both Node.js and Python
- **Process Management**: Additional complexity in managing Python subprocesses
- **IPC Overhead**: Small performance cost for inter-process communication
- **Deployment Complexity**: Multiple runtime environments to manage

### Neutral
- **Learning Curve**: Team needed to understand Python-Node.js integration patterns
- **Debugging**: Requires understanding both JavaScript and Python components
- **Configuration**: Multiple configuration files (package.json, requirements.txt, etc.)

## Implementation Details

### Makefile Integration
Extended existing Makefile with web UI targets:
- `make web-ui-install`: Install dependencies
- `make web-ui-dev`: Development mode
- `make web-ui-build`: Production build
- `make web-ui-sandbox`: Full sandbox mode with firmware upload
- `make web-ui-test`: Run tests with coverage

### Testing Strategy
- **Unit Tests**: Jest for frontend, pytest for backend Python components
- **Integration Tests**: End-to-end WebSocket communication testing
- **Hardware Mocking**: Mock hardware interface for reliable automated testing
- **Coverage Requirements**: 85% minimum coverage achieved (90.31%)

### Performance Optimizations
- **WebSocket Throttling**: Prevent hardware overload
- **State Management**: Efficient React state updates
- **Command Queuing**: Serial command execution to prevent conflicts
- **Connection Pooling**: Reuse Python processes where possible

## Monitoring and Metrics

### Success Metrics Achieved
- **Response Time**: Hardware commands complete within 100ms requirement
- **Test Coverage**: 90.31% (exceeds 85% requirement)
- **Integration Stability**: Zero regressions in existing CLI tools
- **User Experience**: Single page application with real-time updates

### Ongoing Monitoring
- WebSocket connection stability
- Hardware command response times
- Python subprocess health
- Memory usage and resource consumption

## Future Considerations

### Potential Improvements
- **Containerization**: Docker support for easier deployment
- **Horizontal Scaling**: Multiple hardware interfaces
- **Advanced Analytics**: Test result trending and analysis
- **Mobile Support**: Responsive design for tablet/mobile use

### Migration Path
If the Python-Node.js integration becomes a bottleneck:
1. Gradually migrate HIL logic to TypeScript
2. Implement direct serial communication in Node.js
3. Maintain backward compatibility during transition
4. Retire Python components once migration is complete

## References

- [Multi-Sonicator-IO Project README](../../README.md)
- [Hardware Configuration](../../config/hardware-config.yaml)
- [Pin Matrix Documentation](../../docs/planning/pin-matrix.md)
- [HIL Framework Documentation](../../test/acceptance/hil_framework/README.md)
- [Web UI API Documentation](./API.md)
- [Web UI User Guide](./USER_GUIDE.md)

---

**Date**: 2025-01-09  
**Author**: Development Team  
**Reviewers**: Architecture Team, QA Team  
**Status**: Implemented and Deployed
