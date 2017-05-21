/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include "graph3.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SCROLL_WIDTH  (unsigned int)640
#define SCROLL_HEIGHT (unsigned int)100

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* Pointer to the scrolling buffer. */
unsigned char far *scroll_buffer = NULL;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Show_View_Port(char far *buffer, int pos)
{
	/* Copy a portion of the scrolling buffer into the video buffer. */
	unsigned int y, scroll_off, screen_off;

	/* There are 100 rows that must be moved. Move the data row by row. */
	for (y = 0; y < SCROLL_HEIGHT; y++) {
		/* Compute the starting offset into scroll buffer.
		y * 640 + pos */
		scroll_off = (y << 9) + (y << 7) + pos;

		/* Compute the starting offset in video RAM.
		y * 320 + 80 */
		screen_off = ((y + 50) << 8) + ((y + 50) << 6) + 80;

		/* Move the data. */
		memmove(&video_buffer[screen_off], &buffer[scroll_off], 160);
	}
}

void Plot_Pixel_Scroll(int x, int y, unsigned char color)
{
	/* This function plots pixels into the scroll buffer with our new
	virtual screen size of 640x100. */

	/* Use the fact that 640*y = 512*y + 128*y = y<<9 + y<<7. */
	scroll_buffer[(y << 9) + (y << 7) + x] = color;
}

void Draw_Terrain(void)
{
	/* This function draws the terrain into the scroll buffer. */
	int x, y = 70, y1, index;

	/* Clear out memory first. */
	memset(scroll_buffer, 0, SCROLL_WIDTH * SCROLL_HEIGHT);

	/* Draw a few stars. */
	for (index = 0; index < 200; index++) {
		Plot_Pixel_Scroll(rand() % SCROLL_WIDTH, rand() % 70, 15);
	}

	/* Draw some mountains. */
	for (x = 0; x < SCROLL_WIDTH; x++) {
		/* Compute the offset. */
		y += -1 + rand() % 3;

		/* Make sure the terrain stays within a reasonable boundary. */
		if (y > 90) y = 90;
		else if (y < 40) y = 40;

		/* Plot the dot in the scroll buffer. */
		Plot_Pixel_Scroll(x, y, 2);

		for (y1 = y + 1; y1 < 100; y1++) {
			Plot_Pixel_Scroll(x, y1, 10);
		}
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0,
		sx = 0;		/* Scrolling view port position */

	char buffer[100];

	/* Allocate memory for the scroll buffer. */
	scroll_buffer = malloc(SCROLL_WIDTH * SCROLL_HEIGHT);
	if (!scroll_buffer) {
		printf("Could not allocate scroll buffer.");
		return;
	}

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Put up some information. */
	Blit_String(0, 0, 15, "Use < > to move. Press Q to quit.", 0);

	/* Draw the mountains. */
	Draw_Terrain();

	/* Show the initial view. */
	Show_View_Port(scroll_buffer, sx);

	/* Main loop. */
	while (!done) {
		/* Has the user pressed a key? */
		if (kbhit()) {
			switch (getch()) {
				case ',':
					/* Move the window to the left, but not too far. */
					sx -= 2;
					if (sx < 0) sx = 0;
					break;

				case '.':
					/* Move the window to the right, but not too far. */
					sx += 2;
					if (sx > SCROLL_WIDTH - 160) sx = SCROLL_WIDTH - 160;
					break;

				case 'q':
					/* Is the user trying to bail? */
					done = 1;
					break;
			}

			/* Copy the view port to the screen. */
			Show_View_Port(scroll_buffer, sx);

			sprintf(buffer, "Viewport position = %3d", sx);
			Blit_String(0, 12, 15, buffer, 0);
		}
	}

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);
}
