/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include "graph3.h"
#include "graph6.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* Size of cell-based matrix. */
#define CELL_COLUMNS	10
#define CELL_ROWS		6

/* Size of a cell, in pixels. */
#define CELL_WIDTH		32
#define CELL_HEIGHT		32

/* Number of screens in game. */
#define NUM_SCREENS		6

/* Speed at which the player moves. */
#define ROBO_MOVE		8

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture imagery_pcx,		/* all sprites */
            intro_pcx;			/* intro screen */

sprite back_cells,				/* background cells sprite */
       robopunk;				/* Robopunk! */

/* Use an array of 2-D matrices to hold the screens. */
char **universe[NUM_SCREENS] = {NULL, NULL, NULL, NULL, NULL, NULL};

/* Note: 10x6 cells, where each cell is represented by an ASCII character.
This makes it easier to draw each screen by hand). Later, the ASCII
characters are translated to bitmap IDs so the screen image can be drawn. */
char *screen_1[CELL_ROWS] = {
	"           ",
	"##*###*####",
	"###########",
	"<==========",
	"######:####",
	"####<=;=>##"
};
char *screen_2[CELL_ROWS] = {
	"      ###  ",
	"      #:#  ",
	"#######:###",
	"=======;===",
	"#<==>######",
	"###########"
};
char *screen_3[CELL_ROWS] = {
	"      ##<=>",
	"  #*##<==>#",
	"####*######",
	"===========",
	"###########",
	"###########"
};
char *screen_4[CELL_ROWS] = {
	"###        ",
	"#<=>##     ",
	"####<==>###",
	"===========",
	"###########",
	"#<==>######"
};
char *screen_5[CELL_ROWS] = {
	"   #<=>#   ",
	" #:#***#:##",
	"##:#####:##",
	"==;=====;==",
	"###########",
	"###########"
};
char *screen_6[CELL_ROWS] = {
	"           ",
	"##         ",
	"#*#*##     ",
	"========>  ",
	"#########  ",
	"#########  "
};

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Draw_Screen(char **screen)
{
	/* This function draws a screen by using the data in the universe array.
	Each element in the universe array is a 2-D matrix of cells. These cells
	are ASCII characters that represent the request bit map that should be
	placed in the cell location. */
	char *curr_row;
	int index_x, index_y, cell_number;

	/* Translation table for screen database, used to convert the ASCII
	characters into ID numbers */
	static char back_cell_lookup[] = {
		0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0,
	/* SP  !  "  #  $  %  &  '  (  )  *  +  ,  -  . */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 7, 1, 2, 3, 0,
	/*  /  0  1  2  3  4  5  6  7  8  9  :  ;  <  =  >  ? */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/*  @  A  B  C  D  E  F  G  H  I  J  K  L  M  N  O  P */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	/*  Q  R  S  T  U  V  W  X  Y  Z  [  \  ]  ^  _ */
	};

	/* Clear out the double buffer. */
	Fill_Double_Buffer(0);

	/* Now draw the screen, row by row. */
	for (index_y = 0; index_y < CELL_ROWS; index_y++) {
		/* Get the current row for speed. */
		curr_row = screen[index_y];

		/* Do the row. */
		for (index_x = 0; index_x < CELL_COLUMNS; index_x++) {
			/* Extract the cell out of the data structure and blit it onto
			the screen. */
			cell_number = back_cell_lookup[curr_row[index_x] - 32];

			/* Compute the screen x and y. */
			back_cells.x = index_x * back_cells.width;
			back_cells.y = index_y * back_cells.height;

			/* Figure out which bitmap to draw. */
			back_cells.curr_frame = cell_number;

			/* Draw the bitmap. */
			Draw_Sprite_DB(&back_cells);
		}
	}
}

void Rotate_Lights(void)
{
	/* This function uses color rotation to move the walkway lights. Three
	color registers are used.
	Note: this function has static variables, which track timing parameters
	and also whether the function has been entered yet. */
	static int clock = 0, entered_yet = 0;

	RGB_color color, color_1, color_2, color_3;

	/* This function blinks the running lights on the walkway. */
	if (!entered_yet) {
		/* Reset the palette registers 96, 97, 98 to red, black, black. */
		color.red   = 255;
		color.green = 0;
		color.blue  = 0;
		Set_Palette_Register(96, &color);

		color.red = color.green = color.blue = 0;
		Set_Palette_Register(97, &color);
		Set_Palette_Register(98, &color);

		/* The system has initialized, so flag it. */
		entered_yet = 1;
	}

	/* Try to rotate the light colors; that is, color rotation. */

	/* Is it time to rotate? */
	if (++clock == 3) {
		/* Get the colors. */
		Get_Palette_Register(96, &color_1);
		Get_Palette_Register(97, &color_2);
		Get_Palette_Register(98, &color_3);

		/* Set the colors. */
		Set_Palette_Register(97, &color_1);
		Set_Palette_Register(98, &color_2);
		Set_Palette_Register(96, &color_3);

		/* Reset the clock. */
		clock = 0;
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

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int index,
	    curr_screen = 0,
	    done = 0;

/* S E C T I O N  1 ////////////////////////////////////////////////////// */

	/* Create a double buffer. */
	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

/* S E C T I O N  2 ////////////////////////////////////////////////////// */

	/* Load the intro screen and display it for a few seconds. */
	PCX_Init(&intro_pcx);
	PCX_Load("roboint.pcx", &intro_pcx, 1);
	PCX_Show_Buffer(&intro_pcx);

	/* Let the user see it. */
	Delay(50);
	PCX_Delete(&intro_pcx);

/* S E C T I O N  3 ////////////////////////////////////////////////////// */

	/* Load in the background and animation cells. */
	PCX_Init(&imagery_pcx);
	PCX_Load("robopunk.pcx", &imagery_pcx, 1);

	/* Create a sprite for Robopunk. */
	Sprite_Init(&robopunk, 0, 0, 0, 0, 0, 0, 32, 32);

	/* Create a sprite to hold the background cells. */
	Sprite_Init(&back_cells, 0, 0, 0, 0, 0, 0, 32, 32);

	/* Extract animation cells for Robopunk. */
	PCX_Grab_Bitmap(&imagery_pcx, &robopunk, 0, 3, 0);
	PCX_Grab_Bitmap(&imagery_pcx, &robopunk, 1, 5, 0);
	PCX_Grab_Bitmap(&imagery_pcx, &robopunk, 2, 4, 0);
	PCX_Grab_Bitmap(&imagery_pcx, &robopunk, 3, 5, 0);
	PCX_Grab_Bitmap(&imagery_pcx, &robopunk, 4, 6, 0);
	PCX_Grab_Bitmap(&imagery_pcx, &robopunk, 5, 1, 0);
	PCX_Grab_Bitmap(&imagery_pcx, &robopunk, 6, 2, 0);
	PCX_Grab_Bitmap(&imagery_pcx, &robopunk, 7, 1, 0);
	PCX_Grab_Bitmap(&imagery_pcx, &robopunk, 8, 0, 0);

	/* Extract background cells. */
	for (index = 0; index < 8; index++) {
		PCX_Grab_Bitmap(&imagery_pcx, &back_cells, index, index, 1);
	}

	/* We're done with the PCX file, so obliterate it. */
	PCX_Delete(&imagery_pcx);

/* S E C T I O N  4 ////////////////////////////////////////////////////// */

	/* Set up the universe data structure. */
	universe[0] = (char **)screen_1;
	universe[1] = (char **)screen_2;
	universe[2] = (char **)screen_3;
	universe[3] = (char **)screen_4;
	universe[4] = (char **)screen_5;
	universe[5] = (char **)screen_6;

	Draw_Screen((char **)universe[curr_screen]);
	Show_Double_Buffer();

	/* Place Robopunk. */
	robopunk.x = 160;
	robopunk.y = 74;
	robopunk.curr_frame = 0;

	/* Scan the background under Robopunk. */
	Behind_Sprite_DB(&robopunk);

/* S E C T I O N  5 ////////////////////////////////////////////////////// */

	/* Main event loop. */
	while (!done) {

/* S E C T I O N  6 ////////////////////////////////////////////////////// */

		/* Erase Robopunk. */
		Erase_Sprite_DB(&robopunk);

		/* Test whether the user has pressed a key. */
		if (kbhit()) {

/* S E C T I O N  7 ////////////////////////////////////////////////////// */

			/* Get the key. */
			switch (getch()) {
				case 'a':
					/* Move the player to the left. */

					/* Advance the animation frame and move the player. */
					/* Test whether the player is moving to the right. If so,
					show the player turning before moving. */

					if (robopunk.curr_frame > 0 &&
						robopunk.curr_frame < 5) {
						robopunk.curr_frame = 0;
					} else if (robopunk.curr_frame == 0) {
						robopunk.curr_frame = 5;
					} else {
						/* The player is already in leftward motion,
						so continue. */
						if (++robopunk.curr_frame > 8) {
							robopunk.curr_frame = 5;
						}

						/* Move the player to the left. */
						robopunk.x -= ROBO_MOVE;

						/* Test whether the edge was hit. */
						if (robopunk.x < 8) {
							/* Test whether there's another screen to the
							left. */
							if (curr_screen == 0) {
								robopunk.x += ROBO_MOVE;
							} else {
								/* Warp Robopunk to the other edge of the
								screen, and change screens. */
								robopunk.x = SCREEN_WIDTH - 40;

								/* Scroll to the next screen to the left. */
								curr_screen--;
								Draw_Screen((char **)universe[curr_screen]);
							}
						}
					}

					break;

				case 's':
					/* Move the player to the right. */

					/* Advance the animation frame and move the player. */
					/* Test whether the player is moving to the left. If so,
					show the player turning before moving. */

					if (robopunk.curr_frame > 4) {
						robopunk.curr_frame = 0;
					} else if (robopunk.curr_frame == 0) {
						robopunk.curr_frame = 1;
					} else {
						/* The player is already in rightward motion,
						so continue. */
						if (++robopunk.curr_frame > 4) {
							robopunk.curr_frame = 1;
						}

						/* Move the player to the right. */
						robopunk.x += ROBO_MOVE;

						/* Test whether the edge was hit. */
						if (robopunk.x > SCREEN_WIDTH - 40) {
							/* Test whether there's another screen to the
							right. */
							if (curr_screen == 5) {
								robopunk.x -= ROBO_MOVE;
							} else {
								/* Warp Robopunk to the other edge of the
								screen, and change screens. */
								robopunk.x = 8;

								/* Scroll to the next screen to the right. */
								curr_screen++;
								Draw_Screen((char **)universe[curr_screen]);
							}
						}
					}

					break;

				case 'q':
					/* Exit the demo. */
					done = 1;
					break;

				default: break;
			}
		}

/* S E C T I O N  8 ////////////////////////////////////////////////////// */

		/* Scan the background under Robopunk. */
		Behind_Sprite_DB(&robopunk);

		/* Draw him. */
		Draw_Sprite_DB(&robopunk);

/* S E C T I O N  9 ////////////////////////////////////////////////////// */

		/* Move the walkway lights. */
		Rotate_Lights();

		/* lag: brief help message */
		Blit_String_DB(0, 0, 2, "A< S> Q!", 1);

		/* Show the double buffer. */
		Show_Double_Buffer();

		/* Wait a bit... */
		Delay(1);
	}

/* S E C T I O N  10 ///////////////////////////////////////////////////// */

	/* Use one of the screen effects as the exit. */
	Dissolve();

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);

	/* Free the double buffer. */
	Delete_Double_Buffer();
}
