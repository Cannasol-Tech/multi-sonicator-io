#!/bin/bash
"""
CI Setup Script for Multi-Sonicator Pipeline
Sets up the build environment for PlatformIO
"""

set -e

echo "Setting up Multi-Sonicator CI Environment"
echo "=========================================="

# Check if PlatformIO is available
if ! command -v pio &> /dev/null; then
    echo "PlatformIO not found, installing..."
    pip install platformio
fi

# Check PlatformIO version
echo "PlatformIO version:"
pio --version

# Create test results directory
mkdir -p test_results

# Try to check platformio.ini
echo "Checking platformio.ini..."
if [ -f "platformio.ini" ]; then
    echo "platformio.ini found"
    pio project config
else
    echo "platformio.ini not found!"
    exit 1
fi

echo "CI environment setup complete"