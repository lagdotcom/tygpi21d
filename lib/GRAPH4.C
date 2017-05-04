/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <alloc.h>
#include <stdio.h>
#include <string.h>
#include "graph3.h"
#include "graph4.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void PCX_Init(pcx_picture_ptr image)
{
	/* This function allocates the buffer region needed to load a
	PCX file. */
	image->buffer = (char far *)farmalloc(SCREEN_WIDTH * SCREEN_HEIGHT + 1);
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
	char far *temp_buffer;

	/* Open the file. */
	fp = fopen(filename, "rb");

	/* Load the header */
	temp_buffer = (char far*)image;
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
	farfree(image->buffer);
}

void PCX_Show_Buffer(pcx_picture_ptr image)
{
	/* Just copy the PCX buffer into the video buffer. */
	char far *data;

	data = image->buffer;

	asm push ds;                    /* save the data segment */
	asm les di, video_buffer;       /* point es:di to video buffer */
	asm lds si, data;               /* point ds:si to data area */
	asm mov cx, 320*200/2;          /* move 32000 words */
	asm cld;                        /* set direction to forward */
	asm rep movsw;                  /* do the string operation */
	asm pop ds;                     /* restore the data segment */
}

void PCX_Grab_Bitmap(pcx_picture_ptr image, sprite_ptr sprite,
	int sprite_frame, int grab_x, int grab_y)
{
	/* This function grabs a bitmap from the PCX frame buffer. It uses
	the convention that the 320x200-pixel matrix is subdivided into a
	smaller matrix of NxN adjacent squares. */
	int x_off, y_off, x, y;
	char far *sprite_data;

	/* First, allocate the memory for the sprite in the structure. */
	sprite->frames[sprite_frame] = (char far*)
		farmalloc(sprite->width * sprite->height + 1);

	/* Create an alias to the sprite frame for each of access. */
	sprite_data = sprite->frames[sprite_frame];

	/* Now load the sprite data into the sprite frame array from the
	PCX picture. */
	x_off = sprite->width  * grab_x;
	y_off = sprite->height * grab_y;

	/* lag: I changed all the +1 here because I don't want to put
	borders around all of my sprites */

	/* Compute the starting y address. */
	y_off = y_off * (image->header.width + 1);

	for (y = 0; y < sprite->height; y++) {
		for (x = 0; x < sprite->width; x++) {
			/* Get the next byte of the current row and place it into
			the next position in the sprite frame data buffer. */
			sprite_data[y * sprite->width + x] =
				image->buffer[y_off + x_off + x];
		}

		y_off += image->header.width + 1;
	}

	sprite->num_frames++;
}

void Sprite_Init(sprite_ptr sprite, int x, int y, int ac, int as,
	int mc, int ms, int width, int height)
{
	/* This function initializes a sprite with the sent data. */
	int index;

	sprite->x            = x;
	sprite->y            = y;
	sprite->x_old        = x;
	sprite->y_old        = y;
	sprite->width        = width;
	sprite->height       = height;
	sprite->anim_clock   = ac;
	sprite->anim_speed   = as;
	sprite->motion_clock = mc;
	sprite->motion_speed = ms;
	sprite->curr_frame   = 0;
	sprite->state        = SPRITE_DEAD;
	sprite->num_frames   = 0;
	sprite->background   = (char far *)farmalloc(width * height + 1);

	/* Set all bitmap pointers to null. */
	for (index = 0; index < MAX_SPRITE_FRAMES; index++) {
		sprite->frames[index] = NULL;
	}
}

void Sprite_Delete(sprite_ptr sprite)
{
	/* This function deletes all the memory associated with
	a sprite. */
	int index;

	farfree(sprite->background);

	for (index = 0; index < MAX_SPRITE_FRAMES; index++) {
		farfree(sprite->frames[index]);
	}
}

void Draw_Sprite(sprite_ptr sprite)
{
	/* This function draws a sprite on the screen, row by row,
	really quickly. Note the user of shifting to implement
	multiplication. */
	char far *work_sprite;
	int work_offset = 0, offset, x, y;
	unsigned char data;

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
				video_buffer[offset + x] = data;
			}
		}

		/* Move to the next line in the video buffer and in the
		sprite bitmap buffer. */
		offset      += SCREEN_WIDTH;
		work_offset += sprite->width;
	}
}

void Behind_Sprite(sprite_ptr sprite)
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
			   (char far *)&video_buffer[offset],
			   sprite->width);

		/* Move to the next line in the video buffer and in the
		sprite bitmap buffer. */
		offset      += SCREEN_WIDTH;
		work_offset += sprite->width;
	}
}

void Erase_Sprite(sprite_ptr sprite)
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
		memcpy((char far *)&video_buffer[offset],
			   (char far *)&work_back[work_offset],
			   sprite->width);

		/* Move to the next line in the video buff and in the
		sprite bitmap buffer. */
		offset      += SCREEN_WIDTH;
		work_offset += sprite->width;
	}
}

int Sprite_Collide(sprite_ptr sprite_1, sprite_ptr sprite_2)
{
	/* This function tests the bounding boxes of each sprite to
	see whether a collision has occurred. IF so, a 1 is returned;
	otherwise, a 0 is returned. */
	int dx, dy;
	int avgw, avgh; /* lag */

	/* Compute the amount of overlap, if any. */
	dx = abs(sprite_1->x - sprite_2->x);
	dy = abs(sprite_1->y - sprite_2->y);

	/* lag: compute the average sprite size, so we can compare
	against that instead of assuming both sprites are equal size */
	avgw = (sprite_1->width  + sprite_2->width)  >> 1;
	avgh = (sprite_1->height + sprite_2->height) >> 1;

	/* Test the x and y extents. Note how the width and height are
	decreased by a percentage of the actual width and height. This
	is to make the bounding box a little more realistic, as very
	seldom will an object be rectangular. This helps to ensure
	that there's a solid collision. */

	/* lag: might be a good idea to make this scaling factor
	configurable? */
	if (dx < (avgw - (avgw >> 3)) && dy < (avgh - (avgh >> 3))) {
		return 1;
	} else {
		return 0;
	}
}
