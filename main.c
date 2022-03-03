/* main.c (formerly mipslabmain.c)

   This file written 2015 by Axel Isaksson,
   Modified 2015, 2017 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   Modified 2022-03-03 by Viggo Hermansson
   Modified 2022-03-03 by Silvia Lü

   For copyright and licensing, see file COPYING */

#include <pic32mx.h>
#include "project.h"

int main() {
	/*
	  This will set the peripheral bus clock to the same frequency
	  as the sysclock. That means 80 MHz, when the microcontroller
	  is running at 80 MHz. Changed 2017, as recommended by Axel.
	*/
	SYSKEY = 0xAA996655;  /* Unlock OSCCON, step 1 */
	SYSKEY = 0x556699AA;  /* Unlock OSCCON, step 2 */
	while(OSCCON & (1 << 21)); /* Wait until PBDIV ready */
	OSCCONCLR = 0x180000; /* clear PBDIV bit <0,1> */
	while(OSCCON & (1 << 21));  /* Wait until PBDIV ready */
	SYSKEY = 0x0;  /* Lock OSCCON */
	
	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;
	
	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;
	
	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);
	
	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;
	/* SPI2STAT bit SPIROV = 0; */
	SPI2STATCLR = 0x40;
	/* SPI2CON bit CKP = 1; */
    SPI2CONSET = 0x40;
	/* SPI2CON bit MSTEN = 1; */
	SPI2CONSET = 0x20;
	/* SPI2CON bit ON = 1; */
	SPI2CONSET = 0x8000;
	
	display_init();
	

	/* ~~~ OUR STUFF BELOW THIS LINE ~~~ */

	// All (V)
	
	// -- init -- 
	// start timer, enable interrupt
	// (S)
	timer_init();
    exception_setup();


	// init io, leds
	ioinit();
	setleds(0);

	while (1) {

		direction = 0;
		snake_start = 0;
		snake_end = 0;
		get_longer = 0;
		head_x = 5;
		head_y = 5;

		// (S)
		time = 0;
		tmr_countr = 0;
		apple_count = 0;
		apples_until_speedup = 3;
		speed_var = 20;
		allow_direction = 1;

		clear_buffer(gamebuffer, 516);

		// show the borders
		display_image(gamebuffer, 0);

		// init snake length
		uint8_t snake_start_length = 1;

		display_string(1, "     SNAKE");
		display_string(2, "  Press button");
		display_update();

		// (S)
		while (getbtns() != 0);
		while(getbtns()==0){ 
			countr = countr%0xffffffff; // increase countr but no overflow
			countr++;
	    }

	    // game border
		int i;
		for (i = 0; i < 96; i++) {
			set_pixel(gamebuffer, i, 31);
			set_pixel(gamebuffer, i, 0);
		}
		for (i = 0; i < 32; i++) {
			set_pixel(gamebuffer, 0, i);
			set_pixel(gamebuffer, 95, i);
		}

		gameplay = 1; // start game

		get_apple();

		snake_move(head_x, head_y);
	    display_image(gamebuffer, 0);
		
		// -- game play -- 
		while (gameplay) {
			// turns dot into snake - init code 
			if (snake_start_length > 0) {
				get_longer = 1;
				snake_start_length--;
			}

			// 'toggle' var for btn
			int btns = getbtns();
			static int pressed = 0xF;
			// if any is pressd
			if (btns != 0) {
				// which btn pressed
				if (btns & 0x1) {
					// snake cannot double back on itself
					if ((direction != 3) && (pressed & 0x1) && allow_direction) {
						pressed &= ~0x1;
						direction = 0;
						// (S)
						allow_direction = 0;
					}
				}
				if (btns & 0x2) {
					if ((direction != 2) && (pressed & 0x2) && allow_direction) {
						pressed &= ~0x2;
						direction = 1;
						allow_direction = 0;
					}
				}
				if (btns & 0x4) {
					if ((direction != 1) && (pressed & 0x4) && allow_direction) {
						pressed &= ~0x4;
						direction = 2;
						allow_direction = 0;
					}
				}
				if (btns & 0x8) {
					if ((direction != 0) && (pressed & 0x8) && allow_direction) {
						pressed &= ~0x8;
						direction = 3;
						allow_direction = 0;
					}
				}
			}

			// clr 'toggle', untoggle
			if ((~pressed & 0x1) && (~btns & 0x1)) {
				pressed = (pressed & ~0x1) | 0x1;
			}
			if ((~pressed & 0x2) && (~btns & 0x2)) {
				pressed = (pressed & ~0x2) | 0x2;
			}
			if ((~pressed & 0x4) && (~btns & 0x4)) {
				pressed = (pressed & ~0x4) | 0x4;
			}
			if ((~pressed & 0x8) && (~btns & 0x8)) {
				pressed = (pressed & ~0x8) | 0x8;
			}
		}
		
		char str[] = "   Score:       ";
		score_to_string(apple_count - 1, str);
		display_string(1, "   GAME OVER");
		display_string(2, str);
		display_update();

		quicksleep(50000);

		while (getbtns() != 0);
		while (getbtns() == 0);
	}
}
