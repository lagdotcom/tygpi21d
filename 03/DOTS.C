/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "graph3.h"     /* this is all we need to include so the program
						knows all the #defines, structures, prototypes */

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int index;          /* loop index */

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Plot 10,000 dots. */
	for (index = 0; index < 10000; index++) {
		Plot_Pixel_Fast(rand() % 320, rand() % 200, rand() % 256);
	}

	while (!kbhit()) {}

	/* Put the computer back into text mode. */
	Set_Video_Mode(TEXT_MODE);
}
