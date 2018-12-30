/** @file timer.c
 ** @author Justin Brown
 ** @description cs-162 Final Project
 ** @date June 4th, 2013
**/

#ifndef TIMER_H
#define TIMER_H

#include <avr/interrupt.h>

//clock frequency should be 8000000

// 9600 baud
#define BAUD_RATE 51
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define CK1024 (1<<CS02) | (0<<CS01) | (1<<CS00)

unsigned char initialize_TIMER0();
unsigned char check_TIMER0();
unsigned char set_TIMER0(unsigned char clock, unsigned char count);

#endif // TIMER_H
