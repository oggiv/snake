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
/* time val int */
extern unsigned int time;
/* apple coord */
extern unsigned int apple_x;
extern unsigned int apple_y;

void display_image(const uint8_t *data, int x);
void display_init(void);
void display_string(int line, char *s);
void display_update(void);
uint8_t spi_send_recv(uint8_t data);
void quicksleep(int cyc);



/* ~~~ OUR STUFF BELOW THIS LINE ~~~ */
/* Pixel graphics buffer */
extern uint8_t gamebuffer[516];

/* Variables and functions for the snake */
extern uint8_t direction;
extern uint16_t snake_coordinates[705];
extern uint16_t snake_start;
extern uint16_t snake_end;
extern uint8_t apple_eaten;
extern void snake_move(uint8_t snake_x, uint8_t snake_y);

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

/* rand pos generator for apple*/
void rand_int();

/* Control interrupts */
void enable_interrupt(void);
void exception_setup();

/* Timer setup */
void timer_init();
unsigned int get_time();
