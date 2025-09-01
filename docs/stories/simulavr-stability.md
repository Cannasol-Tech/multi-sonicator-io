# Story: Simulavr Emulator Stability & HIL Scaffolding

Status: In Progress
Owner: dev (James)
Created: 2025-08-20

## Story

Stabilize Docker-based simulavr path by enabling deterministic stepping via a SWIG `StepOnce()` wrapper and validate emulator startup and PTY creation with smoke BDD. In parallel, document and scaffold a single‑channel Arduino Uno R4 WiFi HIL harness to unblock future hardware testing (untested for now).

## Acceptance Criteria

- Emulator Docker image builds with patched SWIG exposing `StepOnce()`.
- Running smoke BDD (profile `simulavr`) brings up emulator, creates `/tmp/tty-msio`, and passes smoke scenarios.
- If emulator fails, `StepOnce()` availability is verified inside the container and logs are captured.
- Pin matrix updated with Wrapper Pin/Test Point for S1; S2–S4 marked N/A for Uno R4 single-channel.
- Arduino wrapper limited to 1 channel with safe defaults and comments pointing to the matrix.
- Documentation clearly states HIL harness is untested and out of CI scope for now.

## Tasks / Subtasks

- [ ] 1. Rebuild emulator image with SWIG patch
  - [ ] 1.a Verify Dockerfile injection for `StepOnce()`
  - [ ] 1.b Build: `make emu-docker-build`
  - [ ] 1.c Smoke: `make emu-docker-run-smoke` (or `emu-docker-test-smoke`)
  - [ ] 1.d If fail: check `/tmp/emu.log`, verify symbol `StepOnce` in Python
- [x] 2. Document Uno R4 WiFi single-channel harness
  - [x] 2.a Fill S1 wrapper pins; mark S2–S4 N/A in `docs/planning/pin-matrix.md`
  - [x] 2.b Update `test/hardware/arduino_test_wrapper/arduino_test_wrapper.ino` to single channel
  - [x] 2.c Add README note: untested, not for CI
- [ ] 3. Update implementation plan status once smoke passes
- [ ] 4. Expand acceptance tests incrementally after emulator stable

## RTU Fallback Server Notes

- Unit binding: Server bound to Modbus unit/slave `2` to match test client.
- Register mapping (indices are zero-based internal):
  - Control 40005..40008 → indices 5..8 (writes start/stop units 1..4)
  - Status 40021..40024 → indices 21..24 (bit0 reflects running)
  - Active Count 40035 → index 35
  - Active Mask 40036 → index 36
- Logging: Debug prints use `flush=True` and are visible in `/tmp/emu.log`.

## Dev Agent Record

### Debug Log

- 2025-08-20: Created story scaffold. Updated pin matrix and wrapper for single‑channel Uno R4 WiFi. Added README note.
- 2025-08-20: RTU fallback server stabilized; bound to unit 2 and aligned register indices. Smoke tests for start/stop, count, mask are green.

### Completion Notes

- Pin matrix and wrapper changes align with `include/config.h` signal names and will remain untested until hardware is available.

### File List

- Created: `docs/stories/simulavr-stability.md`
- Updated: `docs/planning/pin-matrix.md`
- Updated: `test/hardware/arduino_test_wrapper/arduino_test_wrapper.ino`
- Updated: `test/hardware/README.md`

### Change Log

- 2025-08-20: Story created; docs and wrapper updated to single-channel HIL.
- 2025-08-20: Added RTU fallback mapping/unit details; marked plan update task for smoke pass.
