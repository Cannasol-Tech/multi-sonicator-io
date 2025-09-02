# Arduino as ISP Setup Guide for ATmega32A Programming

## Overview

This guide provides step-by-step instructions for setting up an Arduino Uno R4 WiFi as an In-System Programmer (ISP) to program the ATmega32A microcontroller in the Multi-Sonicator I/O Controller project.

## Hardware Requirements

### Required Components

- **Arduino Uno R4 WiFi** (programmer)
- **ATmega32A microcontroller** (target)
- **Breadboard or development board** for ATmega32A
- **16MHz crystal oscillator** with 2x 22pF capacitors
- **10kΩ resistor** (pull-up for RESET)
- **Jumper wires** for connections
- **USB cable** for Arduino

### Optional Components

- **10µF capacitor** (to disable Arduino auto-reset)
- **LED + 220Ω resistor** (status indication)
- **Multimeter** (for voltage verification)

## Pin Connections

### Arduino Uno R4 WiFi to ATmega32A ISP Wiring

| Arduino Pin | ATmega32A Pin | Signal | Description |
|-------------|---------------|--------|-------------|
| Pin 10      | Pin 9 (RESET) | RESET  | Target reset control |
| Pin 11 (MOSI)| Pin 6 (MOSI) | MOSI   | Master Out Slave In |
| Pin 12 (MISO)| Pin 7 (MISO) | MISO   | Master In Slave Out |
| Pin 13 (SCK) | Pin 8 (SCK)  | SCK    | Serial Clock |
| 5V          | Pin 10 (VCC) | VCC    | Power supply |
| GND         | Pin 11 (GND) | GND    | Ground |

### ATmega32A Support Circuitry

#### Crystal Oscillator (16MHz)

- **Pin 12 (XTAL1)**: Connect to 16MHz crystal + 22pF capacitor to GND
- **Pin 13 (XTAL2)**: Connect to 16MHz crystal + 22pF capacitor to GND

#### Reset Circuit

- **Pin 9 (RESET)**: Connect 10kΩ pull-up resistor to VCC

#### Power Supply

- **Pin 10 (VCC)**: Connect to Arduino 5V
- **Pin 11 (GND)**: Connect to Arduino GND
- **Pin 30 (AVCC)**: Connect to VCC (analog power)
- **Pin 31 (AGND)**: Connect to GND (analog ground)

## Software Setup

### Step 1: Install Arduino IDE

1. Download Arduino IDE 2.x from [arduino.cc](https://www.arduino.cc/en/software)
2. Install and launch the Arduino IDE
3. Verify Arduino Uno R4 WiFi board support is installed

### Step 2: Upload ArduinoISP Sketch

1. Connect Arduino Uno R4 WiFi to computer via USB
2. In Arduino IDE:
   - Go to **File → Examples → 11.ArduinoISP → ArduinoISP**
   - Select **Tools → Board → Arduino Uno R4 WiFi**
   - Select correct **Tools → Port** (e.g., `/dev/cu.usbmodem2101`)
   - Click **Upload** button

3. Verify upload success in output console

### Step 3: Install PlatformIO (if not already installed)

```bash
# Install PlatformIO Core
pip install platformio

# Or install via VSCode extension
# Extensions → Search "PlatformIO IDE" → Install
```

## Hardware Assembly

### Step 1: Breadboard Setup

1. **Insert ATmega32A** into breadboard (ensure proper orientation - pin 1 marker)
2. **Connect power rails**: VCC to positive rail, GND to negative rail
3. **Add crystal oscillator**:
   - Insert 16MHz crystal between pins 12 and 13
   - Connect 22pF capacitors from each crystal pin to GND
4. **Add reset pull-up**: 10kΩ resistor from pin 9 (RESET) to VCC

### Step 2: ISP Connections

1. **Connect Arduino to breadboard power**:
   - Arduino 5V → Breadboard positive rail
   - Arduino GND → Breadboard negative rail

2. **Connect ISP signals** per pin table above:
   - Arduino Pin 10 → ATmega32A Pin 9 (RESET)
   - Arduino Pin 11 → ATmega32A Pin 6 (MOSI)  
   - Arduino Pin 12 → ATmega32A Pin 7 (MISO)
   - Arduino Pin 13 → ATmega32A Pin 8 (SCK)

### Step 3: Verification

1. **Power verification**:
   - Measure 5V between ATmega32A VCC and GND
   - Verify crystal oscillator connections
   - Check RESET pull-up (should read ~5V)

2. **Connection verification**:
   - Verify all ISP connections with multimeter continuity test
   - Ensure no short circuits between adjacent pins

## Programming Process

### Step 1: Test ISP Connection

```bash
# Navigate to project directory
cd /path/to/multi-sonicator-io

# Test ISP connection (should show ATmega32A signature)
pio run -e atmega32a_arduino_isp --target fuses
```

**Expected output**: Device signature should show `0x1e9502` (ATmega32A)

### Step 2: Set Fuses (One-time setup)

```bash
# Set fuses for external 16MHz crystal
pio run -e atmega32a_arduino_isp --target fuses

# Verify fuse settings
avrdude -c arduino -P /dev/cu.usbmodem2101 -b 19200 -p m32 -v
```

**Expected fuse values**:

- **LFUSE**: `0xFF` (external crystal, full swing)
- **HFUSE**: `0xD9` (JTAG disabled, SPI enabled)  
- **EFUSE**: `0xFF` (default)

### Step 3: Build and Upload Firmware

```bash
# Build firmware
make build

# Upload via Arduino ISP
make upload-to-device

# Alternative: Direct PlatformIO upload
pio run -e atmega32a_arduino_isp --target upload
```

## Troubleshooting

### Common Issues and Solutions

#### Issue: "Device signature doesn't match"

**Symptoms**: `Expected signature 0x1e9502, got 0x000000`
**Solutions**:

- Check all ISP wiring connections
- Verify power supply (5V at ATmega32A)
- Ensure crystal oscillator is connected properly
- Check for short circuits

#### Issue: "Can't enter programming mode"

**Symptoms**: `avrdude: initialization failed, rc=-1`
**Solutions**:

- Add 10µF capacitor between Arduino RESET and GND
- Check RESET line connection to ATmega32A pin 9
- Verify Arduino is running ArduinoISP sketch
- Try different USB port/cable

#### Issue: "Fuse setting failed"

**Symptoms**: Fuse programming errors or wrong values
**Solutions**:

- Ensure stable power supply
- Check crystal oscillator frequency (16MHz)
- Verify ISP wiring integrity
- Use slower programming speed (`-B 10`)

#### Issue: Upload works but program doesn't run

**Symptoms**: Programming successful but no activity
**Solutions**:

- Verify fuse settings (especially LFUSE for crystal)
- Check power supply stability
- Verify pin assignments in code match hardware
- Add status LED for debugging

### Debug Commands

```bash
# Verbose ISP connection test
avrdude -c arduino -P /dev/cu.usbmodem2101 -b 19200 -p m32 -v

# Read current fuse settings
avrdude -c arduino -P /dev/cu.usbmodem2101 -b 19200 -p m32 -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

# Slow programming (for unstable connections)
avrdude -c arduino -P /dev/cu.usbmodem2101 -b 19200 -p m32 -B 10 -v

# Test with LED blink program
pio run -e atmega32a_arduino_isp -t upload --verbose
```

## Safety and Best Practices

### Safety Considerations

- **Power off** when making wiring changes
- **Double-check connections** before applying power
- **Use proper grounding** to prevent static damage
- **Verify voltage levels** before connecting

### Best Practices

- **Label connections** during initial setup
- **Document working configurations** for future reference
- **Test with simple programs** before complex code
- **Keep backup of working fuse settings**
- **Use version control** for all firmware changes

## Integration with Development Workflow

### Daily Development

1. **One-time setup**: Follow this guide to configure Arduino ISP
2. **Regular development**: Use `make upload-to-device` for firmware updates
3. **Testing**: Use hardware sandbox mode for interactive testing

### CI/CD Integration

- Arduino ISP is for **local development only**
- CI/CD pipeline uses **emulation** for automated testing
- Hardware validation done in **dedicated test environment**

## Appendix: Hardware Diagrams

### ATmega32A Pinout Reference

```
           ATmega32A PDIP-40
     (XCK/T0) PB0  1 ┌─────┐ 40  PA0 (ADC0)
         (T1) PB1  2 │     │ 39  PA1 (ADC1)  
     (INT2/AIN0) PB2  3 │     │ 38  PA2 (ADC2)
      (OC0/AIN1) PB3  4 │     │ 37  PA3 (ADC3)
          (/SS) PB4  5 │     │ 36  PA4 (ADC4)
        (MOSI) PB5  6 │     │ 35  PA5 (ADC5)
        (MISO) PB6  7 │     │ 34  PA6 (ADC6)
         (SCK) PB7  8 │     │ 33  PA7 (ADC7)
              /RESET  9 │     │ 32  AREF
                VCC 10 │     │ 31  AGND
                GND 11 │     │ 30  AVCC
              XTAL2 12 │     │ 29  PC7 (TOSC2)
              XTAL1 13 │     │ 28  PC6 (TOSC1)
         (RXD) PD0 14 │     │ 27  PC5 (TDI)
         (TXD) PD1 15 │     │ 26  PC4 (TDO)
        (INT0) PD2 16 │     │ 25  PC3 (TMS)
        (INT1) PD3 17 │     │ 24  PC2 (TCK)
        (OC1B) PD4 18 │     │ 23  PC1 (SDA)
        (OC1A) PD5 19 │     │ 22  PC0 (SCL)
         (ICP) PD6 20 └─────┘ 21  PD7 (OC2)
```

### ISP Connection Diagram

```
Arduino Uno R4 WiFi        ATmega32A Development Board
     ┌─────────────┐              ┌─────────────┐
     │         D10 ├──────────────┤ 9  (RESET)  │
     │         D11 ├──────────────┤ 6  (MOSI)   │
     │         D12 ├──────────────┤ 7  (MISO)   │  
     │         D13 ├──────────────┤ 8  (SCK)    │
     │          5V ├──────────────┤ 10 (VCC)    │
     │         GND ├──────────────┤ 11 (GND)    │
     └─────────────┘              └─────────────┘

Additional ATmega32A connections:
- 16MHz Crystal: Pins 12-13 with 22pF caps to GND  
- Reset Pull-up: 10kΩ resistor from Pin 9 to VCC
- Power: Pin 30 (AVCC) to VCC, Pin 31 (AGND) to GND
```

---

✅ **This setup guide provides everything needed for reliable ATmega32A programming via Arduino ISP for the Multi-Sonicator I/O Controller project.**
