# Multi-Sonicator Implementation Plan

Source PRD: `docs/Requirements/prd-v1.0.0.md`
Plan owner: Stephen Boyett (approval required before each phase)

Policy: One [CURRENT-TASK] at a time. Tests mandatory; CI must be green. Doxygen comments for all public APIs. Single source of truth for register map (PRD + `include/config.h`).

## Project Structure (repo-aware)
- firmware/
  - src/, include/ (keep `include/config.h`)
  - PlatformIO stays at repo root for now (`platformio.ini`)
- test/
  - unit/ (PlatformIO native tests)
  - acceptance/ (Behave BDD mapped to PRD; Simulavr)
  - hardware/
    - arduino_test_wrapper/arduino_test_wrapper.ino (HIL wrapper)
- scripts/
  - ci/ (report generation, schema validation)
  - emulation/ (Simulavr/Docker)
- tools/simulavr/
- docs/
  - Requirements/, Architecture/, executive-report-standard.md
  - planning/ (this plan, pin-matrix)

## Epics and Tasks

- #0: Pin Assignment Finalization [ ] (est: 4 days) [CURRENT-TASK]
  - #0.1: Draft single pin matrix from schematics/hardware + `include/config.h` → `docs/planning/pin-matrix.md` [ ] (est: 1 day)
  - #0.2: Cross-check PRD signals vs hardware (Overload, Freq ÷10, Freq Lock, Start, Reset, Power, Amplitude) and electrical constraints [ ] (est: 1 day)
  - #0.3: Align `include/config.h` with matrix; add doxygen comments and links to PRD sections [ ] (est: 1 day)
  - #0.4: Update HIL wrapper pins/README to match matrix [ ] (est: 1 day)
  - Gate: Sign-off required before starting #2 Firmware.

- #1: Foundations & Testing Infrastructure [ ] (est: 3 days)
  - #1.1: Confirm test dirs + PRD testing types/paths [x] (est: 1 day)
  - #1.2: HIL wrapper scaffold (PING smoke) [x] (est: 1 day)
  - #1.3: Behave HIL profile/env for serial port; smoke scenario (PING → OK PONG) [ ] (est: 1 day)

- #2: Firmware Implementation (ATmega32A, PlatformIO) [ ] (est: 12 days)
  - #2.1: Architecture skeleton: HAL → MODBUS → Control → Safety/Monitoring → Services [ ] (est: 2 days)
  - #2.2: HAL (GPIO, timers, ADC, ext. 16 MHz crystal config) [ ] (est: 2 days)
  - #2.3: MODBUS RTU slave @115200 8N1; register handlers per `include/config.h` [ ] (est: 3 days)
  - #2.4: Sonicator control: amplitude 20–100, start/stop, overload reset pulse semantics [ ] (est: 2 days)
  - #2.5: Monitoring: power scale (Pin 5, 5.44 mV/W), frequency (Pin 4 ÷10), overload, freq lock [ ] (est: 2 days)
  - #2.6: Safety & watchdog: ensure E‑stop integrity; ≤100 ms reflection instrumentation [ ] (est: 1 day)

- #3: MODBUS Contract Freeze & Traceability [ ] (est: 4 days)
  - #3.1: Canonical register table in docs (normalized from Requirements) [ ] (est: 1 day)
  - #3.2: One-to-one mapping annotations in `include/config.h` (addresses, ranges, semantics) [ ] (est: 1 day)
  - #3.3: Drift policy: PRD/version bump required for changes [ ] (est: 0.5 day)
  - #3.4: Traceability matrix (features ↔ scenarios ↔ registers) [ ] (est: 1.5 days)

- #4: Acceptance Tests (Simulavr) [ ] (est: 6 days)
  - #4.1: Stable Simulavr harness build/run (`scripts/emulation/`, `tools/simulavr/`) [ ] (est: 2 days)
  - #4.2: Behave features US‑001..003 mapped to PRD; step defs for register IO [ ] (est: 2 days)
  - #4.3: Timing asserts (≤100 ms reflection), status flag transitions (40021–40024) [ ] (est: 2 days)

- #5: Hardware Harness & HIL Enablement [ ] (est: 6 days)
  - #5.1: Harness doc: DUT pins ↔ wrapper pins (sync with pin-matrix) [ ] (est: 1 day)
  - #5.2: Finalize wrapper command set; implement baseline commands (SET/READ/PULSE) [ ] (est: 2 days)
  - #5.3: Behave HIL tag/profile running the same scenarios on hardware [ ] (est: 2 days)
  - #5.4: HIL artifact: `final/hil-session.json` (rig ID, wrapper version, mapping checksum) [ ] (est: 1 day)

- #6: HMI/PLC Integrator Deliverables [ ] (est: 5 days)
  - #6.1: Integrator notes: 4xxxx vs raw offsets, addressing examples, retries, RTU timing [ ] (est: 2 days)
  - #6.2: Sample PLC/HMI reads/writes and widgets (status/power/frequency/amplitude) [ ] (est: 2 days)
  - #6.3: Handoff checklist (addresses, version, acceptance status) [ ] (est: 1 day)

- #7: Executive Report & CI/CD [ ] (est: 6 days)
  - #7.1: Exec report generator in `scripts/ci/` per `docs/executive-report-standard.md` [ ] (est: 2 days)
  - #7.2: GitHub Actions: unit/acceptance/HIL (when available), schema validation, artifacts [ ] (est: 2 days)
  - #7.3: Coverage for Python tooling; explore C/C++ coverage feasibility [ ] (est: 2 days)

- #8: PlatformIO Profiles & Release [ ] (est: 3 days)
  - #8.1: PlatformIO envs (simulation vs hardware), flags for crystal and watchdog [ ] (est: 1 day)
  - #8.2: Release workflow: tag, attach `final/` artifacts, version bump script [ ] (est: 1 day)
  - #8.3: Post-release verification checklist (exec report pass, CI green) [ ] (est: 1 day)

- #9: Documentation Completeness [ ] (est: 3 days)
  - #9.1: Doxygen comments across firmware modules; generate docs [ ] (est: 1 day)
  - #9.2: Architecture document updates and layering responsibilities [ ] (est: 1 day)
  - #9.3: PRD appendix updates as implementation stabilizes [ ] (est: 1 day)

- #10: Optional — Cloud Telemetry Planning [ ] (est: 3 days) (default)
  - #10.1: Telemetry data model and ring buffer strategy [ ] (est: 1 day)
  - #10.2: Auth/rate-limit requirements [ ] (est: 1 day)
  - #10.3: Defer implementation pending scope approval [ ] (est: 1 day)

Note: Estimates are placeholders; adjust based on discovery.

"""
Note: Remember to update this plan after each completed task or subtask by:
  1. Checking off [ ] to [x].
  2. Removing [CURRENT-TASK] from the completed item.
  3. Adding [CURRENT-TASK] to the next active task or subtask.
"""
