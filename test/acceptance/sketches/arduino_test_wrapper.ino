/*
 * arduino_test_wrapper.ino
 *
 * Purpose:
 *  - Hardware-in-the-Loop (HIL) wrapper to electrically interface with the ATmega32A DUT.
 *  - Exposes a simple serial command protocol for Behave/pytest to stimulate inputs and read outputs.
 *  - Validates MODBUS contract semantics end-to-end via GPIO/ADC interactions and timing assertions.
 *
 * Location:
 *  - test/acceptance/sketches/arduino_test_wrapper/arduino_test_wrapper.ino
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
// Uno R4 WiFi single-channel harness (S4 ONLY). See docs/planning/pin-matrix.md (SOLE SOURCE OF TRUTH).
// This matches the sonicator-harness.ino implementation - only S4 is physically connected.

struct SonPins {
  uint8_t OVERLOAD_IN;   // Drives opto input seen by DUT PDx (active level per harness)
  uint8_t FREQ_DIV10_IN; // Generates frequency ÷10 to DUT PBx
  uint8_t FREQ_LOCK_IN;  // Drives opto input seen by DUT PBx
  uint8_t START_OUT;     // Reads DUT start (if observed) or drives if simulating external
  uint8_t RESET_OUT;     // Reads DUT reset (if observed) or drives if simulating external
  uint8_t POWER_ADC;     // Reads analog proxy for power sense if loopbacked
  uint8_t AMPLITUDE_PWM; // PWM output for amplitude control (shared across all sonicators)
};

// Single channel (S4) mapping per pin-matrix SOLE SOURCE OF TRUTH
static SonPins S4_PINS = {
  /* OVERLOAD_IN */   A2,  // OVERLOAD_4 (Harness -> DUT PD3 Pin 17)
  /* FREQ_DIV10_IN */ 7,   // FREQ_DIV10_4 (Harness -> DUT PB0 Pin 1) - D7 per pin-matrix
  /* FREQ_LOCK_IN */  8,   // FREQ_LOCK_4 (Harness -> DUT PB4 Pin 5) - D8 per pin-matrix
  /* START_OUT */     A3,  // START_4 (DUT PC0 Pin 22 -> Harness) - A3 per pin-matrix
  /* RESET_OUT */     A4,  // RESET_4 (DUT PC1 Pin 23 -> Harness) - A4 per pin-matrix
  /* POWER_ADC */     A1,  // POWER_SENSE_4 (DUT PA7 Pin 33 analog) - A1 per pin-matrix
  /* AMPLITUDE_PWM */ 9    // AMPLITUDE_ALL (Harness -> DUT PD7 Pin 21) - D9 PWM per pin-matrix
};

// Shared amplitude line to DUT AMP_C (PD7) - matches sonicator-harness.ino
static const uint8_t PIN_AMPLITUDE_ALL = 9; // PWM-capable pin (Uno R4 WiFi)

// System comms (DB9-0) to DUT UART (optional for wrapper functions)
static const uint8_t PIN_UART_RXD_TO_DUT = 10; // drives DUT PD0 via level/interface (DB9-0 Pin 8)
static const uint8_t PIN_UART_TXD_FROM_DUT = 11; // reads DUT PD1 (DB9-0 Pin 9)

// Status LED 2-pin terminal (LED-TERM)
static const uint8_t PIN_STATUS_LED = 12; // drives LED terminal if needed for tests

static void setSafeDefaults() {
  // Configure S4 pins (only physically implemented unit)
  pinMode(S4_PINS.OVERLOAD_IN, OUTPUT);   digitalWrite(S4_PINS.OVERLOAD_IN, LOW);
  pinMode(S4_PINS.FREQ_DIV10_IN, OUTPUT); digitalWrite(S4_PINS.FREQ_DIV10_IN, LOW);
  pinMode(S4_PINS.FREQ_LOCK_IN, OUTPUT);  digitalWrite(S4_PINS.FREQ_LOCK_IN, LOW);
  pinMode(S4_PINS.START_OUT, INPUT_PULLUP);  // Read DUT output
  pinMode(S4_PINS.RESET_OUT, INPUT_PULLUP);  // Read DUT output
  pinMode(S4_PINS.POWER_ADC, INPUT);

  // Configure shared pins
  pinMode(PIN_AMPLITUDE_ALL, OUTPUT);    analogWrite(PIN_AMPLITUDE_ALL, 0);
  pinMode(PIN_UART_RXD_TO_DUT, OUTPUT);  digitalWrite(PIN_UART_RXD_TO_DUT, HIGH); // idle high if via level shifter
  pinMode(PIN_UART_TXD_FROM_DUT, INPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);       digitalWrite(PIN_STATUS_LED, LOW);
}

static void handleLine(String line) {
  line.trim();
  if (line.length() == 0) return;

  // Parse command
  if (line.equalsIgnoreCase("PING")) {
    Serial.println("OK PONG");
    return;
  }

  if (line.equalsIgnoreCase("INFO")) {
    Serial.println("OK HIL-Wrapper/0.1.0 S4-ONLY");
    return;
  }

  // SET commands for S4 only
  if (line.startsWith("SET OVERLOAD 4 ")) {
    int value = line.substring(15).toInt();
    digitalWrite(S4_PINS.OVERLOAD_IN, value ? HIGH : LOW);
    Serial.println("OK");
    return;
  }

  if (line.startsWith("SET FREQ 4 ")) {
    // For now, just acknowledge - frequency generation would need timer setup
    Serial.println("OK");
    return;
  }

  if (line.startsWith("SET LOCK 4 ")) {
    int value = line.substring(11).toInt();
    digitalWrite(S4_PINS.FREQ_LOCK_IN, value ? HIGH : LOW);
    Serial.println("OK");
    return;
  }

  // READ commands for S4
  if (line.equalsIgnoreCase("READ STATUS 4")) {
    bool start = !digitalRead(S4_PINS.START_OUT);  // Active low with pullup
    bool reset = !digitalRead(S4_PINS.RESET_OUT);  // Active low with pullup
    bool overload = digitalRead(S4_PINS.OVERLOAD_IN);
    bool locked = digitalRead(S4_PINS.FREQ_LOCK_IN);

    Serial.print("OK RUN=");
    Serial.print(start ? "1" : "0");
    Serial.print(" OVL=");
    Serial.print(overload ? "1" : "0");
    Serial.print(" LOCK=");
    Serial.println(locked ? "1" : "0");
    return;
  }

  if (line.equalsIgnoreCase("READ POWER 4")) {
    int adc = analogRead(S4_PINS.POWER_ADC);
    Serial.print("OK POWER=");
    Serial.println(adc);
    return;
  }

  // Commands for non-implemented units (S1-S3)
  if (line.startsWith("SET OVERLOAD ") && (line.indexOf(" 1 ") > 0 || line.indexOf(" 2 ") > 0 || line.indexOf(" 3 ") > 0)) {
    Serial.println("ERR UNIT_NOT_IMPLEMENTED");
    return;
  }

  Serial.println("ERR UNKNOWN_COMMAND");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  setSafeDefaults();
  Serial.println("OK WRAPPER_READY S4-ONLY");
}

void loop() {
  static String buf;
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n') { handleLine(buf); buf = ""; }
    else if (c != '\r') { buf += c; }
  }
}
