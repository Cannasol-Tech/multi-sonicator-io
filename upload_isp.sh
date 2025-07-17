#!/bin/bash

# Upload script for Arduino Uno R4 WiFi
# This script helps with the manual upload process

echo "=========================================="
echo "Arduino Uno R4 WiFi Upload Instructions"
echo "=========================================="
echo
echo "1. Double-press the RESET button on your Arduino Uno R4 WiFi"
echo "2. The board will enter bootloader mode (power LED will dim)"
echo "3. Press ENTER when ready to upload..."
read -p ""

echo "Uploading ArduinoISP sketch..."
arduino-cli upload --fqbn arduino:renesas_uno:unor4wifi --port /dev/cu.usbmodemF412FA9B73C82 ArduinoISP.ino

echo "Upload complete!"
echo
echo "Next steps:"
echo "1. Add 10µF capacitor between RESET and GND pins on Arduino"
echo "2. Connect ATmega32A to Arduino using ISP wiring"
echo "3. Run the programming command"
