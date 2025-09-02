# Story: Single Sonicator Control Implementation

## Status: Ready for Development

## Story

As a **sonicator operator**,
I want **control and monitoring capabilities for a single CT2000 sonicator unit**,
So that **I can start, stop, adjust amplitude, and monitor the unit's performance remotely**.

## Epic Context

### Epic 1: Foundational Control & Communication

Goal: Establish core firmware structure, basic sonicator control, and MODBUS communication layer

## Story Context

### Technology Stack

- ATmega32A GPIO and PWM peripherals for sonicator interface
- CT2000 sonicator unit communication protocol
- C/C++ embedded control algorithms
- MODBUS register integration for remote control

### Integration Points

- HAL GPIO/PWM/ADC interfaces for hardware control
- MODBUS register map for remote access
- CT2000 sonicator hardware interface (DB9 connector)
- Future multi-sonicator orchestration

## Acceptance Criteria

### Control Requirements

1. Start/Stop control functional via MODBUS register 0x0100
2. Amplitude control (20-100%) functional via MODBUS register 0x0101
3. Amplitude setpoint clamped to valid range (20-100%) per FR3
4. Overload reset command functional via MODBUS register 0x0102
5. Safe startup state (OFF) on power-up per FR10

### Monitoring Requirements

6. Real-time power measurement (Watts) available in MODBUS register 0x0110
7. Operating frequency monitoring (Hz) available in MODBUS register 0x0111  
8. Status flags (Overload, Frequency Lock, COMM_FAULT) in MODBUS register 0x0112
9. Telemetry data updated at minimum 10Hz per NFR9
10. Communication fault detection when sonicator disconnected per FR7

### Performance Requirements

11. Command latency <200ms per NFR1
12. State changes reflected in MODBUS registers within 100ms per NFR2
13. Amplitude control accuracy within ±2% of setpoint
14. Safe shutdown on communication loss >1 second per FR11

## Technical Implementation Details

### Sonicator Control Module (`modules/control/sonicator.h`)

```c
typedef enum {
    SONICATOR_STATE_OFF,
    SONICATOR_STATE_STARTING,
    SONICATOR_STATE_RUNNING,
    SONICATOR_STATE_STOPPING,
    SONICATOR_STATE_OVERLOAD,
    SONICATOR_STATE_COMM_FAULT
} sonicator_state_t;

typedef struct {
    uint8_t unit_id;               // Sonicator unit identifier (1-4)
    sonicator_state_t state;       // Current operational state
    uint8_t amplitude_setpoint;    // Amplitude setpoint (20-100%)
    uint8_t amplitude_actual;      // Actual amplitude reading
    uint16_t power_watts;          // Power consumption in Watts
    uint16_t frequency_hz;         // Operating frequency in Hz
    bool overload_flag;            // Overload condition detected
    bool frequency_lock;           // Frequency lock achieved
    bool comm_fault;               // Communication fault with unit
    uint32_t last_update_ms;       // Last telemetry update timestamp
} sonicator_t;

// Core control functions
void sonicator_init(sonicator_t* unit, uint8_t unit_id);
void sonicator_start(sonicator_t* unit);
void sonicator_stop(sonicator_t* unit);
void sonicator_set_amplitude(sonicator_t* unit, uint8_t amplitude);
void sonicator_reset_overload(sonicator_t* unit);
void sonicator_update_telemetry(sonicator_t* unit);
bool sonicator_is_active(const sonicator_t* unit);
```

### Hardware Interface Implementation

#### Pin Assignments (per pin matrix documentation)

```c
// Sonicator #1 interface pins
#define SONICATOR1_START_PIN    PC0  // Start/Stop control output
#define SONICATOR1_AMPLITUDE_PIN PC1  // PWM amplitude control output  
#define SONICATOR1_RESET_PIN    PC2  // Overload reset pulse output
#define SONICATOR1_POWER_ADC    0    // ADC channel for power monitoring
#define SONICATOR1_FREQ_INPUT   PD3  // Frequency feedback input
#define SONICATOR1_STATUS_INPUT PD4  // Status/overload input
```

#### Control Signal Generation

- **Start/Stop**: Digital output (HIGH = start, LOW = stop)
- **Amplitude**: PWM signal with RC filtering for analog voltage
- **Overload Reset**: Pulse output (100ms pulse width)
- **Power Monitoring**: ADC reading with voltage-to-watts conversion
- **Frequency Measurement**: Input capture for frequency counting
- **Status Monitoring**: Digital input for overload/status flags

### State Machine Implementation

```c
void sonicator_state_machine(sonicator_t* unit) {
    switch (unit->state) {
        case SONICATOR_STATE_OFF:
            // Handle start command
            break;
        case SONICATOR_STATE_STARTING:
            // Wait for frequency lock
            break;
        case SONICATOR_STATE_RUNNING:
            // Monitor operation and handle commands
            break;
        case SONICATOR_STATE_STOPPING:
            // Graceful shutdown sequence
            break;
        case SONICATOR_STATE_OVERLOAD:
            // Handle overload condition
            break;
        case SONICATOR_STATE_COMM_FAULT:
            // Handle communication fault
            break;
    }
}
```

### MODBUS Integration

- Register 0x0100: Start/Stop control (0 = stop, 1 = start)
- Register 0x0101: Amplitude setpoint (20-100%)
- Register 0x0102: Overload reset (write 1 to reset)
- Register 0x0110: Power reading (Watts)
- Register 0x0111: Frequency reading (Hz)
- Register 0x0112: Status flags (bit-packed)

## Definition of Done

- [ ] Single sonicator start/stop control functional
- [ ] Amplitude control operational with proper clamping
- [ ] Overload reset mechanism working
- [ ] Real-time power monitoring accurate
- [ ] Frequency monitoring operational
- [ ] Status flag detection working
- [ ] MODBUS register integration complete
- [ ] State machine handles all operational scenarios
- [ ] Telemetry updates at required 10Hz rate
- [ ] Command latency verified <200ms
- [ ] Unit tests achieve >90% coverage
- [ ] **HIL verification: Complete sonicator control validated with actual CT2000 unit**
- [ ] **HIL verification: Amplitude control accuracy verified (±2% of setpoint)**
- [ ] **HIL verification: Power monitoring accuracy validated with test equipment**
- [ ] **HIL verification: Frequency measurement accuracy verified**
- [ ] **HIL verification: State machine transitions tested under real conditions**
- [ ] **HIL verification: MODBUS integration tested with external master**
- [ ] **HIL verification: Safety features tested (communication loss, overload)**
- [ ] Hardware validation on actual CT2000 unit
- [ ] Communication fault detection tested
- [ ] Code review completed for safety compliance

## Dependencies

### Prerequisites

- Epic 1 Story 0: HIL Testing Framework & Hardware Verification completed
- Epic 1 Story 2: Hardware Abstraction Layer (HAL) Implementation completed
- Epic 1 Story 3: MODBUS RTU Communication Framework completed
- CT2000 sonicator unit available for testing
- Pin matrix documentation finalized

### Enables Next Stories

- Epic 2: Multi-sonicator management and coordination
- Epic 2: Safety features and watchdog implementation
- Epic 2: Advanced HIL testing and system validation

## Risk Assessment

**Primary Risk:** CT2000 interface protocol and timing requirements unknown or complex
**Mitigation:** Early hardware testing and protocol validation with CT2000 unit
**Verification:** Successful control and monitoring of actual sonicator hardware

**Secondary Risk:** Amplitude control accuracy difficult to achieve within ±2% requirement
**Mitigation:** Calibration procedures and feedback control implementation
**Verification:** Precision measurement validation with test equipment

## Notes

- Focus on single-unit operation before multi-unit complexity
- All timing requirements must be validated with real hardware
- State machine must handle all edge cases and fault conditions
- Implementation must support future extension to multiple units
- Safety considerations paramount for industrial equipment control
