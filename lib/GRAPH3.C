/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <io.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <bios.h>
#include <fcntl.h>
#include <mem.h>
#include <math.h>
#include <string.h>
#include "graph3.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Fill_Screen(int value)
{
	/* This function uses the "_fmemset" function to fill the video buffer
	with a sent value. */
	memset(video_buffer, (char)value, SCREEN_WIDTH * SCREEN_HEIGHT + 1);
}

void Blit_Char(int xc, int yc, char c, int color, int trans_flag)
{
	/* This function uses the ROM 8x8 character set to blit a character on
	the video screen. Notice the trick used to extract bits out of each
	character byte that comprises a line. */
	int offset, x, y;
	char far *work_char;
	unsigned char bit_mask = 0x80;

	/* Compute the starting offset in the ROM character look-up table. */
	work_char = rom_char_set + c * CHAR_HEIGHT;

	/* Compute the offset of the character in the video buffer. */
	offset = (yc << 8) + (yc << 6) + xc;

	for (y = 0; y < CHAR_HEIGHT; y++) {
		/* Reset the bit mask. */
		bit_mask = 0x80;

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

void Blit_String(int x, int y, int color, char *string, int trans_flag)
{
	/* This function blits an entire string on the screen with fixed spacing
	between each character. It calls blit_char. */
	int index;

	for (index = 0; string[index] != 0; index++) {
		Blit_Char(
			x + (index << 3),
			y,
			string[index],
			color,
			trans_flag
		);
	}
}

void Plot_Pixel(int x, int y, char color)
{
	/* This function plots a pixel on the video screen by multiplying the
	row by 320 and adding the column. */
	video_buffer[320 * y + x] = color;
}

void Plot_Pixel_Fast(int x, int y, unsigned char color)
{
	/* Plots the pixel in the desired color a little quicker using binary
	shifting to accomplish the multiplications. */

	/* Use the fact that 320*y = 256*y + 64*y = y<<8 + y<<6 */
	video_buffer[((y << 8) + (y << 6)) + x] = color;
}

void Set_Palette_Register(int index, RGB_color_ptr color)
{
	/* This function sets a single color look-up table value indexed by
	index with the value in the color structure. */

	/* Tell the VGA card we're going to update a palette register. */
	outp(PALETTE_MASK, 0xFF);

	/* Tell the VGA card which register we'll be updating */
	outp(PALETTE_REGISTER_WR, index);

	/* Now update the RGB triple.
	Note: the same port is used each time */
	outp(PALETTE_DATA, color->red);
	outp(PALETTE_DATA, color->green);
	outp(PALETTE_DATA, color->blue);
}

void Get_Palette_Register(int index, RGB_color_ptr color)
{
	/* This function gets the data out of a color look-up register and
	places the data into color */

	/* Set the palette mask register */
	outp(PALETTE_MASK, 0xFF);

	/* Tell the VGA card which reigster we'll be reading */
	outp(PALETTE_REGISTER_RD, index);

	/* Now extract the data. */
	color->red   = inp(PALETTE_DATA);
	color->green = inp(PALETTE_DATA);
	color->blue  = inp(PALETTE_DATA);
}

void Create_Cool_Palette(void)
{
	/* This function creates a nifty palette: 64 shades of grey, 64
	of red, 64 of green, and 64 of blue. */
	RGB_color color;
	int index;

	/* swipe through the color registers and create four banks of 64 */
	for (index = 0; index < 64; index++) {
		/* These are the grays: */
		color.red   = index;
		color.green = index;
		color.blue  = index;
		Set_Palette_Register(index, (RGB_color_ptr)&color);

		/* These are the reds: */
		color.red   = index;
		color.green = 0;
		color.blue  = 0;
		Set_Palette_Register(index + 64, (RGB_color_ptr)&color);

		/* These are the greens: */
		color.red   = 0;
		color.green = index;
		color.blue  = 0;
		Set_Palette_Register(index + 128, (RGB_color_ptr)&color);

		/* These are the blues: */
		color.red   = 0;
		color.green = 0;
		color.blue  = index;
		Set_Palette_Register(index + 192, (RGB_color_ptr)&color);
	}

	/* Make color 0 black. */
	color.red   = 0;
	color.green = 0;
	color.blue  = 0;
	Set_Palette_Register(0, (RGB_color_ptr)&color);
}

void Delay(int clicks)
{
	/* This function uses the internal timekeeper (the one that runs at 18.2
	clicks/sec) to time a delay. You can find the 32-bit value of this timer
	at 0000:046Ch. */
	unsigned int far *clock = (unsigned int far *)0x0000046CL;
	unsigned int now;

	/* Get the current time */
	now = *clock;

	/* Wait until the time has gone past the current time plus the amount we
	wanted to wait. Note that each tick is approximately 55 milliseconds. */
	while(abs(*clock - now) < clicks) {}
}

void Set_Video_Mode(int mode)
{
	/* Use video interrupt 10h to set the video mode to the sent value */
	union REGS inregs, outregs;

	inregs.h.ah = 0;                    /* Set the video mode subfunction. */
	inregs.h.al = (unsigned char)mode;  /* Video mode to which to change. */

	int86(0x10, &inregs, &outregs);
}

void H_Line(int x1, int x2, int y, unsigned int color)
{
	/* Draw a horizontal line using the memset function.
	Note: x2 > x1 */

	memset((char far*)(video_buffer + ((y << 8) + (y << 6)) + x1),
		color, x2 - x1 + 1);
}

void H_Line_Fast(int x1, int x2, int y, unsigned int color)
{
	/* A fast horizontal line renderer uses WORD-sized writes instead of
	BYTE-sized writes. The only problem is that the endpoints of the h line
	must be	taken into account. Test whether the endpoints of the horizontal
	are on WORD boundaries; that is, that they are evenly divisible by 2.
	Basically, we must consider the two endpoints of the line separately if
	we want to write WORDs at a time (or, in other words, two pixels at a
	time).
	Note: x2 > x1 */
	unsigned int first_word, middle_word, last_word, line_offset, index;

	/* Test the 1's bit of the starting x. */
	if (x1 & 0x0001) {
		first_word = color << 8;
	} else {
		/* Repliocate color into both bytes. */
		first_word = (color << 8) | color;
	}

	/* Test the 1's bit of the ending x. */
	if (x2 & 0x0001) {
		last_word = (color << 8) | color;
	} else {
		/* Place color in high byte of word only. */
		last_word = color;
	}

	/* Now we can draw the horizontal line two pixels at a time. */
	line_offset = (y << 7) + (y << 5);
	/* y * 160, because there are 160 words/line. */

	/* Compute the middle color. */
	middle_word = (color << 8) | color;

	/* Left endpoint. */
	video_buffer_w[line_offset + (x1 >> 1)] = first_word;

	/* The middle of the line. */
	for (index = (x1 >> 1) + 1; index < (x2 >> 1); index++) {
		video_buffer_w[line_offset + index] = middle_word;
	}

	/* Right endpoint. */
	video_buffer_w[line_offset + (x2 >> 1)] = last_word;
}

void V_Line(int y1, int y2, int x, unsigned int color)
{
	/* Draw a vertical line.
	Note: y2 > y1 */
	unsigned int line_offset, index;

	/* Compute the starting position */
	line_offset = (y1 << 8) + (y1 << 6) + x;

	for (index = 0; index <= y2 - y1; index++) {
		video_buffer[line_offset] = color;

		/* Move to the next line. */
		line_offset += 320;
	}
}
