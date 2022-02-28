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

	ioinit();
	setleds(0);
	clearBuffer(gamebuffer, 516);

	int i;
	for (i = 0; i < 96; i++) {
		setPixel(gamebuffer, i, 31);
		setPixel(gamebuffer, i, 0);
	}
	for (i = 0; i < 32; i++) {
		setPixel(gamebuffer, 0, i);
		setPixel(gamebuffer, 95, i);
	}

	display_image(gamebuffer, 0);

	uint8_t head_x = 5;
	uint8_t head_y = 5;

	uint8_t snake_start_length = 4;

	while (1) {

		if (snake_start_length > 0) {
			get_longer = 1;
			snake_start_length--;
		}
		get_longer = 1;

		int btns = getbtns();
		static int pressed = 0xF;
		if (btns != 0) {
			if (btns & 0x1) {
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

		snake_move(head_x, head_y);
		display_image(gamebuffer, 0);

		quicksleep(600000);

		if ((head_x < 47) && (head_x >= 0)) {
			if ((head_y < 15) && (head_y >= 0)) {
				switch (direction) {
					case 0:
						head_x++;
						break;
					case 1:
						head_y++;
						break;
					case 2:
						head_y--;
						break;
					case 3:
						head_x--;
						break;
				}
			}
		}
	}
}
