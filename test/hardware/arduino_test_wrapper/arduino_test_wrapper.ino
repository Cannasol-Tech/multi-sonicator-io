/*
 * arduino_test_wrapper.ino
 *
 * Purpose:
 *  - Hardware-in-the-Loop (HIL) wrapper to electrically interface with the ATmega32A DUT.
 *  - Exposes a simple serial command protocol for Behave/pytest to stimulate inputs and read outputs.
 *  - Validates MODBUS contract semantics end-to-end via GPIO/ADC interactions and timing assertions.
 *
 * Location:
 *  - test/hardware/arduino_test_wrapper/arduino_test_wrapper.ino
 *
 * Protocol (draft):
 *  - Commands are ASCII lines ending with \n. Responses are lines prefixed with "OK " or "ERR ".
 *  - Examples:
 *      SET FREQ <unit> <hz>         // simulate frequency output ÷10
 *      SET OVERLOAD <unit> <0|1>    // set/clear overload input
 *      SET FREQ_LOCK <unit> <0|1>   // set/clear frequency lock input
 *      READ STATUS <unit>           // read back status pins
 *      READ POWER <unit>            // read analog power proxy (if configured)
 *      PULSE RESET <unit> <ms>      // generate reset pulse and report timing
 *  - TODO: finalize mapping to DUT headers per include/config.h and harness doc.
 *
 * Safety:
 *  - Always default outputs to safe/neutral states on boot and on protocol errors.
 *
 * Notes:
 *  - This is a stub to establish structure. Pin mappings and concrete behavior must be implemented
 *    alongside harness documentation and kept in sync with include/config.h.
 */

#include <Arduino.h>

// Pin mappings from host Arduino to DUT harness headers.
// Uno R4 WiFi single-channel harness (S1 only). See docs/planning/pin-matrix.md.
// DB9-1 for S1, DB9-0 for comms, LED-TERM for status LED.

struct SonPins {
  uint8_t OVERLOAD_IN;   // Drives opto input seen by DUT PDx (active level per harness)
  uint8_t FREQ_DIV10_IN; // Generates frequency ÷10 to DUT PBx
  uint8_t FREQ_LOCK_IN;  // Drives opto input seen by DUT PBx
  uint8_t START_OUT;     // Reads DUT start (if observed) or drives if simulating external
  uint8_t RESET_OUT;     // Reads DUT reset (if observed) or drives if simulating external
  uint8_t POWER_ADC;     // Reads analog proxy for power sense if loopbacked
};

// Single channel (S1) mapping per pin-matrix
static SonPins S[1] = {
  /* S1 (DB9-1) */ {2, 3, 4, 5, 6, A0}
};

// Shared amplitude line to DUT AMP_C (PD7)
static const uint8_t PIN_AMPLITUDE_ALL = 9; // PWM-capable pin (Uno R4 WiFi)

// System comms (DB9-0) to DUT UART (optional for wrapper functions)
static const uint8_t PIN_UART_RXD_TO_DUT = 10; // drives DUT PD0 via level/interface (DB9-0 Pin 8)
static const uint8_t PIN_UART_TXD_FROM_DUT = 11; // reads DUT PD1 (DB9-0 Pin 9)

// Status LED 2-pin terminal (LED-TERM)
static const uint8_t PIN_STATUS_LED = 12; // drives LED terminal if needed for tests

static void setSafeDefaults() {
  for (int i = 0; i < 1; ++i) {
    pinMode(S[i].OVERLOAD_IN, OUTPUT);   digitalWrite(S[i].OVERLOAD_IN, LOW);
    pinMode(S[i].FREQ_DIV10_IN, OUTPUT); digitalWrite(S[i].FREQ_DIV10_IN, LOW);
    pinMode(S[i].FREQ_LOCK_IN, OUTPUT);  digitalWrite(S[i].FREQ_LOCK_IN, LOW);
    pinMode(S[i].START_OUT, OUTPUT);     digitalWrite(S[i].START_OUT, LOW);
    pinMode(S[i].RESET_OUT, OUTPUT);     digitalWrite(S[i].RESET_OUT, LOW);
    pinMode(S[i].POWER_ADC, INPUT);
  }
  pinMode(PIN_AMPLITUDE_ALL, OUTPUT);    analogWrite(PIN_AMPLITUDE_ALL, 0);
  pinMode(PIN_UART_RXD_TO_DUT, OUTPUT);  digitalWrite(PIN_UART_RXD_TO_DUT, HIGH); // idle high if via level shifter
  pinMode(PIN_UART_TXD_FROM_DUT, INPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);       digitalWrite(PIN_STATUS_LED, LOW);
}

static void handleLine(String line) {
  line.trim();
  if (line.length() == 0) return;

  // Very simple stub: recognize PING for smoke
  if (line.equalsIgnoreCase("PING")) {
    Serial.println("OK PONG");
    return;
  }

  // TODO: parse and implement protocol as described above
  Serial.println("ERR UNIMPLEMENTED");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  setSafeDefaults();
  Serial.println("OK WRAPPER_READY");
}

void loop() {
  static String buf;
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n') { handleLine(buf); buf = ""; }
    else if (c != '\r') { buf += c; }
  }
}
