/* I N C L U D E S /////////////////////////////////////////////////////// */

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

/* G L O B A L S ///////////////////////////////////////////////////////// */

bool redraw_fp;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Draw_First_Person_Tile(coord x, coord y, char ps, char pe, char cmod)
{
	tile *under;
	wall *left, *right, *centre;

	under = &Z.tiles[x][y];
	if (under->floor) {
		TrapeziumH_DB(under->floor + cmod, XA, XB, XC - 1, XD + 1, YB, YD + 1);
	}

	if (under->ceil) {
		TrapeziumH_DB(under->ceil + cmod, XA, XB, XC - 1, XD + 1, YA, YC - 1);
	}

	left = Wall_Offset(x, y, P.facing, Left);
	if (left->texture) {
		TrapeziumV_DB(left->texture + cmod, XA, XC - 1, YA + 1, YB - 1, YC + 1, YD - 1);
	}

	right = Wall_Offset(x, y, P.facing, Right);
	if (right->texture) {
		TrapeziumV_DB(right->texture + cmod, XB, XD + 1, YA + 1, YB - 1, YC + 1, YD - 1);
	}

	centre = Wall_Offset(x, y, P.facing, Ahead);
	if (centre->texture) {
		Square_DB(centre->texture + cmod, XC, XD, YC, YD);

		return false;
	}

	return true;
}

void Clear_First_Person(void)
{
	int x, y;

	for (x = SX; x <= EX; x++) {
		for (y = SY; y <= EY; y++) {
			Plot_Pixel_Fast_DB(x, y, 0);
		}
	}
}

void Draw_First_Person(void)
{
	int x, y, ox, oy;

	Clear_First_Person();

	ox = Offset_X(P.facing);
	oy = Offset_Y(P.facing);

	if (Draw_First_Person_Tile(P.x, P.y, 0, P1, 0)) {
		x = P.x + ox;
		y = P.y + oy;

		if (Draw_First_Person_Tile(x, y, P1, P2, 16)) {
			x += ox;
			y += oy;

			Draw_First_Person_Tile(x, y, P2, P3, 32);
		}
	}

	redraw_fp = false;
}
