/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "graph3.h"
#include "graph4.h"

/* M A I N /////////////////////////////////////////////////////////////// */

void main(int argc, char **argv)
{
	long index;					/* loop counter */
	pcx_picture background_pcx;	/* This PCX structure holds the background */
	FILE *fp;					/* Used to see whether the file exists. */

	/* Make sure there's a file name. */
	if (argc < 2) {
		printf("\nUsage: pcxshow filename.pcx");
		return;
	}

	/* Test whether the file exists, but not for the PCX extension. */
	fp = fopen(argv[1], "rb");
	if (!fp) {
		printf("\nFile:%s - not found!", argv[1]);
		return;
	} else {
		fclose(fp);
	}

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Load in the background. */
	PCX_Init(&background_pcx);
	PCX_Load(argv[1], &background_pcx, 1);
	PCX_Show_Buffer(&background_pcx);
	PCX_Delete(&background_pcx);

	while (!kbhit()) {}

	/* Dissolve the screen... in one line, might I add! */
	for (index = 0; index <= 300000; index++) {
		Plot_Pixel_Fast(rand() % 320, rand() % 200, 0);
	}

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);
}
