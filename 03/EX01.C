/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include "graph3.h"

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int color, direction;

	Set_Video_Mode(VGA256);

	/* set up our palette so we can use the reds */
	Create_Cool_Palette();

	color = 64;
	direction = 1;

	while (!kbhit()) {
		Plot_Pixel_Fast(100, 100, color);

		/* reds go from 64 - 127 */
		color += direction;
		if (color == 127) {
			direction = -1;
		} else if (color == 64) {
			direction = 1;
		}

		Delay(1);
	}

	Set_Video_Mode(TEXT_MODE);
}
