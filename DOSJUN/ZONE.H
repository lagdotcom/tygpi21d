#ifndef DOSJUN_ZONE_H
#define DOSJUN_ZONE_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define ZONE_HEADER_PADDING			8
#define WALL_PADDING				2
#define TILE_PADDING				5
#define ENCOUNTER_SIZE	6
#define ETABLE_SIZE		6

typedef enum {
	thNone = 0,

	thTest,
} thing;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	colour texture;
	wall_type type;

	char unused[WALL_PADDING];
} wall;

typedef struct {
	wall walls[4];
	colour floor, ceil;
	string_id description;
	script_id on_enter;
	etable_id etable;
	tile_flag flags;
	thing_id thing;

	char unused[TILE_PADDING];
} tile;

typedef struct {
	monster_id monsters[ENCOUNTER_SIZE];
	UINT8 minimum[ENCOUNTER_SIZE];
	UINT8 maximum[ENCOUNTER_SIZE];
} encounter;

typedef struct {
	UINT8 possibilities;
	encounter_id encounters[ETABLE_SIZE];
	UINT8 percentages[ETABLE_SIZE];
} etable;

typedef struct {
	VERSION_HEADER;

	char campaign_name[8];
	coord width, height;
	string_id num_strings;
	script_id num_scripts;
	encounter_id num_encounters;
	string_id num_code_strings;
	etable_id num_etables;

	char unused[ZONE_HEADER_PADDING];
} zone_header;

typedef struct {
	zone_header header;

	tile *tiles;
	char **strings;
	bytecode **scripts;
	length *script_lengths;
	encounter *encounters;
	char **code_strings;
	etable *etables;
} zone;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Zone(zone *z);
void Initialise_Zone(zone *z);
bool Load_Zone(char *filename, zone *z);
bool Save_Zone(char *filename, zone *z);

#endif
