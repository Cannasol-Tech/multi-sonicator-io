# Feature Prompt: HMI and PLC Integration

## Purpose

Define and implement the interface between the Multi Sonicator I/O Controller and the customer’s existing HMI/PLC system using MODBUS RTU. This includes control signal mapping, screen design, and sequence automation.

---

### Implementation Steps

1. **PLC Ladder Logic Design**
   - Update existing Velocio PLC (e.g., 1630c) to:
     - Read MODBUS holding registers for each unit’s status
     - Write amplitude and start/stop commands
     - Detect overload and lockout conditions
   - Implement MODBUS polling at 50–100ms interval

2. **HMI Screen Configuration**
   - Display per-unit:
     - Amplitude slider
     - Start/Stop toggle
     - Frequency and power readings
     - Overload indicator
   - Add global emergency stop
   - Show current alarms and allow reset or acknowledgment

3. **Control Sequences**
   - Implement automated sequences:
     - Batch Start (all units with delay)
     - Timed runtime control
     - Overload reset cooldown enforcement

4. **Interface Documentation**
   - Provide MODBUS register list to PLC integrator
   - Include address, data type, units, and scaling
   - Document timing expectations and retry behavior

5. **Test and Validation**
   - Connect live system to PLC
   - Verify all register transactions work bi-directionally
   - Run automation tests with simulated overloads
   - Validate alarm and control synchronization

6. **Fail-Safe Integration**
   - If MODBUS fails:
     - All units must stop
     - HMI should indicate lost communication
     - Add watchdog recovery procedure

---

### Acceptance Criteria

- [ ] All 4 sonicators are controllable from HMI via PLC
- [ ] MODBUS register map is fully utilized and functional
- [ ] HMI shows accurate, real-time data
- [ ] Emergency stop and alarm acknowledgment function via PLC
- [ ] Documentation provided for integrator

---

### Dependencies

- MODBUS implementation (`4`, `5`)
- Orchestration system (`7`)
- Alarm manager (`9`)

---

### Estimated Duration

10 hours

---

### Notes

This is the final integration for local industrial use. Future Firebase/cloud control should remain logically separate from HMI/PLC pathways.

---

### Next Step

Once complete, mark `13-hmi-plc-integration.md` as ✅ and proceed to `14-testing-framework.md`
