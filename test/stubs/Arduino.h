// Minimal Arduino.h stub for native unit testing
// Provides enough symbols for tests to compile under PlatformIO native
#ifndef ARDUINO_STUB_H
#define ARDUINO_STUB_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>

// Basic Arduino types and macros
#ifndef HIGH
#define HIGH 0x1
#endif
#ifndef LOW
#define LOW 0x0
#endif

#ifndef INPUT
#define INPUT 0x0
#endif
#ifndef OUTPUT
#define OUTPUT 0x1
#endif

using String = std::string;

// Analog pin aliases for tests expecting Arduino-style A0..A7
#ifndef A0
#define A0 0
#endif
#ifndef A1
#define A1 1
#endif
#ifndef A2
#define A2 2
#endif
#ifndef A3
#define A3 3
#endif
#ifndef A4
#define A4 4
#endif
#ifndef A5
#define A5 5
#endif
#ifndef A6
#define A6 6
#endif
#ifndef A7
#define A7 7
#endif

inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t) {}
inline int digitalRead(uint8_t) { return LOW; }
inline int analogRead(uint8_t) { return 0; }
inline void analogWrite(uint8_t, int) {}

inline void delay(unsigned long ms) { usleep(ms * 1000); }
inline void delayMicroseconds(unsigned int us) { usleep(us); }
inline unsigned long millis() { return 0; }
inline unsigned long micros() { return 0; }

class HardwareSerial {
 public:
  inline void begin(unsigned long) {}
  inline void end() {}
  inline int available() { return 0; }
  inline int read() { return -1; }
  inline size_t write(uint8_t) { return 1; }
  inline void flush() {}

  // Convenience for tests
  inline void print(const char* s) { (void)s; }
  inline void println(const char* s="") { (void)s; }
};

static HardwareSerial Serial;

#endif // ARDUINO_STUB_H
