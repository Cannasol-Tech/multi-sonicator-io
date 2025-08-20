# User Stories — MVP Firmware for Multi‑Sonicator Controller

Owner: Product (PO)
Source: docs/Requirements/prd-v1.0.0.md
Scope: Business-facing stories; technical acceptance will be appended by Architect after PO approval.

---

## US-001 — Start/Stop a Single Sonicator
- Role/Need: As an operator, I can start or stop any individual sonicator channel (S1–S4).
- Value: Enables independent control for process steps and safety interventions.
- Acceptance (business-facing):
  - When I issue a start, the unit begins operating and indicates active state.
  - When I issue a stop, the unit ceases operation and indicates inactive state.
  - Commands take effect promptly and are reflected in status views.
- Traceability: PRD §5 (Functional), §10 (External API), §13 (Testing)
- Priority: High
- Status: Draft

Technical acceptance (register-level):
- Write Start/Stop to 40005–40008 (per-unit; 0=stop, 1=start). Reflection to status within ≤100 ms.
- Status Flags 40021–40024 bit0 must mirror running state for corresponding unit.
- Active Sonicator Count (40035) and Mask (40036) consistent with per-unit running states.
- HIL: Use wrapper to observe/detect start output if applicable; verify MODBUS register reflection.

## US-002 — Overload Detection and Reset per Unit
- Role/Need: As an operator, I can see when a unit is in overload and clear the overload when safe.
- Value: Protects equipment and ensures predictable recovery.
- Acceptance:
  - Overload is clearly indicated per unit when present.
  - Clearing/resetting restores normal operation if the condition is gone.
  - System prevents restart if overload persists.
- Traceability: PRD §5, §10, §13
- Priority: High
- Status: Draft

Technical acceptance (register-level):
- Overload indication: Status Flags 40021–40024 bit1 = 1 when overload condition present (per unit).
- Reset: Write 1 to 40009–40012 to trigger overload reset (pulse semantics); firmware clears internally.
- If overload persists (bit1 remains 1), starting via 40005–40008 must be rejected or auto-stopped; document behavior.
- Reflection latency for flag changes ≤100 ms.
- HIL: Drive overload input via wrapper; assert flags and start inhibition behavior.

## US-003 — Set Amplitude (Common Control)
- Role/Need: As an operator, I can set the output amplitude for the system.
- Value: Achieves consistent process intensity across runs.
- Acceptance:
  - I can set amplitude within the allowed range.
  - The current amplitude setting is visible.
  - Out‑of‑range inputs are rejected with clear feedback.
- Traceability: PRD §5, §10, §13
- Priority: High
- Status: Draft

Technical acceptance (register-level):
- Amplitude setpoints: 40001–40004 accept 20–100 (integer). If design enforces shared output, values must remain coherent; document tie-breaking.
- Writes outside 20–100 rejected or clamped per spec; define exact behavior and reflect in register.
- Reflection to physical output (AMP_C PD7) verified via wrapper ADC within tolerance (±2% of nominal mapping) and ≤100 ms.
- HIL: Measure amplitude line via wrapper ADC; correlate with setpoint mapping.

## US-004 — View Frequency and Lock Status per Unit
- Role/Need: As an operator, I can view the operating frequency and whether the unit is frequency‑locked.
- Value: Confirms unit stability and process quality.
- Acceptance:
  - Frequency is displayed per unit and updates regularly when running.
  - Lock status is visible and accurate.
  - If a unit is stopped, values are stable and do not mislead.
- Traceability: PRD §5, §10, §13
- Priority: High
- Status: Draft

Technical acceptance (register-level):
- Frequency: 40017–40020 report Hz; tolerance and update period documented. When stopped, value either holds last-known or reports 0 per spec—document choice.
- Lock flag: 40021–40024 bit2 reflects frequency lock input.
- Reflection latency ≤100 ms from input change to register update.
- HIL: Provide ÷10 frequency signal via wrapper to each unit; toggle lock line; assert register values and flags.

## US-005 — System Status Indicator (Single LED)
- Role/Need: As an operator/technician, I can rely on a single system status LED for high‑level health.
- Value: Quick at‑a‑glance status on the line.
- Acceptance:
  - LED reflects normal, fault, and activity states per documented scheme.
  - Behavior is consistent across power cycles.
  - Behavior is documented in the manual.
- Traceability: PRD §5, Architecture doc (System Services)
- Priority: Medium
- Status: Draft

## US-006 — MODBUS Communication for Control & Monitoring
- Role/Need: As an integrator, I can control and read the system via MODBUS RTU from a PLC/HMI.
- Value: Seamless integration with industrial automation.
- Acceptance:
  - A documented set of holding/input registers supports all key operations and status.
  - Read/write operations behave predictably, with clear error feedback on invalid requests.
  - Communication performance is adequate for line operation.
- Traceability: PRD §10 (API), §5, §13
- Priority: High
- Status: Draft

## US-007 — Fault Handling and Safe Defaults
- Role/Need: As an operator, I need the system to fail safe on errors and recover predictably.
- Value: Prevents unsafe behavior and reduces downtime.
- Acceptance:
  - On startup or error, outputs assume safe defaults.
  - Watchdog or equivalent recovery is employed and documented.
  - After recovery, system state is clearly communicated and controllable.
- Traceability: PRD §6 (Non‑functional), §5, Architecture doc (System Services)
- Priority: High
- Status: Draft

## US-008 — Calibration and Scaling
- Role/Need: As a technician, I can apply calibration so reported power and control levels match reality.
- Value: Ensures accuracy and repeatability across devices.
- Acceptance:
  - Calibration values can be set (process described) and used by the system.
  - Reported values reflect calibration.
  - Documentation covers procedure and limits.
- Traceability: PRD §6, §13
- Priority: Medium
- Status: Draft

## US-009 — Operate Multiple Units Concurrently
- Role/Need: As an operator, I can run any subset of units at the same time without interference.
- Value: Increases throughput and flexibility.
- Acceptance:
  - Starting/stopping any unit does not destabilize others.
  - Frequency/lock and status remain accurate per unit.
  - System performance remains acceptable with four units active.
- Traceability: PRD §5, §6, §13
- Priority: High
- Status: Draft

## US-010 — System Information & Versioning
- Role/Need: As an integrator, I can read firmware version and device identification.
- Value: Supports traceability and fleet management.
- Acceptance:
  - Version and identity information are readable via a standard interface.
  - Values persist across power cycles and match release artifacts.
  - Documentation includes a compatibility statement.
- Traceability: PRD §10, Release process docs
- Priority: Medium
- Status: Draft

---

## Enablers — Technical Stories (Architecture/Dev)

### EN-001 — Simulation Harness via pysimulavr
- Role/Need/Value: As a developer, I need a pysimulavr runner exposing a stable UART PTY so I can run acceptance tests without hardware.
- Acceptance:
  - `scripts/emulation/cli.py` builds and launches the firmware ELF and creates stable symlink `/tmp/tty-msio`.
  - Behave sim steps in `test/acceptance/steps/steps_sim.py` connect over MODBUS RTU and exercise registers.
  - Status reflection latency ≤100 ms, matching PRD timing.
- Traceability: PRD §13 (Testing), §5 (Functional), emu deps `scripts/emulation/requirements-emulation.txt`.

### EN-002 — HIL Serial Protocol Definition
- Role/Need/Value: As QA/Dev, I need a minimal serial protocol for the Arduino wrapper to drive inputs/read outputs with parity to simulation.
- Acceptance:
  - Spec at `docs/Testing/hil-serial-protocol.md` documents PING/INFO, SET (START/RESET, OVL, LOCK, FREQ, AMPLITUDE), READ (STATUS, COUNT/MASK, ANALOG), timing (≤100 ms), and error codes.
  - Behave HIL steps reference these commands; parity of observable behavior with sim is documented.
- Traceability: PRD §13, Architecture (Testing/HIL).

### EN-003 — CI: Run Simulation Acceptance Tests
- Role/Need/Value: As a team, I need CI to run Behave in simulation to block regressions.
- Acceptance:
  - `.github/workflows/ci.yml` job executes `behave -D profile=simulavr` for at least US‑001..US‑004 feature tags.
  - CI publishes logs/artifacts and fails the job on any scenario failure.
- Traceability: PRD §17 (Risk/Process), Testing strategy.

### EN-004 — Register Map Drift Guardrail
- Role/Need/Value: As Architect/PO, I need a check that firmware register constants match the PRD map to prevent divergence.
- Acceptance:
  - Script compares PRD register table against firmware constants (e.g., `include/config.h`), producing a clear diff.
  - CI job fails on mismatches and points to offending items.
- Traceability: PRD §10 (API), §13 (Testing), repo guardrails.

### EN-005 — Coverage Gate
- Role/Need/Value: As a team, I need an enforced coverage threshold to maintain quality.
- Acceptance:
  - Unit + acceptance coverage aggregated with a single report artifact.
  - CI gate enforces ≥85% line coverage (per Definition of Done); CI fails under threshold.
- Traceability: Definition of Done, CI pipeline.

---

Notes
- After PO approval, Architect will add technical acceptance to each story: register addresses, timing guarantees, tolerance bands, and HIL references.
- Behave feature files will mirror these stories under `test/acceptance/features/`.
