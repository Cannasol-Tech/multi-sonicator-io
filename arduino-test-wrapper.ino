/**
 * @file arduino_test_wrapper.ino
 * @brief Arduino hardware test wrapper for ATtiny85 HIL
 *
 * Canonical Pin Mapping (ATtiny85 DIP) - Updated for ISP-compatible wiring:
 *   PB0 (Pin5)  AIR_OUT      -> Arduino D11 (input to Arduino)
 *   PB1 (Pin6)  PWM_OUT      -> Arduino D12 (digital monitor for duty cycle)
 *   PB2 (Pin7)  PUMP_OUT     -> Arduino D13 (input to Arduino)
 *   PB3 (Pin2)  SONIC_IN     <- Arduino D7 (driven by Arduino, active-low: LOW = ON)
 *   PB4 (Pin3)  PWM_IN       <- Arduino D5 (driven PWM generator to DUT)
 *   PB5 (Pin1)  STATUS_OUT   -> Arduino D10 (optional monitor; reset unless fuse changed)
 *
 * Reset (PB5/RST) normally cannot be used as GPIO unless high-voltage fuse change done.
 * If STATUS_OUT not enabled, D10 can be left unconnected.
 *
 * NOTE: Maintain this mapping in sync with docs/pin-map.md.
 */

#include <Arduino.h>

// Forward declarations for functions used before their definitions
void handleCommand(const String& cmd);
void reportStatus();
void monitorATtiny();
float readPwmDutyCycle();
int readPwmRaw();
void resetPwmAverage();
void resetAttiny();
void updateExpectedValues(uint8_t pwmInput);


// --- ATtiny connection (Arduino side logical pins) ---
const uint8_t ATTINY_PUMP_OUT_MON  = 13;   // Reads PUMP_OUT (PB2) - Updated for ISP wiring
const uint8_t ATTINY_PWM_OUT_MON   = 12;   // Digital reads PWM_OUT (PB1) - Updated for ISP wiring
const uint8_t ATTINY_AIR_OUT_MON   = 11;   // Reads AIR_OUT (PB0) - Updated for ISP wiring
const uint8_t ATTINY_RESET_OUT  = 10;   // (optional) DWEN (debug wire enable) - Updated for ISP wiring
const uint8_t ATTINY_SONIC_IN_DRV  = 7;    // Drives SONIC_IN (PB3) active-low
const uint8_t ATTINY_PWM_IN_DRV    = 5;    // Drives PWM_IN (PB4) (PWM capable)

// --- Status LED for debugging ---
const uint8_t STATUS_LED_PIN       = LED_BUILTIN; // Built-in LED (usually pin 13)

// --- PWM Control Constants (shared with main codebase) ---
// Try to include main codebase constants for consistency
#ifdef ARDUINO
  // Arduino environment - define constants locally if not available from main codebase
  #ifndef PWM_PASS_MIN
  #define PWM_PASS_MIN            2     // Minimum accepted PWM duty cycle (2%)
  #endif
  #ifndef PWM_PASS_MAX_ACCEPT
  #define PWM_PASS_MAX_ACCEPT     80    // Maximum accepted PWM duty cycle (80%)
  #endif
  #ifndef PWM_OUTPUT_CAP_PCT
  #define PWM_OUTPUT_CAP_PCT      77    // Output duty cycle cap (77% maximum)
  #endif
  #ifndef PUMP_ON_PCT
  #define PUMP_ON_PCT             2     // PWM duty cycle threshold to turn pump ON (<=2% PWM → HIGH output)
  #endif
  #ifndef PUMP_OFF_PCT
  #define PUMP_OFF_PCT            94    // PWM duty cycle threshold to turn pump OFF (>=94% PWM → LOW output)
  #endif
#else
  // Include main codebase constants when available
  #include "constants.h"
#endif

// --- State ---
bool testMode = false;
bool programmingMode = false;
unsigned long testStartTime = 0;
String lastCommand;
String firmwareVersion = "1.0.0";
uint8_t currentPwmInput = 0;  // Track what Arduino is generating

// --- Expected State Tracking ---
bool expectedSonicatorState = false;  // Expected sonicator state
bool expectedPumpState = false;       // Expected pump state (true = ON/LOW output, false = OFF/HIGH output)
bool expectedAirState = false;        // Expected air state (true = HIGH/ON, false = LOW/OFF)
float expectedPwmOutput = 0.0;        // Expected PWM output percentage

// --- Timer Progress Tracking ---
unsigned long timerStartTime = 0;     // When the air delay timer started

// Timer duration - can be dynamically configured
unsigned long TIMER_DURATION = 300000UL; // Default to 5 minutes for testing
bool timerRunning = false;            // Is the air delay timer currently running
bool lastSonicatorState = false;     // Previous sonicator state for edge detection

// Build detection and timer configuration
enum BuildType {
  BUILD_UNKNOWN = 0,
  BUILD_TEST = 1,      // 10 second timer
  BUILD_PRODUCTION = 2 // 5 minute timer
};
BuildType detectedBuildType = BUILD_UNKNOWN;
bool actualSonicatorState = false;   // Current sonicator state (for status display)

// --- LED blinking state ---
unsigned long lastLedToggle = 0;
bool ledState = false;
const unsigned long LED_BLINK_INTERVAL = 1000; // 1 second blink

// --- Auto status reportinpg ---
unsigned long lastStatusReport = 0;
const unsigned long STATUS_REPORT_INTERVAL = 1000; // 1 second auto status
bool autoStatusEnabled = true;  // Enable automatic status reporting by default

// --- Color Support Configuration ---
// Set to 1 for ANSI colors (works in VSCode, proper terminals)
// Set to 0 for plain text (works in Arduino IDE Serial Monitor)
#define ENABLE_COLORS 1

#if ENABLE_COLORS
  // ANSI Color Codes for terminals that support them
  #define COLOR_RESET   "\033[0m"
  #define COLOR_RED     "\033[31m"
  #define COLOR_GREEN   "\033[32m"
  #define COLOR_BLUE    "\033[34m"
  #define COLOR_CYAN    "\033[36m"
  #define COLOR_YELLOW  "\033[33m"
  #define COLOR_BOLD    "\033[1m"
#else
  // Plain text markers for Arduino IDE Serial Monitor
  #define COLOR_RESET   ""
  #define COLOR_RED     "[ERR]"
  #define COLOR_GREEN   "[OK]"
  #define COLOR_BLUE    ""
  #define COLOR_CYAN    "[INFO]"
  #define COLOR_YELLOW  ""
  #define COLOR_BOLD    ""
#endif

// --- Helpers ---
inline void sonicOn()  { digitalWrite(ATTINY_SONIC_IN_DRV, LOW); }   // active-low
inline void sonicOff() { digitalWrite(ATTINY_SONIC_IN_DRV, HIGH); }

void generatePwmInput(uint8_t dutyPct) {
  // Simple software PWM (placeholder): for real harness prefer hardware timer if precise duty needed
  // For now store desired duty and let monitor loop optionally refine (left minimal to avoid blocking).
  currentPwmInput = dutyPct;  // Track what we're generating

  // Update expected values based on PWM input
  updateExpectedValues(dutyPct);

  analogWrite(ATTINY_PWM_IN_DRV, map(dutyPct, 0, 100, 0, 255));
}

// Helper function to print colored text
void printColored(const String& text, const char* color) {
  Serial.print(color);
  Serial.print(text);
  Serial.print(COLOR_RESET);
}

// Helper function to print status with color based on correctness
void printStatus(const String& label, const String& actual, const String& expected, bool isCorrect, bool isMonitored = true) {
  Serial.print(label);
  if (isMonitored) {
    if (isCorrect) {
      printColored(actual, COLOR_GREEN);
    } else {
      printColored(actual + " (exp: " + expected + ")", COLOR_RED);
    }
  } else {
    printColored(actual, COLOR_CYAN);  // Light blue for unmonitored
  }
}

// Generate horizontal status bar
String generateStatusBar(float percentage, int width = 20) {
  String bar = "[";
  int filled = (int)((percentage / 100.0) * width);

  for (int i = 0; i < width; i++) {
    if (i < filled) {
      #if ENABLE_COLORS
        bar += "█";  // Unicode filled block (for terminals)
      #else
        bar += "#";  // ASCII filled block (for Arduino IDE)
      #endif
    } else {
      #if ENABLE_COLORS
        bar += "░";  // Unicode empty block (for terminals)
      #else
        bar += "-";  // ASCII empty block (for Arduino IDE)
      #endif
    }
  }
  bar += "] " + String(percentage, 1) + "%";
  return bar;
}

// Detect ATtiny85 build type by measuring timer behavior
BuildType detectATtinyBuildType() {
  // This function attempts to detect if the ATtiny85 is running production or test firmware
  // by observing timer behavior patterns

  // For now, we'll use a simple heuristic:
  // - If we can communicate and get consistent responses, assume it's working
  // - The actual detection would require observing timer behavior over time

  // TODO: Implement actual build detection by measuring timer duration
  // For now, return test build as default
  return BUILD_TEST;
}

// Configure timer duration based on detected or specified build type
void configureTimerDuration(BuildType buildType) {
  detectedBuildType = buildType;

  switch (buildType) {
    case BUILD_PRODUCTION:
      TIMER_DURATION = 300000UL; // 5 minutes
      break;
    case BUILD_TEST:
      TIMER_DURATION = 10000UL;  // 10 seconds
      break;
    default:
      TIMER_DURATION = 10000UL;  // Default to test timing
      break;
  }
}

// Calculate timer progress percentage (0% = just started, 100% = finished)
float getTimerProgress() {
  if (!timerRunning) {
    return 0.0;  // Timer not running
  }

  unsigned long elapsed = millis() - timerStartTime;
  if (elapsed >= TIMER_DURATION) {
    return 100.0;  // Timer finished
  }

  return (float(elapsed) / float(TIMER_DURATION)) * 100.0;
}

// Update timer state and air control expectations based on sonicator activity
void updateTimerState() {
  bool sonicLevel = digitalRead(ATTINY_SONIC_IN_DRV); // HIGH=OFF, LOW=ON
  actualSonicatorState = !sonicLevel;  // Invert for logical state (update global)

  // Detect sonicator state changes (edge detection)
  if (actualSonicatorState != lastSonicatorState) {
    // Sonicator just turned OFF - start the 5-minute air delay timer
    if (lastSonicatorState && !actualSonicatorState) {
      timerStartTime = millis();
      timerRunning = true;
      expectedAirState = true;  // Air should stay ON during delay
    }

    // Sonicator just turned ON - stop the timer, air should turn ON immediately
    if (!lastSonicatorState && actualSonicatorState) {
      timerRunning = false;
      expectedAirState = true;  // Air should turn ON when sonicator is active
    }

    // Update pump expectations when sonicator state changes
    updateExpectedValues(currentPwmInput);

    lastSonicatorState = actualSonicatorState;
  }

  // Update air expectations based on current state
  if (actualSonicatorState) {
    // Sonicator is ON - air should be ON
    expectedAirState = true;
  } else if (timerRunning) {
    // Sonicator is OFF but timer running - air should stay ON
    expectedAirState = true;
  } else {
    // Sonicator is OFF and no timer - air should be OFF
    expectedAirState = false;
  }

  // Auto-stop timer when it reaches 5 minutes
  if (timerRunning && (millis() - timerStartTime) >= TIMER_DURATION) {
    timerRunning = false;
    expectedAirState = false;  // Air should turn OFF after 5-minute delay
  }
}

// Update expected values based on PWM input
void updateExpectedValues(uint8_t pwmInput) {
  // PWM Output expectations (PWM_PASS_MIN-PWM_PASS_MAX_ACCEPT% pass through, capped at PWM_OUTPUT_CAP_PCT%)
  if (pwmInput == 0) {
    expectedPwmOutput = 0.0;
  } else if (pwmInput >= PWM_PASS_MIN && pwmInput <= PWM_PASS_MAX_ACCEPT) {
    expectedPwmOutput = (pwmInput > PWM_OUTPUT_CAP_PCT) ? (float)PWM_OUTPUT_CAP_PCT : (float)pwmInput;
  }
  // For PWM <PWM_PASS_MIN% or >PWM_PASS_MAX_ACCEPT%, expected output should remain unchanged (ignored)

  // Pump expectations based on PWM signals AND sonicator override
  // CRITICAL: Pump cannot be turned off while Sonicator is on
  if (actualSonicatorState) {
    // Sonicator is ON - pump MUST be ON regardless of PWM
    expectedPumpState = false;  // Pump ON (expect LOW pin with negative logic)
  } else {
    // Sonicator is OFF - normal PWM-based pump control (NEGATIVE LOGIC)
    if (pwmInput >= PUMP_ON_PCT) {
      expectedPumpState = false;  // Pump ON (expect LOW pin with negative logic)
    } else if (pwmInput <= PUMP_OFF_PCT) {
      expectedPumpState = true;   // Pump OFF (expect HIGH pin with negative logic)
    }
    // For PWM between PUMP_OFF_PCT and PUMP_ON_PCT, pump state should remain unchanged (hysteresis)
  }
}

void setup() {
  Serial.begin(115200);
  while(!Serial){}

  // Status LED
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, HIGH); // Turn on LED during setup

  // Drive pins
  pinMode(ATTINY_SONIC_IN_DRV, OUTPUT);
  pinMode(ATTINY_PWM_IN_DRV, OUTPUT);

  // Monitor pins - Changed from INPUT_PULLUP to INPUT for better signal integrity
  // ATtiny85 outputs should drive these pins directly without pull-up interference
  pinMode(ATTINY_AIR_OUT_MON, INPUT);
  pinMode(ATTINY_PUMP_OUT_MON, INPUT);
  pinMode(ATTINY_PWM_OUT_MON, INPUT);  // Now digital pin D12
  pinMode(ATTINY_RESET_OUT, INPUT_PULLUP); // Keep pull-up for optional/debug pin

  digitalWrite(ATTINY_RESET_OUT, HIGH); // keep ATtiny running
  sonicOff();
  currentPwmInput = 0;  // Initialize PWM input tracking
  analogWrite(ATTINY_PWM_IN_DRV, 0);

  // Initialize expected values
  expectedSonicatorState = false;  // Sonicator starts OFF
  expectedPumpState = false;       // Pump starts LOW (OFF)
  expectedPwmOutput = 0.0;         // PWM starts at 0%

  printColored("ATtiny85 HIL Test Wrapper v2.0.0", COLOR_BOLD);
  Serial.println();
  printColored("Enhanced with Color-Coded Status & Real-Time Verification", COLOR_GREEN);
  Serial.println();
  Serial.println(F("Commands: PING, RESET, SONIC_ON, SONIC_OFF, PWM:<0-100>, STATUS"));
  Serial.println(F("          AUTO_STATUS_ON, AUTO_STATUS_OFF, TEST_START, TEST_STOP, PROGRAM_MODE"));
  printColored("Auto Status: ENABLED (reports every 1 second with color verification)", COLOR_CYAN);
  Serial.println();
  printColored("Color Legend: ", COLOR_BOLD);
  printColored("GREEN=Correct ", COLOR_GREEN);
  printColored("RED=Error ", COLOR_RED);
  printColored("CYAN=Info", COLOR_CYAN);
  Serial.println();

  // LED setup complete - start blinking
  digitalWrite(STATUS_LED_PIN, LOW);
  lastLedToggle = millis();
}

void loop() {
  // Handle serial commands
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if(line.length()) handleCommand(line);
  }

  // Update timer state tracking
  updateTimerState();

  // Automatic status reporting every second (if enabled)
  unsigned long currentTime = millis();
  if (autoStatusEnabled && (currentTime - lastStatusReport >= STATUS_REPORT_INTERVAL)) {
    reportStatus();
    lastStatusReport = currentTime;
  }

  // Test mode monitoring (if enabled)
  if (testMode) {
    monitorATtiny();
    delay(100);
  } else {
    delay(50);  // Small delay to prevent overwhelming the serial output
  }
}

void resetAttiny() {
  // Assert reset (active low)
  pinMode(ATTINY_RESET_OUT, OUTPUT);
  digitalWrite(ATTINY_RESET_OUT, LOW);
  delay(50);

  // Release reset and re-enable pull-up (high-impedance on Arduino side)
  pinMode(ATTINY_RESET_OUT, INPUT_PULLUP);
  digitalWrite(ATTINY_RESET_OUT, HIGH);

  // Allow the ATtiny to boot
  delay(150);

  // Re-sync expectations with current inputs
  bool sonicLevel = digitalRead(ATTINY_SONIC_IN_DRV); // HIGH=OFF, LOW=ON
  actualSonicatorState = !sonicLevel;
  lastSonicatorState = actualSonicatorState;
  expectedSonicatorState = actualSonicatorState;

  // Safe defaults after reset; logic will update on next loop
  expectedPumpState = false;  // LOW (OFF)
  expectedAirState = actualSonicatorState ? true : false; // ON if sonicator held ON
}

void handleCommand(const String& cmd) {
  lastCommand = cmd;

  if (cmd == F("PING")) {
    Serial.println(F("OK:PONG"));
  } else if (cmd == F("RESET")) {
    resetAttiny();
    Serial.println(F("OK:RESET"));
  } else if (cmd == F("TIMER_STATUS")) {
    // Report timer status
    Serial.print(F("OK:TIMER_RUNNING:"));
    Serial.println(timerRunning ? F("TRUE") : F("FALSE"));
    if (timerRunning) {
      unsigned long remaining = TIMER_DURATION - (millis() - timerStartTime);
      Serial.print(F("OK:TIMER_REMAINING:"));
      Serial.println(remaining);
    }
  } else if (cmd == F("TIMER_START")) {
    // Force start timer
    timerStartTime = millis();
    timerRunning = true;
    expectedAirState = true;
    Serial.println(F("OK:TIMER_STARTED"));
  } else if (cmd == F("TIMER_STOP")) {
    // Force stop timer
    timerRunning = false;
    expectedAirState = false;
    Serial.println(F("OK:TIMER_STOPPED"));
  } else if (cmd == F("TIMER_RESTART")) {
    // Restart timer
    timerStartTime = millis();
    timerRunning = true;
    expectedAirState = true;
    Serial.println(F("OK:TIMER_RESTARTED"));
  } else if (cmd == F("SET_PRODUCTION_MODE")) {
    // Configure for production build (5-minute timer)
    configureTimerDuration(BUILD_PRODUCTION);
    Serial.print(F("OK:PRODUCTION_MODE_SET:TIMER_DURATION:"));
    Serial.println(TIMER_DURATION);
  } else if (cmd == F("SET_TEST_MODE")) {
    // Configure for test build (10-second timer)
    configureTimerDuration(BUILD_TEST);
    Serial.print(F("OK:TEST_MODE_SET:TIMER_DURATION:"));
    Serial.println(TIMER_DURATION);
  } else if (cmd == F("DETECT_BUILD")) {
    // Attempt to detect ATtiny85 build type
    BuildType detected = detectATtinyBuildType();
    configureTimerDuration(detected);
    Serial.print(F("OK:BUILD_DETECTED:"));
    Serial.print(detected == BUILD_PRODUCTION ? F("PRODUCTION") : F("TEST"));
    Serial.print(F(":TIMER_DURATION:"));
    Serial.println(TIMER_DURATION);
  } else if (cmd == F("GET_TIMER_CONFIG")) {
    // Report current timer configuration
    Serial.print(F("OK:TIMER_CONFIG:"));
    Serial.print(detectedBuildType == BUILD_PRODUCTION ? F("PRODUCTION") : F("TEST"));
    Serial.print(F(":DURATION:"));
    Serial.println(TIMER_DURATION);
  } else if (cmd == F("SONIC_ON")) {
    sonicOn();
    expectedSonicatorState = true;  // Track expected state
    updateExpectedValues(currentPwmInput);  // Update pump expectations
    Serial.println(F("OK:SONIC_ON"));
  } else if (cmd == F("SONIC_OFF")) {
    sonicOff();
    expectedSonicatorState = false;  // Track expected state
    updateExpectedValues(currentPwmInput);  // Update pump expectations
    Serial.println(F("OK:SONIC_OFF"));
  } else if (cmd.startsWith(F("PWM:"))) {
    int val = cmd.substring(4).toInt();
    if (val < 0) val = 0; if (val > 100) val = 100;
    generatePwmInput((uint8_t)val);
    Serial.print(F("OK:PWM_SET:")); Serial.println(val);
  } else if (cmd == F("STATUS")) {
    reportStatus();
  } else if (cmd == F("AUTO_STATUS_ON")) {
    autoStatusEnabled = true;
    Serial.println(F("OK:AUTO_STATUS_ENABLED"));
  } else if (cmd == F("AUTO_STATUS_OFF")) {
    autoStatusEnabled = false;
    Serial.println(F("OK:AUTO_STATUS_DISABLED"));
  } else if (cmd == F("TEST_START")) {
    testMode = true;
    testStartTime = millis();
    Serial.println(F("OK:TEST_MODE_ON"));
  } else if (cmd == F("TEST_STOP")) {
    testMode = false;
    Serial.println(F("OK:TEST_MODE_OFF"));
  } else if (cmd == F("PROGRAM_MODE")) {
    programmingMode = !programmingMode;
    Serial.print(F("OK:PROGRAM_MODE_"));
    Serial.println(programmingMode ? F("ON") : F("OFF"));
  } else {
    Serial.print(F("ERR:UNKNOWN:"));
    Serial.println(cmd);
  }
}

// Rolling average for PWM duty cycle readings
#define PWM_ROLLING_SAMPLES 5
static float pwmReadings[PWM_ROLLING_SAMPLES];
static int pwmReadingIndex = 0;
static bool pwmBufferFilled = false;

// Function to read PWM over multiple complete cycles
float readPwmDutyCycle() {
  // Sample over 5 complete PWM cycles (5 * 8.2ms = 41ms total)
  const int samplesPerCycle = 10;  // 10 samples per 8.2ms cycle
  const int totalSamples = PWM_ROLLING_SAMPLES * samplesPerCycle;  // 50 total samples
  long total = 0;

  // Sample over multiple complete PWM cycles
  for (int i = 0; i < totalSamples; i++) {
    total += (digitalRead(ATTINY_PWM_OUT_MON) == HIGH) ? 1023 : 0;  // Convert digital to analog equivalent
    delayMicroseconds(820);  // 820µs delay = 8.2ms / 10 samples
  }

  float currentReading = (total / (float)totalSamples / 1023.0) * 100.0;

  // Add to rolling average buffer
  pwmReadings[pwmReadingIndex] = currentReading;
  pwmReadingIndex = (pwmReadingIndex + 1) % PWM_ROLLING_SAMPLES;
  if (!pwmBufferFilled && pwmReadingIndex == 0) {
    pwmBufferFilled = true;
  }

  // Calculate rolling average
  float sum = 0;
  int count = pwmBufferFilled ? PWM_ROLLING_SAMPLES : pwmReadingIndex;
  for (int i = 0; i < count; i++) {
    sum += pwmReadings[i];
  }

  return sum / count;
}

// Function to read raw digital value for debugging (single sample)
int readPwmRaw() {
  return (digitalRead(ATTINY_PWM_OUT_MON) == HIGH) ? 1023 : 0;  // Single raw reading (digital converted to analog scale)
}

// Function to reset PWM rolling average (useful after PWM changes)
void resetPwmAverage() {
  pwmReadingIndex = 0;
  pwmBufferFilled = false;
  for (int i = 0; i < PWM_ROLLING_SAMPLES; i++) {
    pwmReadings[i] = 0;
  }
}

void reportStatus() {
  // Read current hardware states
  bool air  = digitalRead(ATTINY_AIR_OUT_MON);
  bool pump = digitalRead(ATTINY_PUMP_OUT_MON);
  float pwmDuty = readPwmDutyCycle();
  int pwmRaw = readPwmRaw();
  bool stat = digitalRead(ATTINY_RESET_OUT);
  bool sonicLevel = digitalRead(ATTINY_SONIC_IN_DRV); // HIGH=OFF, LOW=ON
  bool actualSonicatorState = !sonicLevel;  // Invert for logical state

  // Calculate verification results
  bool sonicatorCorrect = (actualSonicatorState == expectedSonicatorState);
  bool pwmCorrect = false;
  if (currentPwmInput >= PWM_PASS_MIN && currentPwmInput <= PWM_PASS_MAX_ACCEPT) {
    // PWM should pass through (within 5% tolerance)
    pwmCorrect = (abs(pwmDuty - expectedPwmOutput) <= 5.0);
  } else {
    pwmCorrect = true;  // Don't verify out-of-range PWM
  }
  bool pumpCorrect = (pump == expectedPumpState);

  // Header with status bar
  Serial.println();
  printColored("╔══════════════════════════════════════════════════════════════╗", COLOR_BOLD);
  Serial.println();
  printColored("║                    ATTINY85 SYSTEM STATUS                    ║", COLOR_BOLD);
  Serial.println();
  printColored("╠══════════════════════════════════════════════════════════════╣", COLOR_BOLD);
  Serial.println();

  // Timer Progress Bar (5-minute air delay timer)
  float timerProgress = getTimerProgress();
  String timerBar = generateStatusBar(timerProgress, 25);
  Serial.print(F("║ Timer Progress: "));
  if (timerRunning) {
    printColored(timerBar, COLOR_YELLOW);
    Serial.print(F(" "));
    Serial.print(timerProgress, 1);
    Serial.print(F("% ("));
    unsigned long remaining = TIMER_DURATION - (millis() - timerStartTime);
    Serial.print(remaining / 1000);
    Serial.println(F("s left)"));
  } else {
    printColored("[STOPPED] Timer not running", COLOR_CYAN);
    Serial.println();
  }

  // PWM Output Progress Bar
  String pwmBar = generateStatusBar(pwmDuty, 25);
  Serial.print(F("║ PWM Progress:   "));
  printColored(pwmBar, COLOR_BLUE);
  Serial.print(F(" "));
  Serial.print(pwmDuty, 1);
  Serial.println(F("%"));

  printColored("╠══════════════════════════════════════════════════════════════╣", COLOR_BOLD);
  Serial.println();

  // Monitored Outputs (with verification)
  Serial.print(F("║ Sonicator:     "));
  printStatus("", actualSonicatorState ? "ON" : "OFF",
              expectedSonicatorState ? "ON" : "OFF", sonicatorCorrect, true);
  Serial.println();

  Serial.print(F("║ PWM Input:     "));
  printColored(String(currentPwmInput) + "%", COLOR_CYAN);
  Serial.print(F(" → PWM Output: "));
  printStatus("", String(pwmDuty, 1) + "%", String(expectedPwmOutput, 1) + "%", pwmCorrect,
              (currentPwmInput >= PWM_PASS_MIN && currentPwmInput <= PWM_PASS_MAX_ACCEPT));
  Serial.println();

  Serial.print(F("║ Pump Output:   "));
  // Negative logic: LOW = ON, HIGH = OFF
  String pumpStatus = pump ? "HIGH (OFF)" : "LOW (ON)";
  // Show green when correct, red when incorrect, no expected value shown
  if (pumpCorrect) {
    printColored(pumpStatus, COLOR_GREEN);
  } else {
    printColored(pumpStatus, COLOR_RED);
  }
  Serial.println();

  // Pump Control Logic Summary
  Serial.print(F("║ Pump Logic:    "));
  if (actualSonicatorState) {
    printColored("Sonicator ON → Pump FORCED ON", COLOR_CYAN);
  } else if (currentPwmInput >= PUMP_ON_PCT) {
    // Use GREEN for ON to match expected behavior (negative logic)
    printColored("PWM ≥" + String(PUMP_ON_PCT) + "% → Pump ON", COLOR_GREEN);
  } else if (currentPwmInput <= PUMP_OFF_PCT) {
    printColored("PWM ≤" + String(PUMP_OFF_PCT) + "% → Pump OFF", COLOR_BLUE);
  } else {
    printColored("PWM " + String(PUMP_ON_PCT + 1) + "-" + String(PUMP_OFF_PCT - 1) + "% → Pump Unchanged (Hysteresis)", COLOR_YELLOW);
  }
  Serial.println();

  printColored("╠══════════════════════════════════════════════════════════════╣", COLOR_BOLD);
  Serial.println();

  // Air Output with verification and ON/OFF status
  bool airCorrect = (air == expectedAirState);
  Serial.print(F("║ Air Output:    "));
  printStatus("", air ? "HIGH (ON)" : "LOW (OFF)", expectedAirState ? "HIGH (ON)" : "LOW (OFF)", airCorrect, true);
  Serial.println();

  // Air Control Logic Summary
  Serial.print(F("║ Air Logic:     "));
  if (actualSonicatorState) {
    printColored("Sonicator ON → Air ON", COLOR_CYAN);
  } else if (timerRunning) {
    printColored("Delay Timer Running → Air ON", COLOR_YELLOW);
    Serial.print(F(" ("));
    unsigned long remaining = TIMER_DURATION - (millis() - timerStartTime);
    Serial.print(remaining / 1000);
    Serial.print(F("s left)"));
  } else {
    printColored("Timer Expired → Air OFF", COLOR_BLUE);
  }
  Serial.println();

  Serial.print(F("║ RESET Pin:    "));
  printColored(stat ? "HIGH" : "LOW", COLOR_BOLD);
  Serial.println();

  Serial.print(F("║ PWM Raw ADC:   "));
  printColored(String(pwmRaw) + "/1023", COLOR_CYAN);
  Serial.println();

  printColored("╠══════════════════════════════════════════════════════════════╣", COLOR_BOLD);
  Serial.println();

  // System Information
  Serial.print(F("║ Test Mode:     "));
  printColored(testMode ? "ON" : "OFF", COLOR_CYAN);
  Serial.println();

  Serial.print(F("║ Auto Status:   "));
  printColored(autoStatusEnabled ? "ENABLED" : "DISABLED", COLOR_CYAN);
  Serial.println();

  Serial.print(F("║ Uptime:        "));
  printColored(String(millis() / 1000.0, 1) + "s", COLOR_CYAN);
  Serial.println();

  Serial.print(F("║ Last Command:  "));
  printColored(lastCommand.length() > 0 ? lastCommand : "NONE", COLOR_CYAN);
  Serial.println();

  printColored("╚══════════════════════════════════════════════════════════════╝", COLOR_BOLD);
  Serial.println();

  // Summary line with overall status
  int correctCount = (sonicatorCorrect ? 1 : 0) + (pwmCorrect ? 1 : 0) + (pumpCorrect ? 1 : 0);
  Serial.print(F("Status: "));
  if (correctCount == 3) {
    printColored("✓ ALL SYSTEMS NOMINAL", COLOR_GREEN);
  } else {
    printColored("⚠ " + String(3 - correctCount) + " ISSUE(S) DETECTED", COLOR_RED);
  }
  Serial.println();
  Serial.println();
}

void monitorATtiny() {
  static bool lastAir = false;
  static bool lastPump = false;
  static float lastPwmDuty = -1.0;  // Track PWM duty cycle changes
  static bool lastStatus = false;
  static unsigned long lastPeriodic = 0;

  bool air  = digitalRead(ATTINY_AIR_OUT_MON);
  bool pump = digitalRead(ATTINY_PUMP_OUT_MON);
  float pwmDuty = readPwmDutyCycle();  // Use averaged reading
  bool stat = digitalRead(ATTINY_RESET_OUT);

  if (air != lastAir) {
    Serial.print(F("EVENT:AIR_OUT:"));
    Serial.print(air ? F("HIGH (ON)") : F("LOW (OFF)"));
    // Show if this matches expectations
    bool airCorrect = (air == expectedAirState);
    Serial.print(F(" - "));
    printColored(airCorrect ? "CORRECT" : "UNEXPECTED", airCorrect ? COLOR_GREEN : COLOR_RED);
    Serial.println();
    lastAir = air;
  }
  if (pump != lastPump) {
    Serial.print(F("EVENT:PUMP_OUT:")); Serial.println(pump ? F("HIGH") : F("LOW"));
    lastPump = pump;
  }
  // Disable PWM change events to reduce serial spam
  // if (abs(pwmDuty - lastPwmDuty) > 2.0) {  // Report if PWM changes by >2%
  //   Serial.print(F("EVENT:PWM_OUT:")); Serial.print(pwmDuty, 1); Serial.println(F("%"));
  //   lastPwmDuty = pwmDuty;
  // }
  if (stat != lastStatus) {
    Serial.print(F("EVENT:STATUS_OUT:")); Serial.println(stat ? F("HIGH") : F("LOW"));
    lastStatus = stat;
  }

  if (millis() - lastPeriodic > 5000) {
    reportStatus();
    lastPeriodic = millis();
  }
}