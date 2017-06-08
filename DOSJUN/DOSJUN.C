/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "gamelib.h"
#include "dosjun.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture explore_bg;

campaign C;
gamestate G;
items I;
monsters M;
save S;
zone Z;

bool redraw_description;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

char Get_X_Offset(direction dir)
{
	switch (dir) {
		case East: return 1;
		case West: return -1;
		default: return 0;
	}
}

char Get_Y_Offset(direction dir)
{
	switch (dir) {
		case South: return 1;
		case North: return -1;
		default: return 0;
	}
}

bool Is_Coord_Valid(coord x, coord y)
{
	if (x < 0 || x >= Z.header.width) return false;
	if (y < 0 || y >= Z.header.height) return false;
	return true;
}

tile* Get_Adjacent_Tile(coord x, coord y, direction dir, char multiple)
{
	coord ax = x + Get_X_Offset(dir) * multiple;
	coord ay = y + Get_Y_Offset(dir) * multiple;
	
	if (Is_Coord_Valid(ax, ay)) return TILE(Z, ax, ay);
	return (tile*)null;
}

wall* Get_Wall(coord x, coord y, direction dir, relative rel)
{
	tile* under = TILE(Z, x, y);

	switch (dir) {
		case North:
			switch (rel) {
				case rLeft:  return &under->walls[West];
				case rAhead: return &under->walls[North];
				case rRight: return &under->walls[East];
				default: return null;
			}

		case East:
			switch (rel) {
				case rLeft:  return &under->walls[North];
				case rAhead: return &under->walls[East];
				case rRight: return &under->walls[South];
				default: return null;
			}

		case South:
			switch (rel) {
				case rLeft:  return &under->walls[East];
				case rAhead: return &under->walls[South];
				case rRight: return &under->walls[West];
				default: return null;
			}

		case West:
			switch (rel) {
				case rLeft:  return &under->walls[South];
				case rAhead: return &under->walls[West];
				case rRight: return &under->walls[North];
				default: return null;
			}
	}

	return null;
}

void Draw_Description(void)
{
	tile* under = TILE(Z, S.header.x, S.header.y);

	Draw_Square_DB(0, 12, 148, 12 + 37*8, 148 + 5*8, 1);
	if (under->description > 0) {
		Draw_Bounded_String(12, 148, 37, 5, 15, Z.strings[under->description - 1], 0);
	}

	redraw_description = false;
}

bool Try_Move_Forward(void)
{
	wall *centre;
	coord ax, ay;

	centre = Get_Wall(S.header.x, S.header.y, S.header.facing, rAhead);
	if (centre->texture) {
		/* TODO: ow! */
		return false;
	}

	ax = S.header.x + Get_X_Offset(S.header.facing);
	ay = S.header.y + Get_Y_Offset(S.header.facing);

	S.header.x = ax;
	S.header.y = ay;
	redraw_description = true;
	redraw_fp = true;

	return true;
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Show_Dungeon_Screen(void)
{
	int done = 0;

	/* Get background image and palette */
	PCX_Init(&explore_bg);
	PCX_Load("BACK.PCX", &explore_bg, 1);
	memcpy(double_buffer, explore_bg.buffer, SCREEN_WIDTH * SCREEN_HEIGHT);

	redraw_description = true;
	redraw_fp = true;
	redraw_party = true;

	while (!done) {
		if (kbhit()) {
			switch (getch()) {
				/* TODO: replace later */
				case 'q':
					G = gsQuit;
					done = 1;
					break;

				case 'a':
					if (S.header.facing == North) S.header.facing = West;
					else S.header.facing--;
					redraw_fp = true;
					break;

				case 'd':
					if (S.header.facing == West) S.header.facing = North;
					else S.header.facing++;
					redraw_fp = true;
					break;

				case 'w':
					Try_Move_Forward();
					break;
			}
		}

		if (redraw_description) Draw_Description();
		if (redraw_fp) Draw_FP();
		if (redraw_party) Draw_Party_Status();

		Show_Double_Buffer();
		Delay(1);
	}

	/* Cleanup */
	PCX_Delete(&explore_bg);
}

void main(void)
{
	printf("Initialising DOSJUN...");
	Initialise_Campaign(&C);
	Initialise_Items(&I);
	Initialise_Monsters(&M);
	Initialise_Savefile(&S);
	Initialise_Zone(&Z);

	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	printf("OK\n");

	Set_Video_Mode(VGA256);

	G = gsMainMenu;

	while (G != gsQuit) {
		switch (G) {
			case gsMainMenu:
				Show_Main_Menu();
				break;

			case gsDungeon:
				Show_Dungeon_Screen();
				break;
		}
	}

	Set_Video_Mode(TEXT_MODE);

	printf("Cleaning up after DOSJUN...");

	Free_Campaign(&C);
	Free_Items(&I);
	Free_Monsters(&M);
	Free_Savefile(&S);
	Free_Zone(&Z);

	Delete_Double_Buffer();

	printf("OK!\n");
	Stop_Memory_Tracking();
}
