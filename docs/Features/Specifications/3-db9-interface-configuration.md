# Feature Prompt: DB9 Interface Configuration

## Purpose

Map and validate the electrical connections and signal conditioning logic between the ATmega32A controller and each of the four CT2000 sonicators via DB9 ports. Ensure proper isolation, voltage compatibility, and robust signal integrity.

---

### Implementation Steps

1. **DB9 Pin Mapping**
   - Document each pin function per sonicator (Inputs: Overload, Freq Lock, etc.; Outputs: Amplitude, Start, Reset)
   - Create a `db9_map.h` with all signal definitions (4 channels × 9 pins)

2. **Input Signal Conditioning**
   - Configure opto-isolator or voltage divider circuits for 24V/12V input tolerance
   - Validate pull-down and filtering components are functional
   - Use oscilloscope to verify clean transitions

3. **Output Driver Configuration**
   - Drive signals through open-drain or push-pull buffers as required
   - Ensure amplitude output is routed through DAC→op-amp→DB9
   - Validate 0–10V range with multimeter

4. **Isolation Tests**
   - Check isolation between each DB9 port
   - Validate no cross-talk between sonicator control lines

5. **Timing and Edge Validation**
   - Verify timing alignment with oscilloscope
   - Confirm sonicator reacts to control signals with expected latency

---

### Acceptance Criteria

- [ ] All DB9 signals mapped and documented in `db9_map.h`
- [ ] Input voltages are safely clamped and readable by MCU
- [ ] Output drivers meet electrical spec of CT2000
- [ ] Isolation confirmed between all channels
- [ ] Verified signal integrity and timing under load

---

### Dependencies

- Hardware interface layer (`2-hardware-interface-implementation.md`)

---

### Estimated Duration

6 hours

---

### Notes

Each DB9 connector must be fully modular, supporting independent control. All signal lines should be routed through protection circuits. This file should also feed into MODBUS register mapping for traceability.

---

### Next Step

Once complete, mark `3-db9-interface-configuration.md` as ✅ and proceed to `4-modbus-slave-implementation.md`
