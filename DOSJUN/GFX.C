/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Draw_Line_DB(int xo, int yo, int x1, int y1, colour col)
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

void Draw_Diagonal_DB(int xo, int yo, int x1, int y1, colour col)
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

void Draw_Bounded_String(int x, int y, int w, int h, colour col, char *string, bool trans_flag)
{
	int row = 0,
		column = 0,
		i = 0;
	char ch;

	if (string == null) return;

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

void Draw_HorzTrapezium_DB(colour col, int x0, int x1, int x2, int x3, int y0, int y1, bool filled)
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
			Draw_Line_DB(xa, y, xb, y, col);

			xa++;
			xb--;
			y += m;
		}
	} else {
		Draw_Line_DB(x0, y0, x1, y0, col);
		Draw_Diagonal_DB(x0, y0, x2, y1, col);
		Draw_Diagonal_DB(x1, y0, x3, y1, col);
		Draw_Line_DB(x2, y1, x3, y1, col);
	}
}

void Draw_VertTrapezium_DB(colour col, int x0, int x1, int y0, int y1, int y2, int y3, bool filled)
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
			Draw_Line_DB(x, ya, x, yb, col);

			ya++;
			yb--;
			x += m;
		}
	} else {
		Draw_Line_DB(x0, y0, x0, y1, col);
		Draw_Diagonal_DB(x0, y0, x1, y2, col);
		Draw_Diagonal_DB(x0, y1, x1, y3, col);
		Draw_Line_DB(x1, y2 - 1, x1, y3 + 1, col);
	}
}

void Draw_Square_DB(colour col, int x0, int y0, int x1, int y1, bool filled)
{
	int y;

	if (filled) {
		for (y = y0; y <= y1; y++) {
			Draw_Line_DB(x0, y, x1, y, col);
		}
	} else {
		Draw_Line_DB(x0, y0, x1, y0, col);
		Draw_Line_DB(x0, y1, x1, y1, col);
		Draw_Line_DB(x0, y0, x0, y1, col);
		Draw_Line_DB(x1, y0, x1, y1, col);
	}
}


/* Wrap a string to fit in a given box size. You must NOT Free() the returned string. */
char *Word_Wrap(char *string, int width, int height)
{
	char wrapped[200];
	int i = 0,
		last_space = 0,
		x = 0;

	strcpy(wrapped, string);

	while (string[i]) {
		switch (string[i]) {
			case ' ':
				last_space = i;
				break;

			case '\n':
				last_space = i;
				x = -1;
				break;
		}

		x++;
		if (x >= width && last_space > 0) {
			wrapped[last_space] = '\n';

			x = i - last_space;
			last_space = 0;
		}

		i++;
	}

	return wrapped;
}

/* Clears an area, then prints a string into it, with wrapping. */
void Draw_Wrapped_String(int x, int y, int w, int h, colour col, char *string, bool margin)
{
	int columns = w / 8;
	int rows = h / 8;
	char *wrapped;

	if (margin) {
		columns--;
		rows--;

		x += 4;
		y += 4;
		w -= 8;
		h -= 8;
	}

	wrapped = Word_Wrap(string, columns, rows);

	Draw_Square_DB(0, x, y, x + w - 1, y + h - 1, true);
	Draw_Bounded_String(x, y, w, h, col, wrapped, false);
}
