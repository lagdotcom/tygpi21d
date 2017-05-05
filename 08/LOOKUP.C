/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <math.h>
#include <stdlib.h>
#include "gamelib.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NUM_CIRCLES		500
#define FULL_CIRCLE		360

/* G L O B A L S ///////////////////////////////////////////////////////// */

float sin_table[360], cos_table[360];

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int index, x, y, xo, yo, radius, ang;

	/* Create look-up tables. */
	for (index = 0; index < FULL_CIRCLE; index++) {
		sin_table[index] = sin(index * M_PI / 180);
		cos_table[index] = cos(index * M_PI / 180);
	}

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	printf("\nHit any key to draw circles with");
	printf("internal sine and cosine.");
	getch();

	/* Draw circles using the built-in sine and cosine. */
	for (index = 0; index < NUM_CIRCLES; index++) {
		/* Get a random circle. */
		radius = rand() % 50;
		xo     = rand() % SCREEN_WIDTH;
		yo     = rand() % SCREEN_HEIGHT;

		for (ang = 0; ang < FULL_CIRCLE; ang++) {
			x = xo + cos(ang * M_PI / 180) * radius;
			y = yo + sin(ang * M_PI / 180) * radius;

			/* Plot the point of the circle with a little image space
			clipping. */
			if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
				Plot_Pixel_Fast(x, y, 9);
			}
		}
	}

	/* Done. Halt the system and wait for the user to press a key. */
	printf("\nHit any key to see circles drawn with look-up.");
	getch();

	/* Clear the screen. */
	Set_Video_Mode(VGA256);

	/* Draw circles using look-up tables. */
	for (index = 0; index < NUM_CIRCLES; index++) {
		/* Get a random circle. */
		radius = rand() % 50;
		xo     = rand() % SCREEN_WIDTH;
		yo     = rand() % SCREEN_HEIGHT;

		for (ang = 0; ang < FULL_CIRCLE; ang++) {
			x = xo + cos_table[ang] * radius;
			y = yo + sin_table[ang] * radius;

			/* Plot the point of the circle with a little image space
			clipping. */
			if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
				Plot_Pixel_Fast(x, y, 12);
			}
		}
	}

	/* Let the user press a key to exit. */
	printf("\nWow! Hit any key to exit.");
	getch();

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);
}
