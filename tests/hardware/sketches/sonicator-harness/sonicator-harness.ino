#include <Arduino.h>
#include "Harness.h"

// S4-only harness
// Pin map aligned with docs/Planning/pin-matrix.md (source of truth)
// Avoids D0/D1 (USB Serial) and D10–D13 (SPI)
// Semantics per DUT (ATmega32A):
//  - FREQ_DIV10_4, FREQ_LOCK_4, OVERLOAD_4 are inputs to DUT (Arduino drives them)
//  - START_4, RESET_4 are outputs from DUT (Arduino reads them, with INPUT_PULLUP)
//  - POWER_SENSE_4 is analog; read by Arduino unless specified otherwise
//  - AMPLITUDE_ALL is PWM output to DUT

static const SonicatorChannelPins S4_PINS {
  /*startIn*/     A3,   // START_4 (DUT -> Harness)
  /*resetIn*/     A4,   // RESET_4 (DUT -> Harness)
  /*ampInA*/      A1,   // POWER_SENSE_4 (analog)
  /*ampMonA*/     A0,   // AMPLITUDE_MON_4 (analog tap of PWM/RC)
  /*powerPwmOut*/ 9,    // AMPLITUDE_ALL (PWM to DUT)
  /*statusOut*/   A2,   // OVERLOAD_4 (Harness -> DUT)
  /*freqLockOut*/ 8,    // FREQ_LOCK_4 (Harness -> DUT)
  /*freqOut*/     7     // FREQ_DIV10_4 (Harness -> DUT)
};

static SonicatorHarness HARNESS;

static uint32_t g_lastDashboardMs = 0;

static void printDashboardOnce() {
  SonicatorHarnessChannel::Snapshot s;
  HARNESS.getSnapshot(s);

  auto onOff = [](bool v) { return v ? "ON " : "OFF"; };
  auto yesNo = [](bool v) { return v ? "YES" : "NO "; };

  Serial.println(F("+------------------+------------+------------+"));
  Serial.println(F("| Field            |     S4     |   Units    |"));
  Serial.println(F("+------------------+------------+------------+"));
  Serial.print(F("| START            | ")); Serial.print(onOff(s.start)); Serial.println(F("      | logic     |"));
  Serial.print(F("| RESET            | ")); Serial.print(onOff(s.reset)); Serial.println(F("      | logic     |"));
  Serial.print(F("| AMP RAW          | ")); Serial.printf("%4d       | 0-1023   |\n", s.ampRaw);
  Serial.print(F("| AMP MON RAW      | ")); Serial.printf("%4d       | 0-1023   |\n", s.ampMonRaw);
  Serial.print(F("| AMP MON %        | ")); Serial.printf("%3u%%       | percent  |\n", s.ampMonPct);
  Serial.print(F("| POWER            | ")); Serial.printf("%3u%%       | percent  |\n", s.powerPct);
  Serial.print(F("| FREQ OUT         | ")); Serial.printf("%5u Hz   | Hz       |\n", s.freqHz);
  Serial.print(F("| STATUS (OK)      | ")); Serial.print(yesNo(s.status)); Serial.println(F("     | logic     |"));
  Serial.print(F("| FREQ LOCK        | ")); Serial.print(yesNo(s.lock)); Serial.println(F("     | logic     |"));
  Serial.println(F("+------------------+------------+------------+"));
}

static void printHelp() {
  Serial.println(F("Sonicator Harness (S4 only)"));
  Serial.println(F("Commands:"));
  Serial.println(F("  p=<0..100>     set S4 power percent"));
  Serial.println(F("  f=<Hz>         set S4 freq out (Hz)"));
  Serial.println(F("  m=n|o|l        S4 mode: normal|overload|nolock"));
  Serial.println(F("  report         print current status"));
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { /* wait for USB */ }

  HARNESS.begin(S4_PINS);

  // Defaults
  Serial.println();
  printHelp();
  Serial.println();
}

void loop() {
  HARNESS.update();
  // ASCII dashboard at 1 Hz
  if (millis() - g_lastDashboardMs > 1000) {
    printDashboardOnce();
    g_lastDashboardMs = millis();
  }
}
