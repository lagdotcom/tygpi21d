#ifndef DOSJUN_FILES_H
#define DOSJUN_FILES_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define FILE_MAGIC		"JUN"
#define VERSION_1		1

#define NAME_SIZE		24
#define PARTY_SIZE		6
#define ENCOUNTER_SIZE	6
#define ETABLE_SIZE		6

typedef enum {
	slWeapon,
	slOffHand,
	slHead,
	slBody,
	slFeet,
	slAccessory
} slot_id;
#define NUM_SLOTS		6
#define INVENTORY_SIZE	10

typedef enum {
	sMaxHP,
	sMaxMP,
	sMinDamage,
	sMaxDamage,
	sArmour,
	sStrength,
	sDexterity,
	sIntelligence,
	sHP,
	sMP
} stat_id;
#define NUM_STATS		10

typedef enum {
	itPrimaryWeapon,
	itSmallWeapon,
	itTwoHandedWeapon,
	itShield,
	itHelmet,
	itBodyArmour,
	itFootwear,
	itJewellery,
	itPotion,
	itScroll
} item_types;

typedef enum {
	jFighter,	/* Str */
	jCleric,	/* Str, Int */
	jMage,		/* Int */
	jRogue,		/* Dex */
	jRanger,	/* Str, Dex */
	jBard		/* Int, Dex */
} jobs;

typedef enum {
	ifNone = 0,
	ifLight = 1,
	ifHeavy = 2,
	ifStacked = 4
} itemflags;

typedef enum {
	spNone,
	spHeal
} itemspecials;

typedef enum {
	vfNone = 0,
	vfEquipped = 1
} inventoryflags;

typedef enum {
	wtNormal,
	wtDoor,
	wtLockedDoor
} walltypes;

#define Set_Version_Header(h) strncpy(h.magic, FILE_MAGIC, 3); h.version = VERSION_1

/* S T R U C T U R E S /////////////////////////////////////////////////// */

#define VERSION_HEADER char magic[3]; unsigned char version

#define CAMPAIGN_HEADER_PADDING		23
#define ITEMS_HEADER_PADDING		27
#define ITEM_PADDING				6
#define MONSTERS_HEADER_PADDING		27
#define MONSTER_PADDING				10
#define SAVE_HEADER_PADDING			14
#define ZONE_HEADER_PADDING			8

#define WALL_PADDING				2
#define TILE_PADDING				8

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
	item_flag flags;
	unsigned long value;
	special_id special;
	int special_argument1;
	int special_argument2;
	unsigned char unused[ITEM_PADDING];
	unsigned int stats[NUM_STATS];
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
	char image[8];
	monster_id id;
	unsigned int stats[NUM_STATS];

	unsigned char unused[MONSTER_PADDING];
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
	item_id item;
	inventory_flag flags;
	unsigned char quantity;
} inventory;

typedef struct {
	char name[NAME_SIZE];
	job_id job;
	unsigned char level;
	unsigned int stats[NUM_STATS];
	inventory items[INVENTORY_SIZE];
} character;

typedef struct {
	VERSION_HEADER;

	char campaign_name[8];
	zone_id zone;
	direction facing;
	coord x, y;
	unsigned char num_characters;
	zone_id num_zones;

	unsigned char unused[SAVE_HEADER_PADDING];
} save_header;

typedef struct {
	save_header header;

	character characters[PARTY_SIZE];
	int *script_globals;
	int **script_locals;
} save;

/* Zone */

typedef struct {
	colour texture;
	wall_type type;

	unsigned char unused[WALL_PADDING];
} wall;

typedef struct {
	wall walls[4];
	colour floor, ceil;
	string_id description;
	script_id on_enter;
	etable_id etable;

	unsigned char unused[TILE_PADDING];
} tile;

typedef struct {
	monster_id monsters[ENCOUNTER_SIZE];
	unsigned char minimum[ENCOUNTER_SIZE];
	unsigned char maximum[ENCOUNTER_SIZE];
} encounter;

typedef struct {
	unsigned char possibilities;
	encounter_id encounters[ETABLE_SIZE];
	unsigned char percentages[ETABLE_SIZE];
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

	unsigned char unused[ZONE_HEADER_PADDING];
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

char *Read_LengthString(FILE *fp);
void Write_LengthString(char *string, FILE *fp);
void IO_Error(char *message);

#endif
