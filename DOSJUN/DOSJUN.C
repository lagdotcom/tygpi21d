/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

pcx_picture explore_bg;
bool redraw_fp,
	redraw_party;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Bline_DB(int xo, int yo, int x1, int y1, unsigned char color)
{
	/* This function uses Bresenham's algorithm (IBM 1965) to draw a line
	from (xo,yo) - (x1,y1). */

	int dx,					/* x difference */
		dy,					/* y difference */
		x_inc,				/* x increment */
		y_inc,				/* y increment */
		error = 0,			/* discriminant */
		index;

	/* Access the video buffer directly for speed. */
	unsigned char far *vb_start = double_buffer;

	/* Precompute the first pixel address in the video buffer. Use shifts for
	multiplication. */
	vb_start += ((unsigned int)yo << 6) + ((unsigned int)yo << 8) +
		(unsigned int)xo;

	/* Compute the deltas. */
	dx = x1 - xo;
	dy = y1 - yo;

	/* Test which direction the line is going in; i.e. the slope angle. */
	if (dx >= 0) {
		x_inc = 1;
	} else {
		x_inc = -1;
		dx = -dx;
	}

	/* Test the y component of the slope. */
	if (dy >= 0) {
		y_inc = SCREEN_WIDTH;
	} else {
		y_inc = -SCREEN_WIDTH;
		dy = -dy;
	}

	/* Now, based on which delta is greater, we can draw the line. */
	if (dx > dy) {
		/* Draw the line. */
		for (index = 0; index <= dx; index++) {
			/* Set the pixel */
			*vb_start = color;

			/* Adjust the disciminant. */
			error += dy;

			/* Test if error overflowed. */
			if (error > dx) {
				error -= dx;

				/* Move to the next line. */
				vb_start += y_inc;
			}

			vb_start += x_inc;
		}
	} else {
		for (index = 0; index <= dy; index++) {
			*vb_start = color;

			error += dx;

			if (error > dy) {
				error -= dy;

				vb_start += x_inc;
			}

			vb_start += y_inc;
		}
	}
}

void Dline_DB(int xo, int yo, int x1, int y1, unsigned char color)
{
	int x, y, xd, yd;

	x = xo;
	y = yo;
	if (x1 < xo) xd = -1;
	else if (x1 > xo) xd = 1;
	if (y1 < yo) yd = -1;
	else if (y1 > yo) yd = 1;

	while (x != x1 && y != y1) {
		Plot_Pixel_Fast_DB(x, y, color);

		x += xd;
		y += yd;
	}
}

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

bool Valid_Coord(char x, char y)
{
	if (x < 0 || x >= ZONE_WIDTH) return false;
	if (y < 0 || y >= ZONE_HEIGHT) return false;
	return true;
}

tile* Tile_Ahead(char x, char y, char direction, char multiple)
{
	char ax = x + Offset_X(direction) * multiple;
	char ay = y + Offset_Y(direction) * multiple;
	
	if (Valid_Coord(ax, ay)) return &Z.tiles[ax][ay];
	return (tile*)null;
}

wall* Wall_Offset(char x, char y, char direction, char selection)
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
	#define zts(x, y, f, c, wn, we, ws, ww) { Z.tiles[x][y].floor = f; Z.tiles[x][y].ceil = c; Z.tiles[x][y].walls[DIR_N].texture = wn; Z.tiles[x][y].walls[DIR_E].texture = we; Z.tiles[x][y].walls[DIR_S].texture = ws; Z.tiles[x][y].walls[DIR_W].texture = ww; }
	zts(0, 0, 7, 8, 7, 0, 7, 7);
	zts(1, 0, 7, 8, 7, 7, 7, 0);
	#undef zts
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

void Draw_First_Person(void)
{
	#define SX 8
	#define SY 8
	#define EX 135
	#define EY 135
	#define P1 22
	#define P2 36

	int x, y;
	tile *under, *ahead;
	wall *left, *right, *centre;

	/* Clear viewport */
	for (x = SX; x <= EX; x++) {
		for (y = SY; y <= EY; y++) {
			Plot_Pixel_Fast_DB(x, y, 0);
		}
	}

	/* Tile under party */
	under = &Z.tiles[P.x][P.y];
	if (under->floor) {
		Dline_DB(SX, EY, SX + P1, EY - P1, under->floor);
		Dline_DB(EX, EY, EX - P1, EY - P1, under->floor);
		Bline_DB(SX + P1, EY - P1, EX - P1, EY - P1, under->floor);
	}

	if (under->ceil) {
		Dline_DB(SX, SY, SX + P1, SY + P1, under->ceil);
		Dline_DB(EX, SY, EX - P1, SY + P1, under->ceil);
		Bline_DB(SX + P1, SY + P1, EX - P1, SY + P1, under->ceil);
	}

	left = Wall_Offset(P.x, P.y, P.facing, SEL_LEFT);
	if (left->texture) {
		Dline_DB(SX, SY + 1, SX + P1, SY + P1 + 1, left->texture);
		Dline_DB(SX, EY - 1, SX + P1, EY - P1 - 1, left->texture);
		Bline_DB(SX + P1, SY + P1 + 1, SX + P1, EY - P1 - 1, left->texture);
	}

	right = Wall_Offset(P.x, P.y, P.facing, SEL_RIGHT);
	if (right->texture) {
		Dline_DB(EX, SY + 1, EX - P1, SY + P1 + 1, right->texture);
		Dline_DB(EX, EY - 1, EX - P1, EY - P1 - 1, right->texture);
		Bline_DB(EX - P1, SY + P1 + 1, EX - P1, EY - P1 - 1, right->texture);
	}

	centre = Wall_Offset(P.x, P.y, P.facing, SEL_AHEAD);
	if (centre->texture) {
		Bline_DB(SX + P1 + 1, SY + P1 + 1, EX - P1 - 1, SY + P1 + 1, centre->texture);
		Bline_DB(SX + P1 + 1, EY - P1 - 1, EX - P1 - 1, EY - P1 - 1, centre->texture);
		Bline_DB(SX + P1 + 1, SY + P1 + 2, SX + P1 + 1, EY - P1 - 2, centre->texture);
		Bline_DB(EX - P1 - 1, SY + P1 + 2, EX - P1 - 1, EY - P1 - 2, centre->texture);
	} else {
		/* Tile ahead of party */
		ahead = Tile_Ahead(P.x, P.y, P.facing, 1);
		if (ahead != null) {
			if (ahead->floor) {
				Dline_DB(SX + P1, EY - P1, SX + P2, EY - P2, ahead->floor);
				Dline_DB(EX - P1, EY - P1, EX - P2, EY - P2, ahead->floor);
				Bline_DB(SX + P2, EY - P2, EX - P2, EY - P2, ahead->floor);
			}

			if (ahead->ceil) {
				Dline_DB(SX + P1, SY + P1, SX + P2, SY + P2, ahead->ceil);
				Dline_DB(EX - P1, SY + P1, EX - P2, SY + P2, ahead->ceil);
				Bline_DB(SX + P2, SY + P2, EX - P2, SY + P2, ahead->ceil);
			}
		}
	}

	redraw_fp = false;

	#undef P2
	#undef P1
	#undef EY
	#undef EX
	#undef SY
	#undef SX
}

/* M A I N /////////////////////////////////////////////////////////////// */

void main(void)
{
	int done = 0;

	if (!Create_Double_Buffer(SCREEN_HEIGHT)) {
		printf("\nNot enough memory to create double buffer.");
		return;
	}

	/* Get background image and palette */
	PCX_Init(&explore_bg);
	PCX_Load("BACK.PCX", &explore_bg, 1);
	memcpy(double_buffer, explore_bg.buffer, SCREEN_WIDTH * SCREEN_HEIGHT);

	Set_Video_Mode(VGA256);

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
