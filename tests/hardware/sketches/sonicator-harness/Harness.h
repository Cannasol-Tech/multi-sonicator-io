#pragma once
#include <Arduino.h>

struct SonicatorChannelPins {
  // DUT -> Harness
  uint8_t startIn;   // digital in
  uint8_t resetIn;   // digital in
  uint8_t ampInA;    // analog in (A0/A1)
  uint8_t ampMonA;   // analog in to monitor AMPLITUDE_ALL (optional, 255=disabled)

  // Harness -> DUT
  uint8_t powerPwmOut;   // PWM (RC filter for analog level)
  uint8_t statusOut;     // digital out
  uint8_t freqLockOut;   // digital out
  uint8_t freqOut;       // digital out (square wave)
};

enum class Scenario : uint8_t { Normal = 0, Overload = 1, NoLock = 2 };

class SonicatorHarnessChannel {
public:
  SonicatorHarnessChannel(const SonicatorChannelPins& pins, const char* name)
    : _p(pins), _name(name) {}

  void begin() {
    pinMode(_p.startIn, INPUT_PULLUP);
    pinMode(_p.resetIn, INPUT_PULLUP);

    pinMode(_p.statusOut, OUTPUT);
    pinMode(_p.freqLockOut, OUTPUT);
    pinMode(_p.powerPwmOut, OUTPUT);
    pinMode(_p.freqOut, OUTPUT);

    // Defaults
    digitalWrite(_p.statusOut, HIGH);   // status OK
    digitalWrite(_p.freqLockOut, HIGH); // locked
    digitalWrite(_p.freqOut, LOW);
    analogWrite(_p.powerPwmOut, 0);

    _lastFreqToggle = micros();
  }

  void setScenario(Scenario s) { _scenario = s; }
  void setPowerPercent(uint8_t pct) { _powerPct = constrain(pct, 0, 100); }
  void setFreqHz(uint16_t hz) { _freqHz = hz; }
  void setEventLogging(bool enabled) { _eventLogging = enabled; }
  void setPeriodicReporting(bool enabled) { _periodicReports = enabled; }

  struct Snapshot {
    const char* name;
    bool start;
    bool reset;
    int ampRaw;
    int ampMonRaw;     // AMPLITUDE_ALL monitor raw (0-1023), -1 if disabled
    uint8_t ampMonPct; // Derived percent from ampMonRaw (0-100)
    uint8_t powerPct;
    uint16_t freqHz;
    bool status;
    bool lock;
  };

  void getSnapshot(Snapshot& s) const {
    s.name = _name;
    s.start = _lastStart;
    s.reset = _lastReset;
    s.ampRaw = _lastAmpRaw;
    s.ampMonRaw = _lastAmpMonRaw;
    s.ampMonPct = (_lastAmpMonRaw >= 0) ? (uint8_t)map(_lastAmpMonRaw, 0, 1023, 0, 100) : 0;
    s.powerPct = _powerPct;
    s.freqHz = _freqHz;
    s.status = digitalRead(_p.statusOut);
    s.lock = digitalRead(_p.freqLockOut);
  }

  void update() {
    // Read DUT outputs (into harness)
    bool startSig = digitalRead(_p.startIn) == LOW ? true : false; // active low if pullups
    bool resetSig = digitalRead(_p.resetIn) == LOW ? true : false;

    if (_eventLogging) {
      if (startSig != _lastStart) {
        Serial.print(F("EVENT:")); Serial.print(_name); Serial.print(F(":START:"));
        Serial.println(startSig ? F("ON") : F("OFF"));
        _lastStart = startSig;
      }
      if (resetSig != _lastReset) {
        Serial.print(F("EVENT:")); Serial.print(_name); Serial.print(F(":RESET:"));
        Serial.println(resetSig ? F("ACTIVE") : F("IDLE"));
        _lastReset = resetSig;
      }
    } else {
      // Still update last values quietly
      _lastStart = startSig;
      _lastReset = resetSig;
    }

    int ampRaw = analogRead(_p.ampInA);
    if (_eventLogging) {
      if (abs(ampRaw - _lastAmpRaw) > 8) { // reduce spam
        Serial.print(F("EVENT:")); Serial.print(_name); Serial.print(F(":AMP_RAW:"));
        Serial.println(ampRaw);
        _lastAmpRaw = ampRaw;
      }
    } else {
      _lastAmpRaw = ampRaw;
    }

    // Optional: monitor AMPLITUDE_ALL analog via separate tap
    if (_p.ampMonA != 255) {
      int am = analogRead(_p.ampMonA);
      if (_eventLogging) {
        if (_lastAmpMonRaw < 0 || abs(am - _lastAmpMonRaw) > 8) {
          Serial.print(F("EVENT:")); Serial.print(_name); Serial.print(F(":AMP_MON_RAW:"));
          Serial.println(am);
          _lastAmpMonRaw = am;
        }
      } else {
        _lastAmpMonRaw = am;
      }
    } else {
      _lastAmpMonRaw = -1;
    }

    // Drive harness outputs according to scenario
    switch (_scenario) {
      case Scenario::Normal:
        digitalWrite(_p.statusOut, HIGH);
        digitalWrite(_p.freqLockOut, HIGH);
        break;
      case Scenario::Overload:
        digitalWrite(_p.statusOut, LOW);
        digitalWrite(_p.freqLockOut, LOW);
        break;
      case Scenario::NoLock:
        digitalWrite(_p.statusOut, HIGH);
        digitalWrite(_p.freqLockOut, LOW);
        break;
    }

    // POWER analog via PWM duty (simple mapping; add RC on DUT side)
    uint8_t duty = map(_powerPct, 0, 100, 0, 255);
    analogWrite(_p.powerPwmOut, duty);

    // Generate FREQ_OUT square wave using micros() (non-blocking)
    if (_freqHz > 0) {
      uint32_t halfPeriodUs = 500000UL / _freqHz; // half period
      uint32_t now = micros();
      if (now - _lastFreqToggle >= halfPeriodUs) {
        _freqState = !_freqState;
        digitalWrite(_p.freqOut, _freqState ? HIGH : LOW);
        _lastFreqToggle = now;
      }
    } else {
      digitalWrite(_p.freqOut, LOW);
    }

    // Optional periodic status every 1s (disabled by default)
    if (_periodicReports) {
      if (millis() - _lastReportMs > 1000) {
        report();
        _lastReportMs = millis();
      }
    }
  }

  void report() {
    // Compact 1-line summary suitable for log parsing
    // REPORT:<name> start=<0/1> reset=<0/1> ampRaw=<0-1023> power=<%> freq=<Hz> status=<0/1> lock=<0/1>
    Serial.print(F("REPORT:")); Serial.print(_name);
    Serial.print(F(" start=")); Serial.print(_lastStart ? 1 : 0);
    Serial.print(F(" reset=")); Serial.print(_lastReset ? 1 : 0);
    Serial.print(F(" ampRaw=")); Serial.print(_lastAmpRaw);
    Serial.print(F(" ampMonRaw=")); Serial.print(_lastAmpMonRaw);
    Serial.print(F(" ampMonPct=")); Serial.print((_lastAmpMonRaw >= 0) ? map(_lastAmpMonRaw, 0, 1023, 0, 100) : 0);
    Serial.print(F(" power=")); Serial.print(_powerPct);
    Serial.print(F(" freq=")); Serial.print(_freqHz);
    Serial.print(F(" status=")); Serial.print(digitalRead(_p.statusOut));
    Serial.print(F(" lock=")); Serial.println(digitalRead(_p.freqLockOut));
  }

private:
  SonicatorChannelPins _p;
  const char* _name;

  // State
  bool _lastStart = false;
  bool _lastReset = false;
  int  _lastAmpRaw = -1;
  int  _lastAmpMonRaw = -1;
  Scenario _scenario = Scenario::Normal;
  uint8_t _powerPct = 0;
  uint16_t _freqHz = 20000; // default 20 kHz square
  bool _eventLogging = false; // disabled by default
  bool _periodicReports = false; // disabled by default

  // Timing
  uint32_t _lastReportMs = 0;
  uint32_t _lastFreqToggle = 0;
  bool _freqState = false;
};

class SonicatorHarness {
public:
  // Single-channel begin (e.g., S4-only)
  void begin(const SonicatorChannelPins& p1) {
    ch1 = new SonicatorHarnessChannel(p1, "S4");
    ch2 = nullptr;
    ch1->begin();
  }

  void begin(const SonicatorChannelPins& p1, const SonicatorChannelPins& p2) {
    ch1 = new SonicatorHarnessChannel(p1, "S1");
    ch2 = new SonicatorHarnessChannel(p2, "S2");
    ch1->begin();
    ch2->begin();
  }

  void update() {
    if (ch1) ch1->update();
    if (ch2) ch2->update();
    handleSerial();
  }

  // Snapshot helpers
  void getSnapshot(SonicatorHarnessChannel::Snapshot& s1) const {
    if (ch1) ch1->getSnapshot(s1);
  }
  void getSnapshots(SonicatorHarnessChannel::Snapshot& s1, SonicatorHarnessChannel::Snapshot& s2) const {
    if (ch1) ch1->getSnapshot(s1);
    if (ch2) {
      ch2->getSnapshot(s2);
    } else {
      s2.name = "N/A";
      s2.start = false;
      s2.reset = false;
      s2.ampRaw = 0;
      s2.powerPct = 0;
      s2.freqHz = 0;
      s2.status = false;
      s2.lock = false;
    }
  }

  void handleSerial() {
    // UI supports both legacy two-channel cmds and single-channel aliases:
    // Single: p=<0-100>, f=<Hz>, m=[n|o|l], report
    // Legacy: s1p=..., s1f=..., s1m=... (applies to ch1 if present)
    while (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) continue;

      // Single-channel aliases
      if (line.startsWith("p=")) { if (ch1) ch1->setPowerPercent((uint8_t)line.substring(2).toInt()); ack(); continue; }
      if (line.startsWith("f=")) { if (ch1) ch1->setFreqHz((uint16_t)line.substring(2).toInt()); ack(); continue; }
      if (line.startsWith("m=")) { if (ch1) setMode(*ch1, line.substring(2)); continue; }
      if (line == "report") { if (ch1) ch1->report(); if (ch2) ch2->report(); continue; }

      // Legacy two-channel commands (kept for compatibility)
      if (line.startsWith("s1p=")) { if (ch1) ch1->setPowerPercent((uint8_t)line.substring(4).toInt()); ack(); }
      else if (line.startsWith("s2p=")) { if (ch2) ch2->setPowerPercent((uint8_t)line.substring(4).toInt()); ack(); }
      else if (line.startsWith("s1f=")) { if (ch1) ch1->setFreqHz((uint16_t)line.substring(4).toInt()); ack(); }
      else if (line.startsWith("s2f=")) { if (ch2) ch2->setFreqHz((uint16_t)line.substring(4).toInt()); ack(); }
      else if (line.startsWith("s1m=")) { if (ch1) setMode(*ch1, line.substring(4)); }
      else if (line.startsWith("s2m=")) { if (ch2) setMode(*ch2, line.substring(4)); }
      else { Serial.println(F("ERR:unknown cmd")); }
    }
  }

private:
  SonicatorHarnessChannel* ch1 = nullptr;
  SonicatorHarnessChannel* ch2 = nullptr;

  void ack() { Serial.println(F("OK")); }
  void setMode(SonicatorHarnessChannel& ch, const String& m) {
    if (m == "n") ch.setScenario(Scenario::Normal);
    else if (m == "o") ch.setScenario(Scenario::Overload);
    else if (m == "l") ch.setScenario(Scenario::NoLock);
    else { Serial.println(F("ERR:mode")); return; }
    ack();
  }
};
