/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "graph3.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Blit_Char_Bicolor(int xc, int yc, char c, int color1, int color2, int trans_flag)
{
	/* This function uses the ROM 8x8 character set to blit a character on
	the video screen. Notice the trick used to extract bits out of each
	character byte that comprises a line. */
	int offset, x, y, color;
	char far *work_char;
	unsigned char bit_mask = 0x80;

	/* Compute the starting offset in the ROM character look-up table. */
	work_char = rom_char_set + c * CHAR_HEIGHT;

	/* Compute the offset of the character in the video buffer. */
	offset = (yc << 8) + (yc << 6) + xc;

	for (y = 0; y < CHAR_HEIGHT; y++) {
		/* Reset the bit mask. */
		bit_mask = 0x80;

		/* Which colour? */
		if (y < (CHAR_HEIGHT >> 1)) {
			color = color1;
		} else {
			color = color2;
		}

		for (x = 0; x < CHAR_WIDTH; x++) {
			/* Test for a transparent pixel; that is, 0. If the pixel is not
			transparent, draw it. */
			if ((*work_char & bit_mask)) {
				video_buffer[offset + x] = color;
			} else if (!trans_flag) {
				/* or plot black if not transparent */
				video_buffer[offset + x] = 0;
			}

			/* Shift the bit mask. */
			bit_mask = bit_mask >> 1;
		}

		/* Move to the next line in the cideo buffer and in the ROM character
		data area. */
		offset += SCREEN_WIDTH;
		work_char++;
	}
}

void Blit_String_Bicolor(int x, int y, int color1, int color2, char *string, int trans_flag)
{
	int index;

	for (index = 0; string[index] != 0; index++) {
		Blit_Char_Bicolor(
			x + (index << 3),
			y,
			string[index],
			color1,
			color2,
			trans_flag
		);
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	Set_Video_Mode(VGA256);

	/* set up our palette so we can use the reds */
	Create_Cool_Palette();

	Blit_String_Bicolor(10, 10, 127,  96, "Check out my text.", 1);
	Blit_String_Bicolor(10, 40, 191, 160, "It rocks.", 1);

	while (!kbhit()) {}

	Set_Video_Mode(TEXT_MODE);
}
