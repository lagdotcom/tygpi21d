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

char Offset_X(direction dir)
{
	switch (dir) {
		case East: return 1;
		case West: return -1;
		default: return 0;
	}
}

char Offset_Y(direction dir)
{
	switch (dir) {
		case South: return 1;
		case North: return -1;
		default: return 0;
	}
}

bool Valid_Coord(coord x, coord y)
{
	if (x < 0 || x >= Z.header.width) return false;
	if (y < 0 || y >= Z.header.height) return false;
	return true;
}

tile* Tile_Ahead(coord x, coord y, direction dir, char multiple)
{
	coord ax = x + Offset_X(dir) * multiple;
	coord ay = y + Offset_Y(dir) * multiple;
	
	if (Valid_Coord(ax, ay)) return TILE(Z, ax, ay);
	return (tile*)null;
}

wall* Wall_Offset(coord x, coord y, direction dir, relative rel)
{
	tile* under = TILE(Z, x, y);

	switch (dir) {
		case North:
			switch (rel) {
				case Left:  return &under->walls[West];
				case Ahead: return &under->walls[North];
				case Right: return &under->walls[East];
				default: return null;
			}

		case East:
			switch (rel) {
				case Left:  return &under->walls[North];
				case Ahead: return &under->walls[East];
				case Right: return &under->walls[South];
				default: return null;
			}

		case South:
			switch (rel) {
				case Left:  return &under->walls[East];
				case Ahead: return &under->walls[South];
				case Right: return &under->walls[West];
				default: return null;
			}

		case West:
			switch (rel) {
				case Left:  return &under->walls[South];
				case Ahead: return &under->walls[West];
				case Right: return &under->walls[North];
				default: return null;
			}
	}

	return null;
}

void Draw_Description(void)
{
	tile* under = TILE(Z, S.header.x, S.header.y);

	Square_DB(0, 12, 148, 12 + 37*8, 148 + 5*8, 1);
	if (under->description > 0) {
		Blit_String_Box(12, 148, 37, 5, 15, Z.strings[under->description - 1], 0);
	}

	redraw_description = false;
}

bool Try_Move_Forward(void)
{
	wall *centre;
	coord ax, ay;

	centre = Wall_Offset(S.header.x, S.header.y, S.header.facing, Ahead);
	if (centre->texture) {
		/* TODO: ow! */
		return false;
	}

	ax = S.header.x + Offset_X(S.header.facing);
	ay = S.header.y + Offset_Y(S.header.facing);

	S.header.x = ax;
	S.header.y = ay;
	redraw_description = true;
	redraw_fp = true;

	return true;
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Dungeon_Screen(void)
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
					G = State_Quit;
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
		if (redraw_fp) Draw_First_Person();
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
	Campaign_Init(&C);
	Items_Init(&I);
	Monsters_Init(&M);
	Savefile_Init(&S);
	Zone_Init(&Z);

	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	printf("OK\n");

	Set_Video_Mode(VGA256);

	G = State_MainMenu;

	while (G != State_Quit) {
		switch (G) {
			case State_MainMenu:
				Main_Menu();
				break;

			case State_Dungeon:
				Dungeon_Screen();
				break;
		}
	}

	Set_Video_Mode(TEXT_MODE);

	printf("Cleaning up after DOSJUN...");

	Campaign_Free(&C);
	Items_Free(&I);
	Monsters_Init(&M);
	Savefile_Free(&S);
	Zone_Free(&Z);

	Delete_Double_Buffer();

	printf("OK!\n");
}
