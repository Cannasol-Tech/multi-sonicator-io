# Arduino UNO R3 as ISP Programmer for ATmega32A

## Quick Setup Steps

### 1. Program UNO R3 with ArduinoISP
1. Connect UNO R3 to computer via USB
2. Open Arduino IDE
3. Load `File → Examples → 11.ArduinoISP → ArduinoISP`
4. Select `Tools → Board → Arduino UNO`
5. Select correct COM port
6. Upload the ArduinoISP sketch

### 2. Add Reset Capacitor
- **CRITICAL:** Add 10µF capacitor between RESET and GND pins on UNO R3
- This prevents the UNO from auto-resetting during programming
- **Remove this cap when you need to reprogram the UNO itself**

### 3. Wire UNO R3 to ATmega32A

| UNO R3 Pin | ATmega32A Pin | Signal | Notes |
|------------|---------------|---------|-------|
| **D10**    | **9**         | /RESET | Reset line |
| **D11**    | **6 (PB5)**   | MOSI   | Master Out Slave In |
| **D12**    | **7 (PB6)**   | MISO   | Master In Slave Out |
| **D13**    | **8 (PB7)**   | SCK    | Serial Clock |
| **5V**     | **10, 30**    | VCC    | Power (+5V) |
| **GND**    | **11, 31**    | GND    | Ground |

### 4. Test LED Connection
- Connect LED with 220Ω resistor between Pin 1 (PB0) and GND
- Longer LED leg (anode) goes to Pin 1
- Shorter LED leg (cathode) goes through resistor to GND

## Programming Commands

### Using the Upload Script:
```bash
./upload_blink.sh
```

### Manual PlatformIO Commands:
```bash
# Build only
pio run

# Build and upload
pio run --target upload
```

### Manual avrdude Command:
```bash
avrdude -C ~/.platformio/packages/tool-avrdude/avrdude.conf \
        -p m32 -P /dev/cu.usbmodem* -c stk500v1 -b 19200 \
        -U flash:w:.pio/build/atmega32a/firmware.hex:i
```

## Troubleshooting

### Upload Fails:
- ✅ Check all wiring connections
- ✅ Verify 10µF cap is on UNO R3 RESET
- ✅ Ensure UNO R3 has ArduinoISP sketch loaded
- ✅ Check COM port in platformio.ini
- ✅ Try different USB cable/port

### LED Doesn't Blink:
- ✅ Check LED polarity (long leg to Pin 1)
- ✅ Verify 220Ω resistor is in series with LED
- ✅ Measure voltage on Pin 1 (should toggle 0V-5V)
- ✅ Check ATmega32A power supply (5V on pins 10,30)

### No Communication:
- ✅ Double-check MOSI/MISO aren't swapped
- ✅ Ensure SCK and RESET connections are solid
- ✅ Verify UNO R3 isn't auto-resetting (add cap!)

## Expected Behavior

✅ **Success:** LED blinks ON for 1 second, OFF for 1 second, continuously  
✅ **Timing:** Very precise 1-second intervals using Timer0 interrupt  
✅ **Power:** ATmega32A draws ~1mA when running at 8MHz internal oscillator

---

**Note:** This setup uses the ATmega32A's internal 8MHz oscillator, so no external crystal is needed for this simple test.
