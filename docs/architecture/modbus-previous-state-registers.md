---
title: MODBUS Previous-State Read-Only Registers
audience: PLC/HMI Integration Team
status: Draft (API stabilized for Story 4.1)
updated: 2025-09-15
---

Overview
- Purpose: Expose diagnostic context from before the last shutdown without ever auto‑resuming operation.
- Safety: On any power-up, master=IDLE and all units=STOPPED. Previous-state values are diagnostic only.
- Access: All registers below are read‑only via MODBUS.

System Registers
- 0x0005 PREV_ACTIVE_MASK
  - Type: uint16 (bit0..bit3 map to units 1..4)
  - Meaning: Bitmask of units that were running prior to the last shutdown snapshot
- 0x0006 LAST_SHUTDOWN_REASON
  - Type: uint16 (enum)
  - Values: 0=normal, 1=watchdog, 2=power‑loss, 3=e‑stop, 4=fault

Per‑Unit Registers (for unit N=1..4)
- Addressing: Base block for unit N is 0x0100 + (N-1)*0x20
  - 0xN14 PREV_STATE
    - Type: uint16 (enum sonicator_state_t)
    - Values: 0=STOPPED, 1=STARTING, 2=RUNNING, 3=STOPPING, 4=OVERLOAD, 5=FAULT, 6=UNKNOWN
  - 0xN15 PERSISTED_AMPLITUDE
    - Type: uint16 (percent 0..100). For diagnostics; enforcement remains in control path.
  - 0xN16 LAST_FAULT_CODE
    - Type: uint16 (project‑specific fault codes)
  - 0xN17 LAST_STATE_TIMESTAMP_LO
    - Type: uint16 (low 16 bits of a monotonic timestamp); high word reserved for future use

Behavioral Notes
- Snapshot timing: The firmware snapshots previous‑state context on emergency stop and on global disable.
- Power‑up: Units do not resume automatically. Operators must explicitly command start per safety policy.
- Write attempts: Writes to these addresses are ignored by the slave (treated as read‑only).

PLC/HMI Usage Examples
- Display last known running units:
  - Read 0x0005; show each unit as previously active if the bit is set.
- Show last shutdown reason in alarms/history:
  - Read 0x0006; map enum to text.
- Per‑unit detail page (Unit 2):
  - Read 0x0134 (PREV_STATE) and 0x0135 (PERSISTED_AMPLITUDE) for context.

Traceability
- Story: docs/agile/stories/4.1.multi-unit-state-management.md (AC‑11)
- Canonical addresses: include/register_map.h
- Firmware structs: src/modules/communication/modbus_registers.h

Cautions
- Do not use these diagnostics to drive outputs automatically.
- Register layout may expand (reserved slots remain read‑only).

