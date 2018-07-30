#ifndef DOSJUN_DOSJUN_H
#define DOSJUN_DOSJUN_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "types.h"

#include "buffs.h"
#include "campaign.h"
#include "code.h"
#include "combat.h"
#include "djn.h"
#include "font.h"
#include "fp.h"
#include "grf.h"
#include "gfx.h"
#include "items.h"
#include "input.h"
#include "io.h"
#include "jobs.h"
#include "list.h"
#include "main.h"
#include "music.h"
#include "mus_sng.h"
#include "monsters.h"
#include "npc.h"
#include "party.h"
#include "save.h"
#include "sound.h"
#include "status.h"
#include "strings.h"
#include "timer.h"
#include "zone.h"

#include "jbard.h"
#include "jcleric.h"
#include "jfighter.h"
#include "jmage.h"
#include "jranger.h"
#include "jrogue.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define TILE(z, x, y) (&z->tiles[(y) * z->header.width + (x)])

#define BLACK	0
#define GREY	7
#define DARK	8
#define RED		9
#define GREEN	10
#define YELLOW	11
#define BLUE	12
#define MAGENTA 13
#define CYAN    14
#define WHITE   15

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern djn *gDjn;
extern campaign *gCampaign;
extern grf *gFont;
extern gamestate gState;
extern party *gParty;
extern djn *gSave;
extern zone *gZone;
extern overlay *gOverlay;
extern globals *gGlobals;
extern palette *gPalette;

extern bool
	trigger_on_enter,
	trigger_zone_enter,
	redraw_everything,
	redraw_description,
	redraw_fp,
	redraw_party,
	just_moved,
	can_save;

extern point2d gTopLeft;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void dief(char *format, ...);
wall *Get_Wall(coord x, coord y, dir dir, relative rel);
char Get_X_Offset(dir dir);
char Get_Y_Offset(dir dir);
bool Is_Coord_Valid(coord x, coord y);
void Redraw_Dungeon_Screen(bool script);
char *Resolve_String(int id);
void Show_Game_String(const char *string, bool wait_for_key);
void Show_Game_String_Context(const char *string, bool wait_for_key, file_id speaker, file_id target);
dir Turn_Left(dir dir);
dir Turn_Right(dir dir);

#endif
