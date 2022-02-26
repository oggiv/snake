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

	unsigned int x_pos = 0;
	unsigned int y_pos = 0;

	while (1) {

		setBlock(gamebuffer, x_pos, y_pos);
		display_image(gamebuffer, 0);

		quicksleep(800000);

		clearBlock(gamebuffer, x_pos, y_pos);

		if (++x_pos > 46) {
			x_pos = 0;
			if (++y_pos > 14) {
				y_pos = 0;
			}
		}
	}
}
