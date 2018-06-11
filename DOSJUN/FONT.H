#ifndef DOSJUN_FONT_H
#define DOSJUN_FONT_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "grf.h"
#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define FONT_HEADER_PADDING 14

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Draw_Font(int x, int y, colour col, char *string, grf *f, bool trans_flag);
int Draw_Font_Char(int sx, int sy, colour col, char ch, grf *f, bool trans_flag);

/* Clears an area, then prints a string into it, with wrapping. */
void Draw_Wrapped_Font(int x, int y, int w, int h, colour col, char *string, grf *f, bool margin);

#endif
