/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "graph3.h"
#include "graph6.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define HELP		"Press 'Q' to Quit, '<' '>' to Scale."

/* G L O B A L S ///////////////////////////////////////////////////////// */

sprite object;
pcx_picture imagery_pcx;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Scale_Sprite(sprite_ptr sprite, float scale)
{
	/* This function scales a sprite by computing the number of source pixels
	neded to satisfy the number of destination pixels.
	Note: this function works in the double buffer. */
	char far *work_sprite;

	int work_offset = 0,
		offset,
		x, y;

	unsigned char data;

	float y_scale_index,
	      x_scale_step,
	      y_scale_step,
	      x_scale_index;

	/* Set the first source pixel. */
	y_scale_index = 0;

	/* Compute the floating-point step. */
	y_scale_step = sprite->height / scale;
	x_scale_step = sprite->width / scale;

	/* Alias a pointer to the sprite for ease of access. */
	work_sprite = sprite->frames[sprite->curr_frame];

	/* Compute the offset of the sprite in the video buffer. */
	offset = (sprite->y << 8) + (sprite->y << 6) + sprite->x;

	/* Scale the object row by row. */
	for (y = 0; y < (int)scale; y++) {
		/* Copy the next row into the screen buffer using memcpy. */
		x_scale_index = 0;

		for (x = 0; x < (int)scale; x++) {
			/* Test for a transparent pixel; that is, 0. If the pixel is not
			transparent, draw it. */
			data = work_sprite[work_offset + (int)x_scale_index];
			if (data) {
				double_buffer[offset + x] = data;
			}

			x_scale_index += x_scale_step;
		}

		/* Using the floating scale_step, index to the next source pixel. */
		y_scale_index += y_scale_step;

		/* Move to the next line in the video buffer and in the sprite
		bit-map buffer. */
		offset      += SCREEN_WIDTH;
		work_offset = sprite->width * (int)y_scale_index;
	}
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0;		/* exit flag */
	char buffer[128];	/* used to build up info string */
	float scale = 32;	/* initial object scale */

	/* Create a double buffer. */
	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Initialize the PCX file that holds all the animation cells. */
	PCX_Init(&imagery_pcx);

	/* Load the PCX file that holds the cells. */
	PCX_Load("robopunk.pcx", &imagery_pcx, 1);
	Sprite_Init(&object, 0, 0, 0, 0, 0, 0, 32, 32);

	/* Grab the bit map. */
	PCX_Grab_Bitmap(&imagery_pcx, &object, 0, 3, 0);

	/* Initialize the sprite. */
	object.curr_frame = 0;
	object.x          = 160 - (object.width >> 1);
	object.y          = 100 - (object.height >> 1);

	/* Clear the double buffer. */
	Fill_Double_Buffer(0);

	/* Show the user the scaled texture. */
	Scale_Sprite(&object, scale);
	Show_Double_Buffer();
	Blit_String(0, 190, 10, HELP, 1);

	/* Main loop. */
	while (!done) {
		/* Has the user hit a key? */
		if (kbhit()) {
			switch (getch()) {
				case '.':
					/* Scale the object larger. */
					if (scale < 180) {
						scale += 4;
						object.x -= 2;
						object.y -= 2;
					}
					break;

				case ',':
					/* Scale the object smaller. */
					if (scale > 4) {
						scale -= 4;
						object.x += 2;
						object.y += 2;
					}
					break;

				case 'q':
					/* Let's go! */
					done = 1;
					break;

				default: break;
			}

			/* Create a clean slate. */
			Fill_Double_Buffer(0);

			/* Scale the sprite and render it into the double buffer. */
			Scale_Sprite(&object, scale);

			/* Show the double buffer. */
			Show_Double_Buffer();
			Blit_String(0, 190, 10, HELP, 1);
			sprintf(buffer, "Current scale = %f  ", scale / 32);
			Blit_String(0, 8, 10, buffer, 1);
		}
	}

	/* Delete the PCX file. */
	PCX_Delete(&imagery_pcx);

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);

	/* Delete the double buffer. */
	Delete_Double_Buffer();
}
