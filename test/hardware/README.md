# Hardware-in-the-Loop (HIL) Testing

Location: `test/hardware/`

Purpose:
- Run Behave BDD acceptance scenarios against real hardware via an Arduino-based wrapper.
- Stimulate DUT inputs and observe outputs to validate MODBUS contract end-to-end.

Components:
- `arduino_test_wrapper/arduino_test_wrapper.ino`: Arduino sketch exposing a simple serial command protocol.

Protocol (draft):
- ASCII line protocol over USB CDC. Examples:
  - `PING` -> `OK PONG`
  - `SET FREQ <unit> <hz>`
  - `SET OVERLOAD <unit> <0|1>`
  - `SET FREQ_LOCK <unit> <0|1>`
  - `READ STATUS <unit>`
  - `PULSE RESET <unit> <ms>`
- Finalize command set alongside DUT harness and keep in sync with `include/config.h`.

Getting Started:
1. Open the sketch in Arduino IDE/CLI and flash to the wrapper board.
2. Connect the wrapper to the DUT harness (pins per harness doc).
3. From a terminal, verify with a serial monitor: send `PING`, expect `OK PONG`.
4. Point Behave HIL profile to the wrapper serial port (to be defined in `behave.ini`/env var).

Notes:
- Wrapper must default to safe states on boot and protocol errors.
- Keep harness mapping documented and versioned with this folder.
 - Current harness profile: Arduino Uno R4 WiFi, single-channel (S1 only). Untested prototype; do not enable in CI.
 - See `docs/planning/pin-matrix.md` for the authoritative mapping (Wrapper Pin and Test Point columns).
