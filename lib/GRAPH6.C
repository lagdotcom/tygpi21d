/* I N C L U D E S /////////////////////////////////////////////////////// */

#define GRAPH_6_EXPORT

#include <alloc.h>
#include <dos.h>
#include <string.h>
#include "graph3.h"
#include "graph6.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* VGA status register 1 */
#define VGA_INPUT_STATUS_1	0x3DA
	/* bit 3: vsync -
		1: retrace in progress
		0: no retrace */

#define VGA_VSYNC_MASK		0x08
	/* bit 3 */

#define NUM_WORMS		320

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct worm_typ {
	int y;			/* current y position of worm */
	int color;		/* color of worm */
	int speed;		/* speed of worm */
	int counter;	/* counter */
} worm, *worm_ptr;

/* G L O B A L S ///////////////////////////////////////////////////////// */

unsigned char far *double_buffer = NULL;
unsigned int buffer_height = SCREEN_HEIGHT;
unsigned int buffer_size = SCREEN_WIDTH * SCREEN_HEIGHT / 2;

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

void Blit_Char_DB(int xc, int yc, char c, int color, int trans_flag)
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

void Fade_Lights(void)
{
	/* This function fades the lights by slowly decreasing the color values
	in all color registers. */
	int pal_reg, index;
	RGB_color color;

	for (index = 0; index < 30; index++) {
		for (pal_reg = 1; pal_reg <= 255; pal_reg++) {
			/* Get the color to fade. */
			Get_Palette_Register(pal_reg, &color);

			if (color.red > 5) color.red -= 3;
			else color.red = 0;

			if (color.green > 5) color.green -= 3;
			else color.green = 0;

			if (color.blue > 5) color.blue -= 3;
			else color.blue = 0;

			/* Set the color to a diminished intensity. */
			Set_Palette_Register(pal_reg, &color);
		}

		Delay(2);
	}
}

void Dissolve(void)
{
	/* Dissolve the screen by plotting zillions of black pixels. */
	unsigned long index;

	for (index = 0; index <= 300000; index++) {
		Plot_Pixel_Fast(rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, 0);
	}
}

void Melt(void)
{
	/* This function 'melts' the screen by moving little worms at different
	speeds down the screen. These worms change to the color they're eating */
	int index, ticks = 0;
	worm worms[NUM_WORMS];

	/* lag: Clearly, there's something wrong with this function; it only
	initialises half the worms, but then runs off the rest, and it doesn't
	even use the NUM_WORMS constant to refer to the array, except for the
	declaration. I may revisit this later. */

	/* Initialize the worms. */
	for (index = 0; index < 160; index++) {
		worms[index].color   = Get_Pixel(index, 0);
		worms[index].speed   = 3 + rand() % 9;
		worms[index].y       = 0;
		worms[index].counter = 0;

		/* Draw the worm. */
		Plot_Pixel_Fast((index << 1),     0, worms[index].color);
		Plot_Pixel_Fast((index << 1),     1, worms[index].color);
		Plot_Pixel_Fast((index << 1),     2, worms[index].color);

		Plot_Pixel_Fast((index << 1) + 1, 0, worms[index].color);
		Plot_Pixel_Fast((index << 1) + 1, 1, worms[index].color);
		Plot_Pixel_Fast((index << 1) + 1, 2, worms[index].color);
	}

	/* Do the screen melt. */
	while (++ticks < 1800) {
		/* Process each worm. */
		for (index = 0; index < 320; index++) {
			/* Is it time to move the worm? */
			if (++worms[index].counter >= worms[index].speed) {
				/* Reset the counter. */
				worms[index].counter = 0;
				worms[index].color = Get_Pixel(index, worms[index].y + 4);

				/* Has the worm hit bottom? */
				if (worms[index].y < 193) {
					Plot_Pixel_Fast((index << 1),     worms[index].y, 0);
					Plot_Pixel_Fast((index << 1),     worms[index].y + 1,
						worms[index].color);
					Plot_Pixel_Fast((index << 1),     worms[index].y + 2,
						worms[index].color);
					Plot_Pixel_Fast((index << 1),     worms[index].y + 3,
						worms[index].color);

					Plot_Pixel_Fast((index << 1) + 1, worms[index].y, 0);
					Plot_Pixel_Fast((index << 1) + 1, worms[index].y + 1,
						worms[index].color);
					Plot_Pixel_Fast((index << 1) + 1, worms[index].y + 2,
						worms[index].color);
					Plot_Pixel_Fast((index << 1) + 1, worms[index].y + 3,
						worms[index].color);

					worms[index].y++;
				}
			}
		}

		/* Accelerate the melt. */
		if (!(ticks % 500)) {
			for (index = 0; index < 160; index++) {
				worms[index].speed--;
			}
		}
	}
}

void Shear(void)
{
	/* This program 'shears' the screen (for lack of a better word). */
	long index;
	int x, y;

	/* Select the starting point of the shear. */
	x = rand() % SCREEN_WIDTH;
	y = rand() % SCREEN_HEIGHT;

	/* Do it a few times to make sure the whole screen is destroyed. */
	for (index = 0; index < 100000; index++) {
		/* Move the shear. */
		x += 17;		/* note the user of prime numbers */
		y += 13;

		/* Test whether shears are of boundaries. If so, roll them over. */
		if (x > 319) x = x - 319;
		if (y > 199) y = y - 199;

		/* Plot the pixel in black. */
		Plot_Pixel_Fast(x, y, 0);
	}
}
