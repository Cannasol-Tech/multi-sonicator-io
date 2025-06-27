/*
 *  ATmega32A template project
 *  Tested with MightyCore v2.x and PlatformIO
 *
 *  ─────────────────────────────────────────────────────────
 *  Hardware:
 *      • ATmega32A @ 16 MHz crystal  (board_build.f_cpu = 16000000L)
 *      • Arduino-as-ISP or USBasp for uploading
 *      • LED on PC0  (physical pin 40, MightyCore “Arduino pin 0”)
 *  ─────────────────────────────────────────────────────────
 */

#include <Arduino.h>

/* -------- Pin definitions -------- */
const uint8_t LED_PIN   = 0;      // MightyCore maps “0”  → PC0
const uint8_t BUTTON_PIN = 1;     // PC1 just as an example input

/* -------- Globals -------- */
volatile bool toggled = false;

/* -------- Setup -------- */
void setup()
{
    /* GPIO */
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);   // active-low button

    /* Serial (UART0 = PD0/PD1) */
    Serial.begin(9600);
    while (!Serial);                     // wait for terminal (optional)

    Serial.println(F("ATmega32A template – hello, world!"));

    /* Timer1 example: 1 Hz interrupt */
    TCCR1A = 0;                          // normal mode
    TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10); // CTC, prescale 1024
    OCR1A  = F_CPU / 1024 / 1 - 1;       // 1 second exactly
    TIMSK |= _BV(OCIE1A);                // enable compare-match A ISR
}

/* -------- Main loop -------- */
void loop()
{
    // Blink LED under software control whenever button is pressed
    if (digitalRead(BUTTON_PIN) == LOW) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
    }

    // Periodic debug
    static uint32_t last = 0;
    if (millis() - last > 1000) {
        last = millis();
        Serial.print(F("Uptime (ms): "));
        Serial.println(last);
    }

    // The LED also toggles inside the Timer1 ISR every second
}

/* -------- Timer1 Compare-Match ISR -------- */
ISR(TIMER1_COMPA_vect)
{
    toggled = !toggled;
    digitalWrite(LED_PIN, toggled);
}
