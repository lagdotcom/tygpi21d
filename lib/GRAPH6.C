/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <alloc.h>
#include <dos.h>
#include <string.h>
#include "graph3.h"
#include "graph6.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

static unsigned char far *double_buffer = NULL;
static unsigned int buffer_height = SCREEN_HEIGHT;
static unsigned int buffer_size = SCREEN_WIDTH * SCREEN_HEIGHT / 2;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

int Create_Double_Buffer(int num_lines)
{
	/* Allocate enough memory to hold the double buffer. */
	if ((double_buffer = (unsigned char far*)
		farmalloc(SCREEN_WIDTH * (num_lines + 1))) == NULL) {
		return 0;
	}

	/* Set the height of the buffer and compute its size. */
	buffer_height = num_lines;
	buffer_size = SCREEN_WIDTH * num_lines / 2;

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
	/* This function copies the double buffer into the video buffer. */
	asm push ds;                    /* save ds on stack */
	asm mov cx, buffer_size;		/* this is the size in WORDs */
	asm les di, video_buffer;		/* es:di is the destination */
	asm lds si, double_buffer;		/* es:si is the source */
	asm cld;						/* make sure to move forward */
	asm rep movsw;					/* move all the WORDs */
	asm pop ds;						/* restore ds */
}

void Delete_Double_Buffer(void)
{
	/* This function frees up the memory allocated by the double buffer.
	Be sure to use FAR version. */
	if (double_buffer) {
		farfree(double_buffer);
		double_buffer = NULL;		/* lag: cleanup! */
	}
}

void Plot_Pixel_Fast_DB(int x, int y, unsigned char color)
{
	/* Plots the pixel in the desired color a little quicker using binary
	shifting to accomplish the multiplications. */

	/* Use the fact that 320*y = 256*y + 64*y = y<<8 + y<<6 */
	double_buffer[((y << 8) + (y << 6)) + x] = color;
}

void Behind_Sprite_DB(sprite_ptr sprite)
{
	/* This function scans the background behind a sprite so that,
	when the sprite is drawn, the background isn't obliterated. */
	char far *work_back;
	int work_offset = 0, offset, y;

	/* Alias a pointer to the background for ease of access. */
	work_back = sprite->background;

	/* Compute the offset of the sprite in the video buffer. */
	offset = (sprite->y << 8) + (sprite->y << 6) + sprite->x;

	for (y = 0; y < sprite->height; y++) {
		/* Copy the next row out of the screen buffer into the
		sprite background buffer. */
		memcpy((char far *)&work_back[work_offset],
			   (char far *)&double_buffer[offset],
			   sprite->width);

		/* Move to the next line in the video buffer and in the
		sprite bitmap buffer. */
		offset      += SCREEN_WIDTH;
		work_offset += sprite->width;
	}
}

void Erase_Sprite_DB(sprite_ptr sprite)
{
	/* Replace the background that was behind the sprite. */

	/* This function replaces the background that was saved from
	where a sprite was going to be placed. */
	char far *work_back;
	int work_offset = 0, offset, y;

	/* Alias a pointer to the background for ease of access. */
	work_back = sprite->background;

	/* Compute the offset of the sprite in the video buffer. */
	offset = (sprite->y << 8) + (sprite->y << 6) + sprite->x;

	for (y = 0; y < sprite->height; y++) {
		/* Copy the next row out of the screen buffer into the
		sprite background buffer. */
		memcpy((char far *)&double_buffer[offset],
			   (char far *)&work_back[work_offset],
			   sprite->width);

		/* Move to the next line in the video buff and in the
		sprite bitmap buffer. */
		offset      += SCREEN_WIDTH;
		work_offset += sprite->width;
	}
}

void Draw_Sprite_DB(sprite_ptr sprite)
{
	/* This function draws a sprite on the screen, row by row,
	really quickly. Note the user of shifting to implement
	multiplication. */
	char far *work_sprite;
	int work_offset = 0, offset, x, y;
	char data;

	/* Alias a pointer to the sprite for ease of access. */
	work_sprite = sprite->frames[sprite->curr_frame];

	/* Compute the offset of the sprite in the video buffer. */
	offset = (sprite->y << 8) + (sprite->y << 6) + sprite->x;

	for (y = 0; y < sprite->height; y++) {
		for (x = 0; x < sprite->width; x++) {
			/* Test for a transparent pixel; that is, 0. If the
			pixel is not transparent, draw it. */
			data = work_sprite[work_offset + x];

			if (data) {
				double_buffer[offset + x] = data;
			}
		}

		/* Move to the next line in the video buffer and in the
		sprite bitmap buffer. */
		offset      += SCREEN_WIDTH;
		work_offset += sprite->width;
	}
}

void Draw_Sprite_Clipped_DB(sprite_ptr sprite, int min_x, int min_y,
	int max_x, int max_y)
{
	/* This function draws a sprite into the double buffer and clips it to
	the sent clipping boundary. The function is drawn out to show each step
	clearly. */
	char far *work_sprite;
	int work_offset = 0,
	    offset, x_off,
	    x, y, xs, ys,
	    xe, ye,
	    clip_width, clip_height;
	char data;

	/* Extract the sprite position. */
	xs = sprite->x;
	ys = sprite->y;

	/* Compute the end of the sprite bounding box in screen coordinates. */
	xe = xs + sprite->width  - 1;
	ye = ys + sprite->height - 1;

	/* Test whether the sprite is totally invisible; that is, whether the
	sprite is beyond the screen boundaries. */
	if ((xs >= max_x) ||
		(ys >= max_y) ||
		(xs <= (min_x - sprite->width)) ||
		(ys <= (min_y - sprite->height))) {
		/* lag: shouldn't this use xe and ye? */
		return;
	}

	/* The sprite must be partially visible. Therefore, compute the region
	that must be drawn. */

	/* Clip in the x direction. */
	if (xs < min_x) xs = min_x;
	else if (xe >= max_x) xe = max_x - 1;

	/* Clip in the y direction. */
	if (ys < min_y) ys = min_y;
	else if (ye >= max_y) ye = max_y - 1;

	/* Compute the new width and height. */
	clip_width  = xe - xs + 1;
	clip_height = ye - ys + 1;

	/* Compute the working offsets based on the new starting y. */
	work_offset = (sprite->y - ys) * sprite->width;
	x_off = xs - sprite->x;

	/* Now render the clipped sprite. */
	/* Alias the pointer to the sprite for ease of access. */
	work_sprite = sprite->frames[sprite->curr_frame];

	/* Compute the offset of the sprite in the video buffer. */
	offset = (ys << 8) + (ys << 6) + xs;

	for (y = 0; y < clip_height; y++) {
		for (x = 0; x < clip_width; x++) {
			/* Test for a transparent pixel; that is, 0. If the pixel is not
			transparent, draw it. */
			data = work_sprite[work_offset + x];

			if (data) {
				double_buffer[offset + x + x_off] = data;
			}
		}

		offset      += SCREEN_WIDTH;
		work_offset += sprite->width;
	}
}

void Wait_For_Vsync(void)
{
	/* This function waits for the start of a vertical retrace. If a vertical
	retrace is in progress, it waits until the next one. */

	while (inp(VGA_INPUT_STATUS_1) & VGA_VSYNC_MASK) {
		/* Do nothing: VGA is in retrace. */
	}

	while (!(inp(VGA_INPUT_STATUS_1) & VGA_VSYNC_MASK)) {
		/* Do nothing: wait for start of retrace. */
	}

	/* At this point a vertical retrace has just started. */
}
