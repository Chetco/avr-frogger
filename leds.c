#include "leds.h"

/*

Strobing the LED latch:

The LED latch is bit 7 of PORTB. Strobing it means that you are turning the bit on, then
turning the bit off.

*/

//write 0x00,0xFF,0x00 to SPI
//check SPSR register, see datasheet for bit
//Strobe LED Latch
void set_array_green(unsigned char row)
{
	SPDR = 0;
	while(!(SPSR & 	(1<<SPIF))); // wait for red
	SPDR = row;
	while(!(SPSR & (1<<SPIF))); // wait for green
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for blue

	PORTB |= (1<<7);
	_delay_us(1);
	PORTB &= ~(1<<7);
}

void set_array_blue(unsigned char row)
{
	SPDR = 0;
	while(!(SPSR & 	(1<<SPIF))); // wait for red
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for green
	SPDR = row;
	while(!(SPSR & (1<<SPIF))); // wait for blue

	PORTB |= (1<<7);
	_delay_us(1);
	PORTB &= ~(1<<7);
}

void set_array_red(unsigned char row)
{
	SPDR = row;
	while(!(SPSR & 	(1<<SPIF))); // wait for red
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for green
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for blue

	PORTB |= (1<<7);
	_delay_us(1);
	PORTB &= ~(1<<7);
}

void clear_array()
{
	update_row(0, 0, 0);
}

void update_row(uint8_t red, uint8_t green, uint8_t blue)
{
	SPDR = red;
	while(!(SPSR & 	(1<<SPIF))); // wait for red
	SPDR = green;
	while(!(SPSR & (1<<SPIF))); // wait for green
	SPDR = blue;
	while(!(SPSR & (1<<SPIF))); // wait for blue

	PORTB |= (1<<7);
	_delay_us(1);
	PORTB &= ~(1<<7);
}

void led_red(uint8_t y, uint8_t x)
{
	/* Let's index this from the top left, like a normal screen.
	 * However, the LED array is indexed in hardware from the bottom right,
	 * with the bit position controlling y, and PORTE controlling x.
	 */
	//uint8_t temp = PORTE;
	PORTE = (7 - (y & 0x07)) | (PORTE & 0xF8);
	SPDR = (1<<7) >> (x & 0x07);
	while(!(SPSR & (1<<SPIF))); // wait for red
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for green
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for blue

	PORTB |= (1<<7);
	_delay_us(1);
	PORTB &= ~(1<<7);

	//PORTE = temp;


	//to reverse a column, you can use (1 << 7) >> (column & 0x07)


	//to reverse the row, you can use (7 - (row & 0x07)) | (PORTE & 0xF8)
}

void led_green(uint8_t y, uint8_t x)
{
	//uint8_t temp = PORTE;
	PORTE = (7 - (y & 0x07)) | (PORTE & 0xF8);
	//y = (7 - (row & 0x07)) | (PORTE & 0xF8);
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for red
	SPDR = (1<<7) >> (x & 0x07);
	while(!(SPSR & (1<<SPIF))); // wait for green
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for blue

	PORTB |= (1<<7);
	_delay_us(1);
	PORTB &= ~(1<<7);

	//PORTE = temp;
}

void led_blue(uint8_t y, uint8_t x)
{
	//uint8_t temp = PORTE;
	PORTE = (7 - (y & 0x07)) | (PORTE & 0xF8);
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for red
	SPDR = 0;
	while(!(SPSR & (1<<SPIF))); // wait for green
	SPDR = (1<<7) >> (x & 0x07);
	while(!(SPSR & (1<<SPIF))); // wait for blue

	PORTB |= (1<<7);
	_delay_us(1);
	PORTB &= ~(1<<7);

	//PORTE = temp;
}

void led_off(void)
{
	update_row(0, 0, 0);
}
