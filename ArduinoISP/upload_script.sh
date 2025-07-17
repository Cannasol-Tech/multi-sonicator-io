#!/bin/bash

echo "=== Arduino Uno R4 WiFi ArduinoISP Upload Script ==="
echo "1. Press and hold the reset button on your Arduino"
echo "2. Release the reset button when you see the orange light flashing"
echo "3. Press ENTER immediately when the orange light starts flashing"
echo ""
echo "Press ENTER when ready..."
read

echo "Uploading ArduinoISP sketch..."
arduino-cli upload -p /dev/cu.usbmodemF412FA9B73C82 --fqbn arduino:renesas_uno:unor4wifi ArduinoISP.ino

echo "Upload attempt completed. Check the Arduino's LED status."
