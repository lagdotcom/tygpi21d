#ifndef DOSJUN_ITEMS_H
#define DOSJUN_ITEMS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

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
	itScroll,
} item_types;

typedef enum {
	ifNone = 0,
	ifLight = 1,
	ifHeavy = 2,
	ifStacked = 4,
	ifMediumRange = 8,
	ifLongRange = 16,
	ifDexterityWeapon = 32,
} itemflags;

typedef enum {
	spNone,
	spHeal,
} itemspecials;

#define ITEMS_HEADER_PADDING		26
#define ITEM_PADDING				6

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	char name[NAME_SIZE];
	item_id id;
	item_type type;
	item_flag flags;
	UINT32 value;
	special_id special;
	INT16 special_argument1;
	INT16 special_argument2;
	unsigned char unused[ITEM_PADDING];
	stat stats[NUM_STATS];
} item;

typedef struct {
	VERSION_HEADER;

	UINT16 num_items;

	char unused[ITEMS_HEADER_PADDING];
} items_header;

typedef struct {
	items_header header;

	item *items;
} items;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Items(items *i);
void Initialise_Items(items *i);
bool Load_Items(char *filename, items *i);
item *Lookup_Item(items *lib, item_id id);
bool Save_Items(char *filename, items *i);

#endif
