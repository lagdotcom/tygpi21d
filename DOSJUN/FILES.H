#ifndef DOSJUN_FILES_H
#define DOSJUN_FILES_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define FILE_MAGIC		"JUN"
#define VERSION_1		1

#define NAME_SIZE		24
#define PARTY_SIZE		6

/* S T R U C T U R E S /////////////////////////////////////////////////// */

#define VERSION_HEADER char magic[3]; unsigned char version

#define CAMPAIGN_HEADER_PADDING		23
#define ZONE_HEADER_PADDING			22
#define SAVE_HEADER_PADDING			15

/* Campaign */

typedef struct campaign_header_typ {
	VERSION_HEADER;

	zone_id start_zone;
	coord start_x, start_y;
	direction start_facing;
	zone_id num_zones;

	unsigned char unused[CAMPAIGN_HEADER_PADDING];
} campaign_header;

typedef struct campaign_typ {
	campaign_header header;

	char **zones;
} campaign;

/* Save */

typedef struct character_typ {
	char name[NAME_SIZE];
	unsigned short hp, maxhp;
	unsigned short mp, maxmp;
} character;

typedef struct party_typ {
	character characters[PARTY_SIZE];
	zone_id zone;
	direction facing;
	coord x, y;
} party;

typedef struct save_header_typ {
	VERSION_HEADER;

	char campaign_name[8];
	zone_id zone;
	direction facing;
	coord x, y;
	unsigned char num_characters;

	unsigned char unused[SAVE_HEADER_PADDING];
} save_header;

typedef struct save_typ {
	save_header header;

	character characters[PARTY_SIZE];
} save;

/* Zone */

typedef struct wall_typ {
	colour texture;
} wall;

typedef struct tile_typ {
	colour floor, ceil;
	wall walls[4];
	string_id description;
} tile;

typedef struct zone_header_typ {
	VERSION_HEADER;

	coord width, height;
	string_id num_strings;
	script_id num_scripts;

	unsigned char unused[ZONE_HEADER_PADDING];
} zone_header;

typedef struct zone_typ {
	zone_header header;

	tile *tiles;
	char **strings;
	char **scripts;
	length *script_lengths;
} zone;

#endif
