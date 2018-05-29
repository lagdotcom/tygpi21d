#ifndef DOSJUN_GFX_H
#define DOSJUN_GFX_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* S T R U C T U R E S /////////////////////////////////////////////////// */

#define PALETTE_SIZE	256

typedef struct palette {
	RGB_color colours[PALETTE_SIZE];
} palette;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Apply_Palette(palette *p);
void Draw_Line_DB(int xo, int yo, int x1, int y1, colour col);
void Draw_Diagonal_DB(int xo, int yo, int x1, int y1, colour col);
void Draw_Bounded_String(int x, int y, int w, int h, colour col, char *string, bool trans_flag);
void Draw_HorzTrapezium_DB(colour col, int x0, int x1, int x2, int x3, int y0, int y1, bool filled);
void Draw_VertTrapezium_DB(colour col, int x0, int x1, int y0, int y1, int y2, int y3, bool filled);
void Draw_Square_DB(colour col, int x0, int y0, int x1, int y1, bool filled);
bool Read_Palette(FILE *fp, palette *p);

#endif
