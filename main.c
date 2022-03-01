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
	
	// -- init -- 
	// start timer, enable interrupt
	timer_init();
    exception_setup();


	// init io, leds
	ioinit();
	setleds(0);
	clearBuffer(gamebuffer, 516);

	// game border
	int i;
	for (i = 0; i < 96; i++) {
		setPixel(gamebuffer, i, 31);
		setPixel(gamebuffer, i, 0);
	}
	for (i = 0; i < 32; i++) {
		setPixel(gamebuffer, 0, i);
		setPixel(gamebuffer, 95, i);
	}

	// show the borders
	display_image(gamebuffer, 0);

	// init snake length
	uint8_t snake_start_length = 4;

	// wait for player to start game
	// int led_value = 0;
	/* while(getbtns()==0){
		led_value = led_value%0xff;
		se tleds(led_value++);
		quicksleep(60000);
	}*/ 

	gameplay = 1; // start game

	snake_move(head_x, head_y);
    display_image(gamebuffer, 0);
	

	// -- game play -- 
	while (1) {

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
				if ((direction != 3) && (pressed & 0x1)) {
					pressed &= ~0x1;
					direction = 0;
				}
			}

			if (btns & 0x2) {
				if ((direction != 2) && (pressed & 0x2)) {
					pressed &= ~0x2;
					direction = 1;
				}
			}

			if (btns & 0x4) {
				if ((direction != 1) && (pressed & 0x4)) {
					pressed &= ~0x4;
					direction = 2;
				}
			}

			if (btns & 0x8) {
				if ((direction != 0) && (pressed & 0x8)) {
					pressed &= ~0x8;
					direction = 3;
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

		// snake_move(head_x, head_y);
		// display_image(gamebuffer, 0);

		// quicksleep(600 000);
	}
}
