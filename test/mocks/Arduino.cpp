#ifdef NATIVE_TEST
// Arduino mock implementation for native PlatformIO tests (Unity)
// Provides minimalist behavior sufficient for HAL unit tests

#include "Arduino.h"
#include <map>
#include <chrono>
#include <thread>

// ----------------------------------------------------------------------------
// Mocked AVR/Arduino registers (simple globals)
// ----------------------------------------------------------------------------
uint8_t mock_PORTA = 0, mock_PORTB = 0, mock_PORTC = 0, mock_PORTD = 0;
uint8_t mock_DDRA = 0, mock_DDRB = 0, mock_DDRC = 0, mock_DDRD = 0;
uint8_t mock_PINA = 0, mock_PINB = 0, mock_PINC = 0, mock_PIND = 0;
uint8_t mock_TCCR0 = 0, mock_TCCR1A = 0, mock_TCCR1B = 0, mock_TCCR2 = 0;
uint8_t mock_TCNT0 = 0, mock_TCNT1 = 0, mock_OCR0 = 0, mock_OCR1A = 0, mock_OCR2 = 0;
uint8_t mock_TIMSK = 0, mock_UCSRA = 0, mock_UCSRB = 0, mock_UCSRC = 0;
uint8_t mock_UBRRH = 0, mock_UBRRL = 0, mock_UDR = 0;
uint8_t mock_ADMUX = 0, mock_ADCSRA = 0, mock_ADCL = 0, mock_ADCH = 0;

// ----------------------------------------------------------------------------
// Simple state for GPIO and timekeeping
// ----------------------------------------------------------------------------
static std::map<uint8_t, uint8_t> s_pinMode;
static std::map<uint8_t, uint8_t> s_pinState;
static std::map<uint8_t, uint16_t> s_analogValues;

static const auto s_start = std::chrono::steady_clock::now();

// ----------------------------------------------------------------------------
// Core time functions
// ----------------------------------------------------------------------------
unsigned long millis(void) {
    using namespace std::chrono;
    return (unsigned long)duration_cast<milliseconds>(steady_clock::now() - s_start).count();
}

unsigned long micros(void) {
    using namespace std::chrono;
    return (unsigned long)duration_cast<microseconds>(steady_clock::now() - s_start).count();
}

void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void delayMicroseconds(unsigned int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// ----------------------------------------------------------------------------
// Interrupt control (no-op for native tests)
// ----------------------------------------------------------------------------
void sei(void) {}
void cli(void) {}

// ----------------------------------------------------------------------------
// Digital and analog I/O
// ----------------------------------------------------------------------------
void pinMode(uint8_t pin, uint8_t mode) {
    s_pinMode[pin] = mode;
}

void digitalWrite(uint8_t pin, uint8_t value) {
    s_pinState[pin] = (value ? HIGH : LOW);
}

int digitalRead(uint8_t pin) {
    auto it = s_pinState.find(pin);
    if (it == s_pinState.end()) return LOW;
    return it->second ? HIGH : LOW;
}

int analogRead(uint8_t pin) {
    auto it = s_analogValues.find(pin);
    if (it == s_analogValues.end()) return 0;
    return (int)it->second;
}

void analogReference(uint8_t /*type*/) {
    // no-op in mock
}

// ----------------------------------------------------------------------------
// Mock helpers
// ----------------------------------------------------------------------------
void arduino_mock_reset(void) {
    s_pinMode.clear();
    s_pinState.clear();
    s_analogValues.clear();
    mock_PORTA = mock_PORTB = mock_PORTC = mock_PORTD = 0;
    mock_DDRA = mock_DDRB = mock_DDRC = mock_DDRD = 0;
    mock_PINA = mock_PINB = mock_PINC = mock_PIND = 0;
    mock_TCCR0 = mock_TCCR1A = mock_TCCR1B = mock_TCCR2 = 0;
    mock_TCNT0 = mock_TCNT1 = mock_OCR0 = mock_OCR1A = mock_OCR2 = 0;
    mock_TIMSK = mock_UCSRA = mock_UCSRB = mock_UCSRC = 0;
    mock_UBRRH = mock_UBRRL = mock_UDR = 0;
    mock_ADMUX = mock_ADCSRA = mock_ADCL = mock_ADCH = 0;
}

void arduino_mock_set_analog_value(uint8_t pin, uint16_t value) {
    s_analogValues[pin] = value;
}

uint16_t arduino_mock_get_analog_value(uint8_t pin) {
    auto it = s_analogValues.find(pin);
    return (it == s_analogValues.end()) ? 0 : it->second;
}

#endif // NATIVE_TEST

