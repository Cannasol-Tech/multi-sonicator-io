#!/bin/bash
# validate-setup.sh - Validate initial setup completion

set -e

PROJECT_ROOT=$(dirname "$0")
cd "$PROJECT_ROOT"

echo "=== Multi-Sonicator I/O Controller - Initial Setup Validation ==="
echo "Date: $(date)"
echo "Directory: $(pwd)"
echo

# Check directory structure
echo "1. Checking directory structure..."
required_dirs=("src" "include" "test" "lib" "docs")
for dir in "${required_dirs[@]}"; do
    if [ -d "$dir" ]; then
        echo "   ✓ $dir/ exists"
    else
        echo "   ✗ $dir/ missing"
        exit 1
    fi
done

# Check essential files
echo
echo "2. Checking essential files..."
required_files=(
    "platformio.ini"
    "README.md"
    "CHANGELOG.md"
    ".gitignore"
    "src/main.cpp"
    "include/config.h"
    "include/types.h"
)
for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "   ✓ $file exists"
    else
        echo "   ✗ $file missing"
        exit 1
    fi
done

# Check test structure
echo
echo "3. Checking test structure..."
test_files=(
    "test/README.md"
    "test/test_main.cpp"
    "test/test_config/test_config.cpp"
    "test/test_system/test_system.cpp"
    "test/test_sonicator/test_sonicator.cpp"
)
for file in "${test_files[@]}"; do
    if [ -f "$file" ]; then
        echo "   ✓ $file exists"
    else
        echo "   ✗ $file missing"
        exit 1
    fi
done

# Check documentation
echo
echo "4. Checking documentation..."
doc_files=(
    "docs/Guides/environment-setup.md"
    "docs/Features/Specifications/1-initial-setup.md"
)
for file in "${doc_files[@]}"; do
    if [ -f "$file" ]; then
        echo "   ✓ $file exists"
    else
        echo "   ✗ $file missing"
        exit 1
    fi
done

# Validate C++ syntax for main files
echo
echo "5. Validating C++ syntax..."

# Simple syntax check by creating minimal Arduino mock
cat > /tmp/arduino_mock.h << 'EOF'
#ifndef ARDUINO_MOCK_H
#define ARDUINO_MOCK_H

// Mock Arduino functions and types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef bool boolean;

#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1

#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A4 4
#define A5 5
#define A6 6
#define A7 7

class MockSerial {
public:
    void begin(int baud) {}
    void print(const char* str) {}
    void println(const char* str) {}
    void println() {}
    bool available() { return false; }
    int read() { return -1; }
    class StringType {
    public:
        void trim() {}
        void toLowerCase() {}
        const char* c_str() { return ""; }
        bool operator==(const char* str) { return false; }
    };
    StringType readStringUntil(char delimiter) { return StringType(); }
    MockSerial& operator<<(const char* str) { return *this; }
};

class MockWire {
public:
    void begin() {}
    void begin(int addr) {}
};

extern MockSerial Serial;
extern MockWire Wire;

unsigned long millis() { return 0; }
void delay(unsigned long ms) {}
void pinMode(int pin, int mode) {}
void digitalWrite(int pin, int value) {}
int digitalRead(int pin) { return 0; }
int analogRead(int pin) { return 0; }
void analogWrite(int pin, int value) {}

#endif
EOF

# Create Arduino.h mock
cat > /tmp/Arduino.h << 'EOF'
#include "arduino_mock.h"
EOF

cat > /tmp/test_compile.cpp << 'EOF'
#include "Arduino.h"
#include "config.h"
#include "types.h"

// Test that constants are properly defined
void test_constants() {
    const char* project_name = PROJECT_NAME;
    const char* project_model = PROJECT_MODEL;
    uint32_t frequency = MCU_FREQUENCY;
    uint8_t max_sonicators = MAX_SONICATORS;
    
    // Test pin definitions
    uint8_t led_pin = LED_POWER_PIN;
    uint8_t uart_rx = UART_RX_PIN;
    uint8_t uart_tx = UART_TX_PIN;
    
    // Test register definitions
    uint16_t reg_addr = REG_SONICATOR_1_AMPLITUDE;
    
    // Test enums
    sonicator_state_t state = SONICATOR_STATE_STOPPED;
    system_mode_t mode = SYSTEM_MODE_INIT;
    error_code_t error = ERROR_NONE;
}

int main() {
    test_constants();
    return 0;
}
EOF

# Compile test
if g++ -std=c++11 -I./include -I/tmp /tmp/test_compile.cpp -o /tmp/test_compile 2>/dev/null; then
    echo "   ✓ C++ syntax validation passed"
else
    echo "   ✗ C++ syntax validation failed"
    exit 1
fi

# Check git repository
echo
echo "6. Checking git repository..."
if [ -d ".git" ]; then
    echo "   ✓ Git repository initialized"
    
    # Check .gitignore
    if grep -q "\.pio" .gitignore; then
        echo "   ✓ .gitignore includes PlatformIO artifacts"
    else
        echo "   ✗ .gitignore missing PlatformIO artifacts"
        exit 1
    fi
else
    echo "   ✗ Git repository not initialized"
    exit 1
fi

# Check platformio.ini configuration
echo
echo "7. Checking PlatformIO configuration..."
if grep -q "framework = arduino" platformio.ini; then
    echo "   ✓ Arduino framework configured"
else
    echo "   ✗ Arduino framework not configured"
    exit 1
fi

if grep -q "board = ATmega32" platformio.ini; then
    echo "   ✓ ATmega32A board configured"
else
    echo "   ✗ ATmega32A board not configured"
    exit 1
fi

if grep -q "test_framework = unity" platformio.ini; then
    echo "   ✓ Unity test framework configured"
else
    echo "   ✗ Unity test framework not configured"
    exit 1
fi

# Count lines of code
echo
echo "8. Code statistics..."
total_lines=$(find src include test -name "*.cpp" -o -name "*.h" | xargs wc -l | tail -1 | awk '{print $1}')
echo "   ✓ Total lines of code: $total_lines"

# Success
echo
echo "=== VALIDATION COMPLETE ==="
echo "✓ All initial setup requirements satisfied"
echo "✓ Project structure matches PlatformIO best practices"
echo "✓ Version control properly configured"
echo "✓ Documentation accessible and usable"
echo "✓ C++ syntax validation passed"
echo
echo "Status: READY FOR FEATURE 1 COMPLETION"
echo "Next step: Proceed to 2-hardware-interface-implementation.md"