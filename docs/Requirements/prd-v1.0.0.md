# Sonicator Multiplexer — Executive PRD (v1.0)

Company: Cannasol Technologies
Project: Sonicator Multiplexer
Date: 2025-08-15
Owner: Stephen Boyett

---

> Source Documents: This PRD pulls requirements exclusively from the following preliminary documents:
> - `docs/Requirements/project-requirements.md`
> - `docs/Requirements/project-reqs-qna.md`
>
> Implementation references (non-authoritative for requirements):
> - `include/config.h` (defines current register map and constants)

## 1. Executive Summary

Cannasol’s Sonicator Multiplexer extends the automation platform from controlling a single CT2000 sonicator to up to four simultaneously, increasing throughput and enabling standardized monitoring/control across units. MVP focuses on robust multi-unit control via MODBUS RTU and clear HMI integration, with optional cloud telemetry.

Key outcomes:
- 4x capacity from one automation box.
- Operator visibility: per-unit status, power, frequency, overload/frequency-lock.
- Contracted MODBUS register map for seamless PLC/HMI integration.
- Executive Functionality reporting per `docs/executive-report-standard.md`.

---

## 2. Project Overview & Business Case

- Increase batch throughput without duplicating full automation stacks.
- Provide a value-add controller (CAN-SON-CTRL-4) compatible with existing installs.
- Create a single source of truth for functional acceptance via executive reports.

KPIs (targets set in Monitoring section): uptime, MODBUS error rate, overload recovery time, acceptance pass rate.

---

## 3. Stakeholders & Users

- Stakeholders: Cannasol Technologies, Cannasol clients.
- Target users: PLC/HMI operators and maintainers; optional remote monitoring team.
- Approvers: Product owner (Stephen), Hardware advisor (Josh), Client representative (per deployment).

---

## 4. Scope & Objectives

MVP scope:
- Control 4 CT2000 sonicators from 1 automation box.
- Per unit: amplitude control, start/stop, overload reset.
- Per unit monitoring: power, frequency, overload, frequency lock.
- System-level: Active Sonicator Count and Mask.
- Reporting deliverable: Executive Functionality artifacts per `docs/executive-report-standard.md`.

Out of scope (MVP): Cloud telemetry/analytics (listed as nice-to-haves).

---

## 5. Functional Requirements
- Multi-unit control:
  - Set amplitude 20–100% (0–10 VDC shared output path per current design).
  - Start/stop sonication; trigger overload reset.
- Monitoring:
  - Power (Pin 5, 5.44 mV/W), frequency (Pin 4 freq ÷ 10), overload (Pin 1), frequency lock (Pin 3).
- System summary:
  - Active Sonicator Count (0–4) and Mask (bits 0–3 for Sonicators 1–4).
- Timing:
  - State-to-register reflection ≤100 ms (documentation phase requirement).
- HMI contract:
  - MODBUS map documented in `docs/Requirements/project-requirements.md` is the single source of truth for address allocation. `include/config.h` mirrors that contract for firmware implementation.

References (authoritative for requirements):
- `docs/Requirements/project-requirements.md` (hardware/spec tables, interfaces)
- `docs/Requirements/project-reqs-qna.md` (business/system context and decisions)
Implementation reference (non-authoritative):
- `include/config.h` (register map, pins, constants)

---

## 6. Non-Functional Requirements

- Reliability: Watchdog use and safety monitoring documented; no regression to E‑stop paths.
- Performance: MODBUS response within typical RTU timing; register update ≤100 ms; E‑stop and overload reset latency target ≤100 ms end-to-end.
- Maintainability: Modular design; clear separation of HAL/Comms/Control; documented registers.
- Testability: Simulation and acceptance scenarios mapped to PRD items.

---

## 7. Technical Architecture

- MCU: ATmega32A-PU (Arduino framework via PlatformIO).
- Interfaces: 4× CT2000 via DB9 (pins: overload, freq out ÷10, freq lock, start, reset, power, amplitude).
- Clock: 16 MHz external crystal (production decision).
- MODBUS RTU slave; Slave ID = 2; 115200 8N1 (per `include/config.h`).
- Layers: HAL → MODBUS → Sonicator Control → Safety/Monitoring → Services.

- PLC/HMI integration: PLC = Velocio; HMI = HMITool. MODBUS holding registers (4xxxx) used as the contract space.

References: `docs/Architecture/architecture.md`, `docs/Requirements/project-requirements.md`.

---

## 8. Data Model & Storage Strategy

- Primary data model is the MODBUS register map (holding registers). No persistent storage required in MVP.
- Optional (future): ESP32 telemetry buffers and cloud persistence.

---

## 9. User Interface Requirements (HMI/PLC)

- HMI must display per-unit status (Running, Overload, Freq Lock), Power (W), Frequency (Hz), and Amplitude setpoint.
- Provide system widgets for Active Sonicator Count and a per-unit running indicator.
- Use the register table below; do not assume coil/input spaces.
 - Target platforms: Velocio PLC and HMITool HMI. Document any platform-specific address formatting (4xxxx vs raw offset) in integrator notes.

---

## 10. API Specifications (MODBUS RTU)

Function codes: Holding registers (FC03 Read, FC06 Single Write, FC16 Multiple Write). All addresses use 4xxxx logical convention in docs.

Registers (from `include/config.h`):
- Control (R/W): 40001–40012
  - 40001–40004 Amplitude (20–100)
  - 40005–40008 Start/Stop (0/1)
  - 40009–40012 Overload Reset (pulse semantics)
- Monitoring (R): 40013–40024
  - 40013–40016 Power (W)
  - 40017–40020 Frequency (Hz)
  - 40021–40024 Status Flags (bit0: running, bit1: overload, bit2: freq lock)
- System (R / R/W where noted): 40025–40036
  - 40025 System Status (bitfield)
  - 40026 Emergency Stop (R/W per system design)
  - 40027–40030 Runtimes (s)
  - 40031–40034 Overload Counts
  - 40035 Active Sonicator Count (0–4)
  - 40036 Active Sonicator Mask (bits 0–3)

Note: This PRD sets the interface contract. Firmware logic changes are explicitly out-of-scope for this documentation phase.

---

## 11. Security & Safety

- Safety interlocks and E‑stop pathways remain primary; MODBUS writes must not bypass hardware interlocks.
- Watchdog policy: document reset semantics (no code changes in this phase).
- Optional cloud scope: if enabled later, require authenticated write paths and rate limits.

---

## 12. Implementation Plan with Agile User Stories

Epics:
- E1 Firmware Interface Contract (docs-only)
- E2 HMI/PLC Integration
- E3 Executive Reporting
- E4 (Optional) Cloud Telemetry

Sample user stories (docs-only acceptance now):
- US-001: As an operator, I can start/stop each sonicator so I can control production.
  - Acceptance: 40005–40008 reflect commands; status updates ≤100 ms.
- US-002: As an operator, I see Active Sonicator Count/Mask so I know overall activity.
  - Acceptance: 40035/40036 match per-unit status; consistent across transitions 0→N and N→0.
- US-003: As an executive, I receive a release artifact summarizing acceptance pass/fail per `docs/executive-report-standard.md`.
  - Acceptance: Tagged release includes `final/executive-report.json` validating against schema.

---

## 13. Testing Strategy

- Documentation phase: define acceptance scenarios to populate the Executive Report.
- Acceptance tests: Python Behave scenarios mapped to PRD requirements; execute against Simulavr-based simulation harness.
- Unit tests: pytest for C/C++ wrapper/host-side utilities and Python tooling.
- Coverage goal: ≥90% overall when code phase proceeds; maintain trend reports in CI.
- CI gates: GitHub Actions to validate artifacts, enforce schemas for `final/executive-report.json` (and optional coverage/test summaries), and block releases on validation failure per `docs/executive-report-standard.md`.

### 13.1 Hardware-in-the-Loop (HIL) Testing — `arduino_test_wrapper.ino`

- **Purpose**: Provide a USB‑serial controllable Arduino sketch that electrically interfaces to the ATmega32A target, driving/reading all relevant pins to validate the full MODBUS contract and control/monitoring semantics end‑to‑end.
- **Wrapper**: `ArduinoISP/arduino_test_wrapper.ino` (naming/location subject to repo finalization) exposes a simple command protocol over serial for Behave/pytest to:
  - Stimulate inputs (e.g., overload, frequency ÷10, frequency lock)
  - Read outputs (start/stop, reset pulse observation, amplitude PWM/DAC via ADC on wrapper)
  - Proxy MODBUS traffic if required or leave MODBUS master duties to the test host
- **Connectivity**: Test host connects to the wrapper via USB CDC; wrapper connects to the ATmega32A DUT headers mapping 1:1 to `include/config.h` pin functions using a harness.
- **Pin Harness**: A documented harness maps wrapper GPIO/ADC/counter channels to DUT pins (Overload, Freq out ÷10, Freq Lock, Start, Reset, Power, Amplitude). The PRD requires this mapping to be maintained alongside `project-requirements.md` during code phase.
- **Scenarios**: Existing Behave acceptance tests gain a HIL profile that targets the wrapper’s serial commands to provoke conditions and assert register behavior (e.g., 40021–40024 flags, 40017 frequency, 40013 power) within latency targets (≤100 ms).
- **CI Artifacts**: When HIL rigs are available (manual or lab CI), produce the same `final/executive-report.json` plus optional `final/hil-session.json` metadata (rig ID, wrapper version, mapping checksum). HIL is complementary to simulation; either can satisfy MVP acceptance provided results meet the executive schema.
- **Safety**: Wrapper must default to safe states; no command may defeat DUT safety interlocks. E‑stop/overload paths remain hardware‑first.

- **Drivers**: Two modes are supported:
  - Automated: Behave/pytest suites drive the wrapper via serial to run acceptance scenarios unattended.
  - Interactive: A host-side CLI tool provides manual control of wrapper commands for bench bring‑up and diagnostics (e.g., `hil-cli set freq 1 2000`).

---

## 14. Deployment & Release Strategy

- Tagged releases (GitHub) are the unit of record.
- CI/CD: GitHub Actions with gates for schema validation and artifact publication.
- Required deliverables (per `docs/executive-report-standard.md`):
  - `final/executive-report.json` (version 1.0.0 schema)
  - Optionally: `final/coverage-summary.json`, `final/unit-test-summary.json`
- Attach artifacts to releases under `final/` and publish to the Functionality Reports Static Web App (SWA) consumer path as defined in the standard.
- No firmware release required for this documentation phase.

---

## 15. Monitoring & Maintenance

- KPIs (tracked via acceptance/CI and HMI telemetry when applicable):
  - Uptime, Overload events rate, MODBUS error rate, Acceptance pass rate, Coverage (when available).
- Issue tracking via GitHub; link executive report evidence URLs.

---

## 16. Success Metrics & KPIs

- Executive acceptance: 100% of MVP scenarios pass in `executive-report.json`.
- HMI integration: PLC/HMI can read/write all MVP registers successfully.
- Documentation completeness: PRD + Requirements + Standard referenced and consistent.

---

## 17. Risk Assessment & Mitigation

- Register contract drift: Lock `include/config.h` map; changes require version bump and PRD update.
- Hardware variance (Pin 2/8 verification): Completed verification for Overload Reset (Pin 2) and Amplitude Control (Pin 8); maintain regression checks in test plans.
- Clock source mismatch: Resolved—production uses 16 MHz external crystal; ensure PlatformIO/toolchain settings align before code phase.

---

## 18. Timeline & Milestones (Docs Phase)

- PRD v0.1 committed (this document).
- Executive Report scenarios drafted.
- Client review; PRD v1.0 sign-off.

---

## 19. Future Enhancements

- ESP32 diagnostics logging; per-unit runtime uploads.
- TTL predictions across clients; Health % metric (cloud-calculated, HMI shadow display).
- Extended analytics dashboards and alerts.

---

## Requirements Traceability

- Source of truth for all requirements:
  - `docs/Requirements/project-requirements.md`
  - `docs/Requirements/project-reqs-qna.md`
- This PRD normalizes and summarizes those requirements into executive-facing scope, acceptance, and interfaces. For detailed tables (e.g., MODBUS map), always consult the source documents.
- Implementation files such as `include/config.h` mirror the register contract but are not authoritative for requirements.

---

## References

- `docs/Requirements/project-requirements.md` — Detailed requirements, MODBUS table, hardware specs
- `docs/Requirements/project-reqs-qna.md` — Business/system context, decisions, constraints
- `include/config.h` — Firmware constants and register definitions (implementation reference)
- `docs/executive-report-standard.md` — CI/CD artifacts and validation for executive reporting

---

## Document Control

- Version: 1.0.0
- Date: 2025-08-15
- Owner: Stephen Boyett
- Reviewers: Josh (CEO)
- Status: Approved for documentation phase; implementation changes require PRD update if they affect requirements
