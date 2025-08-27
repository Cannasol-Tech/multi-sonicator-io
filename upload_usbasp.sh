#!/bin/bash

# Upload script for ATmega32A using USBASP programmer
# Uses 16MHz external crystal oscillator
# 
# USBASP Wiring to ATmega32A:
# MISO -> Pin 7 (PB6/MISO)
# MOSI -> Pin 6 (PB5/MOSI) 
# SCK  -> Pin 8 (PB7/SCK)
# RST  -> Pin 9 (/RESET)
# VCC  -> Pin 10/30 (VCC) - Optional if target is self-powered
# GND  -> Pin 11/31 (GND)
#
# Make sure 16MHz crystal is connected to XTAL1 (Pin 13) and XTAL2 (Pin 12)

echo "🔧 Building and uploading to ATmega32A using USBASP programmer..."
echo "📌 Make sure USBASP is connected to ATmega32A via ISP pins!"
echo "📌 Ensure 16MHz crystal oscillator is connected to XTAL1/XTAL2!"
echo ""

# Set up avrdude paths
AVRDUDE="$HOME/.platformio/packages/tool-avrdude/bin/avrdude"
AVRDUDE_CONF="$HOME/.platformio/packages/tool-avrdude/avrdude.conf"

# Check if USBASP is connected
echo "Checking USBASP connection..."
$AVRDUDE -C $AVRDUDE_CONF -c usbasp -p m32 -B 10 -F > /tmp/avrdude_test.log 2>&1
if grep -q "Device signature" /tmp/avrdude_test.log; then
    echo "✅ USBASP programmer detected and target responding!"
else
    echo "❌ USBASP programmer not detected or target not responding!"
    echo "   - Check USB connection to USBASP"
    echo "   - Verify ISP wiring to ATmega32A"
    echo "   - Ensure target is powered (5V)"
    echo "   - Check crystal oscillator connections"
    cat /tmp/avrdude_test.log
    exit 1
fi

echo "✅ USBASP programmer detected!"
echo ""

# Build the project
echo "Building project..."
pio run -e atmega32a

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo ""
    
    # Set fuses for 16MHz external crystal (first time only)
    echo "Setting fuses for 16MHz external crystal..."
    echo "LFUSE: 0xFF (external crystal, full swing)"
    echo "HFUSE: 0xD9 (JTAG disabled, SPI enabled)"
    echo "EFUSE: 0xFF (default)"

    $AVRDUDE -C $AVRDUDE_CONF -c usbasp -p m32 -B 10 -F -U lfuse:w:0xFF:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m
    
    if [ $? -eq 0 ]; then
        echo "✅ Fuses set successfully!"
        echo ""
        
        # Upload firmware using PlatformIO
        echo "Uploading firmware to ATmega32A via USBASP..."
        pio run --target upload
        
        if [ $? -eq 0 ]; then
            echo ""
            echo "🎉 Upload successful!"
            echo "💡 LED on Pin 1 (PB0) should now blink every 1 second!"
            echo ""
            echo "Hardware setup:"
            echo "  - ATmega32A running at 16MHz with external crystal"
            echo "  - LED connected to Pin 1 (PB0) with appropriate resistor"
            echo "  - Power supply (5V) connected to VCC pins"
            echo ""
            echo "If LED doesn't blink, check:"
            echo "  - 16MHz crystal connections to XTAL1/XTAL2"
            echo "  - Crystal load capacitors (typically 22pF each to ground)"
            echo "  - LED wiring and current limiting resistor"
            echo "  - Power supply connections"
        else
            echo "❌ Upload failed! Check connections and try again."
        fi
    else
        echo "❌ Fuse programming failed!"
        echo "   - Check USBASP connection"
        echo "   - Verify target power supply"
        echo "   - Try slower programming speed with -B 10"
    fi
else
    echo "❌ Build failed!"
fi
