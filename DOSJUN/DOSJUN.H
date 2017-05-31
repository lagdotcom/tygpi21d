#ifndef DOSJUN_MAIN_H
#define DOSJUN_MAIN_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "types.h"
#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TILE(z, x, y) (&z.tiles[y * z.header.width + x])

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern save S;
extern zone Z;

extern bool
	redraw_description,
	redraw_fp,
	redraw_party;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Input_Number(int x, int y, int* number, int min, int max);
bool Input_String(int x, int y, char* string, int max);
bool Try_Move_Forward(void);
bool Valid_Coord(coord x, coord y);
char Offset_X(direction dir);
char Offset_Y(direction dir);
tile* Tile_Ahead(coord x, coord y, direction dir, char multiple);
void Bline_DB(int xo, int yo, int x1, int y1, colour color);
void Blit_String_Box(int sx, int sy, int w, int h, colour col, char *string, bool trans_flag);
void Dline_DB(int xo, int yo, int x1, int y1, colour col);
void Draw_Character_Status(int index, int x, int y);
void Draw_First_Person(void);
void Draw_Party_Status(void);
void Party_Load(char *filename);
void Party_Save(char *filename);
void Square_DB(colour col, int x0, int y0, int x1, int y1, bool filled);
void TrapeziumH_DB(colour col, int x0, int x1, int x2, int x3, int y0, int y1, bool filled);
void TrapeziumV_DB(colour col, int x0, int x1, int y0, int y1, int y2, int y3, bool filled);
void Zone_Load(char *filename, zone *z);
void Zone_Save(char *filename, zone *z);
wall* Wall_Offset(coord x, coord y, direction dir, relative rel);

#endif
