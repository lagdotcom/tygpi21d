/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <conio.h>
#include <stdio.h>
#include <string.h>
#include "gamelib.h"
#include "dosjun.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture explore_bg;

party P;
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
	if (x < 0 || x >= ZONE_WIDTH) return false;
	if (y < 0 || y >= ZONE_HEIGHT) return false;
	return true;
}

tile* Tile_Ahead(coord x, coord y, direction dir, char multiple)
{
	coord ax = x + Offset_X(dir) * multiple;
	coord ay = y + Offset_Y(dir) * multiple;
	
	if (Valid_Coord(ax, ay)) return &Z.tiles[ax][ay];
	return (tile*)null;
}

wall* Wall_Offset(coord x, coord y, direction dir, relative rel)
{
	tile* under = &Z.tiles[x][y];

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

void Demo(void)
{
	/* Set up party */
	P.zone = 0; P.x = 0; P.y = 0; P.facing = East;

	#define pset(n, _name, _maxhp, _hp, _maxmp, _mp) { \
		strcpy(P.characters[n].name, _name); \
		P.characters[n].maxhp = _maxhp; \
		P.characters[n].hp = _hp; \
		P.characters[n].maxmp = _maxmp; \
		P.characters[n].mp = _mp; \
	}
	pset(0, "Lag.Com",   8,  8, 16, 16);
	pset(1, "Mercury",  21, 18,  4,  1);
	pset(2, "Emptyeye", 17,  3,  0,  0);
	pset(3, "Silver",   15, 14, 14,  8);
	pset(4, "Zan-zan",  10, 10,  0,  0);
	pset(5, "Sizzler",  17, 17,  8,  7);
	#undef pset
	Party_Save("DEMO.SAV");

	Zone_Load("DEMO.ZON");
}

void Draw_Description(void)
{
	tile* under = &Z.tiles[P.x][P.y];

	if (under->description) {
		Blit_String_Box(12, 148, 37, 5, 15, &Z.text[under->description - 1], 0);
	} else {
		/* TODO */
	}
}

bool Try_Move_Forward(void)
{
	wall *centre;
	coord ax, ay;

	centre = Wall_Offset(P.x, P.y, P.facing, Ahead);
	if (centre->texture) {
		/* TODO: ow! */
		return false;
	}

	ax = P.x + Offset_X(P.facing);
	ay = P.y + Offset_Y(P.facing);

	P.x = ax;
	P.y = ay;
	redraw_description = true;
	redraw_fp = true;

	return true;
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0;

	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	Set_Video_Mode(VGA256);

	/* Get background image and palette */
	PCX_Init(&explore_bg);
	PCX_Load("BACK.PCX", &explore_bg, 1);
	memcpy(double_buffer, explore_bg.buffer, SCREEN_WIDTH * SCREEN_HEIGHT);

	Demo();
	redraw_description = true;
	redraw_fp = true;
	redraw_party = true;

	while (!done) {
		if (kbhit()) {
			switch (getch()) {
				/* TODO: replace later */
				case 'q':
					done = 1;
					break;

				case 'a':
					if (P.facing == North) P.facing = West;
					else P.facing--;
					redraw_fp = true;
					break;

				case 'd':
					if (P.facing == West) P.facing = North;
					else P.facing++;
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
	Set_Video_Mode(TEXT_MODE);
	Delete_Double_Buffer();
}
