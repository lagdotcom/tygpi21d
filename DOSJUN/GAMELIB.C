/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <bios.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "djgpp.h"
#include "gamelib.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* VGA status register 1 */
#define VGA_INPUT_STATUS_1	0x3DA
	/* bit 3: vsync -
		1: retrace in progress
		0: no retrace */

#define VGA_VSYNC_MASK		0x08
	/* bit 3 */

#define _KEYBRD_READ			0
#define _KEYBRD_READY			1
#define _KEYBRD_SHIFTSTATUS		2

/* G L O B A L S ///////////////////////////////////////////////////////// */

unsigned char *double_buffer = NULL;
unsigned int buffer_height = SCREEN_HEIGHT;
unsigned int buffer_size = SCREEN_WIDTH * SCREEN_HEIGHT;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Set_Video_Mode(int mode)
{
	/* Use video interrupt 10h to set the video mode to the sent value */
	union REGS inregs, outregs;

	inregs.h.ah = 0;                    /* Set the video mode subfunction. */
	inregs.h.al = (unsigned char)mode;  /* Video mode to which to change. */

	int86(0x10, &inregs, &outregs);
}

void Delay(int clicks)
{
	/* This function uses the internal timekeeper (the one that runs at 18.2
	clicks/sec) to time a delay. You can find the 32-bit value of this timer
	at 0000:046Ch. */
	volatile unsigned int *clock;
	unsigned int now;

	if (!Enter_Nearptr_Mode()) return;
	clock = (unsigned int *)Get_Nearptr(0x046C);

	/* Get the current time */
	now = *clock;

	/* Wait until the time has gone past the current time plus the amount we
	wanted to wait. Note that each tick is approximately 55 milliseconds. */
	while(abs(*clock - now) < clicks) {}

	Leave_Nearptr_Mode();
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

void PCX_Init(pcx_picture_ptr image)
{
	/* This function allocates the buffer region needed to load a
	PCX file. */
	image->buffer = (char *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT + 1);
	if (!image->buffer) {
		printf("\ncouldn't allocate screen buffer");
	}
}

void PCX_Load(char *filename, pcx_picture_ptr image, int enable_palette)
{
	/* This function loads a PCX file into a picture structure. The actual
	image data for the PCX file is decompressed and expanded into a
	secondary buffer within the picture structure. The separate images can
	be grabbed from this buffer later. Also, the header and palette are
	loaded. */
	FILE *fp;
	int num_bytes, index;
	long count;
	unsigned char data;
	char *temp_buffer;

	/* Open the file. */
	fp = fopen(filename, "rb");

	/* Load the header */
	temp_buffer = (char *)image;
	for (index = 0; index < 128; index++) {
		temp_buffer[index] = (char)getc(fp);
	}

	/* Load the data and decompress it into the buffer. */
	count = 0;
	while (count <= SCREEN_WIDTH * SCREEN_HEIGHT) {
		/* Get the first piece of data. */
		data = (unsigned char)getc(fp);

		/* Is this an RLE? */
		if (data >= 192 && data <= 255) {
			/* How many bytes in this run? */
			num_bytes = data - 192;

			/* Get the actual data for the run. */
			data = (unsigned char)getc(fp);

			/* Replicate the data in the buffer. */
			while (num_bytes-- > 0) {
				image->buffer[count++] = data;
			}
		} else {
			/* Actual data; just copy it into the buffer. */
			image->buffer[count++] = data;
		}
	}

	/* Move to the end of the file, then back up 768 bytes to the beginning
	of the palette. */
	fseek(fp, -768L, SEEK_END);

	/* Load the palette into the VGA palette registers. */
	for (index = 0; index < 256; index++) {
		image->palette[index].red   = (unsigned char)(getc(fp) >> 2);
		image->palette[index].green = (unsigned char)(getc(fp) >> 2);
		image->palette[index].blue  = (unsigned char)(getc(fp) >> 2);
	}

	fclose(fp);

	/* Change the palette to the newly loaded palette if commanded. */
	if (enable_palette) {
		/* For each palette register, set it to the new color values. */
		for (index = 0; index < 256; index++) {
			Set_Palette_Register(index, &image->palette[index]);
		}
	}
}

void PCX_Delete(pcx_picture_ptr image)
{
	/* This function deallocates the buffer region used for the PCX file
	load. */
	free(image->buffer);
}

int Create_Double_Buffer(int num_lines)
{
	/* Allocate enough memory to hold the double buffer. */
	if ((double_buffer = (unsigned char*)
		malloc(SCREEN_WIDTH * (num_lines + 1))) == NULL) {
		return 0;
	}

	/* Set the height of the buffer and compute its size. */
	buffer_height = num_lines;
	buffer_size = SCREEN_WIDTH * num_lines;

	/* Fill the buffer with black. */
	memset(double_buffer, 0, SCREEN_WIDTH * num_lines);

	/* Everything was OK. */
	return 1;
}

void Fill_Double_Buffer(int color)
{
	/* This function fills in the double buffer with the sent color. */
	memset(double_buffer, color, SCREEN_WIDTH * buffer_height);
}

/* lag: this originally took a pointer to the buffer */
void Show_Double_Buffer(void)
{
	char *video_buffer;

	if (!Enter_Nearptr_Mode()) return;

	video_buffer = Get_Nearptr(0xA0000);
	memcpy(video_buffer, double_buffer, buffer_size);

	Leave_Nearptr_Mode();
}

void Delete_Double_Buffer(void)
{
	/* This function frees up the memory allocated by the double buffer.
	Be sure to use FAR version. */
	if (double_buffer != NULL) {
		free(double_buffer);
		double_buffer = NULL;		/* lag: cleanup! */
	}
}

void Blit_Char_DB(int xc, int yc, char c, int color, int trans_flag)
{
	/* This function uses the ROM 8x8 character set to blit a character on
	the video screen. Notice the trick used to extract bits out of each
	character byte that comprises a line. */
	int offset, x, y;
	char *work_char;
	unsigned char bit_mask = 0x80;

	if (!Enter_Nearptr_Mode()) return;

	/* Compute the starting offset in the ROM character look-up table. */
	work_char = Get_Nearptr(0xFFA6E + c * CHAR_HEIGHT);

	/* Compute the offset of the character in the video buffer. */
	offset = (yc << 8) + (yc << 6) + xc;

	for (y = 0; y < CHAR_HEIGHT; y++) {
		/* Reset the bit mask. */
		bit_mask = 0x80;

		for (x = 0; x < CHAR_WIDTH; x++) {
			/* Test for a transparent pixel; that is, 0. If the pixel is not
			transparent, draw it. */
			if ((*work_char & bit_mask)) {
				double_buffer[offset + x] = color;
			} else if (!trans_flag) {
				/* or plot black if not transparent */
				double_buffer[offset + x] = 0;
			}

			/* Shift the bit mask. */
			bit_mask = bit_mask >> 1;
		}

		/* Move to the next line in the cideo buffer and in the ROM character
		data area. */
		offset += SCREEN_WIDTH;
		work_char++;
	}

	Leave_Nearptr_Mode();
}

void Blit_String_DB(int x, int y, int color, char *string, int trans_flag)
{
	/* This function blits an entire string on the screen with fixed spacing
	between each character. It calls blit_char. */
	int index;

	for (index = 0; string[index] != 0; index++) {
		Blit_Char_DB(
			x + (index << 3),
			y,
			string[index],
			color,
			trans_flag
		);
	}
}

void Plot_Pixel_Fast_DB(int x, int y, unsigned char color)
{
	/* Plots the pixel in the desired color a little quicker using binary
	shifting to accomplish the multiplications. */

	/* Use the fact that 320*y = 256*y + 64*y = y<<8 + y<<6 */
	double_buffer[((y << 8) + (y << 6)) + x] = color;
}

unsigned int Get_Control_Keys(unsigned int mask)
{
	/* Return the status of all requested control keys. */
	return (mask & bioskey(_KEYBRD_SHIFTSTATUS));
}

unsigned char Get_Ascii_Key(void)
{
	/* If there's a normal ASCII key waiting, return it;
	otherwise, return 0. */
	if (bioskey(_KEYBRD_READY)) {
		return bioskey(_KEYBRD_READ);
	} else {
		return 0;
	}
}

unsigned char Get_Scan_Code(void)
{
	/* If there's a scan code waiting, return it;
	otherwise, return 0. */
	if (bioskey(_KEYBRD_READY)) {
		return bioskey(_KEYBRD_READ) >> 8;
	} else {
		return 0;
	}
}
