#ifndef DOSJUN_FONT_H
#define DOSJUN_FONT_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "grf.h"
#include "common.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

int Char_Width(grf *f, char ch);
int Char_Height(grf *f, char ch);

void Draw_Font(int sx, int sy, colour tint, const char *string, grf *f, bool trans_flag);
int Draw_Font_Char(int sx, int sy, colour tint, char ch, grf *f, bool trans_flag);

size2d Measure_String(grf *f, const char *string);

#endif
