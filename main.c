// new header
/** @file main.c
 ** @author Justin Brown
 ** @description cs-162 Final Project
 ** @date June 4th, 2013
**/

// old header
/**
 * @file wunder.c
 * @author Dan Albert
 * @author Marshal Horn
 * @date Created 12/15/2010
 * @date Last updated 1/20/2013
 * @version 1.0
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
 * This program will test the various components of the Wunderboard when used
 * with the corresponding host test script.
 *
 */

#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "adc.h"
#include "diskio.h"
#include "types.h"
#include "usart.h"
#include "leds.h"
#include "my_util.h"
#include "my_timer.h"

// convenience
#define X_INDEX 0
#define Y_INDEX 1

#define NONE 0
#define LEFT 7
#define UP 6
#define RIGHT 4
#define DOWN 5

// assumes there are more cars than trucks
#define NUM_TRUCKS 3
#define NUM_CARS 5

typedef struct gameEntity_struct
{
	uint8_t location[2];
	bool onBoard;
	uint8_t direction;
} gameEntity_t;

typedef struct denizens_struct
{
	gameEntity_t cars[NUM_CARS];
	gameEntity_t trucks[NUM_TRUCKS];
	gameEntity_t frogger;
} denizens_t;

void initialize(void);
uint8_t getRandomNumber(void);
void initiate_denizens(denizens_t *denizens);
bool checkCollide(denizens_t *denizens);
void moveTrucks(denizens_t *denizens);
void moveCars(denizens_t *denizens);
bool positionIsInsideBorders(uint8_t x, uint8_t y);
bool posInsideVertBorder(uint8_t y);
bool posInsideHorzBorder(uint8_t x);
void drawdenizens(denizens_t *denizens);
void createTruck(denizens_t *denizens);
void createCar(denizens_t *denizens);
uint8_t getInput(void);
void moveFrogger(denizens_t *denizens, uint8_t direction);
bool checkVictory(denizens_t *denizens);
void createInitialState(denizens_t *denizens);

void atobm(char a[8][8], unsigned char bm[8]);
void draw_bitmap(unsigned char *bm, int n);

typedef enum {START, DRAW, MOVE, CHECK_COLLISION, DEFEAT, VICTORY} mainState_t;
mainState_t g_mainState;
mainState_t g_lastState;
bool move_cars, move_trucks, move_frogger, create_vehicle;

//////////////////////////////////////////////////////////

void initialize(void)
{
	CPU_PRESCALE(0);

	USART_init(BAUD_RATE);
	USART_transmit('\f');	// Send form feed to clear the terminal.
	USART_send_string("WunderBoard initializing...\r\n");

	USART_send_string("\tSetting ADC prescaler and disabling free running mode...\r\n");
	setup_ADC(ADC_PRESCALER_32, FALSE);

	USART_send_string("\tEnabling ADC...\r\n");
	ADC_enable();

	USART_send_string("\tSetting ADC reference to Vcc...\r\n");
	ADC_set_reference(ADC_REF_VCC);

	// Configure IO //
	USART_send_string("\tConfiguring IO...\r\n");
	//DDRx corresponds to PORTx/PINx,
	//dependng on direction of data flow -- PORT for output, PIN for input
	DDRA = 0x00;	// Buttons and switches
	DDRB = 0xE7;	// Red enable, green enable and audio out
	DDRC = 0xff;	// Discrete LEDs
	DDRE = 0x47;	// LED Column
	DDRF = 0x00;	// Accelerometer

	// Disable pullups and set outputs low //
	PORTA = 0x00;
	PORTB = 0x01;
	PORTC = 0x81;
	PORTE = 0x00;
	PORTF = 0x00;

	//Set OC1A to toggle
	TCCR1A = 0b01000000;
	// Clk/64 and CTC mode
	TCCR1B = 0b00001011;

	OCR1A = 24;

	USART_send_string("\tSetting SPI\r\n");

	//Set the SPI bus appropriately to use the LED array
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);

}

// http://xkcd.com/221/
uint8_t getRandomNumber(void)
{
	return 4; // chosen by fair dice roll
			   // guaranteed to be random
}

void initiate_denizens(denizens_t *denizens)
{
	uint8_t i;
	denizens->frogger.onBoard = true;
	denizens->frogger.location[X_INDEX] = getRandomNumber();
	denizens->frogger.location[Y_INDEX] = (uint8_t)7;
	denizens->frogger.direction = NONE;
	for(i = 0; i < NUM_TRUCKS; ++i)
	{
		denizens->cars[i].onBoard = true;
        if(rand() % 2)
        {
            denizens->cars[i].direction = LEFT;
            denizens->cars[i].location[X_INDEX] = (uint8_t)7;
        }
        else
        {
            denizens->cars[i].direction = RIGHT;
            denizens->cars[i].location[X_INDEX] = (uint8_t)0;
        }
        denizens->cars[i].location[Y_INDEX] = (uint8_t)(rand() % 7);

        ///////******///////

		denizens->trucks[i].onBoard = true;
        if(rand() % 2)
        {
            denizens->trucks[i].direction = LEFT;
            denizens->trucks[i].location[X_INDEX] = (uint8_t)7;
        }
        else
        {
            denizens->trucks[i].direction = RIGHT;
            denizens->trucks[i].location[X_INDEX] = (uint8_t)0;
        }
        denizens->trucks[i].location[Y_INDEX] = (uint8_t)(rand() % 7);
	}
	for(; i < NUM_CARS; ++i)
	{
		denizens->cars[i].onBoard = true;
        if(rand() % 2)
        {
            denizens->cars[i].direction = LEFT;
            denizens->cars[i].location[X_INDEX] = (uint8_t)7;
        }
        else
        {
            denizens->cars[i].direction = RIGHT;
            denizens->cars[i].location[X_INDEX] = (uint8_t)0;
        }
        denizens->cars[i].location[Y_INDEX] = (uint8_t)(rand() % 7);
	}
}

bool checkCollide(denizens_t *denizens)
{
	uint8_t i;
	for(i = 0; i < NUM_TRUCKS; ++i)
	{
		if(denizens->trucks[i].onBoard)
		{
			if(denizens->frogger.location[Y_INDEX] == denizens->trucks[i].location[Y_INDEX])
			{
				if(denizens->trucks[i].direction == LEFT)
				{
					if(denizens->frogger.location[X_INDEX] == denizens->trucks[i].location[X_INDEX]   ||
					   denizens->frogger.location[X_INDEX] == denizens->trucks[i].location[X_INDEX]+2 ||
					   denizens->frogger.location[X_INDEX] == denizens->trucks[i].location[X_INDEX]+1)
						// check middle location of truck last, because
						// it is unlikely the player will run directly
						// into the middle of the truck, instead they are
						// more likely to touch the front or back of it
					{
							return true;
					}
				}
				else // direction == RIGHT
				{
					if(denizens->frogger.location[X_INDEX] == denizens->trucks[i].location[X_INDEX]   ||
					   denizens->frogger.location[X_INDEX] == denizens->trucks[i].location[X_INDEX]-2 ||
					   denizens->frogger.location[X_INDEX] == denizens->trucks[i].location[X_INDEX]-1)
					{
							return true;
					}
				}
			}
		}

		if(denizens->cars[i].onBoard)
		{
			if(denizens->frogger.location[Y_INDEX] == denizens->cars[i].location[Y_INDEX])
			{
				if(denizens->cars[i].direction == LEFT)
				{
					if(denizens->frogger.location[X_INDEX] == denizens->cars[i].location[X_INDEX] ||
					   denizens->frogger.location[X_INDEX] == denizens->cars[i].location[X_INDEX]+1)
					{
							return true;
					}
				}
				else // direction == RIGHT
				{
					if(denizens->frogger.location[X_INDEX] == denizens->cars[i].location[X_INDEX] ||
					   denizens->frogger.location[X_INDEX] == denizens->cars[i].location[X_INDEX]-1)
					{
							return true;
					}
				}
			}
		}
	}

	for(; i < NUM_CARS; ++i)
	{
		if(denizens->cars[i].onBoard)
		{
			if(denizens->frogger.location[Y_INDEX] == denizens->cars[i].location[Y_INDEX])
			{
				if(denizens->cars[i].direction == LEFT)
				{
					if(denizens->frogger.location[X_INDEX] == denizens->cars[i].location[X_INDEX] ||
					   denizens->frogger.location[X_INDEX] == denizens->cars[i].location[X_INDEX]+1)
					{
							return true;
					}
				}
				else // direction == RIGHT
				{
					if(denizens->frogger.location[X_INDEX] == denizens->cars[i].location[X_INDEX] ||
					   denizens->frogger.location[X_INDEX] == denizens->cars[i].location[X_INDEX]-1)
					{
							return true;
					}
				}
			}
		}
	}
	// missed me!
	return false;
}

void moveTrucks(denizens_t *denizens)
{
	uint8_t i;
	for(i = 0; i < 8; ++i)
	{
		if(denizens->trucks[i].onBoard)
		{
			if(denizens->trucks[i].direction == LEFT)
			{
				if(denizens->trucks[i].location[X_INDEX] < (uint8_t)254) // change to 0 - truckSize (0 - 2)
				{ // off the left side of the board
					denizens->trucks[i].onBoard = false;
					denizens->trucks[i].location[X_INDEX] = (uint8_t)128;
					denizens->trucks[i].location[Y_INDEX] = (uint8_t)128;
					denizens->trucks[i].direction = NONE;
				}
				else
				{ // move the entity
					--(denizens->trucks[i].location[X_INDEX]);
				}
			}
			else if(denizens->trucks[i].direction == RIGHT)
			{
				if(denizens->trucks[i].location[X_INDEX] > (uint8_t)9) // change to 7 + truckSize (7 + 2)
				{ // off the right side of the board
					denizens->trucks[i].onBoard = false;
					denizens->trucks[i].location[X_INDEX] = (uint8_t)128;
					denizens->trucks[i].location[Y_INDEX] = (uint8_t)128;
					denizens->trucks[i].direction = NONE;
				}
				else
				{ // move the entity
					++(denizens->trucks[i].location[X_INDEX]);
				}
			}
		}
	}
}

void moveCars(denizens_t *denizens)
{
	uint8_t i;
	for(i = 0; i < 8; ++i)
	{
		if(denizens->cars[i].onBoard)
		{
			if(denizens->cars[i].direction == LEFT)
			{
				if(denizens->cars[i].location[X_INDEX] < (uint8_t)255) // change to 0 - carSize (0 - 1)
				{ // off the left side of the board
					denizens->cars[i].onBoard = false;
					denizens->cars[i].location[X_INDEX] = (uint8_t)128;
					denizens->cars[i].location[Y_INDEX] = (uint8_t)128;
					denizens->cars[i].direction = NONE;
				}
				else
				{ // move the entity
					--(denizens->cars[i].location[X_INDEX]);
				}
			}
			else if(denizens->cars[i].direction == RIGHT)
			{
				if(denizens->cars[i].location[X_INDEX] > (uint8_t)8) // change to 7 + carSize (7 + 1)
				{ // off the right side of the board
					denizens->cars[i].onBoard = false;
					denizens->cars[i].location[X_INDEX] = (uint8_t)128;
					denizens->cars[i].location[Y_INDEX] = (uint8_t)128;
					denizens->cars[i].direction = NONE;
				}
				else
				{ // move the entity
					++(denizens->cars[i].location[X_INDEX]);
				}
			}
		}
	}
}

bool positionIsInsideBorders(uint8_t x, uint8_t y)
{
	return (x < (uint8_t)8 && y < (uint8_t)8) ? true : false;
}

bool posInsideVertBorder(uint8_t y)
{
    return (y < (uint8_t)8) ? true : false;
}

bool posInsideHorzBorder(uint8_t x)
{
    return (x < (uint8_t)8) ? true : false;
}

void drawdenizens(denizens_t *denizens)
{
	uint8_t i, x, y;

	//if(denizens->frogger.onBoard)
		led_green(denizens->frogger.location[X_INDEX], denizens->frogger.location[Y_INDEX]);
		update_row(0,0,0);

	for(i = 0; i < NUM_TRUCKS; ++i)
	{
		if(denizens->trucks[i].onBoard)
		{
			x = denizens->trucks[i].location[X_INDEX];
			y = denizens->trucks[i].location[Y_INDEX];

			if(positionIsInsideBorders(x, y))
			{
				led_red(x, y);
				update_row(0,0,0);
			}

			if(denizens->trucks[i].direction == LEFT)
			{
				if(positionIsInsideBorders(x+1, y))
				{
					led_red(x+1, y);
					update_row(0,0,0);
				}
				if(positionIsInsideBorders(x+2, y))
				{
					led_red(x+2, y);
					update_row(0,0,0);
				}
			}
			else
			{
				if(positionIsInsideBorders(x-1, y))
				{
					led_red(x-1, y);
					update_row(0,0,0);
				}
				if(positionIsInsideBorders(x-2, y))
				{
					led_red(x-2, y);
					update_row(0,0,0);
				}
			}
		}


		if(denizens->cars[i].onBoard)
		{
			x = denizens->cars[i].location[X_INDEX];
			y = denizens->cars[i].location[Y_INDEX];

			if(positionIsInsideBorders(x, y))
			{
				led_blue(x, y);
				update_row(0,0,0);
			}

			if(denizens->cars[i].direction == LEFT)
			{
				if(positionIsInsideBorders(x+1, y))
				{
					led_blue(x+1, y);
					update_row(0,0,0);
				}
			}
			else
			{
				if(positionIsInsideBorders(x-1, y))
				{
					led_blue(x-1, y);
					update_row(0,0,0);
				}
			}
		}
	}

	for(i = 0; i < NUM_CARS; ++i)
	{
		if(denizens->cars[i].onBoard)
		{
			x = denizens->cars[i].location[X_INDEX];
			y = denizens->cars[i].location[Y_INDEX];

			if(positionIsInsideBorders(x, y))
			{
				led_blue(x, y);
				update_row(0,0,0);
			}

			if(denizens->cars[i].direction == LEFT)
			{
				if(positionIsInsideBorders(x+1, y))
				{
					led_blue(x+1, y);
					update_row(0,0,0);
				}
			}
			else
			{
				if(positionIsInsideBorders(x-1, y))
				{
					led_blue(x-1, y);
					update_row(0,0,0);
				}
			}
		}
	}
}

void createTruck(denizens_t *denizens)
{
	uint8_t i;
	for(i = 0; i < NUM_TRUCKS; ++i)
	{
		if(!denizens->trucks[i].onBoard)
		{
			denizens->trucks[i].onBoard = true;
			if(rand() % 2)
			{
				denizens->trucks[i].direction = LEFT;
				denizens->trucks[i].location[X_INDEX] = 7;
			}
			else
			{
				denizens->trucks[i].direction = RIGHT;
				denizens->trucks[i].location[X_INDEX] = 0;
			}
			denizens->trucks[i].location[Y_INDEX] = (rand() % 7);
			return;
		}
	}
}

void createCar(denizens_t *denizens)
{
	uint8_t i;
	for(i = 0; i < NUM_TRUCKS; ++i)
	{
		if(!denizens->cars[i].onBoard)
		{
			denizens->cars[i].onBoard = true;
			if(rand() % 2)
			{
				denizens->cars[i].direction = LEFT;
				denizens->cars[i].location[X_INDEX] = 7;
			}
			else
			{
				denizens->cars[i].direction = RIGHT;
				denizens->cars[i].location[X_INDEX] = 0;
			}
			denizens->cars[i].location[Y_INDEX] = (rand() % 7);
			return;
		}
	}
}

uint8_t getInput(void)
{
    if(PINA == (1<<LEFT))
        return LEFT;
    if(PINA == (1<<RIGHT))
        return RIGHT;
    if(PINA == (1<<UP))
        return UP;
    if(PINA == (1<<DOWN))
        return DOWN;
    return NONE;
}

void moveFrogger(denizens_t *denizens, uint8_t direction)
{
	uint8_t x = denizens->frogger.location[X_INDEX];
	uint8_t y = denizens->frogger.location[Y_INDEX];
	switch(direction) // sometimes its plain english
	{
		case LEFT:
			if(positionIsInsideBorders(x-1, y))
				denizens->frogger.location[X_INDEX] = x-1;
			break;

		case RIGHT:
			if(positionIsInsideBorders(x+1, y))
				denizens->frogger.location[X_INDEX] = x+1;
			break;

		case UP:
			if(positionIsInsideBorders(x, y-1) || y == 0 /* escape! */ )
				denizens->frogger.location[Y_INDEX] = y-1;
			break;

		case DOWN:
			if(positionIsInsideBorders(x, y+1))
				denizens->frogger.location[Y_INDEX] = y+1;
			break;

		default:
			return;
	}
}

bool checkVictory(denizens_t *denizens)
{
	if(denizens->frogger.location[Y_INDEX] > 7)
		return true;
	return false;
}

void createInitialState(denizens_t *denizens)
{

}

ISR(TIMER0_COMPA_vect)
{
    cli();
    static uint8_t car_timer = (uint8_t)0;
	static uint8_t truck_timer = (uint8_t)0;
	static uint8_t frogger_timer = (uint8_t)0;
	static uint8_t create_timer = (uint8_t)0;
	g_lastState = g_mainState;
    TIFR0 |= (1<<OCF0A);
    ++car_timer;
	++truck_timer;
	++frogger_timer;
	++create_timer;

	if(frogger_timer >= 3)
	{
		move_frogger = true;
		frogger_timer = 0;
	}
	if(car_timer >= 9)
	{
		move_cars = true;
		car_timer = 0;
	}
	if(truck_timer >= 13)
	{
		move_trucks = true;
		truck_timer = 0;
	}
	if(create_timer >= 37)
	{
		create_vehicle = true;
		create_timer = 0;
	}

	if(frogger_timer == 0 || car_timer == 0 || truck_timer == 0)
	{
		g_mainState = MOVE;
	}
    sei();
}

void atobm(char a[8][8], unsigned char bm[8])
{//convert the 2d array to a useable bitmap
	int i, j;
	const unsigned char bitfactor[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

   	 // initialize bitmap
	for(i = 0; i < 8; i++)
		bm[i] = 0;

	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < 8; j++)
		{
			if(a[i][j] != '.')
			{
				bm[7-j] += bitfactor[i];
			}
		}
	}
}

void draw_bitmap(unsigned char *bm, int n)
{
    //draw the bitmap n times
    int i;
    for(i = 0; i < n; ++i)
    {
        for(PORTE = 0; PORTE < 8; PORTE++)
        {
            // lights on
            set_array_red(bm[PORTE]);
            // lights off
            set_array_red(0);
            // next column
        }
    }
}

void draw_blue_smile(int n)
{ // draw blue smile n times
	int i;
    	for(i = 0; i < n; ++i)
	{
		if(i % 3 == 0)
			continue;

		PORTE = 1;
		set_array_blue(0b00001000);
		set_array_blue(0b00000000);
		PORTE++;
		set_array_blue(0b01100100);
		set_array_blue(0b00000000);
		PORTE++;
		set_array_blue(0b00000100);
		set_array_blue(0b00000000);
		PORTE++;
		set_array_blue(0b00000100);
		set_array_blue(0b00000000);
		PORTE++;
		set_array_blue(0b01100100);
		set_array_blue(0b00000000);
		PORTE++;
		set_array_blue(0b00001000);
		set_array_blue(0b00000000);
	}
}

int main(void)
{
	initialize();
	initialize_TIMER0();
	g_mainState = DRAW;
	g_lastState = g_mainState;
	uint8_t direction = NONE, i;
	move_cars = move_trucks = move_frogger = create_vehicle = false;
	bool input_sent = false;
	bool ready_to_move = false;

    char shape_cross[8][8] =       {{'x','.','.','.','.','.','.','x'},

                                    {'.','x','.','.','.','.','x','.'},

                                    {'.','.','x','.','.','x','.','.'},

                                    {'.','.','.','x','x','.','.','.'},

                                    {'.','.','.','x','x','.','.','.'},

                                    {'.','.','x','.','.','x','.','.'},

                                    {'.','x','.','.','.','.','x','.'},

                                    {'x','.','.','.','.','.','.','x'}};

    unsigned char bm_cross[8];
	atobm(shape_cross, bm_cross);

	PORTE = 0;

	denizens_t denizens;
	initiate_denizens(&denizens);


	while(!PINA)
	{ // delay until keypress
		;
	}

	// in hindsight I should probably avoid rand() altogether
	// and just use randNum = TCNT0 % N;
	srand(TCNT0);
	sei();

	/************** Main Game Loop **************/
    while(1)
	{
		switch(g_mainState)
		{
			case START:
				createInitialState(&denizens);
				g_mainState = DRAW;
                break;
			case DRAW:
				drawdenizens(&denizens);
				if(PINA)
				{
				    input_sent = true;
				    direction = getInput();
				}
				if(!PINA && input_sent)
				{
				    ready_to_move = true;
				}
                //cli();
				break;

			case MOVE:
				if(create_vehicle)
				{
					if(rand() % 2)
						createTruck(&denizens);
					else
						createCar(&denizens);
				}
				if(move_trucks)
				{
					moveTrucks(&denizens);
					move_trucks = false;
				}
				if(move_cars)
				{
					moveCars(&denizens);
					move_cars = false;
				}
				if(move_frogger && ready_to_move)
				{
					moveFrogger(&denizens, direction);
					move_frogger = false;
				    ready_to_move = false; // these booleans make sure you do not
				    input_sent = false; // move more than once per click
				}
				g_mainState = CHECK_COLLISION;
				break;

			case CHECK_COLLISION:
				if(checkCollide(&denizens))
				{
				    cli();
				    for(i = 0; i < 5; ++i)
				    {
                        draw_bitmap(bm_cross, 1111);
                        _delay_ms(1300);
                        return 0;
				    }
				}
				else if(checkVictory(&denizens))
				{
				    cli();
					for(i = 0; i < 5; ++i)
					{
					    draw_blue_smile(1111);
					    _delay_ms(1300);
					    return 0;
					}
				}
				else
				{
					g_mainState = DRAW;
				}
				break;

			default:
				; // pass
		}
	}
	cli();
	return 0;
}
