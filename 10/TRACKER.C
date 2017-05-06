/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* States the spider (tracker) can be in */
#define TRACKER_ATTACK	0
#define TRACKER_EVADE	1

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture imagery_pcx,
	background_pcx;

sprite player, tracker;

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0;

/* // S E C T I O N  1 /////////////////////////////////////////////////// */

	/* Create a double buffer. */
	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	/* Clear the double buffer. */
	Fill_Double_Buffer(0);

	/* Set the video mode to the 320x200, 256-color mode. */
	Set_Video_Mode(VGA256);

	/* Load the background image into the double buffer. */
	PCX_Init(&background_pcx);
	PCX_Load("trackbak.pcx", &background_pcx, 1);

	/* Copy the background into the double buffer. */
	memcpy(double_buffer, background_pcx.buffer,
		SCREEN_WIDTH * SCREEN_HEIGHT);

	PCX_Delete(&background_pcx);

	Blit_String_DB(0, 0, 10, "Q to exit, T to toggle.", 1);

/* // S E C T I O N  2 /////////////////////////////////////////////////// */

	/* Load in imagery for player. */
	PCX_Init(&imagery_pcx);
	PCX_Load("trackimg.pcx", &imagery_pcx, 0);

	/* Initialize the player and extract the bitmaps. */
	Sprite_Init(&player, 0, 0, 0, 0, 0, 0, 24, 24);
	PCX_Grab_Bitmap(&imagery_pcx, &player, 0, 0, 0);
	PCX_Grab_Bitmap(&imagery_pcx, &player, 1, 1, 0);

	player.x          = 160;
	player.y          = 180;
	player.curr_frame = 0;
	player.state      = 1;

	/* Initialize the tracker and extract the bitmaps. */
	Sprite_Init(&tracker, 0, 0, 0, 0, 0, 0, 24, 24);
	PCX_Grab_Bitmap(&imagery_pcx, &tracker, 0, 0, 1);
	PCX_Grab_Bitmap(&imagery_pcx, &tracker, 1, 1, 1);

	tracker.x          = 20;
	tracker.y          = 20;
	tracker.curr_frame = TRACKER_ATTACK;
	tracker.state      = TRACKER_ATTACK;

/* // S E C T I O N  3 /////////////////////////////////////////////////// */

	/* Scan behind all objects before entering the event loop. */
	Behind_Sprite_DB(&player);
	Behind_Sprite_DB(&tracker);

	while (!done) {
		/* Erase all objects. */
		Erase_Sprite_DB(&player);
		Erase_Sprite_DB(&tracker);

/* // S E C T I O N  4 /////////////////////////////////////////////////// */

		/* Do movement of spider based on mode. */
		if (tracker.state == TRACKER_ATTACK) {
			/* Move the spider toward the player. */

			/* First take care of the x components. */
			if (player.x > tracker.x)		tracker.x += 2;
			else if (player.x < tracker.x)	tracker.x -= 2;

			/* First take care of the y components. */
			if (player.y > tracker.y)		tracker.y += 2;
			else if (player.y < tracker.y)	tracker.y -= 2;
		} else {
			/* Move the spider away from the player. */

			/* First take care of the x components. */
			if (player.x > tracker.x)		tracker.x -= 2;
			else if (player.x < tracker.x)	tracker.x += 2;

			/* First take care of the y components. */
			if (player.y > tracker.y)		tracker.y -= 2;
			else if (player.y < tracker.y)	tracker.y += 2;
		}

/* // S E C T I O N  5 /////////////////////////////////////////////////// */

		/* Do a boundary collision for the spider. */
		if (tracker.x < 0)			tracker.x = 0;
		else if (tracker.x > 194)	tracker.x = 194;

		if (tracker.y < 0)			tracker.y = 0;
		else if (tracker.y > 174)	tracker.y = 174;

/* // S E C T I O N  6 /////////////////////////////////////////////////// */

		/* See whether player is trying to move. */
		if (kbhit()) {
			/* Which key? */
			switch (getch()) {
				/* Use the numeric keypad for movement.
				Note: NumLock must be activated. */

				case '1':
					player.x -= 4;
					player.y += 4;
					break;

				case '2':
					player.y += 4;
					break;

				case '3':
					player.x += 4;
					player.y += 4;
					break;

				case '4':
					player.x -= 4;
					break;

				case '6':
					player.x += 4;
					break;

				case '7':
					player.x -= 4;
					player.y -= 4;
					break;

				case '8':
					player.y -= 4;
					break;

				case '9':
					player.x += 4;
					player.y -= 4;
					break;

				case 't':
					/* Toggle attack mode. */
					if (tracker.state == TRACKER_ATTACK) {
						tracker.state = tracker.curr_frame = TRACKER_EVADE;
					} else {
						tracker.state = tracker.curr_frame = TRACKER_ATTACK;
					}
					break;

				case 'q':
					done = 1;
					break;

				default: break;
			}

/* // S E C T I O N  8 /////////////////////////////////////////////////// */

			/* Do a boundary collision for the spider. */
			if (player.x < 0)			player.x = 304;
			else if (player.x > 304)	player.x = 0;

			if (player.y < 0)			player.y = 184;
			else if (player.y > 184)	player.y = 0;
		}

/* // S E C T I O N  9 /////////////////////////////////////////////////// */

		/* Do animation. */
		if (++player.curr_frame == 2) {
			player.curr_frame = 0;
		}

		/* Scan the background under objects. */
		Behind_Sprite_DB(&player);
		Behind_Sprite_DB(&tracker);

		/* Draw all the imagery. */
		Draw_Sprite_DB(&player);
		Draw_Sprite_DB(&tracker);

		/* Copy the double buffer to the screen. */
		Show_Double_Buffer();

		/* Draw the state of spider on top of the video buffer. */
		if (tracker.state == TRACKER_ATTACK) {
			Blit_String(8, 180, 242, "Mode=Attack", 1);
		} else {
			Blit_String(8, 180, 243, "Mode=Evade", 1);
		}

		/* Wait a sec. */
		Delay(1);
	}

	/* Go back to text mode. */
	Set_Video_Mode(TEXT_MODE);

	/* Free the double buffer. */
	Delete_Double_Buffer();
}
