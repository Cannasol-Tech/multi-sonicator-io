# Multi-Sonicator I/O Controller - Project Summary

**Project Lead:** Stephen Boyett  
**Hardware Advisor:** Josh (CEO)  
**Company:** Cannasol Technologies  
**Document Date:** December 2024  
**Target Delivery:** Initial prototype - July 2025  

---

## Executive Summary

The Multi-Sonicator I/O Controller project extends Cannasol Technologies' proprietary automation system to control up to 4 CT2000 sonicators simultaneously. This industrial-grade controller replaces single-sonicator control with a scalable multi-unit system, enabling increased production capacity for nano-emulsion processing in the cannabis and nutraceutical industries.

**Key Deliverables:**

- ATmega32A-based controller with DB9 serial communication
- Individual CT2000 sonicator control interfaces (up to 4 units)
- Integration with existing Velocio 1630c PLC and HMI system
- Mobile app and HMI extensions for multi-sonicator monitoring
- Industrial-grade enclosure with DIN rail mounting

**Project Value:**

- Increases production capacity 4x (single → multiple sonicators)
- Leverages existing automation infrastructure investment
- Enables premium add-on product sales for Cannasol
- Maintains industrial reliability and compliance standards

---

## Technical Architecture

### System Overview

```text
┌─────────────────┐
│ Mobile App      │
│ (Remote Control)│
└─────────────────┘
         ▲
         │ HTTP/SSL
         ▼
┌─────────────────┐
│ Firebase Cloud  │
│ Database        │
└─────────────────┘
         ▲
         │ HTTP/SSL
         ▼
┌─────────────────┐
│ ESP32 WiFi      │
│ Adapter         │
└─────────────────┘
         ▲
         │ MODBUS RTU
         ▼
┌─────────────────┐
│ HMI System      │
│ (MODBUS Master) │
└─────────────────┘
         │
         │ MODBUS RTU Serial - Daisy Chain
         ▼
   ══════════════════════════════════════════════════════════════════
   ║                    MODBUS RTU Daisy Chain                      ║
   ══════════════════════════════════════════════════════════════════
              │                                      │
              ▼                                      ▼
   ┌─────────────────┐                ┌─────────────────────────────────────┐
   │ Velocio 1630c   │                │ Multi-Sonicator I/O Controller      │
   │ PLC             │                │ (ATmega32A)                         │
   │ (Slave ID = 1)  │                │ (Slave ID = 2)                      │
   └─────────────────┘                │                                     │
                                      │ ┌─────────────────────────────────┐ │
                                      │ │ DB9 #1  DB9 #2  DB9 #3  DB9 #4 │ │
                                      │ │  (to)   (to)    (to)    (to)   │ │
                                      │ │ Son#1   Son#2   Son#3   Son#4   │ │
                                      │ └─────────────────────────────────┘ │
                                      └─────────────────────────────────────┘
                                                │      │      │      │
                                                ▼      ▼      ▼      ▼
                                      ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌─────────────┐
                                      │ CT2000      │ │ CT2000      │ │ CT2000      │ │ CT2000      │
                                      │ Sonicator#1 │ │ Sonicator#2 │ │ Sonicator#3 │ │ Sonicator#4 │
                                      └─────────────┘ └─────────────┘ └─────────────┘ └─────────────┘
```

### Physical Controller Layout

#### **Front Panel Design**

```text
┌─────────────────────────────────────────────────────────────────────┐
│                  Cannasol Multi Sonicator Controller                │
│                         Model: CAN-SON-CTRL-4                       │
│                                                                     │
│  [PWR]  [RUN]  [ERR]  [COMM]                    [STATUS INDICATORS] │
│                                                                     │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐│
│  │ SONICATOR 1 │  │ SONICATOR 2 │  │ SONICATOR 3 │  │ SONICATOR 4 ││
│  │             │  │             │  │             │  │             ││
│  │    DB9      │  │    DB9      │  │    DB9      │  │    DB9      ││
│  │   Female    │  │   Female    │  │   Female    │  │   Female    ││
│  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘│
│                                                                     │
│  [1] [2] [3] [4]   Individual Sonicator Status LEDs                │
│                                                                     │
│  Serial: ____________                              [PROGRAMMING]    │
│                                                       [PORT]        │
└─────────────────────────────────────────────────────────────────────┘
```

#### **Back Panel Design**

```
┌─────────────────────────────────────────────────────────────────────┐
│                         ┌─────────────┐    ┌─────────────┐          │
│                         │ SERIAL COMM │    │   COOLING   │          │
│                         │             │    │    VENTS    │          │
│                         │ TO MAIN     │    │             │          │
│                         │ AUTO BOX    │    │ ┌─┐ ┌─┐ ┌─┐ │          │
│                         │   DB9 Male  │    │ │ │ │ │ │ │ │          │
│                         │(Comm+Power) │    │ └─┘ └─┘ └─┘ │          │
│                         └─────────────┘    └─────────────┘          │
│                                                                     │
│  Model: CAN-SON-CTRL-4    Cannasol Technologies    Made in USA      │
└─────────────────────────────────────────────────────────────────────┘
```

### Hardware Specifications

#### **Microcontroller Platform**

- **MCU:** ATmega32A-PU (DIP-40 package)
- **Clock Source:** External 16MHz crystal (CTS ATS16A-E, ±20ppm)
- **Programming:** Arduino UNO R4 WiFi as ISP programmer
- **Development:** PlatformIO with Arduino framework

#### **CT2000 Sonicator Interface Specifications**

Based on the CT2000 manual analysis and expanded control requirements:

| Parameter | Specification | Interface Method | Current vs New | Verification Needed |
|-----------|---------------|------------------|----------------|-------------------|
| **Operating Frequency** | 20kHz ±0.1% | Pin 4: Frequency Output ÷ 10 | ✓ Monitor → **✓ Monitor** | ✓ Confirmed |
| **Power Output** | 2000W maximum | Pin 5: Power Output (5.44mV/watt) | ✗ Not used → **✓ Monitor** | ✓ Confirmed |
| **Amplitude Control** | 20-100% (0-10VDC) | Pin 8: Amplitude Control Input | ✗ Not used → **✓ Control** | **⚠️ Verify internal switch** |
| **Sonics Start Control** | Switch closure to ground | Pin 7: Sonics Start Input | ✓ Control → **✓ Control** | ✓ Confirmed |
| **Overload Indication** | Open collector output | Pin 1: Overload Indication | ✓ Monitor → **✓ Monitor** | ✓ Confirmed |
| **Overload Reset** | Switch closure to ground | Pin 2: Overload Reset Control | ✗ Not used → **✓ Control** | **⚠️ Verify availability** |
| **Frequency Lock** | Open collector output | Pin 3: Frequency Lock Output | ✓ Monitor → **✓ Monitor** | ✓ Confirmed |

**Critical Verification Requirements:**

- **Pin 2 (Overload Reset)**: Confirm this pin is actually functional and available for overload reset control
- **Pin 8 (Amplitude Control)**: Verify existence and operation of internal switch to enable external amplitude control

#### **DB9 Interface Implementation**

```cpp
// Enhanced CT2000 Interface Mapping - Full Control & Monitoring
struct CT2000Interface {
    // Input signals (from sonicator to controller) - MONITORING
    static constexpr uint8_t OVERLOAD_INDICATION = 1;    // Open collector, active low
    static constexpr uint8_t FREQUENCY_LOCK = 3;         // Open collector, active low  
    static constexpr uint8_t FREQUENCY_OUTPUT = 4;       // Square wave, frequency ÷ 10
    static constexpr uint8_t POWER_OUTPUT = 5;           // Analog, 5.44mV per watt
    
    // Output signals (from controller to sonicator) - CONTROL
    static constexpr uint8_t OVERLOAD_RESET = 2;         // Switch closure to ground
    static constexpr uint8_t SONICS_START = 7;           // Switch closure to ground
    static constexpr uint8_t AMPLITUDE_CONTROL = 8;      // 0-10VDC (20-100% amplitude)
    
    // Reference grounds
    static constexpr uint8_t SIGNAL_GROUND = 6;          // Signal reference
    static constexpr uint8_t EMITTER_COMMON = 9;         // Open collector reference
};
```

#### **Power Supply Architecture**

- **Input Source:** Main Automation Box internal power supply via DB9 connection
- **Input Voltage:** +24VDC from Main Box power supply (Pin 1) with ground return (Pin 4)
- **Local Regulation:** 24V → 12V (LM2596) → 5V (LM7805) cascade regulation inside controller
- **Power Budget:** 200mA @ 5V = 1.0W total system (42mA @ 24V input)
- **Protection:** Input fusing, TVS diodes, EMI filtering
- **Advantages:** Single cable solution, uses existing infrastructure, no separate power supply needed

### ATmega32A Pin Allocation Diagram

```text
                    ATmega32A-PU (DIP-40) Pin Assignment
                         ┌─────────┐
                    PB0──┤1    40├──PA0 (ADC0) - Reserved
                    PB1──┤2    39├──PA1 (ADC1) - Reserved  
                    PB2──┤3    38├──PA2 (ADC2) - Reserved
                    PB3──┤4    37├──PA3 (ADC3) - Reserved
             RESET──┤5    36├──PA4 (ADC4) - Sonicator 1 Power Monitor
                   VCC──┤6    35├──PA5 (ADC5) - Sonicator 2 Power Monitor
                   GND──┤7    34├──PA6 (ADC6) - Sonicator 3 Power Monitor
                 XTAL2──┤8    33├──PA7 (ADC7) - Sonicator 4 Power Monitor
                 XTAL1──┤9    32├──AREF
         PWM Filter 1──┤10   31├──GND
         PWM Filter 2──┤11   30├──AVCC
           Son1_OverLD──┤12   29├──PC7 - Son4_FreqLock
          Son1_FreqOut──┤13   28├──PC6 - Son4_OverLD  
           Son1_FreqLk──┤14   27├──PC5 - Son3_FreqLock
        Son1_StartCtrl──┤15   26├──PC4 - Son3_FreqOut
          Son2_OverLD──┤16   25├──PC3 - Son3_OverLD
         Son2_FreqOut──┤17   24├──PC2 - Son3_StartCtrl
          Son2_FreqLk──┤18   23├──PC1 - PWM Filter 4
       Son2_StartCtrl──┤19   22├──PC0 - PWM Filter 3
              VCC──┤20   21├──GND
                         └─────────┘

Pin Usage Summary:
- UART (MODBUS): PD0 (RX), PD1 (TX) = 2 pins
- PWM Outputs: PB3, PD5, PD6, PD7 (mapped to PC0,PC1,PD5,PD6,PD7) = 4 pins  
- Crystal: XTAL1, XTAL2 = 2 pins
- Power/Ground: VCC, GND, AVCC, AREF = 4 pins
- Reset: RESET = 1 pin
- Sonicator Interfaces: 24 pins (6 pins × 4 sonicators)
- Built-in ADC: PA4-PA7 for power monitoring = 4 pins
- TOTAL: 39 pins used out of 40 available (1 pin unused)

Interface Pin Mapping:
Son1: PD2(OverLD), PD3(FreqOut), PD4(FreqLk), PD5(Start), PB3(PWM), PA4(ADC)
Son2: PD6(OverLD), PD7(FreqOut), PB0(FreqLk), PB1(Start), PD5(PWM), PA5(ADC)  
Son3: PC3(OverLD), PC4(FreqOut), PC5(FreqLk), PC2(Start), PD6(PWM), PA6(ADC)
Son4: PC6(OverLD), PB2(FreqOut), PC7(FreqLk), PB3(Start), PD7(PWM), PA7(ADC)
```

```cpp
// Inside the Multi-Sonicator I/O Controller enclosure:
class MultiSonicatorController {
private:
    ATmega32A microcontroller;           // Main processing unit
    
    // Four separate interface circuits on PCB
    SonicatorInterface sonicator1;       // Connected to front panel DB9 #1
    SonicatorInterface sonicator2;       // Connected to front panel DB9 #2  
    SonicatorInterface sonicator3;       // Connected to front panel DB9 #3
    SonicatorInterface sonicator4;       // Connected to front panel DB9 #4
    
    // Single communication interface
    ModbusSerial main_automation_comm;   // Connected to back panel DB9
    
    // Power supply and regulation (via DB9 from Main Automation Box)
    PowerRegulation local_regulation;    // DB9 24V input → 12V → 5V cascade
    
    // Status and diagnostics
    StatusIndicators front_panel_leds;   // Power, run, error, comm LEDs
    SafetyMonitor safety_systems;        // Emergency stop and fault protection
};
```

#### **Physical Installation Diagram**

```text
Industrial Control Cabinet:
┌─────────────────────────────────────────────────────────────────────┐
│  ┌─────────────────┐                                                │
│  │ Main Automation │                                                │
│  │ Box             │                                                │
│  │ • Velocio PLC   │                                                │
│  │ • HMI System    │                                                │
│  │ • ESP32 WiFi    │         ┌─────────────────────────────────────┐│
│  │ • 24VDC Supply  │◄────────┤ Multi-Sonicator I/O Controller     ││
│  │ [Spare DB9]     │ DB9 Cable│ Model: CAN-SON-CTRL-4              ││
│  └─────────────────┘(Comm+Pwr)│                                     ││
│                               │ DIN Rail Mounted                   ││
│                               └─────────────────────────────────────┘│
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
                                │
                                │ 4× DB9 Cables to Sonicators
                                ▼
┌─────────────────────────────────────────────────────────────────────┐
│                     Production Floor Layout                         │
│                                                                     │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐             │
│  │ CT2000      │    │ CT2000      │    │ CT2000      │             │
│  │ Sonicator 1 │    │ Sonicator 2 │    │ Sonicator 3 │             │
│  │             │    │             │    │             │             │
│  │ [DB9 Input] │    │ [DB9 Input] │    │ [DB9 Input] │             │
│  └─────────────┘    └─────────────┘    └─────────────┘             │
│                                                                     │
│                          ┌─────────────┐                           │
│                          │ CT2000      │                           │
│                          │ Sonicator 4 │                           │
│                          │             │                           │
│                          │ [DB9 Input] │                           │
│                          └─────────────┘                           │
└─────────────────────────────────────────────────────────────────────┘
```

#### **Cable Connection Summary**

| Connection | Cable Type | Length | Purpose |
|------------|------------|--------|---------|
| **Controller ↔ Main Automation Box** | DB9 Serial, Shielded | 1-3 meters | MODBUS RTU Communication + Power |
| **Controller ↔ Sonicator 1-4** | DB9 Extension, Shielded | 5-15 meters | Individual CT2000 Control |

#### **Enclosure Mounting Options**

```text
Option 1: DIN Rail Mount (Recommended)
┌─────────────────────────────────────────┐
│ ┌─────────────────────────────────────┐ │
│ │    Multi Sonicator Controller      │ │
│ │    Model: CAN-SON-CTRL-4           │ │
│ └─────────────────────────────────────┘ │
│ ═══════════════════════════════════════ │ ← DIN Rail
│                                         │
│ Other DIN Rail Mounted Equipment:       │
│ • Power Supplies                        │
│ • Terminal Blocks                       │
│ • Other Controllers                     │
└─────────────────────────────────────────┘

Option 2: Panel Mount
┌─────────────────────────────────────────┐
│                                         │
│        [4 Mounting Holes]               │
│ ┌─────────────────────────────────────┐ │
│ │    Multi Sonicator Controller      │ │
│ │    Model: CAN-SON-CTRL-4           │ │
│ └─────────────────────────────────────┘ │
│        [4 Mounting Holes]               │
│                                         │
└─────────────────────────────────────────┘
```

### Software Architecture

#### **Enhanced Software Architecture**

```cpp
// Main system classes with expanded CT2000 control capabilities
class SonicatorInterface {
    // Full CT2000 control and monitoring (instantiated 4x)
    
    // CONTROL FUNCTIONS (New capabilities)
    void setAmplitude(uint8_t percentage);        // 20-100% via 0-10VDC output
    void startSonication();                       // Switch closure to Pin 7
    void stopSonication();                        // Open circuit to Pin 7
    void resetOverload();                         // Pulse switch closure to Pin 2
    
    // MONITORING FUNCTIONS (Enhanced from current system)
    bool isOverloaded();                          // Read Pin 1 (open collector)
    bool isFrequencyLocked();                     // Read Pin 3 (open collector)
    uint16_t getCurrentFrequency();               // Measure Pin 4 (freq ÷ 10)
    uint16_t getCurrentPowerOutput();             // Read Pin 5 (5.44mV/watt)
    
    // STATUS REPORTING (For MODBUS registers)
    SonicatorStatus getCompleteStatus();          // All parameters in one struct
    void updateControlOutputs();                  // Apply control commands
    bool performSelfDiagnostic();                // Verify interface integrity
};

// Enhanced status structure for complete monitoring
struct SonicatorStatus {
    bool is_running;                              // Sonics start state
    bool is_overloaded;                           // Overload condition
    bool is_frequency_locked;                     // Frequency lock status
    uint8_t amplitude_setpoint;                   // Commanded amplitude %
    uint16_t actual_frequency_hz;                 // Measured frequency
    uint16_t actual_power_watts;                  // Measured power output
    uint32_t total_runtime_seconds;               // Accumulated runtime
    uint32_t overload_count;                      // Number of overload events
};
```

#### **Enhanced MODBUS Register Mapping**

| Register | Function | Access | Data Type | Description |
|----------|----------|--------|-----------|-------------|
| **CONTROL REGISTERS** |
| 40001-40004 | Sonicator 1-4 Amplitude Control | R/W | uint16 (20-100%) | Set amplitude percentage |
| 40005-40008 | Sonicator 1-4 Start/Stop | R/W | bool | Start/stop sonication |
| 40009-40012 | Sonicator 1-4 Overload Reset | W | bool | Pulse to reset overload |
| **MONITORING REGISTERS** |
| 40013-40016 | Sonicator 1-4 Power Output | R | uint16 (watts) | Actual power measurement |
| 40017-40020 | Sonicator 1-4 Frequency | R | uint16 (Hz) | Actual frequency measurement |
| 40021-40024 | Sonicator 1-4 Status Flags | R | uint16 (bitfield) | Overload, freq lock, running |
| **SYSTEM REGISTERS** |
| 40025 | System Status | R | uint16 (bitfield) | Overall system health |
| 40026 | Emergency Stop | R/W | bool | Master emergency stop |
| 40027-40030 | Runtime Counters | R | uint32 | Individual sonicator runtime |
| 40031-40034 | Overload Counters | R | uint16 | Overload event counts |

| 40035 | Active Sonicator Count | R | uint16 (0-4) | Number of sonicators currently running |
| 40036 | Active Sonicator Mask | R | uint16 (bits 0-3) | Bitmask: S1..S4 running = 1 |

##### Integrator Notes (Velocio/HMITool)

- Addressing: Docs use holding-register style (4xxxx). Some dialogs expect 0-based raw offset (e.g., 40001 → offset 0). Confirm HMITool field type and enter accordingly.
- RTU Settings: 115200 8N1, Slave ID = 2.
- Suggested Polling:
  - Power/Frequency (40013–40020): 5–10 Hz
  - Status/Mask/Count (40021–40024, 40035–40036): 10–20 Hz
  - Runtimes (40027–40030): 1 Hz
- Write Semantics:
  - Start/Stop (40005–40008): 0=Stop, 1=Start (level-based)
  - Overload Reset (40009–40012): Pulse 1 then 0 (one-shot)
- Frequency Scaling: CT2000 pin 4 provides freq ÷ 10. Registers report Hz; HMI display should show Hz (internal scaling handled in firmware).
- Latency Targets: Register reflection ≤100 ms; E‑stop/Overload reset path ≤100 ms end-to-end.

#### **Status Flags Bitfield Definition**

```cpp
// Sonicator Status Flags (Registers 40021-40024)
struct StatusFlags {
    uint16_t running        : 1;  // Bit 0: Sonication active
    uint16_t overloaded     : 1;  // Bit 1: Overload condition
    uint16_t freq_locked    : 1;  // Bit 2: Frequency lock achieved
    uint16_t amplitude_fault: 1;  // Bit 3: Amplitude control fault
    uint16_t comm_fault     : 1;  // Bit 4: CT2000 communication fault
    uint16_t reserved       : 11; // Bits 5-15: Reserved for future use
};
```

---

## Hardware Implementation

### PCB Design Specifications

- **Layer Count:** 4-layer professional PCB
- **Dimensions:** 100mm × 80mm (fits standard industrial enclosure)
- **Layer Stack:** Signal/Ground/Power/Signal
- **Crystal Layout:** <10mm traces, ground guard, proper decoupling
- **Connector Types:** Industrial locking DB9, screw terminals

### Component Selection

| Category | Component | Part Number | Specifications | New Requirement |
|----------|-----------|-------------|----------------|-----------------|
| **Crystal** | 16MHz External | CTS ATS16A-E | ±20ppm, HC-49S, 20pF load | Same |
| **Load Caps** | 22pF Ceramic | Generic C0G | ±5%, 50V rating | Same |
| **Power Reg** | Buck Converter | LM2596 Module | 24V→12V, 3A capacity | Same |
| **Linear Reg** | 5V Regulator | LM7805 | 12V→5V, 1A capacity | Same |
| **PWM Filter** | 1kΩ Resistor | CF14JT1K00CT-ND | 1/4W, 5% | **4x for PWM filters** |
| **PWM Filter** | 10µF Capacitor | 445-173297-1-ND | 50V, X7R ceramic | **4x for PWM filters** |
| **Op-Amps** | Dual Op-Amp | LM358N × 2 | 0-10V amplitude scaling | **2 units for PWM scaling** |
| **Gain Resistors** | 10kΩ 1% | S10KQCT-ND | For op-amp gain setting | **8x for precise gain** |
| **Freq Counter** | Timer Input | ATmega32A Timer1 | Frequency measurement (Pin 4) | **Built-in frequency monitoring** |
| **Optocouplers** | High-speed | 6N137 × 8 | Signal isolation | **8 units: 4 inputs + 4 outputs** |
| **Relay Drivers** | Low-side | ULN2003 × 2 | Overload reset control | **Pin 2 control** |
| **Connectors** | DB9 Male | Standard × 4 | Industrial grade | **4 sonicator interfaces** |
| **ISP Programmer** | AVR Programmer | Arduino Uno R4 WiFi as ISP (primary) or USBasp (alternative) | In-system programmer | **Development and production programming** |

### Programming Options

#### **Primary Option: Arduino UNO R4 WiFi as ISP (Development)**

```markdown
Hardware Required:
- Arduino UNO R4 WiFi (already available in development setup)
- Load ArduinoISP sketch onto Arduino
- 10µF capacitor for reset bypass during programming

Advantages:
- Uses existing development hardware
- No additional cost for prototyping
- Well-documented Arduino framework integration
- Supported by PlatformIO out of the box
- Good for iterative development and testing
```

#### **Alternative Option: Dedicated ISP Programmer (Production)**

```markdown
Hardware Required:
- USBasp Programmer (~$8-15) OR
- Atmel AVR-ISP-MK2 (~$35) OR  
- Atmel-ICE (~$125 - professional grade)

Advantages:
- Dedicated programming tool for production
- Faster programming cycles in manufacturing
- Professional development approach
- No need to tie up Arduino hardware for programming
- More reliable for high-volume production programming
```

Disadvantages:
- Ties up Arduino UNO R4 WiFi
- Slower programming
- Less reliable for production use
```

### Enclosure Specifications

- **Material:** Die-cast aluminum (EMI shielding)
- **Protection:** IP65 (dust and water resistant)
- **Mounting:** DIN rail clips with panel mount option
- **Dimensions:** 200mm × 150mm × 80mm
- **Thermal:** Natural convection cooling

---

## Software Implementation

### Development Environment

- **IDE:** VS Code with PlatformIO extension
- **Framework:** Arduino framework for ATmega32A
- **Compiler:** AVR-GCC with optimization flags
- **Debugging:** Serial monitor, LED indicators
- **Version Control:** Git repository with project documentation

### Programming Workflow

1. **ISP Setup:** Arduino UNO R4 WiFi loaded with ArduinoISP sketch
2. **Fuse Programming:** External crystal configuration (LFUSE=0xFF, HFUSE=0xD9)
3. **Code Upload:** PlatformIO upload via ISP programmer
4. **Testing:** Individual sonicator interface validation
5. **Integration:** Full system testing with PLC communication

### Key Software Features

- **Real-time Control:** Response time faster than the MODBUS communication protocol requirements so the bottleneck is the MODBUS
- **Modular Architecture:** Easily extensible for additional sonicators
- **Error Handling:** Comprehensive fault detection and recovery
- **MODBUS Compliance:** Full RTU protocol implementation
- **Diagnostic Logging:** System status and fault reporting

#### Build & Test Workflow Requirements (Make targets)

- Every testing workflow MUST expose a corresponding Makefile target:
  - make test-unit → Runs unit tests locally and in CI
  - make test-acceptance → Runs BDD acceptance tests (simulation or HIL via profiles)
  - make test-hardware → Runs hardware-in-the-loop tests against connected DUT
- CI and local developers SHALL invoke tests via these Make targets (not raw tool CLIs) to keep parity.
- Acceptance Criteria:
  - The project Makefile defines the three targets above and they succeed on supported environments.
  - CI workflows call only these targets to run tests.
  - Documentation (README/CONTRIBUTING) references these targets for developers.

---

## Integration Requirements

### Existing System Modifications

#### **Main Automation Box Configuration Changes**

```markdown
Velocio 1630c PLC Updates:
- Configure spare DB9 port for MODBUS RTU communication
- Add MODBUS slave device (Multi Sonicator Controller) to network
- Update ladder logic for multi-sonicator control
- Implement register mapping for 4-sonicator operation
```

#### **HMI Interface Extensions**

```markdown
HMI System Updates:
- Add multi-sonicator monitoring screens
- Individual amplitude control for each unit
- Real-time power and frequency displays  
- Alarm management for overload conditions
- System status and diagnostics pages
```

#### **Mobile App Enhancements**

```
Firebase/Mobile App Updates:
- Extend data model for multiple sonicators
- Add UI components for 4-unit control
- Real-time monitoring dashboard
- Push notifications for fault conditions
- Historical data logging and trends
```

### System Compatibility

- **Backward Compatibility:** System gracefully handles single sonicator operation
- **Auto-Detection:** Controller identifies connected sonicator count
- **Fallback Mode:** Continues operation if individual sonicators fault
- **Upgrade Path:** Existing installations can add controller without system changes

---

## Reporting Deliverables (Executive Functionality Report)

This project MUST publish Executive Functionality artifacts conforming to `docs/executive-report-standard.md`.

- **Required (per release/tag):**
  - `final/executive-report.json` (schema v1.0.0)
    - Includes acceptance scenario results covering MVP features:
      - Control 4 sonicators from one automation box
      - Per unit: amplitude control, start/stop, overload reset
      - Monitoring: power, frequency, overload, frequency lock
      - System: Active Sonicator Count (`REG_ACTIVE_SONICATOR_COUNT`), Mask (`REG_ACTIVE_SONICATOR_MASK`)
    - Summary totals and timestamps per the standard

- **Optional (recommended):**
  - `final/coverage-summary.json` (overall coverage totals)
  - `final/unit-test-summary.json` (unit test aggregates)
  - `final/executive-report.md` (human-readable summary)

- **Publishing process (CI):**
  - Artifacts are attached to the GitHub Release for the commit/tag
  - Filenames MUST use the `final/` prefix and correct content types
  - JSON MUST include `version`, `owner`, `repo`, `releaseTag`, `commit`, `createdAt`

- **Acceptance Criteria:**
  - A tagged release contains `final/executive-report.json` that validates against the standard
  - Scenarios mapped to PRD requirements demonstrate pass/fail with durations
  - If optional files are present, they follow the shapes in the standard

---

### Acceptance Criteria — Active Sonicator Registers

- **Register Contract**
  - `REG_ACTIVE_SONICATOR_COUNT` (40035) returns integer 0–4
  - `REG_ACTIVE_SONICATOR_MASK` (40036) returns bitmask with bits 0–3 for Sonicators 1–4
- **Latency**
  - Values reflect state changes (start/stop/reset/overload clear) within ≤100 ms
- **Consistency**
  - Count equals the number of set bits in Mask at all times
  - Per-unit `REG_SONICATOR_[1..4]_STATUS` bit0 (running) aligns with Mask bits
- **Error Handling**
  - During comm faults, registers maintain last-known-good until status indicates recovery (document fault semantics in PRD)
- **Test Evidence (Documentation-only phase)**
  - Executive report scenarios enumerate transitions 0→N and N→0 and assert expected values
  - No firmware change in this phase; scenarios define acceptance expected outcomes

---

## Risk Analysis & Mitigation

### Technical Risks

| Risk | Probability | Impact | Mitigation Strategy |
|------|-------------|--------|-------------------|
| **MODBUS Integration Issues** | Medium | High | Prototype testing with existing PLC, fallback serial protocol |
| **EMI from Sonicators** | Medium | Medium | Proper shielding, filtering, isolated interfaces |
| **Component Lead Times** | Low | Medium | Order critical components early, identify alternatives |
| **Power Supply Noise** | Low | Medium | Multi-stage filtering, separate analog/digital supplies |

### Schedule Risks

| Risk | Probability | Impact | Mitigation Strategy |
|------|-------------|--------|-------------------|
| **PCB Fabrication Delays** | Medium | High | Use expedited service, local PCB house |
| **Software Integration Complexity** | Low | Medium | Modular development, incremental testing |
| **Enclosure Customization** | Medium | Low | Standard enclosure with minimal modifications |

### Business Risks

| Risk | Probability | Impact | Mitigation Strategy |
|------|-------------|--------|-------------------|
| **Customer Acceptance** | Low | High | Early customer validation, prototype demonstration |
| **Regulatory Compliance** | Low | Medium | Use standard industrial components, documentation |
| **Support Complexity** | Medium | Low | Comprehensive documentation, modular design |

---

## Regulatory & Compliance

### Market Requirements

- **Primary Market:** United States industrial equipment
- **Industry:** Cannabis/nutraceutical processing equipment
- **Applications:** Nano-emulsion production for beverages and extracts

### Compliance Standards

- **Electrical Safety:** UL recognition for components, proper grounding
- **EMI/EMC:** FCC Part 15 compliance for industrial equipment
- **Environmental:** RoHS compliance for electronic components
- **Quality:** ISO 9001 manufacturing processes

### Industry-Specific Considerations

- **Cannabis Regulations:** Equipment used in licensed facilities
- **Food Safety:** Potential food-grade processing applications
- **Documentation:** Complete technical documentation for regulatory submissions

---

## Project Timeline

### Development Phase (2 Weeks)

#### **Week 1: Hardware Design & Procurement**

- [ ] **Day 1-2:** Finalize hardware specifications and component selection
- [ ] **Day 3-4:** PCB design and layout using EasyEDA
- [ ] **Day 5:** Submit PCB for fabrication (expedited 3-day service)
- [ ] **Day 6:** Order components, enclosure, and **USBasp ISP programmer**
- [ ] **Day 7:** Setup development environment and ISP programming workflow

#### **Week 2: Software Development & Integration**

- [ ] **Day 8-9:** Core software development (MODBUS, enhanced sonicator interfaces)
- [ ] **Day 10:** PCB assembly and initial hardware testing with ISP programmer
- [ ] **Day 11-12:** Software integration and enhanced control testing
- [ ] **Day 13:** PLC integration and MODBUS communication testing
- [ ] **Day 14:** Final system validation with full CT2000 control capabilities

### Post-Development Activities

- **Week 3:** Customer demonstration and feedback
- **Week 4:** Documentation completion and production setup
- **Ongoing:** Customer installations and support

---

## Budget Analysis

### Development Costs

| Category | Budget Allocation | Actual Estimate |
|----------|------------------|-----------------|
| **Components** | $400 | $180 |
| **PCB Fabrication** | $75 | $65 |
| **Enclosure** | $50 | $45 |
| **ISP Programmer** | $25 | $20 |
| **Testing & Validation** | $30 | $25 |
| **Miscellaneous** | $20 | $15 |
| **Total** | **$400** | **$350** |

### Production Unit Costs (Target: Maximum Value)

| Component Category | Unit Cost | Notes |
|-------------------|-----------|-------|
| **Electronics** | $120-150 | Volume pricing for 12+ units/year, enhanced components |
| **Enclosure & Assembly** | $50-70 | Local assembly, standard enclosure |
| **Testing & QC** | $20-30 | Enhanced automated test procedures |
| **Documentation** | $10-15 | Technical manuals, certificates |
| **Total Unit Cost** | **$200-265** | Target selling price: $800-1200 |

### Return on Investment

- **Development Investment:** $400 + 2 weeks labor
- **Production Margin:** $400-600 per unit (high-value industrial equipment)
- **Annual Volume:** 12 units projected
- **Annual Revenue Potential:** $6,000-9,600
- **Break-even:** First 2-3 units sold

---

## Success Metrics

### Technical Performance

- **Response Time:** Faster than MODBUS protocol requirements (MODBUS is the bottleneck)
- **Accuracy:** Frequency output ±100Hz (meets specification)
- **Reliability:** >99.9% uptime during continuous operation
- **EMI Compliance:** Passes FCC Part 15 industrial equipment standards

### Business Performance

- **Customer Satisfaction:** Seamless integration with existing systems
- **Time to Market:** 2-week development timeline met
- **Production Quality:** Zero defects in first production run
- **Support Efficiency:** <1 hour average technical support time

### Integration Success

- **PLC Communication:** Reliable MODBUS operation with existing Velocio system
- **HMI Integration:** Intuitive multi-sonicator control interface
- **Mobile App:** Real-time monitoring and control capabilities
- **Backward Compatibility:** Graceful single-sonicator operation mode when only one sonicator is connected and when multi-sonicator-io is not connected

---

## Post-Project Considerations

### Production Scaling

- **Manufacturing:** Contract PCB assembly, local final assembly
- **Quality Control:** Automated test procedures, statistical process control
- **Documentation:** Complete technical package for regulatory compliance
- **Support:** Remote diagnostics, comprehensive user manuals

### Future Enhancements

- **Capacity Expansion:** 6-8 sonicator capability with minimal hardware changes
- **Advanced Features:** Recipe management, process optimization algorithms
- **Connectivity:** Ethernet interface for larger installations
- **Analytics:** Machine learning for predictive maintenance

### Technology Evolution

- **Microcontroller Upgrade:** Migration path to more powerful MCU if needed
- **Communication Protocols:** Industrial Ethernet, OPC-UA readiness
- **Cloud Integration:** Enhanced IoT capabilities for remote monitoring
- **AI Integration:** Process optimization and predictive analytics

---

## Conclusion

The Multi-Sonicator I/O Controller project represents a strategic expansion of Cannasol Technologies' automation capabilities, enabling 4x production capacity increase while leveraging existing infrastructure investments. The technical approach balances industrial reliability requirements with rapid development timeline and budget constraints.

**Key Success Factors:**

1. **Proven Technology Stack:** ATmega32A + MODBUS + industrial components
2. **Modular Architecture:** Scalable, maintainable software design
3. **Existing System Integration:** Minimal disruption to current operations
4. **Industrial Standards:** Professional-grade reliability and compliance

**Project Outcome:** A production-ready, industrial-grade multi-sonicator controller that positions Cannasol for expanded market opportunities in the growing nano-emulsion processing industry.

---

**Document Control:**

- **Version:** 0.1.0
- **Author:** Stephen Boyett (Lead Developer)
- **Reviewer:** Josh (CEO)
- **Next Review:** February 2025 (post-prototype completion)
- **Distribution:** Development team, stakeholders
