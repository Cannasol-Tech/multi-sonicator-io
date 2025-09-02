# PRD-to-Scenarios Traceability Matrix

Source PRD: `docs/Requirements/prd-v1.0.0.md`
Authoritative requirements sources: `docs/Requirements/project-requirements.md`, `docs/Requirements/project-reqs-qna.md`
Implementation mirror: `include/config.h`

Profiles:

- @simulavr — emulator profile (Phase 1.3)
- @hil — hardware-in-the-loop profile using `ArduinoISP/arduino_test_wrapper.ino`

Tagging conventions:

- Requirement tags: `@req-<short>` (e.g., `@req-amplitude`, `@req-overload`)
- PRD reference tags: `@prd-<section>` (e.g., `@prd-5-functional`, `@prd-10-api`)
- Register tags: `@reg-40001` … `@reg-40036`

## Matrix

| PRD Ref | Requirement (summary) | Behave Feature (path) | Scenario ID | Tags | Profiles | Acceptance / Data Notes | Exec Report Key |
|---|---|---|---|---|---|---|---|
| §4, §5, §10 | Amplitude control per unit (20–100) via 40001–40004 | `test/bdd/features/control.feature` | SC-001 | @req-amplitude @prd-5-functional @prd-10-api @reg-40001-40004 | @simulavr @hil | Write setpoints, verify reflected and clamped; update ≤100 ms | control.amplitude |
| §4, §5, §10 | Start/Stop per unit via 40005–40008 | `test/bdd/features/control.feature` | SC-002 | @req-startstop @prd-5-functional @prd-10-api @reg-40005-40008 | @simulavr @hil | Toggle start/stop, verify status flags and timing | control.start_stop |
| §4, §5, §10 | Overload Reset pulse via 40009–40012 | `test/bdd/features/control.feature` | SC-003 | @req-overload-reset @prd-5-functional @prd-10-api @reg-40009-40012 | @simulavr @hil | Write 1 then auto-clear; electrical pulse observed in HIL | control.overload_reset |
| §4, §5, §10 | Status flags bitfield per-unit 40021–40024 | `test/bdd/features/monitoring.feature` | SC-006 | @req-status-flags @prd-5-functional @prd-10-api @reg-40021-40024 | @simulavr @hil | Stimulate running/overload/freq lock; verify bits | monitoring.status_flags |
| §5 | System Status bitfield 40025 semantics | `test/bdd/features/system.feature` | SC-013 | @req-system-status @prd-5-functional @reg-40025 | @simulavr @hil | Verify E‑stop/watchdog flags mapping | system.status_bitfield |
| §5 | Emergency Stop behavior (R/W per design) 40026 | `test/bdd/features/safety.feature` | SC-014 | @req-estop @prd-5-functional @reg-40026 | @hil | Assert E‑stop blocks outputs; resets properly | safety.estop_register |
| §5 | Per‑unit runtimes 40027–40030 | `test/bdd/features/monitoring.feature` | SC-015 | @req-runtimes @prd-5-functional @reg-40027-40030 | @simulavr @hil | Accumulates while running; resets on power‑cycle per spec | monitoring.runtimes |
| §5 | Overload counts 40031–40034 | `test/bdd/features/monitoring.feature` | SC-016 | @req-overload-counts @prd-5-functional @reg-40031-40034 | @simulavr @hil | Increment on overload events; monotonic | monitoring.overload_counts |
| §5 | Active Sonicator Count/Mask transitions | `test/bdd/features/system.feature` | SC-017 | @req-active-transitions @prd-5-functional @reg-40035 @reg-40036 | @simulavr @hil | Verify 0→N and N→0 stability, no flapping | system.active_transitions |
| §5, §6 | Amplitude setpoint clamping (20–100) | `test/bdd/features/control.feature` | SC-018 | @req-amplitude-clamp @prd-5-functional @prd-6-nfr @reg-40001-40004 | @simulavr @hil | Writes <20 or >100 clamp to bounds | control.amplitude_clamp |
| §5 | Frequency lock flag behavior | `test/bdd/features/monitoring.feature` | SC-019 | @req-freq-lock @prd-5-functional @reg-40021-40024 | @simulavr @hil | Inject lock/unlock; verify debounce if applicable | monitoring.freq_lock |
| §10 | FC16 multiple write boundary cases | `test/bdd/features/modbus.feature` | SC-020 | @req-fc16 @prd-10-api | @simulavr @hil | Cross‑register writes, partial success rules | modbus.fc16_boundaries |
| §10 | Illegal address / function error handling | `test/bdd/features/modbus.feature` | SC-021 | @req-illegal-address @prd-10-api | @simulavr | Return proper exception codes | modbus.illegal_address |
| §10 | Slave ID configuration default and change | `test/bdd/features/modbus.feature` | SC-022 | @req-slave-id @prd-10-api | @simulavr @hil | Default is 2; persists per design; range enforced | modbus.slave_id |
| §6, §10 | CRC errors and error rate tracking (if exposed) | `test/bdd/features/modbus.feature` | SC-023 | @req-crc @prd-6-nfr @prd-10-api | @simulavr | Corrupt frames rejected; optional stats sane | modbus.crc_handling |
| §9, §10 | HMI addressing examples (4xxxx vs offsets) | `test/bdd/features/hmi_integration.feature` | SC-024 | @req-hmi-addressing @prd-9-ui @prd-10-api | @simulavr @hil | Validate read/write flows with 4xxxx convention | integration.hmi_addressing |
| §5, §6 | State→register reflection ≤100 ms (expanded cases) | `test/bdd/features/timing.feature` | SC-009B | @req-latency @prd-5-functional @prd-6-nfr | @simulavr @hil | Multiple signals incl. overload reset and freq changes | nfr.latency_matrix |
| §17 | CI drift check: PRD vs include/config.h | `test/bdd/features/ci.feature` | SC-025 | @req-ci-drift @prd-17-risk | @simulavr | CI script flags mismatches; block merge | ci.drift_check |
| §4, §5, §10 | Overload Reset pulse via 40009–40012 | `test/bdd/features/control.feature` | SC-003 | @req-overload-reset @prd-5-functional @prd-10-api @reg-40009-40012 | @simulavr @hil | Write 1 then auto-clear; electrical pulse observed in HIL | control.overload_reset |
| §4, §5, §10 | Power monitoring 40013–40016 (5.44 mV/W scaling) | `test/bdd/features/monitoring.feature` | SC-004 | @req-power @prd-5-functional @prd-10-api @reg-40013-40016 | @simulavr @hil | Inject power signal; check scaling and units | monitoring.power |
| §4, §5, §10 | Frequency monitoring 40017–40020 (÷10 input) | `test/bdd/features/monitoring.feature` | SC-005 | @req-frequency @prd-5-functional @prd-10-api @reg-40017-40020 | @simulavr @hil | Inject frequency; verify computed Hz | monitoring.frequency |
| §4, §5, §10 | Status flags 40021–40024 (running, overload, freq lock) | `test/bdd/features/monitoring.feature` | SC-006 | @req-status-flags @prd-5-functional @prd-10-api @reg-40021-40024 | @simulavr @hil | Stimulate inputs; ensure correct bits | monitoring.status_flags |
| §4, §5 | Active Sonicator Count 40035 | `test/bdd/features/system.feature` | SC-007 | @req-active-count @prd-5-functional @prd-10-api @reg-40035 | @simulavr @hil | Vary active units; verify count | system.active_count |
| §4, §5 | Active Sonicator Mask 40036 | `test/bdd/features/system.feature` | SC-008 | @req-active-mask @prd-5-functional @prd-10-api @reg-40036 | @simulavr @hil | Verify bitmask across transitions | system.active_mask |
| §5, §6 | State→register reflection ≤100 ms | `test/bdd/features/timing.feature` | SC-009 | @req-latency @prd-5-functional @prd-6-nfr | @simulavr @hil | Measure latency for representative signals and writes | nfr.latency_100ms |
| §9, §10 | HMI/PLC contract adherence (4xxxx convention) | `test/bdd/features/hmi_integration.feature` | SC-010 | @req-hmi @prd-9-ui @prd-10-api | @simulavr @hil | Addressing conventions and example flows | integration.hmi_contract |
| §1, §14 | Executive Report artifact generated | `test/bdd/features/executive_reporting.feature` | SC-011 | @req-exec-report @prd-1-summary @prd-14-release | @simulavr | CI produces `final/executive-report.json` that validates | reporting.exec_artifact |
| §11 | Watchdog policy documented (docs acceptance) | `test/bdd/features/safety.feature` | SC-012 | @req-watchdog-docs @prd-11-security | n/a | Validate presence and content in docs (no firmware change) | safety.watchdog_policy |

## Scenario Stubs (to be scaffolded)

- `test/bdd/features/control.feature`
  - SC-001 Amplitude control per unit
  - SC-002 Start/Stop per unit
  - SC-003 Overload Reset pulse semantics
- `test/bdd/features/monitoring.feature`
  - SC-004 Power scaling
  - SC-005 Frequency measurement
  - SC-006 Status flags
- `test/bdd/features/system.feature`
  - SC-007 Active Sonicator Count
  - SC-008 Active Sonicator Mask
- `test/bdd/features/timing.feature`
  - SC-009 Latency ≤100 ms
- `test/bdd/features/hmi_integration.feature`
  - SC-010 HMI/PLC contract examples
- `test/bdd/features/executive_reporting.feature`
  - SC-011 Executive report artifact
- `test/bdd/features/safety.feature`
  - SC-012 Watchdog policy docs acceptance

## Execution Profiles

- Simulavr: run all scenarios feasible in emulator; timing measured via simulated time or host clock depending on signal source.
- HIL: replicate scenarios using `ArduinoISP/arduino_test_wrapper.ino`; use wrapper commands to stimulate inputs and read outputs; ensure safe electrical defaults.

## Maintenance

- Any change to the register map in `docs/Requirements/project-requirements.md` must update:
  - `include/config.h`
  - This traceability matrix
  - Behave feature files and tags
- CI gate should validate that every PRD requirement here has a corresponding scenario (at least stubbed) and that executive report keys are present in `final/executive-report.json`.
