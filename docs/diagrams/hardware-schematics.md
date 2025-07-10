# PWM Amplitude Control - Schematic Diagrams

## Complete System Block Diagram

```text
┌────────────────────────────────────────────────────────────────────────────┐
│                        Multi-Sonicator I/O Controller                      │
│                                                                            │
│  ┌──────────────┐     ┌─────────────────────────────────────────────────┐  │
│  │   DB9        │     │                ATmega32A                        │  │
│  │   Power      │     │                                                 │  │
│  │   +24VDC     │     │  ┌─────────┐  ┌─────────┐  ┌─────────┐          │  │
│  └──────┬───────┘     │  │  PWM    │  │  ADC    │  │  UART   │          │  │
│         │             │  │ Outputs │  │ Inputs  │  │ MODBUS  │          │  │
│    ┌────▼────┐        │  └────┬────┘  └────┬────┘  └────┬────┘          │  │
│    │ LM2596  │        │       │             │             │             │  │
│    │ 24V→12V │        └───────┼─────────────┼─────────────┼───────────-─┘  │
│    └────┬────┘                │             │             │                │
│         │                     │             │             │                │
│    ┌────▼────┐           ┌────▼────┐    ┌───▼───┐     ┌───▼───┐            │
│    │ LM7805  │           │  PWM    │    │  ADC  │     │ RS-485 │           │
│    │ 12V→5V  │           │ Filters │    │Buffer │     │ Driver │           │
│    └─────────┘           └────┬────┘    └───┬───┘     └───┬───┘            │
│                               │             │             │                │
│                          ┌────▼────┐        │             │                │
│                          │ Op-Amps │        │             │                │
│                          │ 0-10V   │        │             │                │
│                          └────┬────┘        │             │                │ d
│                               │             │             │                │
└───────────────────────────────┼─────────────┼─────────────┼────────────────┘
                                │             │             │
                                ▼             ▼             ▼
                          To Sonicators  From Sonicators  To Main Box
                          (Amplitude)    (Power Monitor)   (MODBUS)
```

## PWM Amplitude Control Circuit (Per Sonicator)

```text
                              +12V
                               │
                               ├─────────────────────┐
                               │                     │
                               │                    ┌┴┐
                               │                    │ │ 10kΩ
                               │                    │ │
                               │                    └┬┘
                               │      ┌──────────────┤
                               │      │              │
                           C1  │      │   U1A        │
ATmega32A              R1  10µF│   ┌──┴───┐         │
PWM Pin ────────[1kΩ]─────┬────┤───┤+     │         │
(PB3/PD5-7)               │    │   │      ├─────────┴──── 0-10V Output
                          │    └───┤-     │              (DB9 Pin 8)
                          │        │      │
                          │        └──┬───┘
                          │           │
                          │          ┌┴┐
                          │          │ │ 10kΩ
                          │          │ │
                          │          └┬┘
                          │           │
                         ─┴─         ─┴─
                         GND         GND

Component Values:
- R1: 1kΩ (PWM filter resistor)
- C1: 10µF (PWM filter capacitor)
- U1A: LM358 (1/2 of dual op-amp)
- Gain: 2x (10kΩ/10kΩ + 1)
- Cutoff frequency: fc = 1/(2πRC) = 16Hz
```

## Power Monitoring ADC Input (Per Sonicator)

```text
From Sonicator                    ATmega32A
DB9 Pin 5                         ADC Input
(Power Output)                    (PA4-PA7)
     │                                │
     │          R1                    │
     ├─────────[10kΩ]─────────────────┤
     │                                │
     │          R2        D1          │
     └─────────[10kΩ]─────┬───│>├─────┤
                          │           │
                          │    D2     │
                          ├───│<├─────┤
                          │           │
                         ─┴─         ─┴─
                         GND         +5V

Component Values:
- R1: 10kΩ (input protection)
- R2: 10kΩ (voltage divider)
- D1: 1N4148 (5V clamp diode)
- D2: 1N4148 (GND clamp diode)

Signal Scaling:
- Input: 0-10.88V (2000W × 5.44mV/W)
- After divider: 0-5.44V
- ADC reading: 0-1024 counts
- Resolution: ~2 watts per count
```

## Digital I/O Interface (Per Sonicator)

```text
Optocoupler Input (Overload, Freq Lock)
                    +5V
                     │
                    ┌┴┐
                    │ │ 4.7kΩ
                    │ │
                    └┬┘
From Sonicator       │         To ATmega32A
DB9 Pin 1/3 ─────────┴─────────── Digital Input
(Open Collector)               (PD2,PD4,etc)

Relay Output (Start Control, Reset)
                    +12V
                     │
                     │ RLY1
                    ┌┴──────┐
ATmega32A      ┌────┤RELAY  ├──── To Sonicator
Digital Out ───┤B   │       │     DB9 Pin 2/7
(via ULN2003)  └────┤       ├──── (Contact Closure)
                    └───────┘
```

## Complete Sonicator Interface

```text
┌──────────────────────────────────────────────────────────────────┐
│                    Single Sonicator Interface                    │
│                                                                  │
│  ATmega32A Pins                              DB9 Female          │
│                                              Connector           │
│                                                                  │
│  PD2 ◄──[Optocoupler]──────────────────────── Pin 1 (Overload)   │
│                                                                  │
│  PD5 ──►[ULN2003]──[Relay]─────────────────── Pin 2 (Reset)      │
│                                                                  │
│  PD4 ◄──[Optocoupler]──────────────────────── Pin 3 (Freq Lock)  │
│                                                                  │
│  PD3 ◄──[Timer/Counter Input]──────────────── Pin 4 (Freq Out)   │
│                                                                  │
│  PA4 ◄──[Protection + Divider]─────────────── Pin 5 (Power Out)  │
│                                                                  │
│  GND ───────────────────────────────────────── Pin 6 (Signal GND)│
│                                                                  │
│  PD5 ──►[ULN2003]──[Relay]─────────────────── Pin 7 (Start)      │
│                                                                  │
│  PB3 ──►[PWM Filter]──[Op-Amp]─────────────── Pin 8 (Amplitude)  │
│                                                                  │
│  GND ───────────────────────────────────────── Pin 9 (Common)    │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

## Power Supply Section

```text
DB9 Power Input                +12V Output           +5V Output
(From Main Box)                     │                     │
     │                              │      L2            │
     │         L1                   │      ┯┯┯┯─────┬───┤
     ├────────┯┯┯┯──────┬──────────┼──────┯===      │   │
     │         ===      │          │                │   │
     │                  │          │   ┌────────────┴┐  │
     │    ┌─────────────┴───┐      │   │   LM7805   ├──┤
     ├────┤ LM2596 Module   ├──────┴───┤  (Linear)   │  │
     │    │ (Buck Converter)│          └────────────┬┘  │
     │    └─────────────┬───┘                       │   │
     │                  │              C3           │   │
     │     C1           │             ┤├───────────┼───┤
     ├────┤├────────────┼─────────────100µF         │   │
     │    100µF         │                           │   │
     │                  │                           │   │
    ─┴─                ─┴─                         ─┴─ ─┴─
    GND                GND                         GND GND

DB9 Power Input Connections:
- Pin 1: +24VDC (from Main Automation Box)
- Pin 4: Power Ground Return
- Cable carries both power and MODBUS communication

Additional Decoupling:
- C2: 10µF ceramic at 12V
- C4: 10µF ceramic at 5V
- C5-C8: 0.1µF ceramic at each IC
- Input protection: TVS diode and fuse on DB9 power input
```

## MODBUS Communication Interface

```text
ATmega32A                    MAX485                   DB9 Male
                            RS-485                   (To Main Box)
                          Transceiver
   
TXD (PD1) ──────────────── DI                           
                                                     Pin 2 (TX)
RXD (PD0) ──────────────── RO        A ─────────────────┤
                                                         │
           ┌──────────────┬ RE       B ─────────────────┤
           │              │                          Pin 3 (RX)
           └──────────────┴ DE
                                                     Pin 5 (GND)
Direction ─────────────────┘                        ─────┤
Control

Note: Same DB9 cable carries both MODBUS and +24VDC power
- Pin 1: +24VDC power input
- Pin 4: Power ground return
- Pins 2,3,5: MODBUS RS-485 communication
```

## PCB Layout Considerations

```text
┌─────────────────────────────────────────────────────────────┐
│                         PCB Top View                        │
│                                                             │
│  [DB9]  ┌──────┐  ┌────────────────────────┐  ┌────────┐  │
│  PWR    │Power │  │                        │  │ MODBUS │  │
│   IN    │Supply│  │      ATmega32A         │  │ RS-485 │  │
│         │Reg.  │  │      (Central)         │  │        │  │
│         └──────┘  └────────────────────────┘  └────────┘  │
│                                                             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │
│  │ Sonicator 1 │  │ Sonicator 2 │  │ Sonicator 3 │        │
│  │  Interface  │  │  Interface  │  │  Interface  │        │
│  │   Section   │  │   Section   │  │   Section   │        │
│  └─────────────┘  └─────────────┘  └─────────────┘        │
│                                                             │
│  ┌─────────────┐  ┌──────────────────────────────┐        │
│  │ Sonicator 4 │  │   PWM Filters & Op-Amps     │        │
│  │  Interface  │  │      (All 4 channels)       │        │
│  │   Section   │  │                              │        │
│  └─────────────┘  └──────────────────────────────┘  [ISP] │
│                                                             │
│  Ground Plane: Uninterrupted on Layer 2                    │
│  Crystal: <10mm from MCU with guard ring                   │
│  DB9 Power Input: Corner with protection circuitry         │
└─────────────────────────────────────────────────────────────┘
```

## Component Placement Guidelines

1. **Crystal Circuit**: Place within 10mm of ATmega32A pins 8-9
2. **PWM Filters**: Group near DB9 connectors to minimize trace length
3. **Op-Amps**: Place close to filters with short feedback paths
4. **Power Supply**: Corner placement with thermal considerations and DB9 input protection
5. **Decoupling Caps**: Within 5mm of each IC power pin
6. **ISP Header**: Edge placement for easy access
7. **DB9 Power Input**: Include TVS diode and fuse protection near connector

## Signal Integrity Notes

- Keep PWM traces short and away from sensitive analog signals
- Use ground pour between digital and analog sections
- Route differential MODBUS signals as paired traces
- Minimize vias on high-frequency signals
- Add test points for PWM, filtered output, and final 0-10V
- Protect DB9 power input with EMI filtering and overvoltage protection
- Separate power regulation section from sensitive analog circuits
