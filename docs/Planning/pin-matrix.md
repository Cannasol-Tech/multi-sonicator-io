# Pin Matrix — DUT ↔ Signals ↔ Wrapper/HIL

Source of truth for pin assignments. Changes here must be reflected in `include/config.h` and the HIL wrapper under `test/hardware/arduino_test_wrapper/`.

Status: DRAFT — to be reviewed and signed off before firmware implementation (Gate for Plan Task #0)

## Legend
- DUT: ATmega32A pin identifier (port/pin and header reference)
- Signal: Functional name (e.g., OVERLOAD, FREQ_DIV10, FREQ_LOCK, START, RESET, POWER_SENSE, AMPLITUDE_CTRL)
- Dir: Direction from DUT perspective (IN/OUT/ANALOG)
- Scale/Notes: Electrical notes, scaling (e.g., 5.44 mV/W), timing, pull-ups/downs
- Wrapper Pin: Arduino wrapper pin used in HIL (if applicable)
- Test Point: Bench header or connector label
- Header Ref: External connector reference. Each channel uses its own DB9: DB9-1 (S1), DB9-2 (S2), DB9-3 (S3), DB9-4 (S4). System comms use DB9-0. Pin map per channel DB9: Pin1=OVERLOAD, Pin2=RESET, Pin3=FREQ_LOCK, Pin4=FREQ÷10, Pin5=POWER, Pin7=START, Pin8=AMPLITUDE. DB9-0: Pin8=UART_RXD, Pin9=UART_TXD

### Sonicator 1

| # | DUT Pin (Port/Pad) | Header Ref | Signal        | Dir    | Scale/Notes                       | Wrapper Pin | Test Point |
|---|---------------------|------------|---------------|--------|----------------------------------|-------------|------------|
| 1 | PB3 (Pin 4)         | DB9-1 Pin 4| FREQ_DIV10_1  | IN     | ÷10 frequency (S1)               | TBD         | TBD        |
| 2 | PB7 (Pin 8)         | DB9-1 Pin 3| FREQ_LOCK_1   | IN     | FLCK_1 via opto                  | TBD         | TBD        |
| 3 | PD6 (Pin 20)        | DB9-1 Pin 1| OVERLOAD_1    | IN     | OL_1 via opto                    | TBD         | TBD        |
| 4 | PC0 (Pin 22)        | DB9-1 Pin 7| START_1       | OUT    | ULN2003A open-collector          | TBD         | TBD        |
| 5 | PC1 (Pin 23)        | DB9-1 Pin 2| RESET_1       | OUT    | ULN2003A open-collector          | TBD         | TBD        |
| 6 | PA4/A4 (Pin 36)     | DB9-1 Pin 5| POWER_SENSE_1 | ANALOG | 5.44 mV/W scaling                | TBD         | TBD        |
| 7 | PD7 (Pin 21)        | DB9-1 Pin 8| AMPLITUDE_ALL | OUT    | Shared AMP_C (0–10V), common     | TBD         | TBD        |

### Sonicator 2

| # | DUT Pin (Port/Pad) | Header Ref | Signal        | Dir    | Scale/Notes                       | Wrapper Pin | Test Point |
|---|---------------------|------------|---------------|--------|----------------------------------|-------------|------------|
| 1 | PB2 (Pin 3)         | DB9-2 Pin 4| FREQ_DIV10_2  | IN     | ÷10 frequency (S2)               | TBD         | TBD        |
| 2 | PB6 (Pin 7)         | DB9-2 Pin 3| FREQ_LOCK_2   | IN     | FLCK_2 via opto                  | TBD         | TBD        |
| 3 | PD5 (Pin 19)        | DB9-2 Pin 1| OVERLOAD_2    | IN     | OL_2 via opto                    | TBD         | TBD        |
| 4 | PC2 (Pin 24)        | DB9-2 Pin 7| START_2       | OUT    | ULN2003A open-collector          | TBD         | TBD        |
| 5 | PC3 (Pin 25)        | DB9-2 Pin 2| RESET_2       | OUT    | ULN2003A open-collector          | TBD         | TBD        |
| 6 | PA5/A5 (Pin 35)     | DB9-2 Pin 5| POWER_SENSE_2 | ANALOG | 5.44 mV/W scaling                | TBD         | TBD        |
| 7 | PD7 (Pin 21)        | DB9-2 Pin 8| AMPLITUDE_ALL | OUT    | Shared AMP_C (0–10V), common     | TBD         | TBD        |

### Sonicator 3

| # | DUT Pin (Port/Pad) | Header Ref | Signal        | Dir    | Scale/Notes                       | Wrapper Pin | Test Point |
|---|---------------------|------------|---------------|--------|----------------------------------|-------------|------------|
| 1 | PB1 (Pin 2)         | DB9-3 Pin 4| FREQ_DIV10_3  | IN     | ÷10 frequency (S3)               | TBD         | TBD        |
| 2 | PB5 (Pin 6)         | DB9-3 Pin 3| FREQ_LOCK_3   | IN     | FLCK_3 via opto                  | TBD         | TBD        |
| 3 | PD4 (Pin 18)        | DB9-3 Pin 1| OVERLOAD_3    | IN     | OL_3 via opto                    | TBD         | TBD        |
| 4 | PC4 (Pin 26)        | DB9-3 Pin 7| START_3       | OUT    | ULN2003A open-collector          | TBD         | TBD        |
| 5 | PC5 (Pin 27)        | DB9-3 Pin 2| RESET_3       | OUT    | ULN2003A open-collector          | TBD         | TBD        |
| 6 | PA6/A6 (Pin 34)     | DB9-3 Pin 5| POWER_SENSE_3 | ANALOG | 5.44 mV/W scaling                | TBD         | TBD        |
| 7 | PD7 (Pin 21)        | DB9-3 Pin 8| AMPLITUDE_ALL | OUT    | Shared AMP_C (0–10V), common     | TBD         | TBD        |

### Sonicator 4

| # | DUT Pin (Port/Pad) | Header Ref | Signal        | Dir    | Scale/Notes                       | Wrapper Pin | Test Point |
|---|---------------------|------------|---------------|--------|----------------------------------|-------------|------------|
| 1 | PB0 (Pin 1)         | DB9-4 Pin 4| FREQ_DIV10_4  | IN     | ÷10 frequency (S4)               | TBD         | TBD        |
| 2 | PB4 (Pin 5)         | DB9-4 Pin 3| FREQ_LOCK_4   | IN     | FLCK_4 via opto                  | TBD         | TBD        |
| 3 | PD3 (Pin 17)        | DB9-4 Pin 1| OVERLOAD_4    | IN     | OL_4 via opto                    | TBD         | TBD        |
| 4 | PC6 (Pin 28)        | DB9-4 Pin 7| START_4       | OUT    | ULN2003A open-collector          | TBD         | TBD        |
| 5 | PC7 (Pin 29)        | DB9-4 Pin 2| RESET_4       | OUT    | ULN2003A open-collector          | TBD         | TBD        |
| 6 | PA7/A7 (Pin 33)     | DB9-4 Pin 5| POWER_SENSE_4 | ANALOG | 5.44 mV/W scaling                | TBD         | TBD        |
| 7 | PD7 (Pin 21)        | DB9-4 Pin 8| AMPLITUDE_ALL | OUT    | Shared AMP_C (0–10V), common     | TBD         | TBD        |

### Common / System

Note: DB9-0 is the communications connector back to the main automation system (Modbus RTU network).

| # | DUT Pin (Port/Pad) | Header Ref | Signal     | Dir | Scale/Notes        | Wrapper Pin | Test Point |
|---|---------------------|------------|------------|-----|--------------------|-------------|------------|
| 1 | PD0 (Pin 14)        | DB9-0 Pin 8| UART_RXD   | IN  | MODBUS RTU RX     | TBD         | TBD        |
| 2 | PD1 (Pin 15)        | DB9-0 Pin 9| UART_TXD   | OUT | MODBUS RTU TX     | TBD         | TBD        |
| 3 | PD2 (Pin 16)        | LED-TERM   | STATUS_LED | OUT | Status LED drive  | TBD         | LED-TERM   |

## Schematic-derived mappings (from screenshot)
- **FREQ_DIV10_4..1**: PB0 (Pin 1), PB1 (Pin 2), PB2 (Pin 3), PB3 (Pin 4) mapped to header 1–4 (S4→S1).
- **FREQ_LOCK_4..1**: PB4 (Pin 5), PB5 (Pin 6), PB6 (Pin 7), PB7 (Pin 8) for S4..S1.
- **START/RESET (1..4)**: START_n on PC6/4/2/0 (Pins 28/26/24/22). RESET_n on PC7/5/3/1 (Pins 29/27/25/23).
- **POWER_SENSE 1..4**: PA4/A4 (Pin 36), PA5/A5 (Pin 35), PA6/A6 (Pin 34), PA7/A7 (Pin 33).
- **AMPLITUDE_ALL**: PD7 (Pin 21) labeled AMP_C.
- **OVERLOAD 1..4**: PD6 (Pin 20), PD5 (Pin 19), PD4 (Pin 18), PD3 (Pin 17) for S1..S4.

## Open discrepancies to resolve (doc ↔ `include/config.h`)
- **Amplitude control**: Schematic shows PD7 (Pin 21, `AMP_C`), but `config.h` defines `PWM_AMPLITUDE_CONTROL_PIN` = A1/PA1 (Pin 39).
- **Frequency lock pins**: Schematic indicates PB4..PB7 for FREQ_LOCK_4..1, while `config.h` maps `SONx_FREQ_LOCK_PIN` to PB1, PB2, PB4, PD7 respectively.
- **Overload pins**: Schematic indicates OL_1..4 at PD6, PD5, PD4, PD3 while `config.h` uses PD2..PD5.

## Actions on Approval
- Update `include/config.h` to match matrix, with doxygen comments citing this file and PRD sections.
- Update `test/hardware/arduino_test_wrapper/arduino_test_wrapper.ino` constants to match Wrapper Pins.
- Update `test/hardware/README.md` harness diagram and mapping.

## References
- `docs/Requirements/prd-v1.0.0.md` — Sections 5, 7, 10
- `include/config.h` — Register map, constants
- Schematics/Hardware docs — pin headers and electrical characteristics

