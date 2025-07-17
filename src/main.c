/**
 * @file main.c
 * @brief Simple ATmega32A pin toggle test - no delays
 * @author Cannasol Technologies
 * @date 2025-07-16
 */

#include <avr/io.h>

int main(void) {
    // Set PB0 (Pin 1) as output
    DDRB |= (1 << PB0);
    
    // Simple infinite loop with basic toggle
    while (1) {
        // Turn LED on
        PORTB |= (1 << PB0);
        
        // Simple delay loop (much longer for 1MHz internal oscillator)
        for (volatile long i = 0; i < 1000000; i++);
        
        // Turn LED off
        PORTB &= ~(1 << PB0);
        
        // Simple delay loop
        for (volatile long i = 0; i < 1000000; i++);
    }
    
    return 0;
}
