# Feature Prompt: MODBUS Slave Implementation

## Purpose

Implement the MODBUS RTU slave communication protocol over UART to allow PLCs or HMIs to control and monitor the Multi Sonicator I/O Controller. This module forms the basis of external system integration.

---

### Implementation Steps

1. **UART Configuration**
   - Configure UART for 115200 baud, 8N1 (8 data bits, no parity, 1 stop bit)
   - Use hardware UART interface (avoid software serial)
   - Implement RX interrupt with circular buffer for frame parsing

2. **MODBUS Frame Parsing**
   - Support Function Codes:
     - `0x03` Read Holding Registers
     - `0x10` Write Multiple Registers
   - Handle address decoding, length validation, and error codes

3. **CRC-16 Calculation**
   - Use standard MODBUS CRC-16 algorithm
   - Validate incoming and outgoing frames
   - Implement as optimized static method (e.g., `modbus_crc.h`)

4. **Slave ID Configuration**
   - Define a default Slave ID (e.g., 0x02)
   - Optionally support reading ID from DIP switch or EEPROM

5. **Register Map Stub**
   - Create basic register bank (e.g., `uint16_t modbus_registers[64]`)
   - Map mock data to registers (e.g., ADC value, GPIO states)
   - Connect with actual hardware values in future phases

6. **MODBUS Test Integration**
   - Use a MODBUS master simulator (e.g., ModScan, QModMaster)
   - Validate round-trip communication and CRC handling
   - Simulate read/write cycles to confirm data alignment

---

### Acceptance Criteria

- [ ] MODBUS slave responds to `0x03` and `0x10` requests with valid CRC
- [ ] Slave ID is configurable and respected
- [ ] UART works reliably under load (no frame loss)
- [ ] Circular buffer and RX interrupt are non-blocking
- [ ] Master software can read/write registers correctly

---

### Dependencies

- Hardware interface layer (`2-hardware-interface-implementation.md`)
- Electrical connections validated (`3-db9-interface-configuration.md`)

---

### Estimated Duration

12 hours

---

### Notes

Ensure this implementation is interrupt-safe and memory-efficient. Avoid blocking UART usage. This module will integrate directly with `SystemController` and safety features in later stages.

---

### Next Step

Once complete, mark `4-modbus-slave-implementation.md` as ✅ and proceed to `5-modbus-register-mapping.md`
