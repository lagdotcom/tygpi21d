/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include "graph3.h"
#include "graph4.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NUM_WORMS		320

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct worm_typ {
	int y;			/* current y position of worm */
	int color;		/* color of worm */
	int speed;		/* speed of worm */
	int counter;	/* counter */
} worm, *worm_ptr;

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture screen_fx;		/* out test screen */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Fade_Lights(void)
{
	/* This function fades the lights by slowly decreasing the color values
	in all color registers. */
	int pal_reg, index;
	RGB_color color;

	for (index = 0; index < 30; index++) {
		for (pal_reg = 1; pal_reg <= 255; pal_reg++) {
			/* Get the color to fade. */
			Get_Palette_Register(pal_reg, &color);

			if (color.red > 5) color.red -= 3;
			else color.red = 0;

			if (color.green > 5) color.green -= 3;
			else color.green = 0;

			if (color.blue > 5) color.blue -= 3;
			else color.blue = 0;

			/* Set the color to a diminished intensity. */
			Set_Palette_Register(pal_reg, &color);
		}

		Delay(2);
	}
}

void Dissolve(void)
{
	/* Dissolve the screen by plotting zillions of black pixels. */
	unsigned long index;

	for (index = 0; index <= 300000; index++) {
		Plot_Pixel_Fast(rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, 0);
	}
}

void Melt(void)
{
	/* This function 'melts' the screen by moving little worms at different
	speeds down the screen. These worms change to the color they're eating */
	int index, ticks = 0;
	worm worms[NUM_WORMS];

	/* Initialize the worms. */
	for (index = 0; index < 160; index++) {
		worms[index].color   = Get_Pixel(index, 0);
		worms[index].speed   = 3 + rand() % 9;
		worms[index].y       = 0;
		worms[index].counter = 0;

		/* Draw the worm. */
		Plot_Pixel_Fast((index << 1),     0, worms[index].color);
		Plot_Pixel_Fast((index << 1),     1, worms[index].color);
		Plot_Pixel_Fast((index << 1),     2, worms[index].color);

		Plot_Pixel_Fast((index << 1) + 1, 0, worms[index].color);
		Plot_Pixel_Fast((index << 1) + 1, 1, worms[index].color);
		Plot_Pixel_Fast((index << 1) + 1, 2, worms[index].color);
	}

	/* Do the screen melt. */
	while (++ticks < 1800) {
		/* Process each worm. */
		for (index = 0; index < 320; index++) {
			/* Is it time to move the worm? */
			if (++worms[index].counter >= worms[index].speed) {
				/* Reset the counter. */
				worms[index].counter = 0;
				worms[index].color = Get_Pixel(index, worms[index].y + 4);

				/* Has the worm hit bottom? */
				if (worms[index].y < 193) {
					Plot_Pixel_Fast((index << 1),     worms[index].y, 0);
					Plot_Pixel_Fast((index << 1),     worms[index].y + 1,
						worms[index].color);
					Plot_Pixel_Fast((index << 1),     worms[index].y + 2,
						worms[index].color);
					Plot_Pixel_Fast((index << 1),     worms[index].y + 3,
						worms[index].color);

					Plot_Pixel_Fast((index << 1) + 1, worms[index].y, 0);
					Plot_Pixel_Fast((index << 1) + 1, worms[index].y + 1,
						worms[index].color);
					Plot_Pixel_Fast((index << 1) + 1, worms[index].y + 2,
						worms[index].color);
					Plot_Pixel_Fast((index << 1) + 1, worms[index].y + 3,
						worms[index].color);

					worms[index].y++;
				}
			}
		}

		/* Accelerate the melt. */
		if (!(ticks % 500)) {
			for (index = 0; index < 160; index++) {
				worms[index].speed--;
			}
		}
	}
}

void Shear(void)
{
	/* This program 'shears' the screen (for lack of a better word). */
	long index;
	int x, y;

	/* Select the starting point of the shear. */
	x = rand() % SCREEN_WIDTH;
	y = rand() % SCREEN_HEIGHT;

	/* Do it a few times to make sure the whole screen is destroyed. */
	for (index = 0; index < 100000; index++) {
		/* Move the shear. */
		x += 17;		/* note the user of prime numbers */
		y += 13;

		/* Test whether shears are of boundaries. If so, roll them over. */
		if (x > 319) x = x - 319;
		if (y > 199) y = y - 199;

		/* Plot the pixel in black. */
		Plot_Pixel_Fast(x, y, 0);
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0,				/* exit flag */
		index = 0;				/* current position in instruction string */

	static char instructions[256] = "";

	char buffer[41];

	/* Build up the instruction string. */
	strcat(instructions, "................................................");
	strcat(instructions, "Press 1 to fade the lights, ");
	strcat(instructions, "Press 2 to dissolve the screen, ");
	strcat(instructions, "Press 3 to melt the screen, ");
	strcat(instructions, "Press 4 to shear the screen.");
	strcat(instructions, "................................................");

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Load in a background picture. */
	PCX_Init(&screen_fx);
	PCX_Load("screenfx.pcx", &screen_fx, 1);
	PCX_Show_Buffer(&screen_fx);
	PCX_Delete(&screen_fx);

	/* Main event loop. */
	while (!done) {
		/* Wait for a keyboard press. */
		if (kbhit()) {
			/* Which special effects did the user want to see? */
			switch (getch()) {
				case '1':
					/* Dim the lights. */
					Fade_Lights();
					break;
				case '2':
					/* Dissolve the screen. */
					Dissolve();
					break;
				case '3':
					/* Melt the screen. */
					Melt();
					break;
				case '4':
					/* Shear the screen. */
					Shear();
					break;

				default: break;
			}

			done = 1;
		}

		/* Extract the substring to be displayed. */
		memcpy(buffer, &instructions[index], 40);

		/* Put a NULL terminator at the end. */
		buffer[40] = 0;

		Blit_String(0, 23 * 8 + 6, 15, buffer, 0);

		/* Increment the instruction index. Roll over if at end of
		instructions. */
		if (++index > 180) index = 0;

		/* Assume the user can read at only 1,000,000 words a second... */
		Delay(2);
	}

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);
}
