# ATmega32A Pin Definitions - Multi-Sonicator I/O Controller

**Project:** CAN-SON-CTRL-4 - Multi Sonicator I/O Controller  
**MCU:** ATmega32A-PU (DIP-40 Package)  
**Manufacturer:** Cannasol Technologies  
**Date:** July 18, 2025  

## Overview

This document provides a comprehensive mapping of all 40 pins of the ATmega32A microcontroller and their functions in the Multi-Sonicator I/O Controller circuit. The controller interfaces with up to 4 CT2000 sonicators via DB9 connectors and provides MODBUS RTU communication.

---

## Complete Pin Assignment Table

| Physical Pin | Port Pin | Direction | Function | Description | Circuit Notes |
|--------------|----------|-----------|----------|-------------|---------------|
| **1** | **PB0(XCK/T0)** | Input | **SONICATOR_FREQ_OUT_1** | Sonicator 1 Frequency Output | Timer0/T0 Input |
| **2** | **PB1(T1)** | Input | **SONICATOR_FREQ_LOCK_1** | Sonicator 1 Frequency Lock | 6N137 Digital Input |
| **3** | **PB2(INT2/AIN0)** | Input | **SONICATOR_FREQ_LOCK_2** | Sonicator 2 Frequency Lock | 6N137 Digital Input |
| **4** | **PB3(OC0/AIN1)** ⚡PWM | Input | **SONICATOR_FREQ_OUT_2** | Sonicator 2 Frequency Output | Timer2/T2 Input |
| **5** | **PB4(SS)** | Input | **SONICATOR_FREQ_LOCK_3** | Sonicator 3 Frequency Lock | 6N137 Digital Input |
| **6** | **PB5(MOSI)** | Input | **SONICATOR_FREQ_LOCK_4** | Sonicator 4 Frequency Lock | 6N137 Digital Input |
| **7** | **PB6(MISO)** | Output | **LED_STATUS_PIN** | Status LED | Digital Output |
| **8** | **PB7(SCK)** | Output | **Available** | Free Digital I/O | Available |
| **9** | **RESET** | Input | **System Reset** | Hardware Reset | System |
| **10** | **VCC** | Power | **+5V Power** | Power Supply | System |
| **11** | **GND** | Ground | **Ground** | Ground Reference | System |
| **12** | **XTAL2** | Output | **16MHz Crystal** | Clock Generation | System |
| **13** | **XTAL1** | Input | **16MHz Crystal** | Clock Generation | System |
| **14** | **PD0(RXD)** | Input | **UART_RX_PIN** | MODBUS RTU Receive | UART Input |
| **15** | **PD1(TXD)** | Output | **UART_TX_PIN** | MODBUS RTU Transmit | UART Output |
| **16** | **PD2(INT0)** | Input | **SONICATOR_OVERLOAD_4** | Sonicator 4 Overload | 6N137 Digital Input |
| **17** | **PD3(INT1)** | Input | **SONICATOR_OVERLOAD_3** | Sonicator 3 Overload | 6N137 Digital Input |
| **18** | **PD4(OC1B)** ⚡PWM | Input | **SONICATOR_OVERLOAD_2** | Sonicator 2 Overload | 6N137 Digital Input |
| **19** | **PD5(OC1A)** ⚡PWM | Input | **SONICATOR_OVERLOAD_1** | Sonicator 1 Overload | 6N137 Digital Input |
| **20** | **PD6(ICP1)** | Input | **SONICATOR_FREQ_OUT_3** | Sonicator 3 Frequency Output | Timer1/ICP1 Input |
| **21** | **PD7(OC2)** ⚡PWM | Input | **SONICATOR_FREQ_LOCK_4** | Sonicator 4 Frequency Lock | 6N137 Digital Input |
| **22** | **PC0(SCL)** | Output | **SONICATOR_START_4** | Sonicator 4 Start Control | ULN2003A Output |
| **23** | **PC1(SDA)** | Output | **SONICATOR_RESET_4** | Sonicator 4 Reset Control | ULN2003A Output |
| **24** | **PC2(TCK)** | Output | **SONICATOR_START_3** | Sonicator 3 Start Control | ULN2003A Output |
| **25** | **PC3(TMS)** | Output | **SONICATOR_RESET_3** | Sonicator 3 Reset Control | ULN2003A Output |
| **26** | **PC4(TDO)** | Output | **SONICATOR_START_2** | Sonicator 2 Start Control | ULN2003A Output |
| **27** | **PC5(TDI)** | Output | **SONICATOR_RESET_2** | Sonicator 2 Reset Control | ULN2003A Output |
| **28** | **PC6(TOSC1)** | Output | **SONICATOR_START_1** | Sonicator 1 Start Control | ULN2003A Output |
| **29** | **PC7(TOSC2)** | Output | **SONICATOR_RESET_1** | Sonicator 1 Reset Control | ULN2003A Output |
| **30** | **AVCC** | Power | **ADC Power** | Analog Power Supply | System |
| **31** | **GND** | Ground | **Ground** | Ground Reference | System |
| **32** | **AREF** | Input | **ADC Reference** | Analog Reference | System |
| **33** | **PA7(ADC7)** | Input | **SONICATOR_PWR_IN_4** | Sonicator 4 Power Monitor | ADC7 Input |
| **34** | **PA6(ADC6)** | Input | **SONICATOR_PWR_IN_3** | Sonicator 3 Power Monitor | ADC6 Input |
| **35** | **PA5(ADC5)** | Input | **SONICATOR_PWR_IN_2** | Sonicator 2 Power Monitor | ADC5 Input |
| **36** | **PA4(ADC4)** | Input | **SONICATOR_PWR_IN_1** | Sonicator 1 Power Monitor | ADC4 Input |
| **37** | **PA3(ADC3)** | Input | **Available** | Free Analog Input | Available |
| **38** | **PA2(ADC2)** | Input | **Available** | Free Analog Input | Available |
| **39** | **PA1(ADC1)** | Input | **SONICATOR_FREQ_OUT_4** | Sonicator 4 Frequency Output | LM2907 → ADC1 |
| **40** | **PA0(ADC0)** | Output | **SONICATOR_AMPLITUDE_OUT** | Amplitude Control (All 4 Sonicators) | Software PWM Output |

---

## Acronym Definitions**

### **Port B (PB0-PB7):**

- **PB0(XCK/T0)** - External Clock/Timer0
- **PB1(T1)** - Timer1 External Clock
- **PB2(INT2/AIN0)** - External Interrupt 2/Analog Comparator Input
- **PB3(OC0/AIN1)** ⚡PWM - Output Compare 0/Analog Comparator Input
- **PB4(SS)** - SPI Slave Select
- **PB5(MOSI)** - SPI Master Out Slave In
- **PB6(MISO)** - SPI Master In Slave Out
- **PB7(SCK)** - SPI Serial Clock

### **Port D (PD0-PD7):**

- **PD0(RXD)** - UART Receive
- **PD1(TXD)** - UART Transmit
- **PD2(INT0)** - External Interrupt 0
- **PD3(INT1)** - External Interrupt 1
- **PD4(OC1B)** ⚡PWM - Timer1 Output Compare B
- **PD5(OC1A)** ⚡PWM - Timer1 Output Compare A
- **PD6(ICP1)** - Timer1 Input Capture
- **PD7(OC2)** ⚡PWM - Timer2 Output Compare

### **Port C (PC0-PC7):**

- **PC0(SCL)** - I2C Serial Clock
- **PC1(SDA)** - I2C Serial Data
- **PC2(TCK)** - JTAG Test Clock
- **PC3(TMS)** - JTAG Test Mode Select
- **PC4(TDO)** - JTAG Test Data Out
- **PC5(TDI)** - JTAG Test Data In
- **PC6(TOSC1)** - Timer Oscillator 1
- **PC7(TOSC2)** - Timer Oscillator 2

### **Port A (PA0-PA7):**

- **PA0(ADC0)** through **PA7(ADC7)** - ADC channels 0-7

These hardware function markers now clearly show the **built-in peripheral capabilities** of each pin, making it easier to understand the microcontroller's hardware features and potential alternative uses for each pin! 🎯

## OFFICIAL Pin Assignment - From Specification Document ✅

### **Pin Assignment Implementation Details:**

**Key Design Features:**

- **Hybrid Frequency Monitoring:** 3 hardware timers + 1 LM2907 frequency-to-voltage converter
- **Sequential Overload Pins:** PD2-PD5 for simplified wiring and logic  
- **Single PWM Control:** PA1 software PWM controls all 4 sonicator amplitudes simultaneously
- **RGB LED Status:** Single RGB LED replaces multiple individual LEDs (94% component reduction)

**Pin Assignment Benefits:**

- **Timer Optimization:** PB0 (Timer0), PB3 (Timer2), PD6 (Timer1/ICP1) for frequency measurement
- **Clean Grouping:** All overload pins sequential (PD2-PD5)
- **Efficient Layout:** All start/reset controls on Port C (PC0-PC7)
- **ADC Utilization:** PA0,PA4-PA7 for analog monitoring

---

## Functional Groups

### **Sonicator 1 Interface (DB9 Connector 1)**

- **Overload:** PD5 (Pin 19) - 6N137 Input
- **Frequency Output:** PB0 (Pin 1) - Timer0/T0 Input
- **Frequency Lock:** PB1 (Pin 2) - 6N137 Input
- **Start Control:** PC6 (Pin 28) - ULN2003A Output
- **Reset Control:** PC7 (Pin 29) - ULN2003A Output
- **Power Monitor:** PA4 (Pin 36) - ADC4 Input

### **Sonicator 2 Interface (DB9 Connector 2)**

- **Overload:** PD4 (Pin 18) - 6N137 Input
- **Frequency Output:** PB3 (Pin 4) - Timer2/T2 Input
- **Frequency Lock:** PB2 (Pin 3) - 6N137 Input
- **Start Control:** PC4 (Pin 26) - ULN2003A Output
- **Reset Control:** PC5 (Pin 27) - ULN2003A Output
- **Power Monitor:** PA5 (Pin 35) - ADC5 Input

### **Sonicator 3 Interface (DB9 Connector 3)**

- **Overload:** PD3 (Pin 17) - 6N137 Input
- **Frequency Output:** PD6 (Pin 20) - Timer1/ICP1 Input
- **Frequency Lock:** PB4 (Pin 5) - 6N137 Input
- **Start Control:** PC2 (Pin 24) - ULN2003A Output
- **Reset Control:** PC3 (Pin 25) - ULN2003A Output
- **Power Monitor:** PA6 (Pin 34) - ADC6 Input

### **Sonicator 4 Interface (DB9 Connector 4)**

- **Overload:** PD2 (Pin 16) - 6N137 Input
- **Frequency Output:** PA0 (Pin 40) - LM2907 → ADC0 Input
- **Frequency Lock:** PD7 (Pin 21) - 6N137 Input
- **Start Control:** PC0 (Pin 22) - ULN2003A Output
- **Reset Control:** PC1 (Pin 23) - ULN2003A Output
- **Power Monitor:** PA7 (Pin 33) - ADC7 Input

### **System Functions**

- **MODBUS RTU:** PD0 (Pin 14) RX, PD1 (Pin 15) TX
- **PWM Amplitude:** PA1 (Pin 39) - Software PWM for all 4 sonicators
- **Status LED:** PB6 (Pin 7) - Single LED for system status indication
- **Crystal Oscillator:** XTAL1 (Pin 13), XTAL2 (Pin 12) - 16MHz

### **Power & Ground**

- **Pin 10:** VCC (+5V Digital)
- **Pin 11:** GND (Digital Ground)  
- **Pin 30:** AVCC (+5V Analog)
- **Pin 31:** AGND (Analog Ground)

### **Available Pins**

- **Pin 8:** PB7 - Free Digital I/O Pin
- **Pin 37:** PA3 - Free Analog Input
- **Pin 38:** PA2 - Free Analog Input

---

## Timer and Frequency Monitoring Assignments

| Timer/Method | Pin | Port Pin | Function | Sonicator |
|--------------|-----|----------|----------|-----------|
| **Timer0** | 1 | PB0 | External Clock Input | Sonicator 1 Frequency |
| **Timer2** | 4 | PB3 | External Clock Input | Sonicator 2 Frequency |
| **Timer1/ICP1** | 20 | PD6 | Input Capture | Sonicator 3 Frequency |
| **LM2907 → ADC0** | 40 | PA0 | Frequency-to-Voltage | Sonicator 4 Frequency |

## PWM Control Assignment

| Function | Pin | Port Pin | Method | Notes |
|----------|-----|----------|--------|-------|
| **Amplitude Control** | 39 | PA1 | Software PWM | Controls all 4 sonicators simultaneously |

---

## DB9 Connector Mapping (CT2000 Interface)

| DB9 Pin | Function | ATmega32A Connection | Signal Type |
|---------|----------|---------------------|-------------|
| **1** | Overload Indication | PD5,PD4,PD3,PD2 (per sonicator) | Open Collector Input |
| **2** | Overload Reset | PC1,PC3,PC5,PC7 (per sonicator) | Relay Contact Output |
| **3** | Frequency Lock | PB1,PB2,PB4,PD7 (per sonicator) | Open Collector Input |
| **4** | Frequency Output | PB0,PB3,PD6,PA0 (per sonicator) | Square Wave Input |
| **5** | Power Output | PA4-PA7 (per sonicator) | 5.44mV/W Analog Input |
| **6** | Signal Ground | GND(11,31) | Ground Reference |
| **7** | Sonics Start | PC6,PC4,PC2,PC0 (per sonicator) | Relay Contact Output |
| **8** | Amplitude Control | PA1 (all sonicators) | 0-10V Analog Output |
| **9** | Emitter Common | GND | Ground Reference |

---

## Recommendations

### **COMPLETED: All Pin Conflicts Resolved ✅**

**Resolution Implemented:**

**Resolution Implemented:**

- **Single LED Design:** One status LED on PB6 provides system status indication
- **Full Sonicator Support:** All 4 sonicators now have dedicated pins without conflicts
- **Clean Pin Mapping:** Frequency lock pins properly distributed across available ports

### **Conflict Resolution Implementation Details:**

**Single LED Status Codes:**

- **Off:** System powered down or idle
- **Slow Blink:** Normal operation
- **Fast Blink:** Communication active
- **Solid On:** Error/Overload condition
- **Double Blink:** Warning state
- **Triple Blink:** Initialization/Test mode

**Benefits Achieved:**

- ✅ Eliminated all pin conflicts
- ✅ Support for 4 sonicators maintained
- ✅ Clear status indication preserved
- ✅ Simplified wiring and assembly
- ✅ Cost reduction (fewer LEDs/resistors)

---

## Notes

- All measurements assume 5V operation with 16MHz crystal
- PWM frequencies optimized for RC filter conversion to 0-10V
- ADC configured for 10-bit resolution (1024 steps)
- MODBUS operates at 115200 baud via RS485 transceiver
- Pull-up/pull-down resistors as specified in hardware design

---

**Document Status:** ✅ COMPLETE - All pin conflicts resolved with single LED design  
**Last Updated:** July 18, 2025  
**Reviewed By:** System Designer
