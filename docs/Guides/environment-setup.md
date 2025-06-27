# Development Environment Setup

This document provides complete instructions for setting up the AVR development environment for the Multi Sonicator I/O Controller project.

## Hardware Requirements

### Target Hardware

- **Microcontroller:** ATmega32A (DIP-40 package)
- **Crystal:** 16MHz external crystal oscillator
- **Power Supply:** 5V regulated (from 24V→12V→5V stages)

### ISP Programmer

- **Arduino Uno R4 WiFi** configured as ISP programmer
- USB cable for connection to development computer

### Additional Hardware

- Breadboard or development board
- Jumper wires for ISP connections
- 10µF capacitor for Arduino reset bypass
- LED indicators for testing

## ISP Wiring Configuration

Connect Arduino Uno R4 WiFi to ATmega32A as follows:

| Arduino Uno R4 WiFi | ATmega32A (DIP-40) | Signal |
|---------------------|-------------------|--------|
| D10                | Pin 9             | /RESET |
| D11                | Pin 6 (PB5)       | MOSI   |
| D12                | Pin 7 (PB6)       | MISO   |
| D13                | Pin 8 (PB7)       | SCK    |
| 5V                 | Pin 10/30         | VCC    |
| GND                | Pin 11/31         | GND    |

**Important:** Add a 10µF capacitor between RESET and GND pins on the Arduino Uno R4 WiFi to prevent auto-reset during programming. Remove this capacitor when you need to re-flash the Arduino itself.

## Software Requirements

### PlatformIO Installation

1. **Install Visual Studio Code**

   ```bash
   # Download from https://code.visualstudio.com/
   ```

2. **Install PlatformIO Extension**
   - Open VS Code
   - Go to Extensions (Ctrl+Shift+X)
   - Search for "PlatformIO IDE"
   - Install the extension

3. **Verify Installation**

   ```bash
   pio --version
   ```

### Arduino IDE Setup (Alternative)

If using Arduino IDE instead of PlatformIO:

1. **Install Arduino IDE 2.x**

   ```bash
   # Download from https://www.arduino.cc/en/software
   ```

2. **Add MightyCore Board Package**
   - File → Preferences
   - Additional Board Manager URLs:

     ```text
     https://mcudude.github.io/MightyCore/package_MCUdude_MightyCore_index.json
     ```

   - Tools → Board → Boards Manager
   - Search for "MightyCore" and install

## Programming the Arduino as ISP

Before programming the ATmega32A, you must flash the Arduino Uno R4 WiFi with ISP firmware:

1. **Connect Arduino Uno R4 WiFi via USB**

2. **Flash ArduinoISP Sketch**

   ```bash
   # In Arduino IDE:
   # File → Examples → 11.ArduinoISP → ArduinoISP
   # Select Board: Arduino Uno R4 WiFi
   # Upload the sketch
   ```

3. **Add Reset Bypass Capacitor**
   - Connect 10µF capacitor between RESET and GND on Arduino
   - This prevents the Arduino from resetting during ISP operations

## Project Setup

### Clone Repository

```bash
git clone <repository-url>
cd multi-sonicator-io
```

### PlatformIO Build and Upload

```bash
# Build the project
pio run

# Upload to ATmega32A via Arduino as ISP
pio run --target upload
```

### Set Fuses (First Time Only)

The ATmega32A requires proper fuse settings for 16MHz external crystal:

```bash
# Using avrdude directly (replace /dev/cu.usbmodem* with actual port)
avrdude -C ~/.platformio/packages/tool-avrdude/avrdude.conf \
        -p atmega32a \
        -P /dev/cu.usbmodem* \
        -c arduino \
        -b 19200 \
        -U lfuse:w:0xFF:m \
        -U hfuse:w:0xD9:m \
        -U efuse:w:0xFF:m
```

**Fuse Explanations:**

- **LFUSE = 0xFF:** External crystal oscillator, slow rising power
- **HFUSE = 0xD9:** JTAG disabled, SPI enabled, watchdog disabled  
- **EFUSE = 0xFF:** Default extended fuse settings

## Verification and Testing

### Basic Blink Test

1. **Upload Blink Example**

   ```cpp
   // Test code in src/main.cpp
   #include <Arduino.h>
   
   #define LED_PIN PB0  // Pin 1 on ATmega32A
   
   void setup() {
     pinMode(LED_PIN, OUTPUT);
   }
   
   void loop() {
     digitalWrite(LED_PIN, HIGH);
     delay(1000);
     digitalWrite(LED_PIN, LOW);
     delay(1000);
   }
   ```

2. **Build and Upload**

   ```bash
   pio run --target upload
   ```

3. **Verify LED Blinking**
   - Connect LED with resistor to Pin 1 (PB0) and GND
   - Should blink at 1Hz if everything is working

### Serial Communication Test

```cpp
#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  Serial.println("ATmega32A Hello World!");
}

void loop() {
  Serial.println("System running...");
  delay(2000);
}
```

## Troubleshooting

### Common Issues

1. **"Device signature = 0x000000"**
   - Check all ISP wiring connections
   - Verify 5V power is reaching ATmega32A
   - Check crystal oscillator connections

2. **"Can't find programmer"**
   - Verify Arduino Uno R4 WiFi has ArduinoISP sketch loaded
   - Check USB port in platformio.ini matches actual port
   - Ensure reset bypass capacitor is installed

3. **Upload fails after initial success**
   - Verify fuse settings haven't been corrupted
   - Check for loose connections
   - Re-flash ArduinoISP sketch if needed

4. **Serial monitor not working**
   - Verify UART pins are correctly connected
   - Check baud rate matches code and monitor settings
   - Ensure proper ground connection

### Debug Commands

```bash
# Check available serial ports
ls /dev/cu.*

# Verbose avrdude output for debugging
avrdude -v -C ~/.platformio/packages/tool-avrdude/avrdude.conf \
        -p atmega32a -P /dev/cu.usbmodem* -c arduino -b 19200

# Read device signature
avrdude -C ~/.platformio/packages/tool-avrdude/avrdude.conf \
        -p atmega32a -P /dev/cu.usbmodem* -c arduino -b 19200 \
        -U signature:r:signature.hex:h
```

## Next Steps

Once the environment is verified:

1. Proceed to **Phase 1.2: Hardware Validation Setup**
2. Test power supply stages
3. Validate crystal oscillator operation
4. Test basic LED indicators
5. Begin hardware interface implementation

## References

- [ATmega32A Datasheet](../ATMEGA32A/ATMEGA32A-Data-Sheet.pdf)
- [MightyCore Documentation](https://github.com/MCUdude/MightyCore)
- [PlatformIO AVR Documentation](https://docs.platformio.org/en/latest/platforms/atmelavr.html)
- [Arduino as ISP Tutorial](https://www.arduino.cc/en/Tutorial/BuiltInExamples/ArduinoISP)
