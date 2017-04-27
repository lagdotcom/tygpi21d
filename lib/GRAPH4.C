/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <alloc.h>
#include <stdio.h>
#include "graph3.h"
#include "graph4.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void PCX_Init(pcx_picture_ptr image)
{
	/* This function allocates the buffer region needed to load a
	PCX file. */
	if (!(image->buffer = (char far*)
		farmalloc(SCREEN_WIDTH * SCREEN_HEIGHT + 1))) {
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
