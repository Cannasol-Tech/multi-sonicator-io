# Feature Prompt: MODBUS Register Mapping

## Purpose

Define the full MODBUS register map for the Multi Sonicator I/O Controller to expose hardware states, control values, and diagnostics through MODBUS RTU interface. This enables seamless integration with HMI, PLC, or SCADA systems.

---

### Implementation Steps

1. **Define Register Layout**
   - Reserve address blocks for each unit:
     - Unit 1: 0x0000–0x001F
     - Unit 2: 0x0020–0x003F
     - Unit 3: 0x0040–0x005F
     - Unit 4: 0x0060–0x007F

2. **Input Register Mapping (Read Holding Registers: `0x03`)**
   - Frequency lock status
   - Overload status
   - Frequency output (Hz, scaled)
   - Power output (W, scaled)
   - Internal temperature (optional)

3. **Output Register Mapping (Write Multiple Registers: `0x10`)**
   - Amplitude command (0–1000 scaled to 0–10V)
   - Start/stop command (bitmask or flag)
   - Overload reset flag
   - Slave enable

4. **Scaling Strategy**
   - Use 1 register (16-bit) per field
   - Float values should be scaled by fixed factor (e.g., x10 or x100)
   - Document all units and scale in `modbus_map.h`

5. **Diagnostics Block (Shared)**
   - System uptime (seconds)
   - Error code
   - Last fault timestamp
   - Slave ID

6. **Create Register Map Header**
   - File: `include/modbus_map.h`
   - Define offsets and field names as macros or enums
   - Example: `#define REG_U1_FREQ_OUT 0x0002`

7. **Documentation**
   - Create `docs/modbus_registers.md`
   - Tabular format with: Address, Label, Description, Type, Units, Scale
   - Include CRC example transaction for reference

---

### Acceptance Criteria

- [ ] All sonicator fields are mapped and addressable via MODBUS
- [ ] Register definitions are centralized in `modbus_map.h`
- [ ] HMI or simulator can read/write all values
- [ ] Scaling and units are clearly documented
- [ ] Values match real-world measurements under test

---

### Dependencies

- MODBUS protocol implementation (`4-modbus-slave-implementation.md`)
- Hardware interface values (`2-hardware-interface-implementation.md`)

---

### Estimated Duration

6 hours

---

### Notes

All future system modules (e.g., `SystemController`, `DiagnosticsSystem`) must write their outputs into this register bank. This is the shared communication layer with the outside world.

---

### Next Step

Once complete, mark `5-modbus-register-mapping.md` as ✅ and proceed to `6-sonicator-control-module.md`
