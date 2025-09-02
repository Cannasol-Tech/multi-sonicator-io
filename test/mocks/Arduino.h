/**
 * @file Arduino.h
 * @brief Arduino framework mock for native testing
 * @author Cannasol Technologies
 * @date 2025-09-02
 * @version 1.0.0
 */

#ifndef ARDUINO_H
#define ARDUINO_H

#ifdef NATIVE_TEST

#include <stdint.h>
#include <stdbool.h>

// Digital pin states
#define HIGH 1
#define LOW  0

// Pin modes
#define INPUT         0
#define OUTPUT        1
#define INPUT_PULLUP  2

// Analog reference types
#define DEFAULT       1
#define INTERNAL      3
#define EXTERNAL      0

// Mock register definitions for ATmega32A
extern uint8_t mock_PORTA, mock_PORTB, mock_PORTC, mock_PORTD;
extern uint8_t mock_DDRA, mock_DDRB, mock_DDRC, mock_DDRD;
extern uint8_t mock_PINA, mock_PINB, mock_PINC, mock_PIND;
extern uint8_t mock_TCCR0, mock_TCCR1A, mock_TCCR1B, mock_TCCR2;
extern uint8_t mock_TCNT0, mock_TCNT1, mock_OCR0, mock_OCR1A, mock_OCR2;
extern uint8_t mock_TIMSK, mock_UCSRA, mock_UCSRB, mock_UCSRC;
extern uint8_t mock_UBRRH, mock_UBRRL, mock_UDR;
extern uint8_t mock_ADMUX, mock_ADCSRA, mock_ADCL, mock_ADCH;

// Register bit definitions
#define WGM01   3
#define WGM00   6
#define WGM21   3
#define WGM20   6
#define WGM13   4
#define WGM12   3
#define CS02    2
#define CS01    1
#define CS00    0
#define CS22    2
#define CS21    1
#define CS20    0
#define COM21   5
#define COM20   4
#define OCIE0   1
#define TOIE0   0
#define OCIE1A  4
#define TOIE1   2
#define ADEN    7
#define ADSC    6
#define REFS1   7
#define REFS0   6
#define RXEN    4
#define TXEN    3
#define RXCIE   7
#define UDRIE   5
#define URSEL   7
#define UCSZ2   2
#define UCSZ1   2
#define UCSZ0   1
#define UPM1    5
#define UPM0    4
#define USBS    3
#define FE      4
#define PE      2

// Register mappings for mock
#define PORTA   mock_PORTA
#define PORTB   mock_PORTB
#define PORTC   mock_PORTC
#define PORTD   mock_PORTD
#define DDRA    mock_DDRA
#define DDRB    mock_DDRB
#define DDRC    mock_DDRC
#define DDRD    mock_DDRD
#define PINA    mock_PINA
#define PINB    mock_PINB
#define PINC    mock_PINC
#define PIND    mock_PIND
#define TCCR0   mock_TCCR0
#define TCCR1A  mock_TCCR1A
#define TCCR1B  mock_TCCR1B
#define TCCR2   mock_TCCR2
#define TCNT0   mock_TCNT0
#define TCNT1   mock_TCNT1
#define OCR0    mock_OCR0
#define OCR1A   mock_OCR1A
#define OCR2    mock_OCR2
#define TIMSK   mock_TIMSK
#define UCSRA   mock_UCSRA
#define UCSRB   mock_UCSRB
#define UCSRC   mock_UCSRC
#define UBRRH   mock_UBRRH
#define UBRRL   mock_UBRRL
#define UDR     mock_UDR
#define ADMUX   mock_ADMUX
#define ADCSRA  mock_ADCSRA
#define ADCL    mock_ADCL
#define ADCH    mock_ADCH

// CPU frequency for calculations
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// Function declarations
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogReference(uint8_t type);
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
unsigned long millis(void);
unsigned long micros(void);

// Interrupt control
void sei(void);
void cli(void);

// Mock-specific functions for testing
void arduino_mock_reset(void);
void arduino_mock_set_analog_value(uint8_t pin, uint16_t value);
uint16_t arduino_mock_get_analog_value(uint8_t pin);

#endif // NATIVE_TEST

#endif // ARDUINO_H
