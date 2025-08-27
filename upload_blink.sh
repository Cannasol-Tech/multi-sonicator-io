#!/bin/bash

# Upload script for ATmega32A LED blink test
# Uses Arduino UNO R3 as ISP programmer
# 
# Wiring (UNO R3 -> ATmega32A):
# D10 -> Pin 9 (/RESET)
# D11 -> Pin 6 (PB5/MOSI) 
# D12 -> Pin 7 (PB6/MISO)
# D13 -> Pin 8 (PB7/SCK)
# 5V  -> Pin 10/30 (VCC)
# GND -> Pin 11/31 (GND)
#
# IMPORTANT: Add 10µF capacitor between RESET and GND on UNO R3!

echo "🔧 Building and uploading LED blink test to ATmega32A..."
echo "📌 Make sure UNO R3 is programmed with ArduinoISP sketch!"
echo "📌 Check that 10µF cap is between RESET-GND on UNO R3!"
echo ""

# Build the project
echo "Building project..."
pio run

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    
    # Upload using PlatformIO
    echo "Uploading to ATmega32A via UNO R3 ISP..."
    pio run --target upload
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "🎉 Upload successful!"
        echo "💡 LED on Pin 1 (PB0) should now blink every 1 second!"
        echo ""
        echo "If LED doesn't blink, check:"
        echo "  - Wiring connections"
        echo "  - 10µF capacitor on UNO R3 RESET"
        echo "  - LED connected to Pin 1 with appropriate resistor"
        echo "  - Power supply (5V) to ATmega32A"
    else
        echo "❌ Upload failed! Check connections and try again."
    fi
else
    echo "❌ Build failed!"
fi
