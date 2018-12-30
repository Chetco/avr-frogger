/** @file timer.h
 ** @author Justin Brown
 ** @description cs-162 Final Project
 ** @date June 4th, 2013
**/

#include "my_timer.h"

/** This function needs to setup the variables used by TIMER0 Compare Match (CTC) mode
with a base clock frequency of clk/1024. This function should return a 1 if it fails and
a 0 if it does not. Remember, by default the Wunderboard runs at 8MHz for its system
clock. @return This function returns a 1 is unsuccessful, else return 0.*/
unsigned char initialize_TIMER0()
{
	//See chapter 13 in the data sheet, and look up each of these values in turn.

	//enable iterrupts
	TIMSK0 &= ~(1<<OCIE0A);
	TIMSK0 |= (1<<OCIE0A);

	/* Set the CTC mode */
	TCCR0A &= ~(1<<WGM00);
	TCCR0A |=  (1<<WGM01);
	TCCR0B &= ~(1<<WGM02);

	/* Set the Clock Frequency */
	TCCR0B |=  (1<<CS00);
	TCCR0B &= ~(1<<CS01);
	TCCR0B |=  (1<<CS02);

  /* Set initial count value */
	OCR0A = 0xff;

	return 0;
}

/** This function checks if TIMER0 has elapsed.
@return This function should return a 1 if the timer has elapsed, else return 0*/
unsigned char check_TIMER0()
{
  //look at the TIFR0 register in the datasheet.
  if(TIFR0 & (1<<OCF0A))
  {
    TIFR0 |= (1<<OCF0A);
    return 1;
  }
  else
    return 0;
}

/** This function takes two values, clock and count. The value of count should be copied
into OCR0A and the value of clock should be used to set CS02:0 in the TCCR0B register.
The TCNT0 variable should also be reset to 0 so that the new timer rate starts from 0.
@param [in] clock Insert Comment @param [in] count Insert Comment @return The function
returns a 1 or error and 0 on successful completion.*/
unsigned char set_TIMER0(unsigned char clock, unsigned char count)
{
  OCR0A = count;
  TCCR0B &= ~((1<<CS00) | (1<<CS01) | (1<<CS02));
  TCCR0B |= clock;
  TCNT0 = 0;
  return 0;
}
