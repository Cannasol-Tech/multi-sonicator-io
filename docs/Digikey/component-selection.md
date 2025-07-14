# Updated Component Selection - PWM Design

## Hardware Implementation

### PCB Design Specifications

- **Layer Count:** 4-layer professional PCB
- **Dimensions:** 100mm × 80mm (fits standard industrial enclosure)
- **Layer Stack:** Signal/Ground/Power/Signal
- **Crystal Layout:** <10mm traces, ground guard, proper decoupling
- **Connector Types:** Industrial locking DB9, screw terminals

### Component Selection

| Category | Component | Part Number | Specifications | Notes |
|----------|-----------|-------------|----------------|-------|
| **Crystal** | 16MHz External | ECS-160-20-46X | ±20ppm, HC-49S, 20pF load | No change |  
| **Load Caps** | 22pF Ceramic | BC1005CT-ND | ±5%, 50V rating | No change |
| **Power Reg** | Buck Converter | LM2596 Module | 24V→12V, 3A capacity | No change |
| **Linear Reg** | 5V Regulator | LM7805 | 12V→5V, 1A capacity | No change |
| **PWM Filter** | 1kΩ Resistor | CF14JT1K00CT-ND | 1/4W, 5% | **NEW: 4x for PWM filters** |
| **PWM Filter** | 10µF Capacitor | 1276-1119-1-ND | 16V, X7R ceramic | **NEW: 4x for PWM filters** |
| **Op-Amps** | Dual Op-Amp | LM358N × 2 | 0-10V amplitude scaling | **REDUCED: Only 2 needed** |
| **Gain Resistors** | 10kΩ 1% | S10KQCT-ND | For op-amp gain setting | **NEW: 8x for precise gain** |
| **Optocouplers** | High-speed | 6N137 × 8 | Signal isolation | No change |
| **Relay Drivers** | Low-side | ULN2003 × 2 | Overload reset control | No change |
| **Connectors** | DB9 Female | Standard × 4 | Industrial grade | No change |
| **Connectors** | DB9 Male | Standard × 1 | To main automation box | No change |
| **ISP Header** | 2x3 Pin Header | 952-2120-ND | For programming | No change |


### Removed Components (Cost Savings)

Components eliminated by switching to PWM-based amplitude control:

| Component | Original Purpose | Replacement | Savings |
|-----------|-----------------|-------------|---------|
| External DAC ICs × 4 | Amplitude control | PWM + Filter | $40 |
| External ADC ICs × 2 | Power monitoring | Built-in ADC | $20 |
| I2C Pull-ups | I2C communication | Not needed | $2 |
| **Total Savings** | | | **$62** |

### Pin Allocation Changes

| Function | Previous Design | New Design |
|----------|----------------|------------|
| Amplitude Control | I2C to external ICs | Direct PWM (PB3, PD5-7) |
| Power Monitoring | I2C to external ICs | Direct ADC (PA4-7) |
| I2C Bus | PC0-PC1 occupied | PC0-PC1 now free |

### PWM Amplitude Control Circuit (Per Sonicator)

```text
PWM Generation → RC Filter → Op-Amp → Sonicator
                             (2x Gain)

Detailed Circuit:
                    ┌──────────────┐
ATmega PWM ──1kΩ──┬─│+             │
                  │ │  LM358       │──→ 0-10V Out
                10µF│    Gain=2    │
                  │ │-             │
                 GND└──────────────┘
                          │
                        ──┴── (Virtual Ground)
```

### Software PWM Configuration

```cpp
// PWM Setup for Amplitude Control
void setupPWM() {
    // Timer1 for PB3 (OC0)
    // Timer1 for PD5 (OC1A)
    // Timer1 for PD6 (OC1B)
    // Timer2 for PD7 (OC2)
    
    // Set PWM frequency to ~4kHz for good filtering
    // 16MHz / 64 prescaler / 256 = 976Hz
    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
    TCCR1B = _BV(CS11) | _BV(CS10);  // Prescaler 64
}

// Set amplitude (20-100%)
void setAmplitude(uint8_t sonicator, uint8_t percentage) {
    uint8_t pwm_value = map(percentage, 20, 100, 51, 255);
    switch(sonicator) {
        case 1: analogWrite(PB3, pwm_value); break;
        case 2: analogWrite(PD5, pwm_value); break;
        case 3: analogWrite(PD6, pwm_value); break;
        case 4: analogWrite(PD7, pwm_value); break;
    }
}
```

### ADC Configuration for Power Monitoring

```cpp
// Built-in ADC Setup
void setupADC() {
    // Set reference to 5V
    analogReference(DEFAULT);
    
    // Configure ADC prescaler for 125kHz ADC clock
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
}

// Read power output (5.44mV/watt)
uint16_t readPower(uint8_t sonicator) {
    uint8_t adc_channel = sonicator + 3;  // PA4-PA7
    uint16_t adc_value = analogRead(adc_channel);
    
    // Convert to watts: 5V/1024 = 4.88mV per bit
    // 5.44mV per watt / 4.88mV per bit = 1.11 bits per watt
    // Therefore: watts = adc_value / 1.11 = adc_value * 0.9
    return (uint16_t)(adc_value * 0.9);
}
```
