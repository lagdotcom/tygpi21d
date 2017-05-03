/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <math.h>
#include "graph3.h"
#include "graph5.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* Global clipping region default value. */
#define POLY_CLIP_MIN_X		0
#define POLY_CLIP_MIN_Y		0

#define POLY_CLIP_MAX_X		319
#define POLY_CLIP_MAX_Y		199

/* Number of rocks in asteroid field. */
#define NUM_ROCKS			10

/* Friction in space: yes, believe it or not! Space applies friction to both
energy and matter. Even in deepest space there are approximately four
hydrogen atoms per cubic cm, and the wave impedence, or energy friction, is
377 ohms. That's why light maxes out at 186,300 miles per second. */
#define FRICTION			.2

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct object_typ {
	int state;			/* State of the rock */
	int rotation_rate;	/* Angle to rotate object per frame */
	int xv, yv;			/* Velocity vector */
	polygon rock;		/* One polygon per rock */
} object, *object_ptr;

/* G L O B A L S ///////////////////////////////////////////////////////// */

object rocks[NUM_ROCKS];

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void vset(polygon_ptr poly, int index, int x, int y)
{
	poly->vertices[index].x = x;
	poly->vertices[index].y = y;
}

void Initialize_Rocks(void)
{
	/* This function initializes all rocks in the asteroid field. */
	int index;			/* loop index */
	float scale;		/* Used to change scale of each rock */

	/* Initialize all rocks in the asteroid field. */
	for (index = 0; index < NUM_ROCKS; index++) {
		/* Build up each rock and add a little noise to each vertex to make
		them look different. */

#define ROCK (rocks[index].rock)
#define NOISE (rand() % 2)
		vset(&ROCK, 0,  4 + NOISE,  4 + NOISE);
		vset(&ROCK, 1,  9 + NOISE, -3 + NOISE);
		vset(&ROCK, 2,  6 + NOISE, -5 + NOISE);
		vset(&ROCK, 3,  2 + NOISE, -3 + NOISE);
		vset(&ROCK, 4, -4 + NOISE, -6 + NOISE);
		vset(&ROCK, 5, -3 + NOISE,  5 + NOISE);
#undef NOISE

		ROCK.num_vertices = 6;
		ROCK.b_color      = 10;
		ROCK.i_color      = 10;
		ROCK.closed       = 1;
		ROCK.filled       = 0;
		ROCK.lxo          = rand() % POLY_CLIP_MAX_X;
		ROCK.lyo          = rand() % POLY_CLIP_MAX_Y;

		/* Compute the velocity */
		rocks[index].xv = -5 + rand() % 10;
		rocks[index].yv = -5 + rand() % 10;

		/* Set the state of the rock to alive, and set the rotatation rate */
		rocks[index].state = 1;
		rocks[index].rotation_rate = -10 + rand() % 20;

		/* Compute the scale */
		scale = ((float)(5 + rand() % 15)) / 10;

		/* Scale the rock to make it look different. */
		Scale_Polygon(&ROCK, scale);
	}
}

void Draw_Rocks(void)
{
	/* This function draws all the asteroids. */
	int index;

	/* Loop through all rocks and draw them. */
	for (index = 0; index < NUM_ROCKS; index++) {
		ROCK.b_color = 10;
		Draw_Polygon_Clip(&ROCK);
	}
}

void Erase_Rocks(void)
{
	/* This function erases all the asteroids. */
	int index;

	/* Loop through all rocks and draw them. */
	for (index = 0; index < NUM_ROCKS; index++) {
		ROCK.b_color = 0;
		Draw_Polygon_Clip(&ROCK);
	}
}

void Move_Rocks(void)
{
	/* This function moves and rotates all the asteroids. */
	int index;

	for (index = 0; index < NUM_ROCKS; index++) {
		/* Translate the polygon. */
		Translate_Polygon(&ROCK, rocks[index].xv, rocks[index].yv);

		/* Rotate the rock. */
		Rotate_Polygon(&ROCK, rocks[index].rotation_rate);

		/* Test for collision of edges. */
		if (ROCK.lxo > 310)		ROCK.lxo = 10;
		else if (ROCK.lxo < 10)	ROCK.lxo = 310;

		if (ROCK.lyo > 190)		ROCK.lyo = 10;
		else if (ROCK.lyo < 10)	ROCK.lyo = 190;
	}
}
#undef ROCK

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0;		/* System exit flag */
	polygon ship;
	float xv = 0,		/* Ship velocity */
	      yv = 0;
	int angle = 90,		/* Ship angle */
	    engines = 0;	/* Tracks whether engines are on */

/* // S E C T I O N  1 /////////////////////////////////////////////////// */

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Create look-up tables for polygon engine. */
	Create_Tables();

	/* Set initial clipping region. */
	Set_Clipping_Region(POLY_CLIP_MIN_X, POLY_CLIP_MIN_Y,
	                    POLY_CLIP_MAX_X, POLY_CLIP_MAX_Y);

/* // S E C T I O N  2 /////////////////////////////////////////////////// */

	/* Build up a little spaceship polygon. */
	vset(&ship,  0,   3, -19);
	vset(&ship,  1,  12,  -1);
	vset(&ship,  2,  17,   2);
	vset(&ship,  3,  17,   9);
	vset(&ship,  4,   8,  14);
	vset(&ship,  5,   5,   8);
	vset(&ship,  6,  -5,   8);
	vset(&ship,  7,  -8,  14);
	vset(&ship,  8, -17,   9);
	vset(&ship,  9, -17,   2);
	vset(&ship, 10, -12,  -1);
	vset(&ship, 11,  -3, -19);
	vset(&ship, 12,  -3,  -8);
	vset(&ship, 13,   3,  -8);

	/* Set the position of the spaceship. */
	ship.lxo = 160;
	ship.lyo = 100;

	/* Fill in important fields. */
	ship.num_vertices = 14;
	ship.b_color      = 1;
	ship.closed       = 1;

	/* Make the ship a little smaller. */
	Scale_Polygon(&ship, 0.75);

	/* Create the asteroid field. */
	Initialize_Rocks();

/* // S E C T I O N  3 /////////////////////////////////////////////////// */

	/* main event loop */
	while (!done) {
		/* Erase all the rocks. */
		Erase_Rocks();

		/* Erase the player's ship. */
		ship.b_color = 0;
		Draw_Polygon_Clip(&ship);

/* // S E C T I O N  4 /////////////////////////////////////////////////// */

		/* Move everything. */
		engines = 0;

		if (kbhit()) {
			switch (getch()) {
				case 's':
					Rotate_Polygon(&ship, 5);
					angle += 5;
					if (angle > 360) {
						angle = 0;
					}
					break;

				case 'a':
					Rotate_Polygon(&ship, -5);
					angle -= 5;
					if (angle < 0) {
						angle = 360;
					}
					break;

				case 'l':
					/* Adjust the velocity vector based on direction */
					xv = xv - cos(angle * M_PI / 180);
					yv = yv - sin(angle * M_PI / 180);

					/* Flag that engines are on. */
					engines = 1;

					/* Control the upper throttle limit. */
					if (xv > 10)		xv = 10;
					else if (xv < -10)	xv = -10;

					if (yv > 10)		yv = 10;
					else if (yv < -10)	yv = -10;

					break;

				case 'q':
					/* Is the user trying to exit? */
					done = 1;
					break;
			}
		}

/* // S E C T I O N  5 /////////////////////////////////////////////////// */

		/* Decelerate the engines if they're off. */
		if (!engines) {
			/* Tend the x and y components of velocity toward 0. */
			if (xv > 0)			xv -= FRICTION;
			else if (xv < 0)	xv += FRICTION;

			if (yv > 0)			yv -= FRICTION;
			else if (yv < 0)	yv += FRICTION;
		}

		/* Test whether ship went offscreen. */
		if (ship.lxo > 310)		ship.lxo = 10;
		else if (ship.lxo < 10)	ship.lxo = 310;

		if (ship.lyo > 190)		ship.lyo = 10;
		else if (ship.lyo < 10)	ship.lyo = 190;

/* // S E C T I O N  6 /////////////////////////////////////////////////// */

		/* Do the actual translation. */
		Translate_Polygon(&ship, xv, yv);

		/* Now move the rocks. */
		Move_Rocks();

/* // S E C T I O N  7 /////////////////////////////////////////////////// */

		/* Draw everything. */
		Draw_Rocks();

		ship.b_color = 9;
		Draw_Polygon_Clip(&ship);

		/* Draw instructions. */
		Blit_String(0, 190, 15, "(A,S)-Rotate, (L)-Thrust, (Q)-Exit", 1);

		/* Just chill here for 1/18.2th of a second. */
		Delay(1);
	}

	/* Reset the video mode back to text. */
	Set_Video_Mode(TEXT_MODE);
}
