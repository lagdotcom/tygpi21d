/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture explore_bg;

party P;
zone Z;

bool redraw_party;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

char Offset_X(char direction)
{
	switch (direction) {
		case DIR_E: return 1;
		case DIR_W: return -1;
		default: return 0;
	}
}

char Offset_Y(char direction)
{
	switch (direction) {
		case DIR_S: return 1;
		case DIR_N: return -1;
		default: return 0;
	}
}

bool Valid_Coord(coord x, coord y)
{
	if (x < 0 || x >= ZONE_WIDTH) return false;
	if (y < 0 || y >= ZONE_HEIGHT) return false;
	return true;
}

tile* Tile_Ahead(coord x, coord y, char direction, char multiple)
{
	coord ax = x + Offset_X(direction) * multiple;
	coord ay = y + Offset_Y(direction) * multiple;
	
	if (Valid_Coord(ax, ay)) return &Z.tiles[ax][ay];
	return (tile*)null;
}

wall* Wall_Offset(coord x, coord y, char direction, char selection)
{
	tile* under = &Z.tiles[x][y];

	switch (direction) {
		case DIR_N:
			switch (selection) {
				case SEL_LEFT:  return &under->walls[DIR_W];
				case SEL_AHEAD: return &under->walls[DIR_N];
				case SEL_RIGHT: return &under->walls[DIR_E];
				default: return null;
			}

		case DIR_E:
			switch (selection) {
				case SEL_LEFT:  return &under->walls[DIR_N];
				case SEL_AHEAD: return &under->walls[DIR_E];
				case SEL_RIGHT: return &under->walls[DIR_S];
				default: return null;
			}

		case DIR_S:
			switch (selection) {
				case SEL_LEFT:  return &under->walls[DIR_E];
				case SEL_AHEAD: return &under->walls[DIR_S];
				case SEL_RIGHT: return &under->walls[DIR_W];
				default: return null;
			}

		case DIR_W:
			switch (selection) {
				case SEL_LEFT:  return &under->walls[DIR_S];
				case SEL_AHEAD: return &under->walls[DIR_W];
				case SEL_RIGHT: return &under->walls[DIR_N];
				default: return null;
			}
	}

	return null;
}

void Demo(void)
{
	/* Set up party */
	P.zone = 0; P.x = 0; P.y = 0; P.facing = DIR_E;

	strcpy(P.characters[0].name, "Lag.Com");

	strcpy(P.characters[1].name, "Mercury");

	strcpy(P.characters[2].name, "Emptyeye");

	strcpy(P.characters[3].name, "Silver");

	strcpy(P.characters[4].name, "Zan-zan");

	strcpy(P.characters[5].name, "Sizzler");

	/* Set up zone */
	#define zts(x, y, f, c, wn, we, ws, ww) { \
		Z.tiles[x][y].floor = f; \
		Z.tiles[x][y].ceil = c; \
		Z.tiles[x][y].walls[DIR_N].texture = wn; \
		Z.tiles[x][y].walls[DIR_E].texture = we; \
		Z.tiles[x][y].walls[DIR_S].texture = ws; \
		Z.tiles[x][y].walls[DIR_W].texture = ww; \
	}
	zts(0, 0, 9,10,11, 0,11,11);
	zts(1, 0, 9,10,11, 0,11, 0);
	zts(2, 0, 9,10,11,11, 0, 0);
	zts(2, 1, 9,10, 0,15, 0,15);
	zts(2, 2, 9,10, 0,15,15, 0);
	zts(1, 2, 9,10,15, 0,15,15);
	#undef zts
	Zone_Save("DEMO.ZON");
}

void Draw_Character_Status(int index, int x, int y)
{
	character* ch = &P.characters[index];

	Blit_String_DB(x, y, 15, ch->name, 1);
}

void Draw_Party_Status(void)
{
	#define SX 148
	#define SY 12

	Draw_Character_Status(0, SX, SY);
	Draw_Character_Status(1, SX, SY + 40);
	Draw_Character_Status(2, SX, SY + 80);
	Draw_Character_Status(3, SX + 80, SY);
	Draw_Character_Status(4, SX + 80, SY + 40);
	Draw_Character_Status(5, SX + 80, SY + 80);

	redraw_party = false;

	#undef SY
	#undef SX
}

bool Try_Move_Forward(void)
{
	wall *centre;
	coord ax, ay;

	centre = Wall_Offset(P.x, P.y, P.facing, SEL_AHEAD);
	if (centre->texture) {
		/* TODO: ow! */
		return false;
	}

	ax = P.x + Offset_X(P.facing);
	ay = P.y + Offset_Y(P.facing);

	P.x = ax;
	P.y = ay;
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
	redraw_party = true;
	redraw_fp = true;

	while (!done) {
		if (kbhit()) {
			switch (getch()) {
				/* TODO: replace later */
				case 'q':
					done = 1;
					break;

				case 'a':
					if (P.facing == DIR_N) P.facing = DIR_W;
					else P.facing--;
					redraw_fp = true;
					break;

				case 'd':
					if (P.facing == DIR_W) P.facing = DIR_N;
					else P.facing++;
					redraw_fp = true;
					break;

				case 'w':
					Try_Move_Forward();
					break;
			}
		}

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
