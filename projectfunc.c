/* projectfunc.c (formerly mipslabfunc.c)
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   This file modified 2022-03-03 by Viggo Hermansson
   This file modified 2022-03-03 by Silvia Lü

   For copyright and licensing, see file COPYING */

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

void display_image(const uint8_t *data, int x) { // Edited by Viggo
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
// (S)
// - Graphics functions -
// clears buffer, all pixels low
void clear_buffer(uint8_t* buffer, unsigned int buffer_length){
	int i;
	for(i = 0; i<buffer_length;i++){
		*buffer = 255;
		buffer++;
	}
}

// set chosen pixel, using (x, y) coordinates (origo is lower left corner)
void set_pixel(uint8_t* buffer, unsigned int x, unsigned int y){
	x = (((31 - y) / 8) * 128 ) + x; // byte
	y = (31-y)%8; // bit

	buffer[x] &= ~(1 << y); // set bit index to 0 --> it's set
}

// clears chosen pixel, sets it to "high" == black
void clr_pixel(uint8_t* buffer, unsigned int x, unsigned int y){
	x = (((31 - y) / 8) * 128 ) + x;
	y = (31-y)%8;

	buffer[x] |= (1 << y); // set bit ind to 1 --> black
}

// uses set pixel to set a block of 2x2 pixels, standard size for this game
// starts in lower left corner of "block"
void set_block(uint8_t* buffer, unsigned int x, unsigned int y){
	unsigned int x_coord = (x*2)+BLOCK_OFFSET;
	unsigned int y_coord = (y*2)+BLOCK_OFFSET;

	int xi, yi;
	for(yi=0; yi<2; yi++){
		for(xi=0; xi<2; xi++){
			set_pixel(buffer, (xi+x_coord), (yi+y_coord));
		}
	}
}

// clrs chosen block
void clr_block(uint8_t* buffer, unsigned int x, unsigned int y){
	unsigned int x_coord = (x*2)+BLOCK_OFFSET;
	unsigned int y_coord = (y*2)+BLOCK_OFFSET;

	int xi, yi;
	for(yi=0; yi<2; yi++){
		for(xi=0; xi<2; xi++){
			clr_pixel(buffer, (xi+x_coord), (yi+y_coord));
		}
	}
}


// (V)
void score_to_string(uint16_t score, char* target_string) {

	uint8_t shift = 0;

	char a = score / 100;
	char b = (score - 100*a) / 10;
	char c = score - 100*a - 10*b;

	a += 48; // 0's ascii code is 48
	b += 48;
	c += 48;

	if (a == '0') { // if apples eaten <100
		a = ' ';
		shift++; // removes the zero
		if (b == '0') { // if apples eaten <10
			b = ' ';
			shift++;
		}
	}

	target_string[9] = a;
	target_string[10 - shift] = b; // if shift, then overwrites the previous
	target_string[11 - shift] = c;
}

// - IO functions - 
void ioinit(void) {
	
	TRISECLR = 0x00FF; // Set LEDs to output (bits 7:0)
	TRISFSET = 0x0002; // Set BTN 1 to input (bit 1)
	TRISDSET = 0x0FE0; // Set BTNs 2-4 to input (bit 11:5)

}

int getbtns(void) {

	/*
		Function prototype: int getbtns(void);
		Parameter: none.
		Return value: The 4 least significant bits of the return value must contain current data from push buttons BTN4, BTN3, BTN2, BTN1. BTN1 corresponds to the least significant bit. All other bits of the return value must be zero.
		Notes: The function getbtns will never be called before Port D has been correctly initialized. The buttons BTN4, BTN3, and BTN2, are connected to bits 7, 6 and 5 of Port D.

				Pin 	Port
		BTN1: 	4		RF1
		BTN2: 	34 		RD5
		BTN3:	36 		RD6
		BTN4:	37 		RD7

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
// (S)
void rand_pos(){
    // modified use of Fibonacci LFSRs and bitwise and (wikipedia)
    countr = (countr>>0)^(countr>>2)^(countr>>5)&0xa55a;

    apple_x = countr%45;
    apple_y = countr%13;
}

// (S)
// from lab 3
void exception_setup(){

    /* tmr2: 
            Flag    IFS(0) <8>
            Enable  IEC(0) <8>
            P       IPC(2) <4:2>
            Sp      IPC(2) <1:0>
    */
    
    IEC(0) = (1 << 8); // enable int 
    IPC(2) = 0x1f; // highest priority
    
    enable_interrupt(); // call ei 
}


// - timer funcs -
// (S)
// from lab 3
void timer_init(){
    TMR2 = 0x0; // clr current tmr val

    PR2 = 3125; // (0,01 s is one clk per) 800 000*256 = 3 125 (80MHz)
    T2CONSET = 0x8070; // bit 15 starts counter, bits 6-4 sets prescale (111 -->> 256:1)
}


// func to call every time apple is eaten
// (S)
void get_apple(void){
    /* ~~ when an apple has been consumed ~~ */
    /* - generate new apple pos, cannot collide w/ anything else - */
    rand_pos();
    while(is_occupied(apple_x, apple_y)){
        rand_pos();
    }
    /* - increase apple count - */
    apple_count++;

    /* - raise flag, to increase length - */
    get_longer=1;

    /* - increase speed on interval - 
            - after three apples (then two, and lastly one)
            - stop at max speed (speed_var = 1)
			- anything % 1 is always 0
    */
    if ((speed_var>1)  &&  ((apple_count%apples_until_speedup)==0)){
        speed_var--;
		if(apples_until_speedup>1){
			apples_until_speedup--;
		}
    }

    set_block(gamebuffer, apple_x, apple_y);
}

// - Game functions -
// (V)
// arg: next coord (start is (5, 5))
// removes last block if not eaten
void snake_move(uint8_t snake_x, uint8_t snake_y) {

	// the val (x, y coord) to write to snake_array
	uint16_t write_value = 0;

	// points to the head of the snake (the x, y coord of the head) 
	// first iter = 1
	// wraps to beginning if reached end
	snake_start++;
	if (snake_start > 704) {
		snake_start = 0;
	}

	// write the arguments (new pos) into nxt array element
	// set a block in these coord
	snake_coordinates[snake_start] = write_value | (snake_x << 8) | snake_y;
	set_block(gamebuffer, snake_x, snake_y);

	// if snake should get longer
	if (get_longer) { 
		get_longer = 0; // clr var
	}

	// if no apple eaten
	// x coord is byte index
	// y coord is bit index
	// 16 bit: 8 msb: x coord, 8 lsbits: y coord
	// clear the last element
	else { 
		uint16_t read_value = snake_coordinates[snake_end]; // first iter, snake_end = 0
		uint8_t clear_x = read_value >> 8;
		uint8_t clear_y = read_value & 0x00FF;
		clr_block(gamebuffer, clear_x, clear_y);
		snake_end++;
		if (snake_end > 704) {
			snake_end = 0;
		}
	}
}


// (V)
uint8_t is_occupied(uint8_t target_x, uint8_t target_y) {

	const unsigned int actual_x = target_x * 2 + BLOCK_OFFSET;
	const unsigned int actual_y = target_y * 2 + BLOCK_OFFSET;

	const int byte_index = (((31 - actual_y) / 8) * 128 ) + actual_x; // the byte in the buffer where the pixel resides
	const int bit_index  = (31 - actual_y) % 8; // the index of the bit within the byte where the pixel is represented

	if (gamebuffer[byte_index] & (0x01 << bit_index)) {
		return 0;
	}
	else {
		return 1;
	}
}


// - Interrupt function -
// (S)
// if game is on: init snake length (once per gameplay)
// switch direction, check if lost (out of bounds, collision w itself)
// check if apple eaten, call get_apple()
void user_isr(){
    // IFS(0), bit 8, if flag is set
    if (IFS(0)&0x100) {
    	// clr flag
        IFS(0) &= ~0x100;
	    if (gameplay){
			// handles speedup
	        tmr_countr++;
	        if ((tmr_countr == speed_var)){
	            tmr_countr = 0;

				// init snake length
				if(snake_start_length>1){
					get_longer=1;
					snake_start_length--;
				}
				
				// change direction, if left, down - gameover
				// (V) and (S)
				switch(direction){
					case 0:
						if(head_x>45){
							setleds(1);
							gameplay=0;
						}
						else{
							head_x++;	
						}
						break;
					case 1:
						if(head_y>13){
							setleds(2);
							gameplay=0;
						}
						else{
							head_y++;
						}
						break;
					case 2:
						if(head_y<1){
							setleds(4);
							gameplay=0;
						}
						else{
							head_y--;
						}
						break;
					case 3:
						if(head_x<1){
							setleds(8);
							gameplay=0;
						}
						else{
							head_x--;
						}
						break;
				}

				// allow for direction change
				allow_direction=1;

				// if snake collides with itself or apple
				if(is_occupied(head_x, head_y)) {
					if(head_x==apple_x && head_y==apple_y){
						get_apple();
					}
					else{
						gameplay=0;
					}
				}

				// frame update w new values
				snake_move(head_x, head_y);
				display_image(gamebuffer, 0);
			}
	    }
	}
}