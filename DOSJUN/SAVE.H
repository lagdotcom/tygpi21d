#ifndef DOSJUN_SAVE_H
#define DOSJUN_SAVE_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SAVE_HEADER_PADDING			12

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

typedef struct character_header {
	char name[NAME_SIZE];
	job_id job;
	UINT16 total_level;
	UINT8 job_level[NUM_JOBS];
	stat_value stats[NUM_STATS];
	inventory items[INVENTORY_SIZE];
	charflag_id flags;
	UINT32 experience;
} character_header;

typedef struct character {
	character_header header;
	list *skills;
	list *buffs;
} character;

typedef struct save_header {
	VERSION_HEADER;

	char campaign_name[8];
	zone_id zone;
	dir_id facing;
	coord x, y;
	UINT8 num_characters;
	zone_id num_zones;
	UINT8 encounter_chance;
	UINT8 danger;

	unsigned char unused[SAVE_HEADER_PADDING];
} save_header;

typedef struct save {
	save_header header;

	character characters[PARTY_SIZE];
	INT16 *script_globals;
	INT16 **script_locals;
} save;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Load_Savefile(char *filename, save *s);
void Free_Savefile(save *s);
void Initialise_Savefile(save *s);
bool Save_Savefile(char *filename, save *s);

#endif
