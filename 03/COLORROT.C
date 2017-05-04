/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <dos.h>
#include <stdio.h>
#include <string.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define VGA256    0x13          /* 320x200x256 */
#define TEXT_MODE 0x03          /* The default text mode */

#define PALETTE_MASK        0x3c6
#define PALETTE_REGISTER_RD 0x3c7
#define PALETTE_REGISTER_WR 0x3c8
#define PALETTE_DATA        0x3c9

#define SCREEN_WIDTH        (unsigned int)320
#define SCREEN_HEIGHT       (unsigned int)200

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct RGB_color_typ {
	unsigned char red;      /* Red component of color 0-63 */
	unsigned char green;    /* Green component of color 0-63 */
	unsigned char blue;     /* Blue component of color 0-63 */
} RGB_color, *RGB_color_ptr;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Set_Palette_Register(int index, RGB_color_ptr color);
void Get_Palette_Register(int index, RGB_color_ptr color);
void Create_Cool_Palette(void);
void H_Line(int x1, int x2, int y, unsigned int color);

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* vram byte ptr */
unsigned char far *video_buffer = (char far *)0xA0000000L;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

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

/* /////////////////////////////////////////////////////////////////////// */

void main(void)
{
	int index,              /* Loop var */
		x1 = 150,           /* x1 and x2 are the edges of the current piece
							   of the road */
		x2 = 170,
		y = 0,              /* y is the current y position of the piece
							   of the road */
		curr_color = 1;     /* the current color being drawn */

	RGB_color color, color_1;

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Create the color palette. */
	Create_Cool_Palette();
	printf("Press any key to exit.");

	/* Draw a road to nowhere. */
	for (y = 80; y < 200; y++) {
		/* Draw the next horizontal piece of road. */
		H_Line(x1, x2, y, curr_color);

		/* Make the road wider */
		if (--x1 < 0) x1 = 0;
		if (++x2 > 319) x2 = 319;

		/* Next color, please. */
		if (++curr_color > 255) curr_color = 1;
	}

	/* Wait for a key to be hit. */
	while (!kbhit()) {
		Get_Palette_Register(1, (RGB_color_ptr)&color_1);

		for (index = 1; index <= 254; index++) {
			Get_Palette_Register(index + 1, (RGB_color_ptr)&color);
			Set_Palette_Register(index,     (RGB_color_ptr)&color);
		}

		Set_Palette_Register(255, (RGB_color_ptr)&color_1);
	}

	/* Put the computer back into text mode. */
	Set_Video_Mode(TEXT_MODE);
}
