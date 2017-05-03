/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <alloc.h>
#include "graph3.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

static unsigned char far *double_buffer = NULL;
static unsigned int buffer_height = SCREEN_HEIGHT;
static unsigned int buffer_size = SCREEN_WIDTH * SCREEN_HEIGHT / 2;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

int Create_Double_Buffer(int num_lines)
{
	/* Allocate enough memory to hold the double buffer. */
	if ((double_buffer = (unsigned char far*)
		farmalloc(SCREEN_WIDTH * (num_lines + 1))) == NULL) {
		return 0;
	}

	/* Set the height of the buffer and compute its size. */
	buffer_height = num_lines;
	buffer_size = SCREEN_WIDTH * num_lines / 2;

	/* Fill the buffer with black. */
	memset(double_buffer, 0, SCREEN_WIDTH * num_lines);

	/* Everything was OK. */
	return 1;
}

void Fill_Double_Buffer(int color)
{
	/* This function fills in the double buffer with the sent color. */
	memset(double_buffer, color, SCREEN_WIDTH * buffer_height);
}

/* lag: this originally took a pointer to the buffer */
void Show_Double_Buffer(void)
{
	/* This function copies the double buffer into the video buffer. */
	asm push ds;                    /* save ds on stack */
	asm mov cx, buffer_size;		/* this is the size in WORDs */
	asm les di, video_buffer;		/* es:di is the destination */
	asm lds si, double_buffer;		/* es:si is the source */
	asm cld;						/* make sure to move forward */
	asm rep movsw;					/* move all the WORDs */
	asm pop ds;						/* restore ds */
}

void Delete_Double_Buffer(void)
{
	/* This function frees up the memory allocated by the double buffer.
	Be sure to use FAR version. */
	if (double_buffer) {
		farfree(double_buffer);
		double_buffer = NULL;		/* lag: cleanup! */
	}
}

void Plot_Pixel_Fast_DB(int x, int y, unsigned char color)
{
	/* Plots the pixel in the desired color a little quicker using binary
	shifting to accomplish the multiplications. */

	/* Use the fact that 320*y = 256*y + 64*y = y<<8 + y<<6 */
	double_buffer[((y << 8) + (y << 6)) + x] = color;
}
