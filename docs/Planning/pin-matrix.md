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

## Table

| # | DUT Pin (Port/Pad) | Header Ref | Signal        | Dir    | Scale/Notes                         | Wrapper Pin | Test Point |
|---|---------------------|------------|---------------|--------|------------------------------------|-------------|------------|
| 1 | TBD                 | TBD        | OVERLOAD_1    | IN     | Active-high? Debounce req?         | TBD         | TBD        |
| 2 | TBD                 | TBD        | FREQ_DIV10_1  | IN     | Frequency ÷10 input (Hz = count*10)| TBD         | TBD        |
| 3 | TBD                 | TBD        | FREQ_LOCK_1   | IN     | Lock indication                     | TBD         | TBD        |
| 4 | TBD                 | TBD        | START_1       | OUT    | Drive level, min pulse width        | TBD         | TBD        |
| 5 | TBD                 | TBD        | RESET_1       | OUT    | Pulse semantics per PRD             | TBD         | TBD        |
| 6 | TBD                 | TBD        | POWER_SENSE_1 | ANALOG | 5.44 mV/W scaling                   | TBD         | TBD        |
| 7 | TBD                 | TBD        | AMPLITUDE_1   | OUT    | 0–10 V path via DAC/PWM stage       | TBD         | TBD        |
| … | …                   | …          | …             | …      | …                                  | …           | …          |

Add rows for units 2–4. Duplicate structure for each channel.

## Actions on Approval
- Update `include/config.h` to match matrix, with doxygen comments citing this file and PRD sections.
- Update `test/hardware/arduino_test_wrapper/arduino_test_wrapper.ino` constants to match Wrapper Pins.
- Update `test/hardware/README.md` harness diagram and mapping.

## References
- `docs/Requirements/prd-v1.0.0.md` — Sections 5, 7, 10
- `include/config.h` — Register map, constants
- Schematics/Hardware docs — pin headers and electrical characteristics
