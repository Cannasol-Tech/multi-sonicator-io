# Project Brief: Sonicator Multiplexer

## Executive Summary

- Cannasol’s Sonicator Multiplexer extends a single automation controller to operate up to four CT2000 sonicators concurrently, increasing throughput and standardizing monitoring/control across units.
- MVP delivers robust MODBUS RTU integration, per-unit control (amplitude, start/stop, overload reset), and monitoring (power, frequency, overload, frequency lock), with clear HMI integration.
- Success is validated through executive reporting artifacts and acceptance criteria mapped to the PRD, ensuring a consistent, testable interface contract for PLC/HMI deployments.

## Problem Statement

- Current state and pain points: Operations rely on manual or single-channel control of CT2000 units, creating bottlenecks and inconsistent outcomes across runs. Telemetry is fragmented, and overload resets are ad hoc rather than standardized via automation.
- Impact: Throughput is capped by sequential processing; operator time per batch is high; QA experiences variability due to inconsistent monitoring; avoidable downtime occurs after overload events. Standardization would reduce cycle time and defects.
- Why existing solutions fall short: Off-the-shelf multiplexers or generic PLC modules don’t provide a clean, testable interface aligned to CT2000 I/O semantics, lack per‑unit telemetry/control at the required granularity, and don’t integrate cleanly with the target PLC/HMI workflows.
- Urgency and why now: Production scaling requires multi-unit parallelization with a contractually stable interface before further expansion. Establishing this now reduces operational risk, improves consistency, and enables faster commissioning.

## Proposed Solution

- Implement a microcontroller-based multiplexer that exposes a stable MODBUS RTU contract to the PLC/HMI while managing four CT2000 units concurrently.
- Provide per-unit control: amplitude setpoint, start/stop, overload reset, and safety interlocks; and per-unit monitoring: power (W), frequency (Hz), overload, frequency lock.
- Establish a hardware harness and HIL wrapper (`test/acceptance/sketches/arduino_test_wrapper/`) to validate I/O semantics, timing, and error handling pre-integration.
- Deliver acceptance tests (unit, HIL, and BDD acceptance in `test/acceptance/`) tied to PRD requirements; integrate in CI (`.github/workflows/ci.yml`).
- Maintain a single source of truth for pins in `config/hardware-config.yaml` (SOLE SOURCE OF TRUTH) synchronized with `include/system_config.h`.

## Target Users

- Primary: Manufacturing engineers and PLC/HMI integrators needing a deterministic, testable interface to operate multiple CT2000 units in parallel.
- Secondary: QA/Operations teams seeking standardized telemetry for traceability and exception handling (overload, frequency lock loss).
- Tertiary: Maintenance/Support personnel requiring diagnostics and safe reset pathways.

## Goals & Success Metrics

- Business Objectives
  - Increase throughput per PLC cell by ≥3x vs single-channel baseline.
  - Reduce operator intervention time per batch by ≥50%.
  - Cut unplanned downtime from overload events by ≥30% via standardized reset flow.
- User Success Metrics
  - PLC receives per-unit telemetry at required cadence with <1% packet loss.
  - HMI can start/stop/reset any unit within ≤200 ms command latency.
  - Amplitude control accuracy within ±2% of setpoint in steady state.
- KPIs
  - MTBF for interface failures: >500 hours.
  - Test coverage ≥85% with 100% passing; HIL regression green for releases.

## MVP Scope

- In Scope (Must Have)
  - Four-channel control/monitoring via MODBUS RTU.
  - Signals: OVERLOAD, FREQ_DIV10, FREQ_LOCK (inputs); START, RESET, AMPLITUDE (outputs); POWER_SENSE (analog in) per unit.
  - HIL wrapper and harness with automated acceptance tests.
  - CI pipeline running unit + HIL tests and generating executive report.
- Out of Scope (MVP)
  - Cloud telemetry/remote dashboards.
  - Automatic calibration beyond basic scaling constants.
  - Advanced analytics beyond simple thresholding and lock-state checks.
- MVP Success Criteria
  - All acceptance tests pass against PRD; PLC demo shows concurrent operation of 4 units executing scripted sequence without errors.

## Post-MVP Vision

- Phase 2 Features
  - Remote telemetry aggregation and batch analytics.
  - Auto-tuning amplitude profiles per recipe.
  - Enhanced diagnostics (frequency stability metrics, overload root-cause hints).
- Long-term Vision
  - Scalable architecture supporting >4 channels and variant CT models.
  - Optional web UI for commissioning and diagnostics.
- Expansion Opportunities
  - Integration with MES for lot/batch traceability and electronic batch records.

## Technical Considerations

- Platform Requirements
  - Target: ATmega32A firmware (`src/main.c`, `src/modules/sonicator/`), Arduino-based HIL harness for lab validation.
  - Interface: MODBUS RTU over serial to PLC/HMI.
  - Performance: Command latency ≤200 ms; telemetry update interval configurable (e.g., 5–10 Hz effective per-unit reporting).
- Technology Preferences
  - Firmware in C with modular architecture; strong doxygen documentation in `include/` and `src/`.
  - Testing: C++ unit tests in `test/`, BDD acceptance with Behave (`test/acceptance/`), HIL in Arduino wrapper.
  - CI: GitHub Actions workflows (`.github/workflows/ci.yml`) run tests and reports.
- Architecture Considerations
  - Centralized config in `include/system_config.h` mirrors `config/hardware-config.yaml` (SOLE SOURCE OF TRUTH).
  - Service-style modules for I/O handling, MODBUS service, and per-unit state machines.
  - Security/Safety: Debounce, watchdogs, safe-state on error; no hardcoded secrets.

## Constraints & Assumptions

- Constraints
  - Electrical characteristics of CT2000 interfaces; 0–10 V amplitude path via DAC/PWM stage; analog scaling (e.g., 5.44 mV/W for power sense).
  - Limited MCU resources (timers, ADC channels, UART bandwidth) across 4 units.
  - Lab availability for HIL; production PLC environment for final validation.
- Assumptions
  - MODBUS RTU is the integration protocol; PLC/HMI expects a stable register map.
  - See: docs/architecture/modbus-previous-state-registers.md for diagnostic, read-only "previous state" registers (no auto-resume).
  - Four units are sufficient for target throughput in Phase 1.
  - Schematics provide clear pin/header references to finalize the pin matrix.

## Risks & Open Questions

- Risks
  - Frequency measurement accuracy and timing contention across four inputs.
  - Overload handling races if resets are not sequenced correctly per unit.
  - Analog noise affecting power-sense scaling and amplitude control accuracy.
- Open Questions
  - Exact MODBUS register layout and scaling conventions—confirm with PLC team.
  - Required telemetry cadence and acceptable command latency—confirm in HMI spec.
  - HIL wrapper serial protocol details and harness wiring pin numbers on the Arduino side—finalize alongside lab bring-up.
- Areas Needing Further Research
  - Best-practice filtering for FREQ_DIV10 and analog smoothing without lagging control.

## Appendices

- References
  - PRD: `docs/prd/prd`
  - Hardware Configuration: `config/hardware-config.yaml` (SOLE SOURCE OF TRUTH — FINALIZED)
  - Implementation Plan: `docs/planning/implementation-plan.md` (TBD)
  - CI: `.github/workflows/ci.yml`

## Next Steps

- Immediate Actions
  1. Confirm hardware configuration is finalized and keep `include/system_config.h` in sync (COMPLETED). config/hardware-config.yaml is the SOLE SOURCE OF TRUTH.
  2. Document STATUS LED on PD2 (physical pin 16) and remove legacy RGB references across docs (COMPLETED).
  3. Implement per-unit I/O services and MODBUS register map.
  4. Build HIL wrapper protocol and acceptance scenarios; ensure CI is green.
  5. Schedule PLC/HMI demo for MVP acceptance.
