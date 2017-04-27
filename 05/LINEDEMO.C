/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "graph3.h"
#include "graph4.h"
#include "graph5.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Bounce(void)
{
	/* This function makes use of the bline function to bounce a line
	around. */
	int xo, yo, x1, y1, x2, y2, x3, y3;
	int dxo, dyo, dx1, dy1, dx2, dy2, dx3, dy3;
	long counter = 0;
	int color = 9;

	/* lag: set up extents */
	int minx = 5;
	int maxx = SCREEN_WIDTH - minx;
	int miny = 5;
	int maxy = SCREEN_HEIGHT - miny;

	/* Starting positions of lines. */
	xo = x2 = rand() % SCREEN_WIDTH;
	yo = y2 = rand() % SCREEN_HEIGHT;
	x1 = x3 = rand() % SCREEN_WIDTH;
	y1 = y3 = rand() % SCREEN_HEIGHT;

	/* Velocities of lines. */
	dxo = dx2 = 2 + rand() % 5;
	dyo = dy2 = 3 + rand() % 5;
	dx1 = dx3 = 2 + rand() % 5;
	dy1 = dy3 = 2 + rand() % 5;

	/* Animation loop. */
	while (!kbhit()) {
		/* Draw the leader. */
		Bline(xo, yo, x1, y1, color);

		/* Move the line. */
		if ((xo += dxo) >= maxx || xo < minx) {
			dxo = -dxo;
		}

		if ((yo += dyo) >= maxy || yo < miny) {
			dyo = -dyo;
		}

		if ((x1 += dx1) >= maxx || x1 < minx) {
			dx1 = -dx1;
		}

		if ((y1 += dy1) >= maxy || y1 < miny) {
			dy1 = -dy1;
		}

		/* Test whether it's time to follow the leader. */
		if (++counter > 50) {
			Bline(x2, y2, x3, y3, 0);

			/* Move the line. */
			if ((x2 += dx2) >= maxx || x2 < minx) {
				dx2 = -dx2;
			}

			if ((y2 += dy2) >= maxy || y2 < miny) {
				dy2 = -dy2;
			}

			if ((x3 += dx3) >= maxx || x3 < minx) {
				dx3 = -dx3;
			}

			if ((y3 += dy3) >= maxy || y3 < miny) {
				dy3 = -dy3;
			}
		}

		/* Wait a while so humans can see it. */
		Delay(1);

		/* Update the color. */
		if (counter > 250) {
			if (++color >= 16) {
				color = 1;
			}

			counter = 51;
		}
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Draw a couple of lines. */
	while (!kbhit()) {
		Bline(rand() % SCREEN_WIDTH,
			  rand() % SCREEN_HEIGHT,
			  rand() % SCREEN_WIDTH,
			  rand() % SCREEN_HEIGHT,
			  rand() % 256);
	}
	getch();

	/* Clear the screen. */
	Set_Video_Mode(VGA256);

	/* Show off a little screen saver. */
	Bounce();

	/* Reset the video mode back to text. */
	Set_Video_Mode(TEXT_MODE);
}
