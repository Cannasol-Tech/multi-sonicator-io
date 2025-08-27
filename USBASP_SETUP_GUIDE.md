# USBASP Programming Setup Guide

## Project Status ✅

Your project has been successfully updated to use USBASP programmer with 16MHz external crystal configuration.

## Hardware Requirements

### USBASP Programmer Connections
Connect your USBASP to the ATmega32A as follows:

| USBASP Pin | ATmega32A Pin | Function |
|------------|---------------|----------|
| MISO       | Pin 7 (PB6)   | Master In, Slave Out |
| MOSI       | Pin 6 (PB5)   | Master Out, Slave In |
| SCK        | Pin 8 (PB7)   | Serial Clock |
| RESET      | Pin 9 (/RESET)| Reset (active low) |
| VCC        | Pin 10/30     | +5V Power (optional if self-powered) |
| GND        | Pin 11/31     | Ground |

### 16MHz Crystal Oscillator Setup
**CRITICAL:** For the firmware to work, you must have:

1. **16MHz Crystal:** Connected between XTAL1 (Pin 13) and XTAL2 (Pin 12)
2. **Load Capacitors:** 22pF ceramic capacitors from each crystal pin to ground
3. **Short Connections:** Keep crystal and capacitor leads as short as possible

```
    ATmega32A
    Pin 13 (XTAL1) ----[16MHz Crystal]---- Pin 12 (XTAL2)
         |                                      |
       [22pF]                                [22pF]
         |                                      |
       GND                                    GND
```

## Programming Commands

### Option 1: Use the Automated Script
```bash
./upload_usbasp.sh
```

### Option 2: Manual Programming
```bash
# Build the project
pio run -e atmega32a

# Set fuses for 16MHz external crystal
~/.platformio/packages/tool-avrdude/bin/avrdude \
  -C ~/.platformio/packages/tool-avrdude/avrdude.conf \
  -c usbasp -p m32 -B 10 -F \
  -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m

# Upload firmware (ignore verification errors if crystal isn't connected yet)
~/.platformio/packages/tool-avrdude/bin/avrdude \
  -C ~/.platformio/packages/tool-avrdude/avrdude.conf \
  -c usbasp -p m32 -B 10 -F -D \
  -U flash:w:.pio/build/atmega32a/firmware.hex:i
```

## Troubleshooting

### If Programming Fails:
1. **Check USBASP Connection:** Verify all 6 ISP wires are connected correctly
2. **Power Supply:** Ensure ATmega32A has stable 5V power
3. **Try Slower Speed:** Add `-B 20` for slower programming
4. **Force Programming:** Use `-F` flag to override connection checks

### If LED Doesn't Blink After Programming:
1. **Crystal Issues:** 
   - Verify 16MHz crystal is connected to XTAL1/XTAL2
   - Check 22pF load capacitors are present
   - Try a different crystal
2. **LED Connection:** 
   - LED should be connected to Pin 1 (PB0) with current limiting resistor
   - Verify LED polarity (anode to pin, cathode to ground via resistor)
3. **Power Supply:** Check 5V is stable and within tolerance

### Verification Errors:
If you see verification errors during programming, this is often normal when:
- Crystal oscillator isn't connected yet
- Chip is configured for external crystal but crystal isn't working
- Programming speed is too fast

The firmware may still be programmed correctly despite verification errors.

## Testing

Once programmed with proper crystal setup:
- LED on Pin 1 (PB0) should blink every 1 second
- Timing should be precise due to 16MHz crystal accuracy

## Configuration Files Updated

- `platformio.ini`: Updated for USBASP and 16MHz external crystal
- `src/main.c`: Comments updated for 16MHz operation
- `upload_usbasp.sh`: New automated programming script

## Backup Configuration

If you need to revert to Arduino ISP programmer:
```bash
pio run -e atmega32a_arduino_isp --target upload
```

This uses the backup environment with 8MHz internal oscillator settings.
