/*	project.h
	Contains global functions and variables.
	*/

#include <stdint.h>

/* Declare bitmap array containing font */
extern const uint8_t const font[128*8];
/* Declare bitmap array containing icon */
extern const uint8_t const icon[256];
/* Declare text buffer for display output */
extern char textbuffer[4][16];

void display_image(const uint8_t *data, int x);
void display_init(void);
void display_string(int line, char *s);
void display_update(void);
uint8_t spi_send_recv(uint8_t data);
void quicksleep(int cyc);



/* ~~~ OUR STUFF BELOW THIS LINE ~~~ */
/* Pixel graphics buffer */
extern uint8_t gamebuffer[516];

/* Functions to draw and erase pixels into our gamebuffer */
void setPixel(uint8_t *target_array, const unsigned int x, const unsigned int y);
void clearPixel(uint8_t *target_array, const unsigned int x, const unsigned int y);
void setBlock(uint8_t *target_array, const unsigned int x, const unsigned int y);
void clearBlock(uint8_t *target_array, const unsigned int x, const unsigned int y);

/* Read from an interact with IO harware */
void ioinit(void);
int getbtns(void);
void setleds(uint8_t led_value);
extern uint8_t led_count;

/* Control interrupts */
void enableInterrupt(void);