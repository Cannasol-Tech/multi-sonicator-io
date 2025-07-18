# OFFICIAL ATmega32A Pin Assignment Specification

**Document Version:** 1.0  
**Date:** January 18, 2025  
**Project:** Multi Sonicator I/O Controller (CAN-SON-CTRL-4)  
**Company:** Cannasol Technologies  
**Microcontroller:** ATmega32A-PU (40-pin DIP)  

---

## Document Control

| Version | Date | Changes | Author |
|---------|------|---------|---------|
| 1.0 | 2025-01-18 | Initial pin assignment specification | Stephen Boyett |

---

## Executive Summary

This document establishes the official pin assignment specification for the Multi Sonicator I/O Controller, implementing control and monitoring for up to 4 CT2000 sonicators via DB9 interfaces with MODBUS RTU communication.

## Key Design Decisions

1. **Hybrid Frequency Monitoring:** 3 hardware timers + 1 LM2907 frequency-to-voltage converter
2. **Sequential Overload Pins:** PD2-PD5 for simplified wiring and logic
3. **Single PWM Control:** PA1 software PWM controls all 4 sonicator amplitudes simultaneously
4. **RGB LED Status:** Single RGB LED replaces 12 individual LEDs (94% component reduction)

---

## Complete Pin Assignment Table

| Physical Pin | Port Pin | Function | Description | Interface Type |
|--------------|----------|----------|-------------|----------------|
| **1** | PB0 | `SON1_FREQ_OUTPUT_PIN` | Sonicator 1 Frequency Output | Timer0/T0 Input |
| **2** | PB1 | `SON1_FREQ_LOCK_PIN` | Sonicator 1 Frequency Lock | 6N137 Digital Input |
| **3** | PB2 | `SON2_FREQ_LOCK_PIN` | Sonicator 2 Frequency Lock | 6N137 Digital Input |
| **4** | PB3 | `SON2_FREQ_OUTPUT_PIN` | Sonicator 2 Frequency Output | Timer2/T2 Input |
| **5** | PB4 | `SON3_FREQ_LOCK_PIN` | Sonicator 3 Frequency Lock | 6N137 Digital Input |
| **6** | PB5 | `LED_RGB_BLUE_PIN` | Status LED Blue Channel | Digital Output |
| **7** | PB6 | `LED_RGB_RED_PIN` | Status LED Red Channel | Digital Output |
| **8** | PB7 | `LED_RGB_GREEN_PIN` | Status LED Green Channel | Digital Output |
| **9** | RESET | System Reset | Hardware Reset | System |
| **10** | VCC | +5V Power | Power Supply | System |
| **11** | GND | Ground | Ground Reference | System |
| **12** | XTAL2 | 16MHz Crystal | Clock Generation | System |
| **13** | XTAL1 | 16MHz Crystal | Clock Generation | System |
| **14** | PD0 | `UART_RX_PIN` | MODBUS RTU Receive | UART Input |
| **15** | PD1 | `UART_TX_PIN` | MODBUS RTU Transmit | UART Output |
| **16** | PD2 | `SON1_OVERLOAD_PIN` | Sonicator 1 Overload | 6N137 Digital Input |
| **17** | PD3 | `SON2_OVERLOAD_PIN` | Sonicator 2 Overload | 6N137 Digital Input |
| **18** | PD4 | `SON3_OVERLOAD_PIN` | Sonicator 3 Overload | 6N137 Digital Input |
| **19** | PD5 | `SON4_OVERLOAD_PIN` | Sonicator 4 Overload | 6N137 Digital Input |
| **20** | PD6 | `SON3_FREQ_OUTPUT_PIN` | Sonicator 3 Frequency Output | Timer1/ICP1 Input |
| **21** | PD7 | `SON4_FREQ_LOCK_PIN` | Sonicator 4 Frequency Lock | 6N137 Digital Input |
| **22** | PC0 | `SON4_START_PIN` | Sonicator 4 Start Control | ULN2003A Output |
| **23** | PC1 | `SON4_RESET_PIN` | Sonicator 4 Reset Control | ULN2003A Output |
| **24** | PC2 | `SON3_START_PIN` | Sonicator 3 Start Control | ULN2003A Output |
| **25** | PC3 | `SON3_RESET_PIN` | Sonicator 3 Reset Control | ULN2003A Output |
| **26** | PC4 | `SON2_START_PIN` | Sonicator 2 Start Control | ULN2003A Output |
| **27** | PC5 | `SON2_RESET_PIN` | Sonicator 2 Reset Control | ULN2003A Output |
| **28** | PC6 | `SON1_START_PIN` | Sonicator 1 Start Control | ULN2003A Output |
| **29** | PC7 | `SON1_RESET_PIN` | Sonicator 1 Reset Control | ULN2003A Output |
| **30** | AVCC | ADC Power | Analog Power Supply | System |
| **31** | GND | Ground | Ground Reference | System |
| **32** | AREF | ADC Reference | Analog Reference | System |
| **33** | PA7 | `ADC_SONICATOR_4_PIN` | Sonicator 4 Power Monitor | ADC7 Input |
| **34** | PA6 | `ADC_SONICATOR_3_PIN` | Sonicator 3 Power Monitor | ADC6 Input |
| **35** | PA5 | `ADC_SONICATOR_2_PIN` | Sonicator 2 Power Monitor | ADC5 Input |
| **36** | PA4 | `ADC_SONICATOR_1_PIN` | Sonicator 1 Power Monitor | ADC4 Input |
| **37** | PA3 | **Available** | **Free Analog Input** | Available |
| **38** | PA2 | **Available** | **Free Analog Input** | Available |
| **39** | PA1 | `PWM_AMPLITUDE_CONTROL_PIN` | Amplitude Control (All 4 Sonicators) | Software PWM Output |
| **40** | PA0 | `SON4_FREQ_OUTPUT_PIN` | Sonicator 4 Frequency Output | LM2907 → ADC0 |

---

## Functional Groupings

### **Sonicator 1 Interface (DB9 Connector 1)**
- **Overload:** PD2 (Pin 16) - 6N137 Input
- **Frequency Output:** PB0 (Pin 1) - Timer0/T0 Input
- **Frequency Lock:** PB1 (Pin 2) - 6N137 Input
- **Start Control:** PC6 (Pin 28) - ULN2003A Output
- **Reset Control:** PC7 (Pin 29) - ULN2003A Output
- **Power Monitor:** PA4 (Pin 36) - ADC4 Input

### **Sonicator 2 Interface (DB9 Connector 2)**
- **Overload:** PD3 (Pin 17) - 6N137 Input
- **Frequency Output:** PB3 (Pin 4) - Timer2/T2 Input
- **Frequency Lock:** PB2 (Pin 3) - 6N137 Input
- **Start Control:** PC4 (Pin 26) - ULN2003A Output
- **Reset Control:** PC5 (Pin 27) - ULN2003A Output
- **Power Monitor:** PA5 (Pin 35) - ADC5 Input

### **Sonicator 3 Interface (DB9 Connector 3)**
- **Overload:** PD4 (Pin 18) - 6N137 Input
- **Frequency Output:** PD6 (Pin 20) - Timer1/ICP1 Input
- **Frequency Lock:** PB4 (Pin 5) - 6N137 Input
- **Start Control:** PC2 (Pin 24) - ULN2003A Output
- **Reset Control:** PC3 (Pin 25) - ULN2003A Output
- **Power Monitor:** PA6 (Pin 34) - ADC6 Input

### **Sonicator 4 Interface (DB9 Connector 4)**
- **Overload:** PD5 (Pin 19) - 6N137 Input
- **Frequency Output:** PA0 (Pin 40) - LM2907 → ADC0 Input
- **Frequency Lock:** PD7 (Pin 21) - 6N137 Input
- **Start Control:** PC0 (Pin 22) - ULN2003A Output
- **Reset Control:** PC1 (Pin 23) - ULN2003A Output
- **Power Monitor:** PA7 (Pin 33) - ADC7 Input

### **System Functions**
- **MODBUS RTU:** PD0 (Pin 14) RX, PD1 (Pin 15) TX
- **PWM Amplitude:** PA1 (Pin 39) - Software PWM for all 4 sonicators
- **RGB Status LED:** PB5 (Pin 6) Blue, PB6 (Pin 7) Red, PB7 (Pin 8) Green
- **Crystal Oscillator:** XTAL1 (Pin 13), XTAL2 (Pin 12) - 16MHz

---

## Interface Specifications

### **CT2000 DB9 Interface Mapping**
| DB9 Pin | Function | ATmega32A Connection | Signal Type |
|---------|----------|---------------------|-------------|
| 1 | Overload Indication | PD2-PD5 (per sonicator) | Open Collector Input |
| 2 | Overload Reset | PC1,PC3,PC5,PC7 (per sonicator) | Relay Contact Output |
| 3 | Frequency Lock | PB1,PB2,PB4,PD7 (per sonicator) | Open Collector Input |
| 4 | Frequency Output | PB0,PB3,PD6,PA0 (per sonicator) | Square Wave Input |
| 5 | Power Output | PA4-PA7 (per sonicator) | 5.44mV/W Analog Input |
| 6 | Signal Ground | GND | Ground Reference |
| 7 | Sonics Start | PC0,PC2,PC4,PC6 (per sonicator) | Relay Contact Output |
| 8 | Amplitude Control | PA1 (all sonicators) | 0-10V Analog Output |
| 9 | Emitter Common | GND | Ground Reference |

### **Frequency Monitoring Implementation**
- **Sonicator 1:** Timer0 external clock input (PB0)
- **Sonicator 2:** Timer2 external clock input (PB3)
- **Sonicator 3:** Timer1 input capture (PD6)
- **Sonicator 4:** LM2907 frequency-to-voltage converter → ADC0 (PA0)

### **PWM Amplitude Control**
- **Single Output:** PA1 (Pin 39) - Software PWM at 1kHz
- **Filter:** 1kΩ resistor + 10µF capacitor (fc = 16Hz)
- **Amplification:** LM358N op-amp (2x gain: 0-5V PWM → 0-10V output)
- **Control:** All 4 sonicators simultaneously (per design requirement)

---

## Electrical Specifications

### **Digital I/O**
- **Logic Levels:** 5V CMOS compatible
- **Input Protection:** 6N137 optocouplers for isolation
- **Output Drivers:** ULN2003A for relay control
- **Pull-up Resistors:** 10kΩ on all digital inputs

### **Analog I/O**
- **ADC Reference:** 5V (AVCC)
- **ADC Resolution:** 10-bit (0-1023 counts)
- **Power Monitoring:** 0-10.88V input range (via voltage divider)
- **Frequency Monitoring:** 0-5V input range (LM2907 output)

### **Timing Requirements**
- **Main Loop:** 10ms interval (100Hz)
- **Safety Monitoring:** 10ms interval
- **Emergency Stop Response:** <50μs
- **MODBUS Timeout:** 500ms

---

## Compliance and Validation

### **Pin Conflicts:** ✅ **RESOLVED**
- All pins have unique, non-conflicting assignments
- No shared functionality between different interfaces
- Hardware timers optimally allocated for frequency measurement

### **Functional Requirements:** ✅ **MET**
- 4 sonicator interfaces fully implemented
- MODBUS RTU communication supported
- Emergency stop and safety monitoring capability
- Single PWM amplitude control as specified

### **Hardware Constraints:** ✅ **SATISFIED**
- All 40 pins of ATmega32A allocated efficiently
- 2 pins (PA2, PA3) reserved for future expansion
- Hardware timer resources optimized for frequency measurement
- Power consumption minimized through efficient pin usage

---

## Implementation Notes

1. **Software PWM:** PA1 amplitude control requires timer interrupt implementation
2. **LM2907 Integration:** External frequency-to-voltage converter needed for Sonicator 4
3. **Optocoupler Isolation:** All digital inputs require 6N137 optocouplers
4. **Relay Drivers:** ULN2003A drivers required for all start/reset outputs
5. **Crystal Oscillator:** 16MHz external crystal required for accurate timing

---

## Approval

This pin assignment specification has been reviewed and approved for implementation.

**Document Status:** ✅ **APPROVED**  
**Implementation Status:** Ready for PCB design and firmware development  
**Next Phase:** Hardware schematic design and component procurement  

---

**End of Document**
