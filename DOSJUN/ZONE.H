#ifndef DOSJUN_ZONE_H
#define DOSJUN_ZONE_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define ZONE_HEADER_PADDING			12
#define WALL_PADDING				1
#define TILE_PADDING				0
#define ENCOUNTER_SIZE	6
#define ETABLE_SIZE		6

typedef enum thing {
	thNone = 0,

	thShiny,
	thBarrel,

	thINVALID
} thing;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct wall {
	file_id texture;
	walltype_id type;

	unsigned char padding[WALL_PADDING];
} wall;

typedef struct tile {
	wall walls[4];
	file_id floor;
	file_id ceil;
	string_id description;
	script_id on_enter;
	etable_id etable;
	tileflag_id flags;
	thing_id thing;
	UINT8 danger;
	script_id on_use;

	unsigned char padding[TILE_PADDING];
} tile;

typedef struct encounter {
	monster_id monsters[ENCOUNTER_SIZE];
	unsigned char minimum[ENCOUNTER_SIZE];
	unsigned char maximum[ENCOUNTER_SIZE];
} encounter;

typedef struct etable {
	unsigned char possibilities;
	encounter_id encounters[ETABLE_SIZE];
	unsigned char percentages[ETABLE_SIZE];
} etable;

typedef struct itempos {
	file_id item;
	coord x;
	coord y;
} itempos;

typedef struct zone_header {
	VERSION_HEADER;

	char campaign_name[8];
	coord width, height;
	UINT8 floor;
	UINT8 unused;
	encounter_id num_encounters;
	etable_id num_etables;
	script_id on_enter;
	script_id on_move;
	UINT16 name_id;
	UINT16 num_locals;
	UINT16 num_items;

	unsigned char padding[ZONE_HEADER_PADDING];
} zone_header;

typedef struct zone {
	zone_header header;

	tile *tiles;
	encounter *encounters;
	etable *etables;
	itempos *items;
} zone;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Zone(zone *z);
void Initialise_Zone(zone *z);
bool Load_Zone(char *filename, zone *z);
bool Read_Zone(FILE *fp, zone *z);
bool Save_Zone(char *filename, zone *z);

#endif
