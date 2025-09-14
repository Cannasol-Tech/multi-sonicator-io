# Implementation Plan

Project: Multi-Sonicator-IO
Owner: Product Owner
Last Updated: 2025-09-10

## Planning Standards

- Follow Axovia Flow standards (`.axovia-flow/sop/`)
- Single Source of Truth for hardware mapping: `config/hardware-config.yaml`
- TDD-first for every story (write tests first, ensure they pass last)
- Keep one source of truth, avoid duplication

## Milestones

- Milestone M0: Epic 0 — HIL Testing Framework & Hardware Verification
- Milestone M1: Epic 1 — Foundational Control & Communication
- Milestone M2: Epic 3 — Hardware Testing Infrastructure Modernization

---

## Epic 0: HIL Testing Framework & Hardware Verification

User Stories (planning checklist — creation of story docs)
- [x] 0.1 Project Structure & CI/CD Pipeline — `docs/stories/0.1.project-structure-cicd-pipeline.md`
- [x] 0.2 HIL Test Harness Setup — `docs/stories/0.2.hil-test-harness-setup.md`
- [x] 0.3 BDD Acceptance Testing Framework — `docs/stories/0.3.bdd-acceptance-testing-framework.md`
- [x] 0.4 Hardware Connection Verification — `docs/stories/0.4.hardware-connection-verification.md`
- [x] 0.5 Arduino Test Wrapper Validation — `docs/stories/0.5.arduino-test-wrapper-validation.md`
- [x] 0.6 Sandbox CLI Implementation — `docs/stories/0.6-sandbox-cli-manual-testing.md`
- [x] 0.7 MODBUS Hardware Validation — `docs/stories/0.7.modbus-hardware-validation.md`
- [x] 0.8 Web UI Testing Interface — `docs/stories/0.8.web-ui-testing-interface.md`
- [x] 0.9 Test Coverage & Documentation — `docs/stories/0.9.test-coverage-and-documentation.md`

Execution Tasks (DoD-driven; tests-first, then implementation)
- [ ] Add acceptance BDD skeletons for each 0.x story (tagged @epic0)
- [ ] Implement incremental HIL harness with simulation fallback
- [ ] Validate pin matrix against `config/hardware-config.yaml` in automated step
- [ ] Ensure CI targets for unit/acceptance tests and coverage reporting

---

## Epic 1: Foundational Control & Communication

User Stories (planning checklist — creation of story docs)
- [x] 1.0 HIL Testing Framework — `docs/stories/1.0-hil-testing-framework.md`
- [x] 1.1 Project Structure & Build System — `docs/stories/1.1-project-structure.md`
- [x] 1.2 Hardware Abstraction Layer (HAL) — `docs/stories/1.2-hal-implementation.md`
- [x] 1.3 MODBUS RTU Communication Framework — `docs/stories/1.3-modbus-framework.md`
- [x] 1.4 Single Sonicator Control — `docs/stories/1.4.single-sonicator-control.md` (S4-only focus)

Execution Tasks (TDD-first)
- [ ] Define register map tables for control/telemetry (`include/register_map.h`)
- [ ] Implement HAL modules (GPIO, ADC, PWM, UART) with unit tests
- [ ] Implement MODBUS slave handlers with boundary and negative tests
- [ ] Implement single sonicator control (start/stop/amplitude) with HIL tests

---

## Notes & Assumptions

- Scope temporarily constrained to S4-only simulation/wrapper mapping per `docs/planning/pin-matrix.md` and `config/hardware-config.yaml`.
- QA Gates exist under `docs/qa/gates/` and should be extended for any newly added stories.

## Tracking

- Update this plan after any story creation or status change.
- Mark tasks complete by changing `[ ]` to `[x]`.
- Add subtasks as implementation details become clearer.

---

## Epic 3: Hardware Testing Infrastructure Modernization

User Stories (planning checklist — creation of story docs)
- [x] 3.0 Web UI Integration — `docs/agile/stories/3.0.integration-story.md`
- [ ] 3.1 Frontend Development — `docs/agile/stories/3.1.frontend-development-react-interface.md`
- [ ] 3.2 Real-Time Logging & Traceability — `docs/agile/stories/3.2.real-time-logging-traceability.md`

Execution Tasks (TDD-first)
- [ ] Implement real-time logging with ring buffers and export endpoints (CSV/JSONL)
- [ ] Add Web UI logging controls and status indicators with tests (≥95% coverage on new modules)
- [ ] Extend HIL publisher with timestamps/correlation IDs; add Behave scenarios tagged `@trace`
- [ ] Ensure `make web-ui-sandbox` supports enabling logging and triggering exports

---

## Epic 4: Multi-Sonicator Coordination & Management

User Stories (planning checklist — creation of story docs)
- [ ] 4.1 Multi-Unit State Management — `docs/stories/4.1.multi-unit-state-management.md`
- [x] 4.2 Individual Control Systems — `docs/stories/4.2.individual-control-systems.md`
- [ ] 4.3 Coordinated Operation Modes — `docs/stories/4.3.coordinated-operation-modes.md`
- [ ] 4.4 Advanced Amplitude Management — `docs/stories/4.4.advanced-amplitude-management.md`
- [ ] 4.5 Real-time Monitoring Systems — `docs/stories/4.5.realtime-monitoring-systems.md`

Execution Tasks (TDD-first)
- [ ] Write unit tests for `SonicatorUnit` (start/stop, amplitude bounds, safety)
- [ ] Implement per-unit MODBUS handlers with boundary/negative tests
- [ ] Add HIL tests for S4-first control per current scope (see `docs/planning/pin-matrix.md`)
- [ ] Validate response times <100ms in integration tests; log artifacts under `test/data/results/`
- [ ] Create feature prompts in `docs/features/` and keep in sync with register map `include/register_map.h`

Notes
- Current scope is S4-only simulation/wrapper mapping (UNO R4: D7, D8, A2, A3, A4, A1, D9 PWM). Implement Unit 4 fully; scaffold Units 1–3 with clear TODOs to avoid duplication later.

---

## Epic 5: Production Deployment & Operations

User Stories (planning checklist — creation of story docs)
- [ ] 5.1 Automated Deployment Pipeline — `docs/stories/5.1.automated-deployment-pipeline.md`

Execution Tasks (TDD-first)
- [ ] Extend `.github/workflows/ci.yml` with build/test/artifact stages for firmware and web-ui
- [ ] Add `release.yml` for tagged builds publishing firmware and docs artifacts
- [ ] Ensure traceability matrix is generated and uploaded each run
- [ ] Build Docker images for `web-ui/frontend` and `web-ui/backend` (optional push on secrets)
