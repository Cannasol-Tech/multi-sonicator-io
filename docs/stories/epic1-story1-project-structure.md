# Story: Project Structure & Build System Setup

## Status: Ready for Development

## Story

As a **firmware developer**,
I want **a properly configured embedded project structure with build system**,
So that **I can develop, test, and deploy ATmega32A firmware efficiently**.

## Epic Context

**Epic 1: Foundational Control & Communication**
- Goal: Establish core firmware structure, basic sonicator control, and MODBUS communication layer

## Story Context

**Technology Stack:**
- ATmega32A microcontroller @ 16MHz external crystal
- Arduino Framework via PlatformIO 
- Unity Test Framework for unit testing
- C/C++ for firmware implementation

**Integration Points:**
- PlatformIO build system
- Arduino as ISP programming interface
- CI/CD pipeline preparation

## Acceptance Criteria

**Functional Requirements:**

1. PlatformIO project builds successfully for ATmega32A target
2. Basic Arduino framework integration operational (Serial, GPIO, Timer functions)
3. Project structure follows embedded standards with proper separation of concerns
4. Initial main.cpp demonstrates basic functionality (LED blink, serial output)

**Build System Requirements:**

5. Build process produces correct ATmega32A binary with proper fuse settings
6. Upload mechanism via Arduino as ISP functional
7. Multiple build environments (development/production) configured
8. Makefile integration with PlatformIO for standardized commands

**Quality Requirements:**

9. Project structure documented in README
10. Build instructions verified on clean environment
11. Initial unit test framework configured and passing
12. Code follows embedded coding standards from docs/standards/

## Technical Implementation Details

### Required Directory Structure
```
src/
├── main.cpp                 # Application entry point
├── modules/                 # Modular firmware components
│   ├── hal/                # Hardware Abstraction Layer
│   ├── communication/      # MODBUS and protocol handling
│   └── control/           # Sonicator control logic
include/
├── config.h               # System configuration constants
└── types.h               # Common type definitions
test/
├── unit/                 # Unity unit tests
└── acceptance/          # HIL and integration tests
```

### Core Configuration Files
- `platformio.ini` with ATmega32A settings (✅ Already exists)
- `config.h` with system constants and pin definitions
- `types.h` with embedded-specific type definitions
- Makefile integration for standardized build commands

### Initial Functionality
- System initialization and watchdog setup
- Basic GPIO control (status LED on PD2 per pin matrix)
- Serial communication at 115200 baud for debugging
- Timer setup for future cooperative scheduler

## Definition of Done

- [ ] Project builds without errors for ATmega32A target
- [ ] Upload to hardware via Arduino as ISP successful
- [ ] Basic functionality demonstrated (LED blink, serial output)
- [ ] Directory structure matches embedded standards
- [ ] Build documentation updated in README
- [ ] Initial unit test passes
- [ ] Code review completed for structure and standards compliance
- [ ] Makefile targets functional

## Dependencies

**Prerequisites:**
- Hardware setup (ATmega32A board, Arduino as ISP)
- PlatformIO installed and configured
- Arduino as ISP programmer setup

**Enables Next Stories:**
- Hardware Abstraction Layer implementation
- MODBUS communication framework
- Sonicator control interface development

## Risk Assessment

**Primary Risk:** Arduino as ISP programming setup complexity
**Mitigation:** Step-by-step documentation and verification
**Verification:** Successful upload and basic functionality test

## Notes

- This story establishes the foundation for all subsequent development
- Focus on simplicity and standards compliance
- All pin assignments must follow docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH)
- Status LED implementation must use PD2 (physical pin 16) per architecture document
