#ifndef DOSJUN_MAIN_H
#define DOSJUN_MAIN_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TILE(z, x, y) (&z.tiles[y * z.header.width + x])

typedef enum {
	gsMainMenu,
	gsDungeon,
	gsQuit
} gamestate;

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern campaign gCampaign;
extern gamestate gState;
extern items gItems;
extern monsters gMonsters;
extern save gSave;
extern zone gZone;

extern bool
	trigger_on_enter,
	redraw_description,
	redraw_fp,
	redraw_party;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

char **Get_Directory_Listing(char *pattern, int *count);
wall *Get_Wall(coord x, coord y, direction dir, relative rel);

#endif
