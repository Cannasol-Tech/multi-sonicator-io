# Feature Prompt: Firebase Integration

## Purpose

Enable cloud synchronization of diagnostics and operational data using Firebase Realtime Database or Firestore. This allows remote monitoring, mobile app interfacing, and long-term data logging.

---

### Implementation Steps

1. **Firebase Schema Design**
   - Example path: `sonicator_data/{device_id}/status`
   - Fields:
     - `uptime`, `power_output`, `overload`, `start_state`, `last_alarm`, etc.
   - Use structured subcollections per unit

2. **Data Forwarding Logic**
   - Implement JSON serialization for current MODBUS or diagnostics state
   - Use UART-to-ESP32 bridge or SPI communication to Firebase-capable microcontroller (e.g. ESP32)
   - Create function `generateFirebasePayload()`

3. **Buffering for Offline Mode**
   - Store latest N snapshots in ring buffer on AVR
   - ESP32 retries upload periodically until buffer clears
   - Timestamp each entry with local uptime + sync correction

4. **Data Compression (Optional)**
   - Encode common values into byte arrays (e.g. `amplitude`, `running`, `overload` → status byte)
   - Use LZ4 or simple delta encoding if payload exceeds limits

5. **Testing Cloud Sync**
   - Create test Firebase project and database path
   - Use serial output to monitor ESP32 push status
   - Confirm updates from hardware appear in Firebase web console

6. **Security Considerations**
   - Use read-only access from AVR side
   - Configure Firebase rules to restrict write access to device ID
   - Ensure secrets stored securely in ESP32 flash

---

### Acceptance Criteria

- [ ] Data payload structure matches schema and contains key metrics
- [ ] ESP32 or gateway uploads new data every N seconds
- [ ] Offline buffering ensures no data loss if connection fails
- [ ] Firebase rules prevent unauthorized writes
- [ ] Data visible and accurate in Firebase dashboard

---

### Dependencies

- Diagnostics and performance data (`10`, `11`)
- MODBUS structure (`5`)
- External ESP32 or Wi-Fi interface assumed

---

### Estimated Duration

8 hours

---

### Notes

The AVR side should not handle Firebase directly. Delegate to ESP32 or intermediary device to reduce firmware complexity. Consider using Firebase SDK in ESP32 with Arduino framework.

---

### Next Step

Once complete, mark `12-firebase-integration.md` as ✅ and proceed to `13-hmi-plc-integration.md`
