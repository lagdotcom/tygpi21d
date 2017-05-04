/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <math.h>
#include <stdlib.h>
#include "graph3.h"
#include "graph4.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TANK_SPEED      4

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	long index;                     /* loop index */
	sprite tank1,                   /* player sprite */
		   tank2;                   /* enemy sprite */
	pcx_picture background_pcx,     /* background imagery */
				objects_pcx;        /* foreground imagery */

	/* direction; also current frame. 0 is north */
	int tank1_direction = 0,
		tank2_direction = 0,
		done = 0;                   /* exit flag */

	float dx, dy, angle;            /* motion vars */

/* S E C T I O N  1 ////////////////////////////////////////////////////// */

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Load in the background. */
	PCX_Init(&background_pcx);
	PCX_Load("outpost.pcx", &background_pcx, 1);
	PCX_Show_Buffer(&background_pcx);

	/* Put up the title. */
	Blit_String(90, 2, 7, "A T T A N K ! ! !", 1);
	PCX_Delete(&background_pcx);

	/* Load the PCX file with the tank cells. */
	/* Load in the player's imagery. */
	PCX_Init(&objects_pcx);
	PCX_Load("tanks.pcx", &objects_pcx, 0);

/* S E C T I O N  2 ////////////////////////////////////////////////////// */

	/* Place tank1 (the player) at the bottom of the screen. */
	Sprite_Init(&tank1, 160, 150, 0, 0, 0, 0, 16, 16);

	/* Place tank2 (the enemy) at the top of the screen. */
	Sprite_Init(&tank2, 160, 50, 0, 0, 0, 0, 16, 16);

	/* Grab all 16 images from the tank's PCX picture. */
	for (index = 0; index < 16; index++) {
		PCX_Grab_Bitmap(&objects_pcx, &tank1, index, index, 0);
		PCX_Grab_Bitmap(&objects_pcx, &tank2, index, index, 1);
	}

	/* Kill the PCX memory and buffers now that we're done with them. */
	PCX_Delete(&objects_pcx);

/* S E C T I O N  3 ////////////////////////////////////////////////////// */

	/* Point the tanks straight up. */
	tank1.curr_frame = tank1_direction;
	tank2.curr_frame = tank2_direction;

	/* Scan the background under the tanks on the first iteration. */
	Behind_Sprite(&tank1);  /* player */
	Behind_Sprite(&tank2);  /* enemy */

	while (!done) {
/* S E C T I O N  4 ////////////////////////////////////////////////////// */

		/* Erase both tanks. */
		Erase_Sprite(&tank1);
		Erase_Sprite(&tank2);

/* S E C T I O N  5 ////////////////////////////////////////////////////// */

		/* Test whether user wants to translate or rotate the tank. */
		if (kbhit()) {
			/* Reset translation factors. */
			dx = dy = 0;

			/* Test what key was pressed. */
			switch (getch()) {
				case '6':
					/* Rotate to the right. Make sure to wrap around. */
					if (++tank1_direction > 15) {
						tank1_direction = 0;
					}
					break;

				case '4':
					/* Rotate to the left. Make sure to wrap around. */
					if (--tank1_direction < 0) {
						tank1_direction = 15;
					}
					break;

				case '8':
					/* Move forward. Based on the direction value, compute
					the translation factors. */
					angle = (90 + 360 - 22.5 * (float)tank1_direction);

					/* Compute factors based on angle and speed. */
					dx = TANK_SPEED * cos(M_PI * angle / 180);
					dy = TANK_SPEED * sin(M_PI * angle / 180);
					break;

				case '2':
					/* Move backward. Based on the direction value, compute
					the translation factors. */
					angle = (90 + 360 - 22.5 * (float)tank1_direction);

					/* Compute factors based on angle and speed. */
					dx = -TANK_SPEED * cos(M_PI * angle / 180);
					dy = -TANK_SPEED * sin(M_PI * angle / 180);
					break;

				case 'q':
					/* Quit. */
					done = 1;
					break;

				default: break;
			}

/* S E C T I O N  6 ////////////////////////////////////////////////////// */

			/* Do the translation. */
			tank1.x += (int)(dx + .5);
			tank1.y -= (int)(dy + .5);

			/* Test whether the player bumped into edge. If so, push the
			player's tank back. */

			/* Set the frame based on the new direction. */
			tank1.curr_frame = tank1_direction;
		}

/* S E C T I O N  7 ////////////////////////////////////////////////////// */

		/* Now move the enemy tank. */

		/* Test whether it's time to turn. */
		if (rand() % 10 == 1) {
			/* Select the direction to turn. */
			switch (rand() % 2) {
				case 0:
					/* Turn right. */
					if (++tank2_direction > 15) {
						tank2_direction = 0;
					}
					break;

				case 1:
					/* Turn left. */
					if (--tank2_direction < 0) {
						tank2_direction = 15;
					}
					break;
			}

			/* Set the frame based on the new direction. */
			tank2.curr_frame = tank2_direction;
		}

/* S E C T I O N  8 ////////////////////////////////////////////////////// */

		/* Compute the angle in radians. */
		angle = (90 + 360 - 22.5 * (float)tank2_direction);

		/* Compute factors based on angle and speed. */
		dx = (TANK_SPEED + rand() % 2) * cos(M_PI * angle / 180);
		dy = (TANK_SPEED + rand() % 2) * sin(M_PI * angle / 180);

		/* Do the translation. */
		tank2.x += (int)(dx + .5);
		tank2.y -= (int)(dy + .5);

/* S E C T I O N  9 ////////////////////////////////////////////////////// */

		/* Test whether enemy has hit an edge. If so, wrap to the other
		side. */
		if (tank2.x > (320 - tank2.width)) {
			tank2.x = 0;
		} else if (tank2.x < 0) {
			tank2.x = 319 - tank2.width;
		}

		if (tank2.y > (200 - tank2.height)) {
			tank2.y = 0;
		} else if (tank2.y < 0) {
			tank2.y = 199 - tank2.height;
		}

/* S E C T I O N  10 ///////////////////////////////////////////////////// */

		/* Scan the background under both tanks. */
		Behind_Sprite(&tank1);
		Behind_Sprite(&tank2);

		/* Draw both tanks. */
		Draw_Sprite(&tank1);
		Draw_Sprite(&tank2);

		/* Test for collision. */
		if (Sprite_Collide(&tank1, &tank2)) {
			/* Do something spectacular! */
		}

		/* Delay the main loop for a second so the user can see a solid
		image. */
		Delay(1);
	}

	/* Dissolve the screen... in one line, might I add! */
	for (index = 0; index <= 300000; index++) {
		Plot_Pixel_Fast(rand() % 320, rand() % 200, 0);
	}

	/* lag: Clean up! */
	Sprite_Delete(&tank1);
	Sprite_Delete(&tank2);

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);
}
