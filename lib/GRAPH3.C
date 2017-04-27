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

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define VGA256    0x13          /* 320x200x256 */
#define TEXT_MODE 0x03          /* The default text mode */

#define PALETTE_MASK        0x3c6
#define PALETTE_REGISTER_RD 0x3c7
#define PALETTE_REGISTER_WR 0x3c8
#define PALETTE_DATA        0x3c9

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 8

#define SCREEN_WIDTH        (unsigned int)320
#define SCREEN_HEIGHT       (unsigned int)200

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct RGB_color_typ {
	unsigned char red;      /* Red component of color 0-63 */
	unsigned char green;    /* Green component of color 0-63 */
	unsigned char blue;     /* Blue component of color 0-63 */
} RGB_color, *RGB_color_ptr;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

#include "graph3.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* vram byte ptr */
unsigned char far *video_buffer = (char far *)0xA0000000L;

/* 8x8 ROM characters */
unsigned char far *rom_char_set = (char far *)0xF000FA6EL;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

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
