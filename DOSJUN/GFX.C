/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Bline_DB(int xo, int yo, int x1, int y1, unsigned char color)
{
	/* This function uses Bresenham's algorithm (IBM 1965) to draw a line
	from (xo,yo) - (x1,y1). */

	int dx,					/* x difference */
		dy,					/* y difference */
		x_inc,				/* x increment */
		y_inc,				/* y increment */
		error = 0,			/* discriminant */
		index;

	/* Access the video buffer directly for speed. */
	unsigned char far *vb_start = double_buffer;

	/* Precompute the first pixel address in the video buffer. Use shifts for
	multiplication. */
	vb_start += ((unsigned int)yo << 6) + ((unsigned int)yo << 8) +
		(unsigned int)xo;

	/* Compute the deltas. */
	dx = x1 - xo;
	dy = y1 - yo;

	/* Test which direction the line is going in; i.e. the slope angle. */
	if (dx >= 0) {
		x_inc = 1;
	} else {
		x_inc = -1;
		dx = -dx;
	}

	/* Test the y component of the slope. */
	if (dy >= 0) {
		y_inc = SCREEN_WIDTH;
	} else {
		y_inc = -SCREEN_WIDTH;
		dy = -dy;
	}

	/* Now, based on which delta is greater, we can draw the line. */
	if (dx > dy) {
		/* Draw the line. */
		for (index = 0; index <= dx; index++) {
			/* Set the pixel */
			*vb_start = color;

			/* Adjust the disciminant. */
			error += dy;

			/* Test if error overflowed. */
			if (error > dx) {
				error -= dx;

				/* Move to the next line. */
				vb_start += y_inc;
			}

			vb_start += x_inc;
		}
	} else {
		for (index = 0; index <= dy; index++) {
			*vb_start = color;

			error += dx;

			if (error > dy) {
				error -= dy;

				vb_start += x_inc;
			}

			vb_start += y_inc;
		}
	}
}

void Dline_DB(int xo, int yo, int x1, int y1, unsigned char color)
{
	int x, y, xd, yd;

	x = xo;
	y = yo;
	if (x1 < xo) xd = -1;
	else if (x1 > xo) xd = 1;
	if (y1 < yo) yd = -1;
	else if (y1 > yo) yd = 1;

	while (x != x1 && y != y1) {
		Plot_Pixel_Fast_DB(x, y, color);

		x += xd;
		y += yd;
	}
}
