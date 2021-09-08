/*
 * TETRIS_ATMEGA32.c
 *
 * Created: 2/26/2021 9:43:58 PM
 * Author : Jonathan
 */ 

#define F_CPU 16000000

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

void inverseGrid();
void setGrid(bool);
void initialize();
void controllerListener();

void generatePiece();
void rotateIfValid();
bool validBelow();
bool validRight();
bool validLeft();
bool validSpawn();
void moveBelow();
void moveLeft();
void moveRight();
void removePiece();
void setPiece();
void clearCheck();
void gameOver();

volatile uint8_t disp_counter; // Determines which column of LEDs to activate
bool grid[20][10] = {0}; // Initialize all pixels as "off"

const uint8_t L_SHAPE[4][2] = {{4, 1}, {3, 1}, {5, 1}, {5, 0}};
const uint8_t J_SHAPE[4][2] = {{5, 1}, {6, 1}, {4, 1}, {4, 0}};
const uint8_t S_SHAPE[4][2] = {{4, 1}, {3, 1}, {4, 0}, {5, 0}};
const uint8_t Z_SHAPE[4][2] = {{5, 1}, {6, 1}, {5, 0}, {4, 1}};
const uint8_t T_SHAPE[4][2] = {{4, 1}, {3, 1}, {5, 1}, {4, 0}};
const uint8_t O_SHAPE[4][2] = {{4, 1}, {5, 1}, {4, 0}, {5, 0}};
const uint8_t I_SHAPE[4][2] = {{4, 0}, {3, 0}, {5, 0}, {6, 0}};

uint8_t current_shape[4][2] = {0};
bool shape_placed = false;

bool left_pressed = false;
bool right_pressed = false;
bool down_pressed = false;
bool up_pressed = false;

int main(void)
{
	srand(543);
	initialize();
	
    while (1) 
    {
		 if (shape_placed)
		 {
			 if (validBelow())
			 {
				 // Move down
				 moveBelow();
			 }
			 else
			 {
				 // Piece Stays Here
				 setPiece();
				 shape_placed = false;
				 
				 clearCheck();
			 }
		 }
		 else // Place the new block
		 {
			 shape_placed = true;
			 generatePiece();
			 if (validSpawn())
			 {
				 setPiece();
			 }
			 else // Game over!
			 {
				 setPiece();
				 gameOver();
			 }
		 }
		 
		 if (down_pressed)
		 {
			 _delay_ms(15);
			 continue;
		 }
		_delay_ms(250);
    }
}

void generatePiece()
{
	
	int chosenIndex = rand() % 7;
	
	switch(chosenIndex) {
		case 0: memcpy(current_shape, L_SHAPE, sizeof(current_shape)); break;
		case 1: memcpy(current_shape, J_SHAPE, sizeof(current_shape)); break;
		case 2: memcpy(current_shape, S_SHAPE, sizeof(current_shape)); break;
		case 3: memcpy(current_shape, Z_SHAPE, sizeof(current_shape)); break;
		case 4: memcpy(current_shape, T_SHAPE, sizeof(current_shape)); break;
		case 5: memcpy(current_shape, O_SHAPE, sizeof(current_shape)); break;
		case 6: memcpy(current_shape, I_SHAPE, sizeof(current_shape)); break;
	}
}

void rotateIfValid()
{
	uint8_t relative_shape[4][2];
	// Compute Relative Coords w.r.t Reference Tile
	for (int i = 0; i < 4; i++)
	{
		relative_shape[i][0] = current_shape[i][0] - current_shape[0][0];
		relative_shape[i][1] = current_shape[i][1] - current_shape[0][1];
	}
	
	// Rotate Coords
	for (int i = 1; i < 4; i++)
	{
		int temp = relative_shape[i][0];
		relative_shape[i][0] = -relative_shape[i][1];
		relative_shape[i][1] = temp;
	}
	
	// Compute Global Coords w.r.t Reference Tile
	for (int i = 0; i < 4; i++)
	{
		relative_shape[i][0] = relative_shape[i][0] + current_shape[0][0];
		relative_shape[i][1] = relative_shape[i][1] + current_shape[0][1];
	}

	/******************
	NEEDS BETTER IMPLEMENTATION
	******************/

	// Check if rotation causes shape to go out of canvas
	bool invalid = (relative_shape[0][0] < 0) | (relative_shape[0][0] > 9) | (relative_shape[1][0] < 0) | (relative_shape[1][0] > 9) | (relative_shape[2][0] < 0) | (relative_shape[2][0] > 9) | (relative_shape[3][0] < 0) | (relative_shape[3][0] > 9);
	
	// Check if rotation lands on an existing block
	removePiece();
	invalid |= grid[relative_shape[0][1]][relative_shape[0][0]] | grid[relative_shape[1][1]][relative_shape[1][0]] | grid[relative_shape[2][1]][relative_shape[2][0]] | grid[relative_shape[3][1]][relative_shape[3][0]];

	if (!invalid && shape_placed)
	{
		memcpy(current_shape, relative_shape, sizeof(current_shape));
	}
	setPiece();	
}

bool validSpawn()
{
	// Check if theres a block in spawn
	bool invalid = grid[current_shape[0][1]][current_shape[0][0]] | grid[current_shape[1][1]][current_shape[1][0]] | grid[current_shape[2][1]][current_shape[2][0]] | grid[current_shape[3][1]][current_shape[3][0]];
	
	return !invalid;
}

void removePiece()
{
	// Remove current piece
	grid[current_shape[0][1]][current_shape[0][0]] = false;
	grid[current_shape[1][1]][current_shape[1][0]] = false;
	grid[current_shape[2][1]][current_shape[2][0]] = false;
	grid[current_shape[3][1]][current_shape[3][0]] = false;
}

bool validBelow()
{
	removePiece();
	
	// Check if hit bottom of canvas
	if (current_shape[0][1] > 18 || current_shape[1][1] > 18 || current_shape[2][1] > 18 || current_shape[3][1] > 18)
	{
		return false;
	}
	// Check if theres a block below
	bool invalid = grid[current_shape[0][1]+1][current_shape[0][0]] | grid[current_shape[1][1]+1][current_shape[1][0]] | grid[current_shape[2][1]+1][current_shape[2][0]] | grid[current_shape[3][1]+1][current_shape[3][0]];

	return !invalid;
}

bool validRight()
{
	removePiece();
	
	// Check if hit right of canvas
	if (current_shape[0][0] == 9 || current_shape[1][0] == 9 || current_shape[2][0] == 9 || current_shape[3][0] == 9)
	{
		return false;
	}
	// Check if theres a block on the right
	bool invalid = grid[current_shape[0][1]][current_shape[0][0]+1] | grid[current_shape[1][1]][current_shape[1][0]+1] | grid[current_shape[2][1]][current_shape[2][0]+1] | grid[current_shape[3][1]][current_shape[3][0]+1];

	return !invalid;
}

bool validLeft()
{
	removePiece();
	
	// Check if hit left of canvas
	if (current_shape[0][0] == 0 || current_shape[1][0] == 0 || current_shape[2][0] == 0 || current_shape[3][0] == 0)
	{
		return false;
	}
	// Check if theres a block on the left
	bool invalid = grid[current_shape[0][1]][current_shape[0][0]-1] | grid[current_shape[1][1]][current_shape[1][0]-1] | grid[current_shape[2][1]][current_shape[2][0]-1] | grid[current_shape[3][1]][current_shape[3][0]-1];

	return !invalid;
}

void moveBelow()
{
	current_shape[0][1] += 1;
	current_shape[1][1] += 1;
	current_shape[2][1] += 1;
	current_shape[3][1] += 1;
	
	setPiece();
}

void moveRight()
{
	current_shape[0][0] += 1;
	current_shape[1][0] += 1;
	current_shape[2][0] += 1;
	current_shape[3][0] += 1;
	
	setPiece();
}

void moveLeft()
{
	current_shape[0][0] -= 1;
	current_shape[1][0] -= 1;
	current_shape[2][0] -= 1;
	current_shape[3][0] -= 1;
	
	setPiece();
}

void setPiece()
{
	grid[current_shape[0][1]][current_shape[0][0]] = true;
	grid[current_shape[1][1]][current_shape[1][0]] = true;
	grid[current_shape[2][1]][current_shape[2][0]] = true;
	grid[current_shape[3][1]][current_shape[3][0]] = true;
}

void  clearCheck()
{
	for (uint8_t y = 0; y < 20; y++)
	{
		uint8_t col_count = 0;
		for (uint8_t x = 0; x < 10; x++)
		{
			if (grid[19-y][x])
			{
				col_count += 1;
			}
		}
		if (col_count == 10)
		{			
			for (uint8_t i = y--; i < 20; i++)
			{
				for (uint8_t x = 0; x < 10; x++)
				{
					grid[19-i][x] = grid[19-i-1][x];
				}
			}
			for (uint8_t x = 0; x < 10; x++)
			{
				grid[0][x] = 0;
			}
		}
	}
}

void gameOver()
{
	inverseGrid();
	_delay_ms(500);
	inverseGrid();
	_delay_ms(500);
	inverseGrid();
	_delay_ms(500);
	inverseGrid();
	_delay_ms(500);
	setGrid(false);
	
	shape_placed = false;
	
	_delay_ms(500);
}

void inverseGrid()
{
	for (uint8_t y = 0; y < 20; y++)
	{
		for (uint8_t x = 0; x < 10; x++)
		{
			grid[y][x] = !grid[y][x];
		}
	}
}

void setGrid(bool mode)
{
	for (uint8_t y = 0; y < 20; y++)
	{
		for (uint8_t x = 0; x < 10; x++)
		{
			grid[y][x] = mode;
		}
	}
}

void initialize()
{
	cli();
	
	// Setup Pins
	DDRA = 0xFF; // All of PORTA are outputs
	PORTA = 0xFF; // Initiate all PORTA as HIGH (PNP transistors/off)
	DDRB = 0xFF; // All of PORTB are outputs
	PORTB = 0x00; // Initiate all of PORTB as LOW (NPN transistors//off)
	DDRC = 0xFF; // All of PORTC are outputs
	PORTC = 0xFF; // Initiate all PORTA as HIGH (PNP transistors/off)
	DDRD = 0xF0; // PD0-3 as Inputs (controller), PD4-7 as Outputs
	PORTD = 0xFF; // Enable internal pull-up resistor (PD0-3), initiate outputs as HIGH (PD4-7)(PNP transistors/off)
	
	// Setup Timer Interrupts
	OCR0 = 255; // Compare tick trigger on this count
	TIMSK = (1 << OCIE0); // Enable timer interrupts
	TCCR0 = (1 << WGM01) | (0 << CS02) | (1 << CS01) | (1 << CS00); // Set CTC Bit | Pre-scaler to 1024

	

	// Variable Initialization
	disp_counter = 0;
	
	PORTB |= (1 << PB0);
	_delay_ms(1000);
	
	sei();
}

void controllerListener()
{
	
	uint8_t input = PIND;
	
	if (!(input & 0x01)) { // PD0 Pressed (Left)
		if (!left_pressed)
		{
			left_pressed = true;
			
			if (validLeft())
			{
				moveLeft();
			} else
			{
				setPiece();
			}
		}
	}
	else
	{
		left_pressed = false;
	}
	
	if (!(input & 0x02)) { // PD1 Pressed (Right)
		if (!right_pressed)
		{
			right_pressed = true;
			
			if (validRight())
			{
				moveRight();
			} else
			{
				setPiece();
			}

		}
	}
	else
	{
		right_pressed =  false;
	}
	
	if (!(input & 0x04)) { // PD2 Pressed (Down)
		if (!down_pressed)
		{
			down_pressed = true;
		}
	}
	else
	{
		down_pressed = false;
	}
	
	if (!(input & 0x08)) { // PD3 Pressed (Up)
		if (!up_pressed)
		{
			up_pressed = true;
			
			rotateIfValid();
		}
	}
	else
	{
		up_pressed = false;
	}
}

ISR(TIMER0_COMP_vect)
{
	controllerListener();
	
	// Reset Transistors
	PORTA = 0xFF;
	PORTC = 0xFF;
	PORTD = 0xFF;
	
	PORTB = 0x01; // Turn off all columns of LEDs except for PWRLED
	
	// ROW SWITCHING
	for (int i = 0; i < 8; i++)
	{
		PORTA &= ~(grid[i][disp_counter] << i); // Convert boolean array to PORTA switching in binary
	}
	for (int i = 8; i < 16; i++)
	{
		PORTC &= ~(grid[i][disp_counter] << (15-i)); // Convert boolean array to PORTC switching in binary
	}
	for (int i = 16; i < 20; i++)
	{
		PORTD &= ~(grid[i][disp_counter] << (23-i)); // Convert boolean array to PORTD switching in binary
	}

	// COLUMN SWITCHING	
	switch(disp_counter)
	{
		case 0: PORTB |= 0x0C; // First Column...
				break;
		case 1: PORTB |= 0x14; // Second Column etc...
				break;
		case 2: PORTB |= 0x24;
				break;
		case 3: PORTB |= 0x44;
				break;
		case 4: PORTB |= 0x84;
				break;
		case 5: PORTB |= 0x82;
				break;
		case 6: PORTB |= 0x42;
				break;
		case 7: PORTB |= 0x22;
				break;
		case 8: PORTB |= 0x12;
				break;
		case 9: PORTB |= 0x0A;
				break;
	}
	disp_counter++;
	if (disp_counter == 10) // Restart display counter
	{
		disp_counter = 0;
	}
	
}