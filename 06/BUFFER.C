/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "graph3.h"
#include "graph6.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define DB_LINES		200

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	/* This function creates a kaleidoscope of colors. */
	int x, y, fcolor = 1, index;

	/* Create a double buffer. */
	if (!Create_Double_Buffer(DB_LINES)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Main event loop. */
	while (!kbhit()) {
		/* Clear out the double buffer with black. */
		Fill_Double_Buffer(0);

		/* Next color */
		if (++fcolor > 15) fcolor = 1;

		/* Draw something in it. */
		for (index = 0; index < 200; index++) {
			/* Make a kaleidoscope of color. */
			x = rand() % (SCREEN_WIDTH / 2);
			y = rand() % (DB_LINES / 2);

			Plot_Pixel_Fast_DB(x, y, fcolor);
			Plot_Pixel_Fast_DB((SCREEN_WIDTH - 1) - x, y, fcolor);
			Plot_Pixel_Fast_DB(x, (DB_LINES - 1) - y, fcolor);
			Plot_Pixel_Fast_DB((SCREEN_WIDTH - 1) - x,
				(DB_LINES - 1) - y, fcolor);
		}

		/* Copy the double buffer to the video buffer. */
		Show_Double_Buffer();

		/* Wait a bit so the user can see it. */
		Delay(2);
	}

	/* Reset the video mode back to text. */
	Set_Video_Mode(TEXT_MODE);

	/* Free the double buffer. */
	Delete_Double_Buffer();
}
