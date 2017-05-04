/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include "graph3.h"
#include "graph6.h"

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	char buffer[128];		/* temporary string buffer */
	long number_vsyncs = 0;	/* tracks the number of retrace cycles */


	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	while (!kbhit()) {
		/* Wait for a vsync. */
		Wait_For_Vsync();

		/* Do graphics or whatever now that we know electron gun is
		retracing. We only have 1/70 of a second, though! Usually, we would
		copy the double buffer to the video RAM. */
		Plot_Pixel_Fast(rand() % 320, rand() % 200, rand() % 256);

		/* Tally the vsyncs. */
		number_vsyncs++;

		/* Print to the screen. */
		sprintf(buffer, "Number of Vsync's = %1d   ", number_vsyncs);
		Blit_String(8, 8, 9, buffer, 0);
	}

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);
}
