# ATmega32A Pin Assignment Documentation

This document details the final pin assignments for the Multi Sonicator I/O Controller, implementing the required configuration changes for the project.

## Microcontroller: ATmega32A-PU (40-pin DIP)

The following table provides a complete overview of the pin assignments:

| Physical Pin | Port Pin | Function                        | Description                         |
|--------------|----------|---------------------------------|-------------------------------------|
| **1**        | PB0      | `SON1_FREQ_OUTPUT_PIN`          | Sonicator 1 Frequency Output (Timer0) |
| **2**        | PB1      | `SON1_FREQ_LOCK_PIN`            | Sonicator 1 Frequency Lock          |
| **3**        | PB2      | `SON2_FREQ_LOCK_PIN`            | Sonicator 2 Frequency Lock          |
| **4**        | PB3      | `SON2_FREQ_OUTPUT_PIN`          | Sonicator 2 Frequency Output (Timer2) |
| **5**        | PB4      | `SON3_FREQ_LOCK_PIN`            | Sonicator 3 Frequency Lock          |
| **6**        | PB5      | `LED_RGB_BLUE_PIN`              | Status LED Blue                     |
| **7**        | PB6      | `LED_RGB_RED_PIN`               | Status LED Red                      |
| **8**        | PB7      | `LED_RGB_GREEN_PIN`             | Status LED Green                    |
| **9**        | RESET    | Reset                           | System Reset                        |
| **10**       | VCC      | Power                           | +5V Power                           |
| **11**       | GND      | Ground                          | Ground                              |
| **12**       | XTAL2    | Crystal                         | 16MHz Crystal                       |
| **13**       | XTAL1    | Crystal                         | 16MHz Crystal                       |
| **14**       | PD0      | `UART_RX_PIN`                   | MODBUS RTU Receive                  |
| **15**       | PD1      | `UART_TX_PIN`                   | MODBUS RTU Transmit                 |
| **16**       | PD2      | `SON1_OVERLOAD_PIN`             | Sonicator 1 Overload                |
| **17**       | PD3      | `SON2_OVERLOAD_PIN`             | Sonicator 2 Overload                |
| **18**       | PD4      | `SON3_OVERLOAD_PIN`             | Sonicator 3 Overload                |
| **19**       | PD5      | `SON4_OVERLOAD_PIN`             | Sonicator 4 Overload                |
| **20**       | PD6      | `SON3_FREQ_OUTPUT_PIN`          | Sonicator 3 Frequency Output (Timer1) |
| **21**       | PD7      | `SON4_FREQ_LOCK_PIN`            | Sonicator 4 Frequency Lock          |
| **22**       | PC0      | `SON4_START_PIN`                | Sonicator 4 Start Control           |
| **23**       | PC1      | `SON4_RESET_PIN`                | Sonicator 4 Reset Control           |
| **24**       | PC2      | `SON3_START_PIN`                | Sonicator 3 Start Control           |
| **25**       | PC3      | `SON3_RESET_PIN`                | Sonicator 3 Reset Control           |
| **26**       | PC4      | `SON2_START_PIN`                | Sonicator 2 Start Control           |
| **27**       | PC5      | `SON2_RESET_PIN`                | Sonicator 2 Reset Control           |
| **28**       | PC6      | `SON1_START_PIN`                | Sonicator 1 Start Control           |
| **29**       | PC7      | `SON1_RESET_PIN`                | Sonicator 1 Reset Control           |
| **30**       | AVCC     | Power                           | ADC Power                           |
| **31**       | GND      | Ground                          | Ground                              |
| **32**       | AREF     | Reference                       | ADC Reference                       |
| **33**       | PA7      | `ADC_SONICATOR_4_PIN`           | Sonicator 4 Power Monitor           |
| **34**       | PA6      | `ADC_SONICATOR_3_PIN`           | Sonicator 3 Power Monitor           |
| **35**       | PA5      | `ADC_SONICATOR_2_PIN`           | Sonicator 2 Power Monitor           |
| **36**       | PA4      | `ADC_SONICATOR_1_PIN`           | Sonicator 1 Power Monitor           |
| **37**       | PA3      | **Available**                   | **Free Analog Input**               |
| **38**       | PA2      | **Available**                   | **Free Analog Input**               |
| **39**       | PA1      | `PWM_AMPLITUDE_CONTROL_PIN`     | **Single PWM for All 4 Sonicators** |
| **40**       | PA0      | `SON4_FREQ_OUTPUT_PIN`          | Sonicator 4 Frequency Output (via LM2907) |

## Key Features

- **Overload Monitoring (PD2-PD5):** All sonicators' overloads consolidated to sequential pins.
- **Frequency Lock and Outputs:** Properly distributed among supported pins.
- **Single PWM Output:** Implemented as requested, using software PWM on PA1.
- **Status LED Indication:** RGB LED grouped on PB5-PB7.
- **Clean and Logical Arrangement:** All functionalities organized for clarity and efficiency.

