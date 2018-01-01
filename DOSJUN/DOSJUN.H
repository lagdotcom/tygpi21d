#ifndef DOSJUN_DOSJUN_H
#define DOSJUN_DOSJUN_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "types.h"

#include "campaign.h"
#include "code.h"
#include "combat.h"
#include "fp.h"
#include "gfx.h"
#include "items.h"
#include "input.h"
#include "io.h"
#include "jobs.h"
#include "list.h"
#include "main.h"
#include "monsters.h"
#include "party.h"
#include "save.h"
#include "sound.h"
#include "zone.h"

#include "bard.h"
#include "rogue.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TILE(z, x, y) (&z.tiles[(y) * z.header.width + (x)])

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern campaign gCampaign;
extern gamestate gState;
extern items gItems;
extern monsters gMonsters;
extern save gSave;
extern zone gZone;

extern bool
	trigger_on_enter,
	trigger_zone_enter,
	redraw_everything,
	redraw_description,
	redraw_fp,
	redraw_party;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

wall *Get_Wall(coord x, coord y, dir dir, relative rel);
char Get_X_Offset(dir dir);
char Get_Y_Offset(dir dir);
bool Is_Coord_Valid(coord x, coord y);
void Redraw_Dungeon_Screen(bool script);
void Show_Game_String(char *string, bool wait_for_key);
dir Turn_Left(dir dir);
dir Turn_Right(dir dir);

#endif
