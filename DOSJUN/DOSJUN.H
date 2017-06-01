#ifndef DOSJUN_MAIN_H
#define DOSJUN_MAIN_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "types.h"
#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TILE(z, x, y) (&z.tiles[y * z.header.width + x])

typedef enum {
	State_MainMenu,
	State_Dungeon,
	State_Quit
} gamestate;

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern campaign C;
extern gamestate G;
extern save S;
extern zone Z;

extern bool
	redraw_description,
	redraw_fp,
	redraw_party;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

wall* Wall_Offset(coord x, coord y, direction dir, relative rel);

#endif
