#ifndef DOSJUN_SAVE_H
#define DOSJUN_SAVE_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "djn.h"
#include "zone.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define PC_PADDING			15

typedef enum charflag {
	cfBackRow = 1
} charflag;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct inventory {
	item_id item;
	inventoryflag_id flags;
	unsigned char quantity;
	item_slot slot;
} inventory;

typedef struct pc_header {
	VERSION_HEADER;
	UINT16 name_id;
	job_id job;
	UINT8 pronouns;
	UINT16 portrait_id;
	UINT16 total_level;
	charflag_id flags;
	UINT32 experience;
	UINT16 attitude;

	char padding[PC_PADDING];

	stat_value stats[NUM_STATS];
	UINT8 job_level[NUM_JOBS];
	inventory items[INVENTORY_SIZE];
} pc_header;

typedef struct pc {
	pc_header header;
	list *skills;
	list *buffs;
	char *name;
} pc;

typedef struct partystatus {
	file_id members[PARTY_SIZE];
	file_id zone;
	dir_id facing;
	coord x, y;
	UINT8 encounter_chance;
	UINT8 danger;
} partystatus;

typedef struct globals {
	UINT16 num_globals;
	UINT16 num_flags;

	INT16 *globals;
	INT16 *flags;
} globals;
#define GLOBALS_HEADER_SZ	sizeof(UINT16)*2

typedef struct zone_overlay {
	UINT16 num_locals;

	list *items;
	INT16 *locals;
} zone_overlay;
#define OVERLAY_HEADER_SZ	sizeof(UINT16)

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Load_Savefile(char *filename, djn *s);
void Free_Globals(globals *g);
void Free_Overlay(zone_overlay *o);
void Free_Savefile(djn *s);
void Initialise_Globals(globals *g, campaign *c);
void Initialise_Overlay(zone_overlay *o, zone *z);
void Initialise_Savefile(djn *s);
void Read_Globals(FILE *fp, globals *g);
void Read_Overlay(FILE *fp, zone_overlay *o);
bool Save_Savefile(char *filename, djn *s);
void Write_Globals(FILE *fp, globals *g);
void Write_Overlay(FILE *fp, zone_overlay *g);

#endif
