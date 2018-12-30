/** 
 * @file
 * @author Matthew Shuman
 * @author Joey Tomlinson
 * @author Dan Albert
 * @date Created 8/5/09
 * @date Last updated 7/19/10
 * @version 1.1
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * @section DESCRIPTION
 * 
 * Includes functions for setting up and reading the hardware ADC on the attiny26.
 * 
 * @section HARDWARE
 * Target Device: ATTINY26L
 * 
 */
#define __AVR_AT90USB1287__

#include <avr/io.h>

#include "adc.h"
#include "pins.h"

void ADC_set_channel(uint8_t channel)
{
	CLEARPINS(ADMUX, ADC_MUX_MASK);
	SETPINS(ADMUX, (channel & ADC_MUX_MASK));
}

void ADC_set_adjust(BOOL left_adjust)
{
	if (left_adjust == 1){
		SETPIN(ADMUX, ADLAR);
	}
	else{
		CLEARPIN(ADMUX, ADLAR);	// See pins.h
	}
}

void ADC_set_prescaler(uint8_t prescale)
{
	CLEARPINS(ADCSRA, ADC_PRESCALER_MASK);
	SETPINS(ADCSRA, (prescale & ADC_PRESCALER_MASK));
}

void ADC_set_free_running(BOOL free_run)
{	
	if (free_run){
		// Set the auto trigger to free running mdoe
		CLEARPIN(ADCSRB, ADTS2);
		CLEARPIN(ADCSRB, ADTS1);
		CLEARPIN(ADCSRB, ADTS0);
		
		// Enable auto trigger
		SETPIN(ADCSRA, ADATE);
	}
	else{
		// Disable auto trigger mode
		CLEARPIN(ADCSRA, ADATE);
	}
}

void ADC_set_reference(uint8_t reference)
{
	CLEARPINS(ADMUX, ADC_REF_MASK);
	SETPINS(ADMUX, (reference & ADC_REF_MASK));
	
	// First read after changing reference is probably garbage
	ADC_start();
	// Wait for the AD conversion to complete
	while (!ADC_done());

	ADC_finish();
}

void ADC_enable_interrupt()
{
	SETPIN(ADCSRA, ADIE);
}

void ADC_disable_interrupt(void)
{
	CLEARPIN(ADCSRA, ADIE);
}

void ADC_enable()
{
	SETPIN(ADCSRA, ADEN);
}

void ADC_disable()
{
	CLEARPIN(ADCSRA, ADEN);
}

void ADC_start()
{
	SETPIN(ADCSRA, ADSC);
}

void ADC_finish()
{
	SETPIN(ADCSRA, ADIF);
}

BOOL ADC_done()
{
	return BOOL(GETPIN(ADCSRA, ADIF));
}

void setup_ADC(uint8_t prescale, BOOL free_run)
{
	ADC_set_prescaler(prescale);	// Set prescaler bits
	ADC_set_free_running(free_run);		// Set free run bit
}

uint8_t read_ADC(uint8_t channel)
{	
	uint8_t temp, temp2;
	
	ADC_set_channel(channel);	// Set ADC channel select bits
	ADC_set_adjust(ADC_RIGHTADJUST);	// Set left/right adjust bit
	
	ADC_start();
	// Wait for the AD conversion to complete
	while (!ADC_done());
	ADC_finish();
	
	temp = ADCL;
	temp2 = ADCH;
	return temp;
}

uint8_t read_ADC_averaged(uint8_t channel, uint8_t samples)
{
	uint8_t i;
	uint16_t accumulator = 0;
	
	for(i = 0; i < samples; i++){
		accumulator += read_ADC(channel);
	}
	
	return accumulator / samples;
}

uint16_t read_precise_ADC(uint8_t channel)
{	
	ADC_set_channel(channel);	// Set ADC channel select bits
	ADC_set_adjust(ADC_RIGHTADJUST);	// Set left/right adjust bit
	
	ADC_start();
	// Wait for the AD conversion to complete
	while (!ADC_done());
	ADC_finish();
	
	return ADCW;
}

uint16_t read_precise_ADC_averaged(uint8_t channel, uint8_t samples)
{
	uint8_t i;
	uint16_t total = 0;
	uint16_t value;
	
	for (i = 0; i < samples; i++){
		total += read_precise_ADC(channel);
	}
	
	value = total / samples;
	
	return value;
}
