# HIL Serial Protocol (Wrapper <-> Host)

Owner: Architect
Status: Draft (MVP scope)
Purpose: Drive inputs and observe outputs of the Multi‑Sonicator firmware via a simple serial protocol for hardware‑in‑the‑loop (HIL) testing. Mirrors simulation capabilities used by Behave tests.

---

## 1. Transport
- Physical: USB CDC ACM (Arduino‑class wrapper) at 115200‑8N1.
- Encoding: ASCII command lines terminated with `\n`.
- Responses: One line per request. Success starts with `OK`, errors start with `ERR`.
- Timing: End‑to‑end reflection to firmware MODBUS registers ≤100 ms for state/measurements.

## 2. Command Summary
- `PING` → `OK PONG`
- `INFO` → `OK <wrapper_version> <build>`
- `SET START <unit> <0|1>`
- `SET RESET <unit>` (pulse)
- `SET OVL <unit> <0|1>`
- `SET LOCK <unit> <0|1>`
- `SET FREQ <unit> <hz>`
- `SET AMPLITUDE <0..100>` (system setpoint as seen by firmware input chain)
- `READ STATUS <unit>` → `OK RUN=<0|1> OVL=<0|1> LOCK=<0|1>`
- `READ COUNT` → `OK COUNT=<0..4>`
- `READ MASK` → `OK MASK=<bitmask>` (bit0=S1 .. bit3=S4)
- `READ ANALOG AMP` → `OK AMP=<adc|percent>` (wrapper shall state units capability)

Notes
- Units are 1..4 (S1..S4). Values outside range => `ERR ARG`.
- Frequencies in Hz (integer). Wrapper implements square‑wave sources per unit input.
- Where hardware cannot implement a specific stimulus, respond `ERR UNSUPPORTED` and document.

## 3. Semantics & Mapping
This protocol is for HIL only; firmware external interface remains MODBUS RTU. Mapping expectations used by tests:
- Start/Stop: Firmware holding registers 40005–40008 (0/1). HIL `SET START` drives the equivalent external input path that results in the same register reflection within ≤100 ms.
- Overload Reset: Firmware 40009–40012 pulse semantics. HIL `SET RESET` applies a reset stimulus; registers clear back to 0 within ≤100 ms.
- Overload Indication: Status registers 40021–40024 bit1 must mirror HIL `SET OVL` within ≤100 ms.
- Lock Indication: Status registers 40021–40024 bit2 must mirror HIL `SET LOCK` within ≤100 ms.
- Frequency Measure: Input frequency driven by `SET FREQ` is reported in 40017–40020 within tolerance (define in PRD/tech AC) and latency ≤100 ms.
- Amplitude: `SET AMPLITUDE` updates the effective control signal such that firmware readback and/or output measurement correlates per spec. The test will validate via MODBUS readback and optional analog read.
- System Summary: COUNT (40035) and MASK (40036) remain consistent with per‑unit running flags.

## 4. Errors
- `ERR ARG` → invalid/missing arguments
- `ERR RANGE` → out‑of‑range value
- `ERR STATE` → operation not allowed in current state
- `ERR UNSUPPORTED` → wrapper cannot fulfill the request
- `ERR HW` → hardware fault in wrapper

## 5. Examples
```
> PING
OK PONG
> INFO
OK HIL-Wrapper/0.1.0 2025-08-20
> SET START 1 1
OK
> READ STATUS 1
OK RUN=1 OVL=0 LOCK=1
> READ COUNT
OK COUNT=1
> READ MASK
OK MASK=0x01
> SET OVL 1 1
OK
> READ STATUS 1
OK RUN=1 OVL=1 LOCK=1
> SET RESET 1
OK
```

## 6. Implementation Notes
- Debounce: Wrapper should debounce GPIO actions to avoid unrealistic chatter.
- Frequency Source: Use a timer to generate per‑unit square waves; document accuracy.
- Analog Amplitude: If available, provide DAC/ADC loop to verify amplitude mapping; otherwise respond `ERR UNSUPPORTED`.
- Logging: Wrapper should log last N commands and errors to assist in debugging.

## 7. Test Traceability
- Behave HIL steps will call these commands and assert corresponding MODBUS register effects:
  - Start/Stop → 40005–40008 writes; 40021–40024 bit0 run flag
  - Overload/Reset → 40009–40012 pulses; 40021–40024 bit1
  - Frequency/Lock → 40017–40020 Hz; 40021–40024 bit2
  - System Summary → 40035/40036
  - Amplitude → 40001–40004 or common mapping per PRD

## 8. Open Points (to finalize)
- Exact tolerance for frequency measurement and amplitude mapping per PRD/tech AC.
- Whether amplitude is global or per‑unit in final design; update mapping accordingly.
- Define analog units for `READ ANALOG AMP` (percent vs raw ADC) and calibration flow.
