/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <dos.h>
#include <conio.h>
#include <stdlib.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NUM_STARS   75

#define PLANE_1     1
#define PLANE_2     2
#define PLANE_3     3

#define VGA256    0x13          /* 320x200x256 */
#define TEXT_MODE 0x03          /* The default text mode */

#define CHAR_WIDTH  8
#define CHAR_HEIGHT 8

/* mode 13h screen dimensions */
#define SCREEN_WIDTH        (unsigned int)320
#define SCREEN_HEIGHT       (unsigned int)200

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct star_type {
	int x, y;       /* position */
	int plane;
	int color;
} star, *star_ptr;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Blit_Char(int cx, int yc, char c, int color, int trans_flag);
void Blit_String(int x, int y, int color, char *string, int trans_flag);
void Plot_Pixel_Fast(int x, int y, unsigned char color);
void Init_Stars(void);
void Set_Video_Mode(int mode);
void Delay(int clicks);

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* vram byte ptr */
unsigned char far *video_buffer = (char far *)0xA0000000L;

/* 8x8 ROM characters */
unsigned char far *rom_char_set = (char far *)0xF000FA6EL;

star stars[NUM_STARS];

int velocity_1 = 2,         /* speeds of each plane */
	velocity_2 = 4,
	velocity_3 = 6;

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

void Init_Stars(void)
{
	/* This function initializes the starfield. */
	int index;

	/* For each star, choose a position, plane, and color. */
	for (index = 0; index < NUM_STARS; index++) {
		/* Initialize each star to a velocity, position, and color. */
		stars[index].x = rand() % 320;
		stars[index].y = rand() % 160;

		/* Decide what star plane the star is in */
		switch (rand() % 3) {
			case 0:
				/* far plane */
				stars[index].plane = PLANE_1;
				stars[index].color = 8;
				break;

			case 1:
				/* mid-distance plane */
				stars[index].plane = PLANE_2;
				stars[index].color = 7;
				break;

			case 2:
				/* near plane */
				stars[index].plane = PLANE_3;
				stars[index].color = 15;
				break;
		}
	}
}

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
	unsigned int far *clock = (unsigned int far *)0x0000046CL;
	unsigned int now;

	/* Get the current time */
	now = *clock;

	/* Wait until the time has gone past the current time plus the amount we
	wanted to wait. Note that each tick is approximately 55 milliseconds. */
	while(abs(*clock - now) < clicks) {}
}

/* /////////////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0,       /* exit flag */
		index;          /* loop index */

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Initialize the starfield data structure. */
	Init_Stars();

	/* Begin the main event loop. */
	while (!done) {
		/* Test whether the user is trying to do something. */
		if (kbhit()) {
			/* What key was pressed? */

			switch (getch()) {
				case '-':
					/* Slow the starfield down. */
					velocity_1 -= 1;
					velocity_2 -= 2;
					velocity_3 -= 3;
					break;

				case '=':
					/* Speed the starfield up. */
					velocity_1 += 1;
					velocity_2 += 2;
					velocity_3 += 3;
					break;

				case 'q':
					/* The user is exiting. */
					done = 1;
					break;

				default: break;
			}
		}

		/* Move the starfields. */
		for (index = 0; index < NUM_STARS; index++) {
			/* Erase the star. */
			Plot_Pixel_Fast(stars[index].x, stars[index].y, 0);

			/* Move the star and test for off-screen condition. */

			/* Each star is in a different plane, so test which plane the
			star is in so that proper velocity can be used. */
			switch (stars[index].plane) {
				case PLANE_1:
					stars[index].x += velocity_1;
					break;

				case PLANE_2:
					stars[index].x += velocity_2;
					break;

				case PLANE_3:
					stars[index].x += velocity_3;
					break;
			}

			/* Test whether the star went off screen. */
			if (stars[index].x > 319) {
				/* Off right edge? */
				stars[index].x = stars[index].x - 320;
			} else if (stars[index].x < 0) {
				/* Off left edge? */
				stars[index].x = 320 + stars[index].x;
			}

			/* Draw the star at the new position. */
			Plot_Pixel_Fast(stars[index].x, stars[index].y, stars[index].color);
		}

		/* Draw the directions again. */
		Blit_String(0, 0, 1, "Press '+' or '-' to change speed.", 1);
		Blit_String(88, 180, 2, "Press 'Q' to exit.", 1);

		/* Wait a second so we can see the stars; otherwise, it'll look like
		warp speed! */
		Delay(1);
	}

	/* Put the computer back into text mode. */
	Set_Video_Mode(TEXT_MODE);
}
