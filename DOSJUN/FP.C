/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <alloc.h>
#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SX 8
#define SY 8
#define EX 135
#define EY 135
#define P1 22
#define P2 36
#define P3 42

#define XA SX + ps
#define XB EX - ps
#define XC SX + pe
#define XD EX - pe
#define YA SY + ps
#define YB EY - ps
#define YC SY + pe
#define YD EY - pe

#define FILLED true

/* G L O B A L S ///////////////////////////////////////////////////////// */

bool redraw_fp;
pcx_picture current_pic;
bool picture_loaded = false;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Draw_FP_Tile(coord x, coord y, char ps, char pe, char cmod)
{
	tile *under;
	wall *left, *right, *centre;

	under = TILE(gZone, x, y);
	if (under->floor) {
		Draw_HorzTrapezium_DB(under->floor + cmod, XA, XB, XC - 1, XD + 1, YB, YD + 1, FILLED);
	}

	if (under->ceil) {
		Draw_HorzTrapezium_DB(under->ceil + cmod, XA, XB, XC - 1, XD + 1, YA, YC - 1, FILLED);
	}

	left = Get_Wall(x, y, gSave.header.facing, rLeft);
	if (left->texture) {
		Draw_VertTrapezium_DB(left->texture + cmod, XA, XC - 1, YA + 1, YB - 1, YC + 1, YD - 1, FILLED);
	}

	right = Get_Wall(x, y, gSave.header.facing, rRight);
	if (right->texture) {
		Draw_VertTrapezium_DB(right->texture + cmod, XB, XD + 1, YA + 1, YB - 1, YC + 1, YD - 1, FILLED);
	}

	centre = Get_Wall(x, y, gSave.header.facing, rAhead);
	if (centre->texture) {
		Draw_Square_DB(centre->texture + cmod, XC, YC, XD, YD, FILLED);

		return false;
	}

	return true;
}

void Clear_FP(void)
{
	int x, y;

	for (x = SX; x <= EX; x++) {
		for (y = SY; y <= EY; y++) {
			Plot_Pixel_Fast_DB(x, y, 0);
		}
	}
}

void Draw_FP(void)
{
	int x, y, ox, oy;

	Clear_FP();

	ox = Get_X_Offset(gSave.header.facing);
	oy = Get_Y_Offset(gSave.header.facing);

	if (Draw_FP_Tile(gSave.header.x, gSave.header.y, 0, P1, 0)) {
		x = gSave.header.x + ox;
		y = gSave.header.y + oy;

		if (Draw_FP_Tile(x, y, P1, P2, 16)) {
			x += ox;
			y += oy;

			Draw_FP_Tile(x, y, P2, P3, 32);
		}
	}

	redraw_fp = false;
}

void Delete_Picture(void)
{
	if (picture_loaded) {
		picture_loaded = false;
		PCX_Delete(&current_pic);
	}
}

void Show_Picture(char *name)
{
	int x, y;
	unsigned char *output, *input;
	char filename[20];
	sprintf(filename, "PICS\\%s.PCX", name);

	Delete_Picture();
	current_pic.buffer = farmalloc(128 * 128);
	PCX_Load(filename, &current_pic, 0);

	/* draw that thing */
	output = &video_buffer[8 * SCREEN_WIDTH + 8];
	input = current_pic.buffer;
	for (y = 0; y < 128; y++) {
		memcpy(output, input, 128);
		input += 128;
		output += SCREEN_WIDTH;
	}

	picture_loaded = true;
}
