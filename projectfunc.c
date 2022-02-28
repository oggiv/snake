/*  projectfunc.c
	Contains project functions for interacting with the hardware.
	*/

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "project.h"

#define BLOCK_OFFSET 1 /* change to compensate for grid */

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

/* quicksleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void quicksleep(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}

/* Display stuff from here */
uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
    DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);
	
	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);
	
	spi_send_recv(0x8D);
	spi_send_recv(0x14);
	
	spi_send_recv(0xD9);
	spi_send_recv(0xF1);
	
	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);
	
	spi_send_recv(0xA1);
	spi_send_recv(0xC8);
	
	spi_send_recv(0xDA);
	spi_send_recv(0x20);
	
	spi_send_recv(0xAF);
}

void display_image(const uint8_t *data, int x) {
	int i, j;
	
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 128; j++)
			spi_send_recv(~data[i*128 + j]);
	}
}

void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;
	
	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}

void display_update(void) {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(0x0);
		spi_send_recv(0x10);
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;
			
			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}



/* ~~~ OUR STUFF BELOW THIS LINE ~~~ */
char readArray(const uint8_t *target_array, const int index) {

	unsigned int byte_index = index / 8;
	unsigned int bit_index = index % 8;

	return (target_array[byte_index] >> bit_index) & 0x01;
}

void writeArray(uint8_t *target_array, const int index, const int value) {

	unsigned int byte_index = index / 8;
	unsigned int bit_index = index % 8;

	target_array[byte_index] &= ~(0x01 << bit_index);
	target_array[byte_index] |= value << bit_index;
}

// - Graphics functions -
// Deactivates all pixels in given buffer
void clearBuffer(uint8_t *target_buffer, unsigned int buffer_length) {

	int bi;
	for (bi = 0; bi < buffer_length; bi++) {
		target_buffer[bi] = 255;
	}

}

// Activates pixel in given buffer. x is between 0 and 127. y is between 0 and 31.
void setPixel(uint8_t *target_array, const unsigned int x, const unsigned int y) {

	int byte_index = (((31 - y) / 8) * 128 ) + x; // the byte in the buffer where the pixel resides
	int bit_index  = (31 - y) % 8; // the index of the bit within the byte where the pixel is represented

	target_array[byte_index] &= ~(0x01 << bit_index);

}

// Deactivates pixel in given buffer. x is between 0 and 127. y is between 0 and 31.
void clearPixel(uint8_t *target_array, const unsigned int x, const unsigned int y) {

	int byte_index = (((31 - y) / 8) * 128 ) + x; // the byte in the buffer where the pixel resides
	int bit_index  = (31 - y) % 8; // the index of the bit within the byte where the pixel is represented

	target_array[byte_index] |= 0x01 << bit_index;
}

void setBlock(uint8_t *target_array, const unsigned int x, const unsigned int y) {

	const unsigned int actual_x = x * 2;
	const unsigned int actual_y = y * 2;

	int ny, nx;
	for (ny = 0; ny < 2; ny++) {
		for (nx = 0; nx < 2; nx++) {
			setPixel(target_array, (nx + actual_x + 1), (ny + actual_y + 1));
		}
	}
}

void clearBlock(uint8_t *target_array, const unsigned int x, const unsigned int y) {

	const unsigned int actual_x = x * 2;
	const unsigned int actual_y = y * 2;

	int ny, nx;
	for (ny = 0; ny < 2; ny++) {
		for (nx = 0; nx < 2; nx++) {
			clearPixel(target_array, (nx + actual_x + BLOCK_OFFSET), (ny + actual_y + BLOCK_OFFSET));
		}
	}
}


// - IO functions - 
void ioinit(void) {
	
	TRISECLR = 0x00FF; // Set LEDs to output
	TRISFSET = 0x0002; // Set BTN 1 to input
	TRISDSET = 0x0FE0; // Set BTNs 2-4 to input

}

int getbtns(void) {

	/*
		Function prototype: int getbtns(void);
		Parameter: none.
		Return value: The 4 least significant bits of the return value must contain current data from push buttons BTN4, BTN3, BTN2, BTN1. BTN1 corresponds to the least significant bit. All other bits of the return value must be zero.
		Notes: The function getbtns will never be called before Port D has been correctly initialized. The buttons BTN4, BTN3, and BTN2, are connected to bits 7, 6 and 5 of Port D.

				Pin 	Port
		BTN1: 4		RF1
		BTN2: 34 	RD5
		BTN3:	36 	RD6
		BTN4:	37 	RD7

	 	0000 1110 = 0x0E
	 	0000 0001 = 0x01
	*/

	return ((PORTD >> 4) & 0x0E) | ((PORTF >> 1) & 0x01);
}

void setleds(uint8_t led_value) {
	/* Sets LEDs to represent the binary value of the given 'led_value' */
	PORTE = (PORTE & 0xFFFFFF00) | led_value;
}

// random position generator (saves x, y pos of the apple)
// (Silvia)
void randint(unsigned int* apple_x, unsigned int* apple_y){
    // int var to increment. 
    static unsigned int countr = 0;
	
	while(getbtns()==0){ 
		countr = countr%0xffff; // increase countr but no overflow
		countr++;
	}
	// rand int seq of shifted bitw xor, and
	// use of Fibonacci's LFSRs
	unsigned int rand_pos = (countr>>0)^(countr>>2)^(countr>>5)&0xa55a;
	
	// assign position of apple
	*apple_x = rand_pos%94;
	*apple_y = rand_pos%30;
}


// - Interrupt functions -
void userisr(void) {
	return;
}

// Silvia
void exception_setup(){

    /* tmr2: 
            Flag    IFS(0) <8>
            Enable  IEC(0) <8>
            P       IPC(2) <4:2>
            Sp      IPC(2) <1:0>
    */
    
    IEC(0) = 1 << 8; // enable int
    IPC(2) = 0x1f; // highest priority
    
    enable_interrupt(); // call ei 
}


// - timer funcs -
// Silvia
void timer_init(){
    TMR2 = 0x0; // clr current tmr val
    TMR3 = 0x0; // clr current tmr val

    // 32 bit TMR mode (bit 3 in TxCON - tmr ctrl)
    T2CONSET = 0x8;
    PR2 = 0x3d0900; // 4 000 000 times (0,05 s is one clk per)
    T2CONSET = 0x8000; // bit 15 starts counter, bits 7-5 sets prescale (111 -->> 256:1)
}

// Silvia
// get current clk value
unsigned int get_time(){
    unsigned int time = TMR2;
    return time;
}

// - Game functions -
void snake_move(uint8_t snake_x, uint8_t snake_y) {

	uint16_t write_value = 0;

	snake_start++;
	if (snake_start > 704) {
		snake_start = 0;
	}
	snake_coordinates[snake_start] = write_value | (snake_x << 8) | snake_y;
	setBlock(gamebuffer, snake_x, snake_y);

	if (apple_eaten) {
		apple_eaten = 0;
	}
	else {
		uint16_t read_value = snake_coordinates[snake_end];
		uint8_t clear_x = read_value >> 8;
		uint8_t clear_y = read_value & 0x00FF;
		clearBlock(gamebuffer, clear_x, clear_y);
		snake_end++;
		if (snake_end > 704) {
			snake_end = 0;
		}
	}

}
