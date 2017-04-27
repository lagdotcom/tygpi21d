/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "graph3.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Fade_Out(void)
{
	int index, fade;
	RGB_color color;

	for (fade = 0; fade < 63; fade++) {
		for (index = 0; index < 256; index++) {
			Get_Palette_Register(index, &color);

			if (color.red > 0)   color.red--;
			if (color.green > 0) color.green--;
			if (color.blue > 0)  color.blue--;

			Set_Palette_Register(index, &color);
		}

		Delay(1);
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	unsigned int index;

	Set_Video_Mode(VGA256);

	for (index = 0; index < 30000; index++) {
		Plot_Pixel_Fast(rand() % 320, rand() % 200, rand() % 256);
	}

	Fade_Out();

	Set_Video_Mode(TEXT_MODE);
}
