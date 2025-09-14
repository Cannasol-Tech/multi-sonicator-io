---
description: Story Review Workflow for Multi-Sonicator-IO Project
auto_execution_mode: 3
---

<!-- Powered by BMAD™ Core -->

# Story Review Workflow

Perform a comprehensive test architecture review with quality gate decisions for Multi-Sonicator-IO stories. This adaptive, risk-aware review creates both story updates and detailed gate files, ensuring firmware safety, hardware compatibility, and system reliability.

## Prerequisites

- Story status must be "Review"
- Developer has completed all tasks and updated the File List
- All automated tests are passing
- Hardware pin changes (if any) made ONLY in `config/hardware-config.yaml`
- Frequency displays limited to two decimal places
- MODBUS register map consistency maintained

## Risk-Based Review Depth

### Auto-escalate to deep review when:
- Hardware configuration (`config/hardware-config.yaml`) modified
- Safety-critical firmware files touched  
- No tests added to story
- Diff > 500 lines
- Previous gate was FAIL/CONCERNS
- Story affects MODBUS communication or sonicator control
- Story has > 5 acceptance criteria

## Comprehensive Analysis Framework

### 1. Requirements Traceability
- Map each acceptance criteria to validating tests (Given-When-Then format)
- Identify coverage gaps across test pyramid:
  - Unit tests (Unity framework, ≥85% coverage)
  - Acceptance tests (Behave BDD + HIL hardware)
  - Integration tests (end-to-end system validation)
- Verify all requirements have corresponding test cases

### 2. Hardware Configuration Review
**CRITICAL - SOLE SOURCE OF TRUTH**
- [ ] All pin assignments reference `config/hardware-config.yaml` ONLY
- [ ] Pin mappings match specifications:
  - PC7-PC0: RST/START signals (ULN2003A drivers)
  - PD7-PD4: FREQ_LOCK signals (6N137 optocouplers)  
  - PA4-PA7: PWR_MONITOR signals (ADC channels)
  - PD7: AMPLITUDE_ALL (shared PWM, 0-10V)
- [ ] DB9 connector mappings maintained
- [ ] HIL test harness mappings updated if needed

### 3. Firmware Safety Assessment
- [ ] Watchdog timer (1000ms timeout, safe-state recovery)
- [ ] Emergency stop response <50μs
- [ ] Communication timeout (1s triggers safe state)
- [ ] Power-on defaults (all sonicators 'off')
- [ ] MODBUS register bounds checking

### 4. Display & UI Validation
- [ ] Frequency displays limited to two decimal places
- [ ] Power readings use 5.44mV/W scaling
- [ ] Web UI changes include Behave acceptance tests
- [ ] Status indicators reflect hardware state

### 5. Code Quality Review
- Arduino Framework & PlatformIO compliance
- Hardware Abstraction Layer (HAL) usage
- Memory constraints (24KB flash, 1.5KB RAM)
- Performance targets:
  - Command latency <200ms end-to-end
  - Telemetry rate ≥10Hz
  - MODBUS response <100ms

### 6. Test Architecture Assessment
- Test level appropriateness (unit vs integration vs HIL)
- HIL harness integration with Arduino Uno R4 WiFi
- Edge case and error scenario coverage
- Test execution time and CI impact
- Mock/stub usage for hardware dependencies

### 7. Standards Compliance
- [ ] WARP.md development guidelines followed
- [ ] Company coding standards (via `make check-standards`)
- [ ] Project structure standards
- [ ] Safety requirements compliance

### 8. Non-Functional Requirements

#### Security
- MODBUS communication validation
- Input sanitization for register writes  
- Buffer overflow protection

#### Performance
- ATmega32A resource utilization
- Real-time response requirements
- Memory footprint analysis

#### Reliability
- Error handling and recovery
- Hardware fault tolerance
- Communication resilience

#### Maintainability
- Code clarity and documentation
- Configuration management
- Test maintainability

## Gate Requirements

### Required Status Contexts
- `ci/unit` - Unity unit tests with coverage
- `ci/acceptance` - Behave BDD with HIL hardware
- `ci/integration` - End-to-end validation  
- `lint/markdown` - Documentation linting
- `standards/check` - Company standards compliance

### Build Commands
- `make build` - ATmega32A firmware build
- `make test-unit` - Unity tests (≥85% coverage)
- `make test-acceptance` - Behave + HIL tests
- `make test-integration` - End-to-end HIL validation
- `make check-standards` - Standards verification

### Gate Decision Criteria

**FAIL Conditions:**
- Security vulnerabilities in MODBUS/safety systems
- Hardware config changes without design review
- Safety timing violations (>50μs emergency stop)
- Unit test coverage <85%
- HIL tests failing with real hardware
- Memory usage exceeding ATmega32A limits

**CONCERNS Conditions:**
- Medium-severity non-critical issues
- Missing edge case tests  
- Documentation gaps
- Performance concerns
- Test maintainability issues

**PASS Conditions:**
- All critical requirements met
- Safety systems validated
- Hardware compatibility confirmed
- Adequate test coverage and quality

## Story Update Protocol

### QA Results Section ONLY
**CRITICAL**: Only update "QA Results" section. Never modify Status, Acceptance Criteria, Tasks, Dev Notes, or other sections.

Append new dated entries:

```markdown
## QA Results

### Review Date: [ISO Date]
### Reviewed By: [Name] (Test Architect)

### Hardware Configuration Review
- [ ] Pin mappings verified against config/hardware-config.yaml
- [ ] Safety timing requirements validated
- [ ] MODBUS register consistency confirmed
- [ ] HIL test coverage adequate

### Code Quality Assessment
[Overall implementation quality assessment]

### Refactoring Performed
[List any refactoring with explanations]

### Compliance Check
- Coding Standards: [✓/✗]
- Project Structure: [✓/✗] 
- Testing Strategy: [✓/✗]
- All ACs Met: [✓/✗]

### Security Review
[Security findings and resolution]

### Performance Considerations  
[Performance analysis and recommendations]

### Gate Status
Gate: [PASS/CONCERNS/FAIL/WAIVED] → docs/qa/gates/{epic}.{story}-{slug}.yml

### Recommended Status
[✓ Ready for Done] / [✗ Changes Required]
```

## Quality Gate File Creation

### Location & Format
- Directory: `docs/qa/gates/` (create if missing)
- Filename: `{epic}.{story}-{slug}.yml`
- Template: Based on `.bmad-core/templates/qa-gate-tmpl.yaml`

### Gate Structure
```yaml
schema: 1
story: '{epic}.{story}'
story_title: '{story title}'
gate: PASS|CONCERNS|FAIL|WAIVED
status_reason: 'Gate decision explanation'
reviewer: 'Quinn (Test Architect)'
updated: '{ISO-8601 timestamp}'

quality_score: 0-100
expires: '{ISO-8601 timestamp}'

evidence:
  tests_reviewed: { count }
  risks_identified: { count }
  trace:
    ac_covered: [1, 2, 3]
    ac_gaps: []

nfr_validation:
  security: { status: PASS|CONCERNS|FAIL, notes: '' }
  performance: { status: PASS|CONCERNS|FAIL, notes: '' }
  reliability: { status: PASS|CONCERNS|FAIL, notes: '' }
  maintainability: { status: PASS|CONCERNS|FAIL, notes: '' }

recommendations:
  immediate: []
  future: []
```

### Quality Score Calculation
```
quality_score = 100 - (20 × FAIL_count) - (10 × CONCERNS_count)
Bounded between 0 and 100
```

## Roles & Responsibilities

### Required Reviewers

**Firmware Engineer**
- Code quality, architecture compliance
- Required for: All firmware changes, HAL modifications

**QA/HIL Specialist**  
- Test strategy, HIL validation, acceptance criteria
- Required for: Test architecture changes, HIL modifications

**Hardware/Safety Engineer**
- Hardware configuration, safety system changes
- Required for: Pin mapping, safety timing, power scaling
- Two-person rule for critical changes

**Product Owner**
- Final sign-off for production deployment
- Required for: Safety-critical, customer-facing changes

## Emergency Procedures

### Critical Issue Response
1. **Immediate**: FAIL gate status
2. **Within 1 hour**: Notify Product Owner & Technical Lead
3. **Within 24 hours**: Root cause analysis & corrective action

### Safety System Failures
1. **Immediate**: Halt deployment pipeline
2. **Immediate**: Notify Safety Engineer & Product Owner
3. **Before Resume**: Full safety system validation
4. **Documentation**: Incident report required

## Key Principles

- Test Architect with authority to improve code directly
- Balance perfection with pragmatism  
- Risk-based prioritization
- Educational feedback for continuous improvement
- Advisory role - teams choose quality bar
- Safety and reliability paramount in industrial firmware

---

**References:**
- `.bmad-core/tasks/review-story.md` - Detailed procedures
- `config/hardware-config.yaml` - **SOLE SOURCE OF TRUTH**
- `docs/WARP.md` - Development guidelines
- `docs/qa/gates/` - Gate decision records
