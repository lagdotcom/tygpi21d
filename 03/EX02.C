/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include "graph3.h"

/* M A I N /////////////////////////////////////////////////////////////// */

void main(int argc, char** argv)
{
	unsigned int index,
		fast = 0,
		dots = 0xffff;

	/* parse args */
	if (argc == 1) {
		printf("usage: %s {fast|slow}", argv[0]);
		return;
	}

	if (!strcmp(argv[1], "fast")) {
		fast = 1;
	} else if (strcmp(argv[1], "slow")) {
		printf("usage: %s {fast|slow}", argv[0]);
		return;
	}

	Set_Video_Mode(VGA256);

	if (fast) {
		for (index = 0; index < dots; index++) {
			Plot_Pixel_Fast(rand() % 320, rand() % 200, rand() % 256);
		}
	} else {
		for (index = 0; index < dots; index++) {
			Plot_Pixel(rand() % 320, rand() % 200, rand() % 256);
		}
	}

	while (!kbhit()) {}

	Set_Video_Mode(TEXT_MODE);
}
