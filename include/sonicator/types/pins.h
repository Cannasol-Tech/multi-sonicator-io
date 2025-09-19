#ifndef SONICATOR_PINS_H
#define SONICATOR_PINS_H
#include "modules/hal/adc.h"

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
 * @note Dir = Direction from DUT (ATMega32a) Perspective (IN/OUT)s
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
 
 
 * @subsection sonicator1_pxins Sonicator 1 ATMega32a Pin Assignment
 * | Pin | Signal        | Description                    |
 * |-----|---------------|--------------------------------|
 * | 4   | FREQ_DIV10_1  | Frequency ÷10 input           |
 * | 8   | FREQ_LOCK_1   | Frequency lock status input   |
 * | 20  | OVERLOAD_1    | Overload detection input      |
 * | 28  | START_1       | Start/enable control output   |
 * | 29  | RESET_1       | Reset control output          |
 * | 36  | POWER_SENSE_1 | Power level analog input      |
 * | 21  | AMPLITUDE_ALL | Amplitude control (shared)    |
 *
 * @subsection sonicator2_pins Sonicator 2 ATMega32a Pin Assignment
 * | Pin | Signal        | Description                    |
 * |-----|---------------|--------------------------------|
 * | 3   | FREQ_DIV10_2  | Frequency ÷10 input           |
 * | 7   | FREQ_LOCK_2   | Frequency lock status input   |
 * | 19  | OVERLOAD_2    | Overload detection input      |
 * | 26  | START_2       | Start/enable control output   |
 * | 27  | RESET_2       | Reset control output          |
 * | 35  | POWER_SENSE_2 | Power level analog input      |
 * | 21  | AMPLITUDE_ALL | Amplitude control (shared)    |
 *
 * @subsection sonicator3_pins Sonicator 3 ATMega32a Pin Assignment
 * | Pin | Signal        | Description                    |
 * |-----|---------------|--------------------------------|
 * | 2   | FREQ_DIV10_3  | Frequency ÷10 input           |
 * | 6   | FREQ_LOCK_3   | Frequency lock status input   |
 * | 18  | OVERLOAD_3    | Overload detection input      |
 * | 24  | START_3       | Start/enable control output   |
 * | 25  | RESET_3       | Reset control output          |
 * | 34  | POWER_SENSE_3 | Power level analog input      |
 * | 21  | AMPLITUDE_ALL | Amplitude control (shared)    |
 *
 * @subsection sonicator4_pins Sonicator 4 ATMega32a Pin Assignment
 * | Pin | Signal        | Description                    |
 * |-----|---------------|--------------------------------|
 * | 1   | FREQ_DIV10_4  | Frequency ÷10 input           |
 * | 5   | FREQ_LOCK_4   | Frequency lock status input   |
 * | 17  | OVERLOAD_4    | Overload detection input      |
 * | 22  | START_4       | Start/enable control output   |
 * | 23  | RESET_4       | Reset control output          |
 * | 33  | POWER_SENSE_4 | Power level analog input      |
 * | 21  | AMPLITUDE_ALL | Amplitude control (shared)    |
 *
 * @subsection system_pins System/Communication Pins
 * | ATMega32A Pin | Signal     | Description                      |
 * |---------------|------------|----------------------------------|
 * | 14            | UART_RXD   | MODBUS RTU receive data         |
 * | 15            | UART_TXD   | MODBUS RTU transmit data        |
 * | 16            | STATUS_LED | System status LED output        |
 *
 * @note DB9-0 is the communications connector for MODBUS RTU network
 * @note All sonicator channels share the AMPLITUDE_ALL signal on pin 21
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
    uint8_t freq_div10_pin;            ///< Input: frequency/10 measurement source
    adc_channel_t power_sense_channel; ///< Analog input channel for power sensing
};


#endif // SONICATOR_PINS_H
