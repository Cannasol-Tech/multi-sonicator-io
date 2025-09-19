#ifndef SONICATOR_PINS_H
#define SONICATOR_PINS_H

/** 
*@file sonicator/types/pins.h
* @brief Sonicator Pinout Definitions
* @author Cannasol Technologies
* @date 2025-09-16
* @version 2.0.0
*
* @details  This file defines the ATmega32A Pins used to control and monitor 
*   The CT2000 Sonciator devices being interfaced with via DB9 Connectors to our 
*   Multi Sonicator I/O Controller ( Sonicator Multiplexer ).  
*   THe pin definitions are confirmed and based on the sole source of truth for the hardware 
*   configuration in `config/hardware-config.yaml`. 
 *
 * @section pin_matrix Pin Matrix 
 * @note Pin = Hardware Pin number on ATMega32a
 * @note Signal = Signal name as defined in `config/hardware-config.yaml`
 * @note Dir = Direction from DUT (ATMega32a) Perspective (IN/OUT)
 * @note IN is a MONITORED signal output from the CT2000 Sonicator to our ATmega32a 
 * @note OUT is a CONTROL signal output by our Multi-Sonicator I/O device to control/configure the CT2000 Sonicator
 * 
 * @details for MONITORED signals, we should pass the value to it's correct MODBUS register for handling by the Main Automation 
 *   System.  ( Sonicator -> **ATMega32a** -> MODBUS )
 * @details for CONTROL signals, we should set the signal based on the value in the respective MODBUS register which is 
 *   being set by the main Automation Box.  ( MODBUS -> **ATMega32a** -> Sonicator )
 * 
 * @details The Sonicator Multiplexer I/O Controller is a MODBUS RTU slave device with four sonicator channels. 
 * @details The ATmega32a is the the microcontroller brain of the Multi Sonicator I/O Controller;  The Multi Sonicator I/O 
 *   Controller's Main purpose is to act as the bridge between the Main Automation Control Box ( Which which has a Human Interface ) 
 *   and the Sonicators.  For monitored signals, we are simply exposing them to the Main Automation Controol Box.  
 *   For control signals, we are simply passing the Configuration from the Main Automation Control Box to the correct sonicator based on the MODBUS Register map. 
 
 
 * @subsection sonicator1_pins Sonicator 1 ATMega32a Pin-to-MODBUS Register Mapping
 * | Pin | Signal        | Direction | MODBUS Register | Address | Description |
 * |-----|---------------|-----------|-----------------|---------|-------------|
 * | 4   | FREQ_DIV10_1  | IN        | REG_SON1_FREQ_DIV10_HZ | 0x0111 | Frequency ÷10 input → stored as Hz÷10 (efficient) |
 * | 8   | FREQ_LOCK_1   | IN        | REG_SON1_STATUS_FLAGS | 0x0112 | Frequency lock status → bit 2 (SON_STATUS_FREQ_LOCK) |
 * | 20  | OVERLOAD_1    | IN        | REG_SON1_STATUS_FLAGS | 0x0112 | Overload detection → bit 1 (SON_STATUS_OVERLOAD) |
 * | 28  | START_1       | OUT       | REG_SON1_START_STOP | 0x0100 | Start/enable control ← MODBUS command |
 * | 29  | RESET_1       | OUT       | REG_SON1_OVERLOAD_RESET | 0x0102 | Reset control ← MODBUS pulse command |
 * | 36  | POWER_SENSE_1 | IN (ADC)  | REG_SON1_POWER_RAW_ADC | 0x0110 | Power level analog → raw ADC (0-1023, cloud converts) |
 * | 21  | AMPLITUDE_ALL | OUT (PWM) | REG_AMPLITUDE_ALL_SP | 0x0101 | Amplitude control ← MODBUS setpoint (20-100%) |
 *
 * @subsection sonicator2_pins Sonicator 2 ATMega32a Pin-to-MODBUS Register Mapping
 * | Pin | Signal        | Direction | MODBUS Register | Address | Description |
 * |-----|---------------|-----------|-----------------|---------|-------------|
 * | 3   | FREQ_DIV10_2  | IN        | REG_SON2_FREQ_DIV10_HZ | 0x0131 | Frequency ÷10 input → stored as Hz÷10 (efficient) |
 * | 7   | FREQ_LOCK_2   | IN        | REG_SON2_STATUS_FLAGS | 0x0132 | Frequency lock status → bit 2 (SON_STATUS_FREQ_LOCK) |
 * | 19  | OVERLOAD_2    | IN        | REG_SON2_STATUS_FLAGS | 0x0132 | Overload detection → bit 1 (SON_STATUS_OVERLOAD) |
 * | 26  | START_2       | OUT       | REG_SON2_START_STOP | 0x0120 | Start/enable control ← MODBUS command |
 * | 27  | RESET_2       | OUT       | REG_SON2_OVERLOAD_RESET | 0x0122 | Reset control ← MODBUS pulse command |
 * | 35  | POWER_SENSE_2 | IN (ADC)  | REG_SON2_POWER_RAW_ADC | 0x0130 | Power level analog → raw ADC (0-1023, cloud converts) |
 * | 21  | AMPLITUDE_ALL | OUT (PWM) | REG_AMPLITUDE_ALL_SP | 0x0121 | Amplitude control ← MODBUS setpoint (20-100%) |
 *
 * @subsection sonicator3_pins Sonicator 3 ATMega32a Pin-to-MODBUS Register Mapping
 * | Pin | Signal        | Direction | MODBUS Register | Address | Description |
 * |-----|---------------|-----------|-----------------|---------|-------------|
 * | 2   | FREQ_DIV10_3  | IN        | REG_SON3_FREQ_DIV10_HZ | 0x0151 | Frequency ÷10 input → stored as Hz÷10 (efficient) |
 * | 6   | FREQ_LOCK_3   | IN        | REG_SON3_STATUS_FLAGS | 0x0152 | Frequency lock status → bit 2 (SON_STATUS_FREQ_LOCK) |
 * | 18  | OVERLOAD_3    | IN        | REG_SON3_STATUS_FLAGS | 0x0152 | Overload detection → bit 1 (SON_STATUS_OVERLOAD) |
 * | 24  | START_3       | OUT       | REG_SON3_START_STOP | 0x0140 | Start/enable control ← MODBUS command |
 * | 25  | RESET_3       | OUT       | REG_SON3_OVERLOAD_RESET | 0x0142 | Reset control ← MODBUS pulse command |
 * | 34  | POWER_SENSE_3 | IN (ADC)  | REG_SON3_POWER_RAW_ADC | 0x0150 | Power level analog → raw ADC (0-1023, cloud converts) |
 * | 21  | AMPLITUDE_ALL | OUT (PWM) | REG_AMPLITUDE_ALL_SP | 0x0141 | Amplitude control ← MODBUS setpoint (20-100%) |
 *
 * @subsection sonicator4_pins Sonicator 4 ATMega32a Pin-to-MODBUS Register Mapping
 * | Pin | Signal        | Direction | MODBUS Register | Address | Description |
 * |-----|---------------|-----------|-----------------|---------|-------------|
 * | 1   | FREQ_DIV10_4  | IN        | REG_SON4_FREQ_DIV10_HZ | 0x0171 | Frequency ÷10 input → stored as Hz÷10 (efficient) |
 * | 5   | FREQ_LOCK_4   | IN        | REG_SON4_STATUS_FLAGS | 0x0172 | Frequency lock status → bit 2 (SON_STATUS_FREQ_LOCK) |
 * | 17  | OVERLOAD_4    | IN        | REG_SON4_STATUS_FLAGS | 0x0172 | Overload detection → bit 1 (SON_STATUS_OVERLOAD) |
 * | 22  | START_4       | OUT       | REG_SON4_START_STOP | 0x0160 | Start/enable control ← MODBUS command |
 * | 23  | RESET_4       | OUT       | REG_SON4_OVERLOAD_RESET | 0x0162 | Reset control ← MODBUS pulse command |
 * | 33  | POWER_SENSE_4 | IN (ADC)  | REG_SON4_POWER_RAW_ADC | 0x0170 | Power level analog → raw ADC (0-1023, cloud converts) |
 * | 21  | AMPLITUDE_ALL | OUT (PWM) | REG_AMPLITUDE_ALL_SP | 0x0161 | Amplitude control ← MODBUS setpoint (20-100%) |
 *
 * @subsection system_pins System/Communication Pin-to-MODBUS Register Mapping
 * | ATMega32A Pin | Signal     | Direction | MODBUS Register | Address | Description |
 * |---------------|------------|-----------|-----------------|---------|-------------|
 * | 14            | UART_RXD   | IN        | ALL_REGISTERS   | ALL     | MODBUS RTU receive data (115200 baud, 8N1) |
 * | 15            | UART_TXD   | OUT       | ALL_REGISTERS   | ALL     | MODBUS RTU transmit data (115200 baud, 8N1) |
 * | 16            | STATUS_LED | OUT       | REG_SYSTEM_STATUS | 0x0000 | System status LED ← system status bits |
 *
 * @note DB9-0 is the communications connector for MODBUS RTU network
 * @note All sonicator channels share the AMPLITUDE_ALL signal on pin 21
 *
 * @subsection pin_register_mapping_summary Pin-to-Register Mapping Summary
 * @details This section shows the complete mapping between ATmega32A physical pins and MODBUS registers
 * @details **Data Flow Direction:**
 * - **IN (Monitor)**: Physical pin → ATmega32A → MODBUS register (Sonicator status to PLC)
 * - **OUT (Control)**: MODBUS register → ATmega32A → Physical pin (PLC commands to Sonicator)
 *
 * @details **Complete Pin-to-Register Mapping Table:**
 * | ATmega32A Pin | Signal | Dir | Sonicator | MODBUS Register | Address | Data Flow |
 * |---------------|--------|-----|-----------|-----------------|---------|-----------|
 * | 1  | FREQ_DIV10_4  | IN  | 4 | REG_SON4_FREQ_DIV10_HZ | 0x0171 | CT2000 freq÷10 → stored as Hz÷10 |
 * | 2  | FREQ_DIV10_3  | IN  | 3 | REG_SON3_FREQ_DIV10_HZ | 0x0151 | CT2000 freq÷10 → stored as Hz÷10 |
 * | 3  | FREQ_DIV10_2  | IN  | 2 | REG_SON2_FREQ_DIV10_HZ | 0x0131 | CT2000 freq÷10 → stored as Hz÷10 |
 * | 4  | FREQ_DIV10_1  | IN  | 1 | REG_SON1_FREQ_DIV10_HZ | 0x0111 | CT2000 freq÷10 → stored as Hz÷10 |
 * | 5  | FREQ_LOCK_4   | IN  | 4 | REG_SON4_STATUS_FLAGS | 0x0172 | CT2000 freq lock → status bit 2 |
 * | 6  | FREQ_LOCK_3   | IN  | 3 | REG_SON3_STATUS_FLAGS | 0x0152 | CT2000 freq lock → status bit 2 |
 * | 7  | FREQ_LOCK_2   | IN  | 2 | REG_SON2_STATUS_FLAGS | 0x0132 | CT2000 freq lock → status bit 2 |
 * | 8  | FREQ_LOCK_1   | IN  | 1 | REG_SON1_STATUS_FLAGS | 0x0112 | CT2000 freq lock → status bit 2 |
 * | 14 | UART_RXD      | IN  | - | ALL_REGISTERS         | ALL    | MODBUS RTU receive (115200 8N1) |
 * | 15 | UART_TXD      | OUT | - | ALL_REGISTERS         | ALL    | MODBUS RTU transmit (115200 8N1) |
 * | 16 | STATUS_LED    | OUT | - | REG_SYSTEM_STATUS     | 0x0000 | System status → LED indicator |
 * | 17 | OVERLOAD_4    | IN  | 4 | REG_SON4_STATUS_FLAGS | 0x0172 | CT2000 overload → status bit 1 |
 * | 18 | OVERLOAD_3    | IN  | 3 | REG_SON3_STATUS_FLAGS | 0x0152 | CT2000 overload → status bit 1 |
 * | 19 | OVERLOAD_2    | IN  | 2 | REG_SON2_STATUS_FLAGS | 0x0132 | CT2000 overload → status bit 1 |
 * | 20 | OVERLOAD_1    | IN  | 1 | REG_SON1_STATUS_FLAGS | 0x0112 | CT2000 overload → status bit 1 |
 * | 21 | AMPLITUDE_ALL | OUT | ALL | REG_SONx_AMPLITUDE_SP | 0x01x1 | MODBUS setpoint → PWM amplitude |
 * | 22 | START_4       | OUT | 4 | REG_SON4_START_STOP   | 0x0160 | MODBUS command → CT2000 start |
 * | 23 | RESET_4       | OUT | 4 | REG_SON4_OVERLOAD_RESET | 0x0162 | MODBUS pulse → CT2000 reset |
 * | 24 | START_3       | OUT | 3 | REG_SON3_START_STOP   | 0x0140 | MODBUS command → CT2000 start |
 * | 25 | RESET_3       | OUT | 3 | REG_SON3_OVERLOAD_RESET | 0x0142 | MODBUS pulse → CT2000 reset |
 * | 26 | START_2       | OUT | 2 | REG_SON2_START_STOP   | 0x0120 | MODBUS command → CT2000 start |
 * | 27 | RESET_2       | OUT | 2 | REG_SON2_OVERLOAD_RESET | 0x0122 | MODBUS pulse → CT2000 reset |
 * | 28 | START_1       | OUT | 1 | REG_SON1_START_STOP   | 0x0100 | MODBUS command → CT2000 start |
 * | 29 | RESET_1       | OUT | 1 | REG_SON1_OVERLOAD_RESET | 0x0102 | MODBUS pulse → CT2000 reset |
 * | 33 | POWER_SENSE_4 | IN  | 4 | REG_SON4_POWER_RAW_ADC | 0x0170 | CT2000 power → raw ADC (0-1023) |
 * | 34 | POWER_SENSE_3 | IN  | 3 | REG_SON3_POWER_RAW_ADC | 0x0150 | CT2000 power → raw ADC (0-1023) |
 * | 35 | POWER_SENSE_2 | IN  | 2 | REG_SON2_POWER_RAW_ADC | 0x0130 | CT2000 power → raw ADC (0-1023) |
 * | 36 | POWER_SENSE_1 | IN  | 1 | REG_SON1_POWER_RAW_ADC | 0x0110 | CT2000 power → raw ADC (0-1023) |
 *
 * @details **Pin Usage Summary:**
 * - **Total Pins Used**: 25 of 32 available ATmega32A pins
 * - **Digital Inputs**: 16 pins (frequency, freq_lock, overload monitoring)
 * - **Digital Outputs**: 9 pins (start, reset control + status LED)
 * - **Analog Inputs**: 4 pins (power sensing via ADC)
 * - **UART Communication**: 2 pins (MODBUS RTU)
 * - **Shared Signals**: 1 pin (AMPLITUDE_ALL controls all 4 sonicators)
 *
 * @section modbus_registers MODBUS Register Map
 * @details Complete MODBUS RTU register map for Multi-Sonicator I/O Controller
 * @details Slave ID: 2, Baud Rate: 115200, Format: 8N1
 * @details Response Time: <100ms per NFR2 requirements
 *
 * @subsection system_status_registers System Status Registers (0x0000-0x000F) - Read Only
 * | Address | Register Name | Access | Description |
 * |---------|---------------|--------|-------------|
 * | 0x0000 | REG_SYSTEM_STATUS | R | Overall system status bits |
 * | 0x0001 | REG_ACTIVE_COUNT | R | Number of active sonicators |
 * | 0x0002 | REG_ACTIVE_MASK | R | Bitmask of running units (b0-b3) |
 * | 0x0003 | REG_WATCHDOG_STATUS | R | Watchdog timer status |
 * | 0x0004 | REG_COMM_ERRORS | R | Communication error count |
 * | 0x0005 | REG_PREV_ACTIVE_MASK | R | Previous active units mask |
 * | 0x0006 | REG_LAST_SHUTDOWN_REASON | R | Last shutdown reason code |
 * | 0x0007 | REG_SYSTEM_UPTIME_LO | R | System uptime low 16 bits |
 * | 0x0008 | REG_SYSTEM_UPTIME_HI | R | System uptime high 16 bits |
 *
 * @subsection global_control_registers Global Control Registers (0x0010-0x001F) - Read/Write
 * | Address | Register Name | Access | Description |
 * |---------|---------------|--------|-------------|
 * | 0x0010 | REG_GLOBAL_ENABLE | R/W | Global system enable (0=disabled, 1=enabled) |
 * | 0x0011 | REG_EMERGENCY_STOP | R/W | Emergency stop command (1=activate) |
 * | 0x0012 | REG_SYSTEM_RESET | R/W | System reset command (special code) |
 * | 0x0013 | REG_TEST_START_INHIBIT | R/W | Per-unit start inhibit mask (b0-b3) |
 * | 0x0014 | REG_GLOBAL_AMPLITUDE | R/W | Global amplitude setpoint (20-100%) |
 *
 * @subsection sonicator1_registers Sonicator 1 Registers (ID=0, 0x0100-0x011F)
 * | Address | Register Name | Access | Description |
 * |---------|---------------|--------|-------------|
 * | 0x0100 | REG_SON1_START_STOP | R/W | Start/Stop control (0=stop, 1=start) |
 * | 0x0101 | REG_SON1_AMPLITUDE_SP | R/W | Amplitude setpoint (20-100%) |
 * | 0x0102 | REG_SON1_OVERLOAD_RESET | W | Overload reset command (pulse 1→0) |
 * | 0x0110 | REG_SON1_POWER_RAW_ADC | R | Raw ADC power reading (0-1023) |
 * | 0x0111 | REG_SON1_FREQ_DIV10_HZ | R | Operating frequency ÷10 (Hz÷10) |
 * | 0x0112 | REG_SON1_STATUS_FLAGS | R | Status flags bitfield |
 * | 0x0113 | REG_SON1_AMPLITUDE_ACT | R | Actual amplitude reading |
 * | 0x0114 | REG_SON1_PREV_STATE | R | Previous state enum |
 * | 0x0115 | REG_SON1_PERSISTED_AMPLITUDE | R | Last setpoint before shutdown |
 * | 0x0116 | REG_SON1_LAST_FAULT_CODE | R | Last fault code |
 * | 0x0117 | REG_SON1_LAST_STATE_TIMESTAMP_LO | R | Low 16 bits of timestamp |
 *
 * @subsection sonicator2_registers Sonicator 2 Registers (ID=1, 0x0120-0x013F)
 * | Address | Register Name | Access | Description |
 * |---------|---------------|--------|-------------|
 * | 0x0120 | REG_SON2_START_STOP | R/W | Start/Stop control (0=stop, 1=start) |
 * | 0x0121 | REG_SON2_AMPLITUDE_SP | R/W | Amplitude setpoint (20-100%) |
 * | 0x0122 | REG_SON2_OVERLOAD_RESET | W | Overload reset command (pulse 1→0) |
 * | 0x0130 | REG_SON2_POWER_RAW_ADC | R | Raw ADC power reading (0-1023) |
 * | 0x0131 | REG_SON2_FREQ_DIV10_HZ | R | Operating frequency ÷10 (Hz÷10) |
 * | 0x0132 | REG_SON2_STATUS_FLAGS | R | Status flags bitfield |
 * | 0x0133 | REG_SON2_AMPLITUDE_ACT | R | Actual amplitude reading |
 * | 0x0134 | REG_SON2_PREV_STATE | R | Previous state enum |
 * | 0x0135 | REG_SON2_PERSISTED_AMPLITUDE | R | Last setpoint before shutdown |
 * | 0x0136 | REG_SON2_LAST_FAULT_CODE | R | Last fault code |
 * | 0x0137 | REG_SON2_LAST_STATE_TIMESTAMP_LO | R | Low 16 bits of timestamp |
 *
 * @subsection sonicator3_registers Sonicator 3 Registers (ID=2, 0x0140-0x015F)
 * | Address | Register Name | Access | Description |
 * |---------|---------------|--------|-------------|
 * | 0x0140 | REG_SON3_START_STOP | R/W | Start/Stop control (0=stop, 1=start) |
 * | 0x0141 | REG_SON3_AMPLITUDE_SP | R/W | Amplitude setpoint (20-100%) |
 * | 0x0142 | REG_SON3_OVERLOAD_RESET | W | Overload reset command (pulse 1→0) |
 * | 0x0150 | REG_SON3_POWER_RAW_ADC | R | Raw ADC power reading (0-1023) |
 * | 0x0151 | REG_SON3_FREQ_DIV10_HZ | R | Operating frequency ÷10 (Hz÷10) |
 * | 0x0152 | REG_SON3_STATUS_FLAGS | R | Status flags bitfield |
 * | 0x0153 | REG_SON3_AMPLITUDE_ACT | R | Actual amplitude reading |
 * | 0x0154 | REG_SON3_PREV_STATE | R | Previous state enum |
 * | 0x0155 | REG_SON3_PERSISTED_AMPLITUDE | R | Last setpoint before shutdown |
 * | 0x0156 | REG_SON3_LAST_FAULT_CODE | R | Last fault code |
 * | 0x0157 | REG_SON3_LAST_STATE_TIMESTAMP_LO | R | Low 16 bits of timestamp |
 *
 * @subsection sonicator4_registers Sonicator 4 Registers (ID=3, 0x0160-0x017F)
 * | Address | Register Name | Access | Description |
 * |---------|---------------|--------|-------------|
 * | 0x0160 | REG_SON4_START_STOP | R/W | Start/Stop control (0=stop, 1=start) |
 * | 0x0161 | REG_SON4_AMPLITUDE_SP | R/W | Amplitude setpoint (20-100%) |
 * | 0x0162 | REG_SON4_OVERLOAD_RESET | W | Overload reset command (pulse 1→0) |
 * | 0x0170 | REG_SON4_POWER_RAW_ADC | R | Raw ADC power reading (0-1023) |
 * | 0x0171 | REG_SON4_FREQ_DIV10_HZ | R | Operating frequency ÷10 (Hz÷10) |
 * | 0x0172 | REG_SON4_STATUS_FLAGS | R | Status flags bitfield |
 * | 0x0173 | REG_SON4_AMPLITUDE_ACT | R | Actual amplitude reading |
 * | 0x0174 | REG_SON4_PREV_STATE | R | Previous state enum |
 * | 0x0175 | REG_SON4_PERSISTED_AMPLITUDE | R | Last setpoint before shutdown |
 * | 0x0176 | REG_SON4_LAST_FAULT_CODE | R | Last fault code |
 * | 0x0177 | REG_SON4_LAST_STATE_TIMESTAMP_LO | R | Low 16 bits of timestamp |
 *
 * @subsection register_usage_notes Register Usage Notes
 * @details **Control Registers (Write Operations):**
 * - Start/Stop: Write 1 to start, 0 to stop sonicator
 * - Amplitude Setpoint: Valid range 20-100% (values outside range are clamped)
 * - Overload Reset: Write 1 then 0 to pulse reset (write-and-clear semantics)
 * - Emergency Stop: Write 1 to activate emergency stop for all units
 * - Global Enable: Must be 1 for any sonicator to operate
 *
 * @details **Status Registers (Read Operations):**
 * - Power Raw ADC: Raw 10-bit ADC reading (0-1023) from CT2000 power output
 * - Frequency ÷10 Hz: CT2000 freq÷10 signal stored directly (multiply by 10 for actual Hz)
 * - Status Flags: Bitfield with running, overload, freq_lock, comm_fault, fault bits
 * - Active Count/Mask: System-wide count and bitmask of currently running units
 *
 * @details **Raw Data Efficiency Benefits:**
 * - Power registers store raw ADC values (0-1023) without conversion
 * - Frequency registers store Hz÷10 values directly from CT2000 pin 4
 * - No floating-point math in ATmega32A, maximizing real-time performance
 * - Cloud/PLC performs all scaling and unit conversions for flexibility
 * - Raw data preserves full precision and allows calibration adjustments
 *
 * @details **Cloud Conversion Formulas:**
 * - **Power (Watts)**: `Power = (ADC_Raw * 5.0V / 1023) / 0.00544V/W`
 * - **Frequency (Hz)**: `Frequency = Freq_Div10_Raw * 10`
 * - **Voltage**: `Voltage = ADC_Raw * 5.0V / 1023`
 * - **Typical ADC Values**: 0-1023 (0-5V), 0-919 for max measurable power
 *
 * @details **Status Flag Bits:**
 * - Bit 0 (0x0001): SON_STATUS_RUNNING - Sonicator is running
 * - Bit 1 (0x0002): SON_STATUS_OVERLOAD - Overload detected
 * - Bit 2 (0x0004): SON_STATUS_FREQ_LOCK - Frequency lock achieved
 * - Bit 3 (0x0008): SON_STATUS_COMM_FAULT - Communication fault
 * - Bit 4 (0x0010): SON_STATUS_OVER_TEMP - Over temperature
 * - Bit 5 (0x0020): SON_STATUS_FAULT - General fault
 *
 * @details **System Status Bits:**
 * - Bit 0 (0x0001): SYSTEM_STATUS_OK - System operating normally
 * - Bit 1 (0x0002): SYSTEM_STATUS_FAULT - System fault detected
 * - Bit 2 (0x0004): SYSTEM_STATUS_EMERGENCY_STOP - Emergency stop active
 * - Bit 3 (0x0008): SYSTEM_STATUS_COMM_FAULT - Communication fault
 * - Bit 4 (0x0010): SYSTEM_STATUS_OVERTEMP - Over temperature detected
 *
 * @details **Performance Requirements:**
 * - Register updates: ≤100ms response time (NFR2)
 * - Telemetry refresh: ≥10Hz update rate (NFR9)
 * - Command latency: ≤200ms end-to-end (NFR1)
 * - MODBUS RTU: 115200 baud, 8N1, Slave ID=2
 */

/**
 * @struct SonicatorPins
 * @brief Defines the hardware pins for a single sonicator channel.
 *
 * @note All outputs are assumed active-high. If your hardware differs,
 *       normalize polarity in the HAL layer or handle it explicitly here.
 */
struct SonicatorPins {
    uint8_t start_pin;                 ///< Output: start/enable control line
    uint8_t reset_pin;                 ///< Output: reset/clear overload (pulse for SONICATOR_RESET_PULSE_MS)
    uint8_t overload_pin;              ///< Input: overload status from sonicator
    uint8_t freq_lock_pin;             ///< Input: frequency-lock indicator
    uint8_t operating_freq_div10;            ///< Input: frequency/10 measurement source
    adc_channel_t power_sense_channel; ///< Analog input channel for power sensing
};


#endif // SONICATOR_PINS_H
