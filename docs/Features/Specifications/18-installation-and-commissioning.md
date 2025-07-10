# Feature Prompt: Installation and Commissioning

## Purpose

Guide the field installation, on-site configuration, and acceptance testing process for deploying the Multi-Sonicator I/O Controller at the customer facility. Ensure a smooth, verified transition to production use.

---

### Implementation Steps

1. **Installation Checklist**
   - Mount controller enclosure on DIN rail or wall
   - Connect power (24VDC)
   - Wire DB9 connectors to each sonicator unit
   - Connect MODBUS (RS-485) to PLC or HMI
   - Install antenna (if wireless backup used)

2. **Initial Power-On Test**
   - Validate diagnostic LED pattern on boot
   - Confirm 5V/12V stages are within range
   - Confirm PLC detects MODBUS device with correct Slave ID (should be 2)

3. **Configuration for Production**
   - Set Slave ID (default: 2) and enable channels via config file or DIP switch
   - Test HMI controls for amplitude, start/stop, and reset
   - Confirm correct signal mapping to each sonicator

4. **Site Acceptance Test (SAT)**
   - Run a full operation cycle with all 4 sonicators
   - Trigger one overload and verify detection + lockout
   - Confirm emergency stop behavior
   - Validate alarm logs are visible via HMI and stored to EEPROM

5. **Operator Training and Handoff**
   - Train production operators on start/stop/reset procedures
   - Review alarm behavior and troubleshooting tips
   - Provide printed or digital user manual
   - Deliver USB or cloud link with full documentation

6. **Final Documentation**
   - Record SAT results in `docs/sat-checklist.md`
   - Sign-off sheet with date, time, client approval
   - Capture diagnostic snapshot and backup EEPROM logs

---

### Acceptance Criteria

- [ ] Controller is physically installed and connected
- [ ] All wiring validated and correctly labeled
- [ ] System operates all sonicators and responds to control
- [ ] SAT completed and signed off
- [ ] Operators are trained and have documentation access

---

### Dependencies

- All firmware and hardware validation completed
- Final build from `16` and docs from `17`

---

### Estimated Duration

8 hours (excluding on-site travel/setup)

---

### Notes

Use printed checklist for SAT to confirm every field step. Backup configuration and logs post-installation.

---

### Next Step

Once complete, mark `18-installation-and-commissioning.md` as ✅ and proceed to `19-post-deployment-support.md`
