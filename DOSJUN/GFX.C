/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Bline_DB(int xo, int yo, int x1, int y1, colour col)
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
			*vb_start = col;

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
			*vb_start = col;

			error += dx;

			if (error > dy) {
				error -= dy;

				vb_start += x_inc;
			}

			vb_start += y_inc;
		}
	}
}

void Dline_DB(int xo, int yo, int x1, int y1, colour col)
{
	int x, y, xd, yd;

	x = xo;
	y = yo;
	if (x1 < xo) xd = -1;
	else if (x1 > xo) xd = 1;
	if (y1 < yo) yd = -1;
	else if (y1 > yo) yd = 1;

	while (x != x1 && y != y1) {
		Plot_Pixel_Fast_DB(x, y, col);

		x += xd;
		y += yd;
	}
}

void Blit_String_Box(int x, int y, int w, int h, colour col, char *string, bool trans_flag)
{
	int row = 0,
		column = 0,
		i = 0;
	char ch;

	while (string[i]) {
		if (row >= h) return;

		ch = string[i++];
		if (ch == ' ' && column == 0) continue;
		if (ch == '\n') {
			column = 0;
			row++;
			continue;
		}

		Blit_Char_DB(x + column*8, y + row*8, ch, col, trans_flag);

		column++;
		if (column >= w) {
			column = 0;
			row++;
		}
	}
}

void TrapeziumH_DB(colour col, int x0, int x1, int x2, int x3, int y0, int y1, bool filled)
{
	int xa = x0,
		xb = x1,
		y = y0,
		ye = y1,
		m;

	if (filled) {
		if (y1 < y0) {
			m = -1;
		} else {
			m = 1;
		}

		while (y != ye) {
			Bline_DB(xa, y, xb, y, col);

			xa++;
			xb--;
			y += m;
		}
	} else {
		Bline_DB(x0, y0, x1, y0, col);
		Dline_DB(x0, y0, x2, y1, col);
		Dline_DB(x1, y0, x3, y1, col);
		Bline_DB(x2, y1, x3, y1, col);
	}
}

void TrapeziumV_DB(colour col, int x0, int x1, int y0, int y1, int y2, int y3, bool filled)
{
	int x = x0,
		xe = x1,
		ya = y0,
		yb = y1,
		m;

	if (filled) {
		if (x1 < x0) {
			m = -1;
		} else {
			m = 1;
		}

		while (x != xe) {
			Bline_DB(x, ya, x, yb, col);

			ya++;
			yb--;
			x += m;
		}
	} else {
		Bline_DB(x0, y0, x0, y1, col);
		Dline_DB(x0, y0, x1, y2, col);
		Dline_DB(x0, y1, x1, y3, col);
		Bline_DB(x1, y2 - 1, x1, y3 + 1, col);
	}
}

void Square_DB(colour col, int x0, int y0, int x1, int y1, bool filled)
{
	int y;

	if (filled) {
		for (y = y0; y <= y1; y++) {
			Bline_DB(x0, y, x1, y, col);
		}
	} else {
		Bline_DB(x0, y0, x1, y0, col);
		Bline_DB(x0, y1, x1, y1, col);
		Bline_DB(x0, y0, x0, y1, col);
		Bline_DB(x1, y0, x1, y1, col);
	}
}
