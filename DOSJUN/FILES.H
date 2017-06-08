#ifndef DOSJUN_FILES_H
#define DOSJUN_FILES_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define FILE_MAGIC		"JUN"
#define VERSION_1		1

#define NAME_SIZE		24
#define PARTY_SIZE		6
#define ENCOUNTER_SIZE	6

typedef enum {
	Weapon,
	OffHand,
	Head,
	Body,
	Feet,
	Accessory
} slot_id;
#define ITEM_SLOTS		6

typedef enum {
	MaxHP,
	MaxMP,
	MinDamage,
	MaxDamage,
	Armour,
	Strength,
	Dexterity,
	Intelligence,
	HP,
	MP
} stat_id;
#define NUM_STATS		10

typedef enum {
	PrimaryWeapon,
	SmallWeapon,
	TwoHandedWeapon,
	Shield,
	Helmet,
	BodyArmour,
	Footwear,
	Jewellery,
	Potion,
	Scroll
} item_types;

typedef enum {
	Fighter,	/* Str */
	Cleric,		/* Str, Int */
	Mage,		/* Int */
	Rogue,		/* Dex */
	Ranger,		/* Str, Dex */
	Bard		/* Int, Dex */
} jobs;

#define IT_LIGHT		1
#define IT_HEAVY		2

#define Set_Version_Header(h) strncpy(h.magic, FILE_MAGIC, 3); h.version = VERSION_1
#define Free_If_Null(r) if (r != null) { \
	free(r); \
	r = null; \
}

/* S T R U C T U R E S /////////////////////////////////////////////////// */

#define VERSION_HEADER char magic[3]; unsigned char version

#define CAMPAIGN_HEADER_PADDING		23
#define ITEMS_HEADER_PADDING		27
#define ITEM_PADDING				11
#define MONSTERS_HEADER_PADDING		27
#define SAVE_HEADER_PADDING			15
#define ZONE_HEADER_PADDING			10

/* Campaign */

typedef struct {
	VERSION_HEADER;

	zone_id start_zone;
	coord start_x, start_y;
	direction start_facing;
	zone_id num_zones;

	unsigned char unused[CAMPAIGN_HEADER_PADDING];
} campaign_header;

typedef struct {
	campaign_header header;

	char **zones;
} campaign;

/* Item */

typedef struct {
	char name[NAME_SIZE];
	item_id id;
	item_type type;
	item_flags flags;
	unsigned long value;
	unsigned char unused[ITEM_PADDING];
	unsigned short stats[NUM_STATS];
} item;

typedef struct {
	VERSION_HEADER;

	unsigned char num_items;

	unsigned char unused[ITEMS_HEADER_PADDING];
} items_header;

typedef struct {
	items_header header;

	item *items;
} items;

/* Monster */

typedef struct {
	char name[NAME_SIZE];
	monster_id id;
	unsigned short stats[NUM_STATS];
} monster;

typedef struct {
	VERSION_HEADER;

	unsigned char num_monsters;

	unsigned char unused[MONSTERS_HEADER_PADDING];
} monsters_header;

typedef struct {
	monsters_header header;

	monster *monsters;
} monsters;

/* Save */

typedef struct {
	char name[NAME_SIZE];
	job_id job;
	unsigned char level;
	unsigned short stats[NUM_STATS];
	item_id items[ITEM_SLOTS];
} character;

typedef struct {
	VERSION_HEADER;

	char campaign_name[8];
	zone_id zone;
	direction facing;
	coord x, y;
	unsigned char num_characters;

	unsigned char unused[SAVE_HEADER_PADDING];
} save_header;

typedef struct {
	save_header header;

	character characters[PARTY_SIZE];
} save;

/* Zone */

typedef struct {
	colour texture;
} wall;

typedef struct {
	colour floor, ceil;
	wall walls[4];
	string_id description;
} tile;

typedef struct {
	monster_id monsters[ENCOUNTER_SIZE];
	unsigned char minimum[ENCOUNTER_SIZE];
	unsigned char maximum[ENCOUNTER_SIZE];
} encounter;

typedef struct {
	VERSION_HEADER;

	char campaign_name[8];
	coord width, height;
	string_id num_strings;
	script_id num_scripts;
	encounter_id num_encounters;
	string_id start_script_strings;

	unsigned char unused[ZONE_HEADER_PADDING];
} zone_header;

typedef struct {
	zone_header header;

	tile *tiles;
	char **strings;
	unsigned char **scripts;
	length *script_lengths;
	encounter* encounters;
} zone;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

char* Get_String(FILE* fp);
void Save_String(char *string, FILE *fp);
void IO_Error(char *message);

#endif
