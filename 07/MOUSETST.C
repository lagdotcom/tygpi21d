/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include "graph3.h"
#include "graph7m.h"

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	long lx, ly;		/* local coords */
	int x, y,
		buttons, num_buttons,
		color = 1;
	char buffer[200];

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Initialize the mouse. */
	Squeeze_Mouse(MOUSE_RESET, NULL, NULL, &num_buttons);

	/* Show the mouse. */
	Squeeze_Mouse(MOUSE_SHOW, NULL, NULL, NULL);

	/* Exit the main loop when the user presses a key. */
	while (!kbhit()) {
		Squeeze_Mouse(MOUSE_BUTT_POS, &x, &y, &buttons);

		/* Display some info. */
		sprintf(buffer, "Mouse x=%d y=%d    ", x, y);
		Blit_String(20, 20, 15, buffer, 0);

		sprintf(buffer, "Buttons=%d  ", buttons);
		Blit_String(20, 30, 15, buffer, 0);

		sprintf(buffer, "Color  =%d  ", color);
		Blit_String(20, 40, color, buffer, 0);

		/* Video easel. */
		Blit_String(0,  0, 15, "V I D E O  E A S E L", 0);
		Blit_String(0, 10, 15, "Press any key to exit.", 0);

		if (buttons == 1) {
			/* Calculate local coordinates */
			lx = x / 2;
			ly = y;

			/* Draw */
			Plot_Pixel_Fast(lx-1, ly-2, color);
			Plot_Pixel_Fast(lx,   ly-2, color);
			Plot_Pixel_Fast(lx-1, ly-1, color);
			Plot_Pixel_Fast(lx,   ly-1, color);
		}

		if (buttons == 2) {
			/* Change color */
			if (++color > 15) color = 0;

			while (buttons == 2) {
				Squeeze_Mouse(MOUSE_BUTT_POS, &x, &y, &buttons);
			}
		}
	}

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);
}
