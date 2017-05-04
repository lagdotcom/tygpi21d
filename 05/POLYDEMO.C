/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include "graph3.h"
#include "graph5.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* Global clipping region default value. */
#define POLY_CLIP_MIN_X		(160 - 30)
#define POLY_CLIP_MIN_Y		(100 - 30)

#define POLY_CLIP_MAX_X		(160 + 30)
#define POLY_CLIP_MAX_Y		(100 + 30)

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Draw_Boundary(int color)
{
	/* Draws in the clipping boundary, if the user is interested in seeing
	it. */

	Bline(POLY_CLIP_MIN_X, POLY_CLIP_MIN_Y,
	      POLY_CLIP_MAX_X, POLY_CLIP_MIN_Y, color);
	Bline(POLY_CLIP_MAX_X, POLY_CLIP_MIN_Y,
	      POLY_CLIP_MAX_X, POLY_CLIP_MAX_Y, color);
	Bline(POLY_CLIP_MAX_X, POLY_CLIP_MAX_Y,
	      POLY_CLIP_MIN_X, POLY_CLIP_MAX_Y, color);
	Bline(POLY_CLIP_MIN_X, POLY_CLIP_MAX_Y,
	      POLY_CLIP_MIN_X, POLY_CLIP_MIN_Y, color);
}

/* M A I N /////////////////////////////////////////////////////////////// */

void vset(polygon_ptr poly, int index, int x, int y)
{
	poly->vertices[index].x = x;
	poly->vertices[index].y = y;
}

void main(void)
{
	/* Track whether the clipping engine is on. */
	int clip_on = 1;
	polygon p1;			/* Working polygon */
	int done = 0;		/* System exit flag */

	/* Set initial clipping region. */
	Set_Clipping_Region(POLY_CLIP_MIN_X, POLY_CLIP_MIN_Y,
	                    POLY_CLIP_MAX_X, POLY_CLIP_MAX_Y);

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Draw some instructions. */
	Blit_String(0,  0, 7, "Press Q - To quit.", 1);
	Blit_String(0, 10, 7, "Press T - To toggle clipping engine.", 1);
	Blit_String(0, 20, 7, "Use Numeric keypad to translate.", 1);

	/* Create look-up tables for polygon engine. */
	Create_Tables();

	/* Build up a little spaceship polygon. */
	vset(&p1,  0,   3, -19);
	vset(&p1,  1,  12,  -1);
	vset(&p1,  2,  17,   2);
	vset(&p1,  3,  17,   9);
	vset(&p1,  4,   8,  14);
	vset(&p1,  5,   5,   8);
	vset(&p1,  6,  -5,   8);
	vset(&p1,  7,  -8,  14);
	vset(&p1,  8, -17,   9);
	vset(&p1,  9, -17,   2);
	vset(&p1, 10, -12,  -1);
	vset(&p1, 11,  -3, -19);
	vset(&p1, 12,  -3,  -8);
	vset(&p1, 13,   3,  -8);

	/* Set the position of the spaceship. */
	p1.lxo = 160;
	p1.lyo = 100;

	/* Fill in important fields. */
	p1.num_vertices = 14;
	p1.b_color = 1;
	p1.closed = 1;

	/* Main event loop */
	while (!done) {
		/* Erase the polygon. */
		p1.b_color = 0;
		if (clip_on) {
			Draw_Polygon_Clip(&p1);
		} else {
			Draw_Polygon(&p1);
		}

		/* Erase the origin of the polygon. */
		Plot_Pixel_Fast(p1.lxo, p1.lyo, 0);

		/* What is the user doing? */
		if (kbhit()) {
			switch (getch()) {
				case '8':
					/* Move the ship up. */
					p1.lyo--;
					break;

				case '2':
					/* Move the ship down. */
					p1.lyo++;
					break;

				case '6':
					/* Move the ship right. */
					p1.lxo++;
					break;

				case '4':
					/* Move the ship left. */
					p1.lxo--;
					break;

				case 't':
					/* Toggle the clipper. */
					clip_on = 1 - clip_on;
					break;

				case 'q':
					/* It's coming now, Khan!!! */
					done = 1;
					break;
			}
		}

		/* Rotate the polygon 5 degrees. */
		Rotate_Polygon(&p1, 5);

		/* Draw the polygon. */
		if (clip_on) {
			p1.b_color = 1;
			Draw_Polygon_Clip(&p1);
			Blit_String(10, 100, 7, "Clipping on. ", 0);
		} else {
			p1.b_color = 12;
			Draw_Polygon(&p1);
			Blit_String(10, 100, 7, "Clipping off.", 0);
		}

		/* Draw a point at origin of polygon. */
		Plot_Pixel_Fast(p1.lxo, p1.lyo, 15);

		/* Let the user see the clipping region. */
		Draw_Boundary(10);

		/* Just chill here for 1/18.2th of a second. */
		Delay(1);
	}

	/* Reset the video mode back to text. */
	Set_Video_Mode(TEXT_MODE);
}
