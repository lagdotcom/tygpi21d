/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include "graph3.h"

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int x = 100,
		y = 100,
		dx = 1,
		dy = 1,
		maxx = SCREEN_WIDTH - 1,
		maxy = SCREEN_HEIGHT - 1;

	Set_Video_Mode(VGA256);

	while (!kbhit()) {
		Plot_Pixel_Fast(x, y, 0);

		x += dx;
		y += dy;
		if (x <= 0 || x >= maxx) {
			dx = -dx;
		}
		if (y <= 0 || y >= maxy) {
			dy = -dy;
		}

		Plot_Pixel_Fast(x, y, 15);

		Delay(1);
	}

	Set_Video_Mode(TEXT_MODE);
}
