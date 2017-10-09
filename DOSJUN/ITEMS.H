#ifndef DOSJUN_ITEMS_H
#define DOSJUN_ITEMS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef enum itemtype {
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
} itemtype;

typedef enum itemflag {
	ifNone = 0,
	ifLight = 1,
	ifHeavy = 2,
	ifStacked = 4,
	ifMediumRange = 8,
	ifLongRange = 16,
	ifDexterityWeapon = 32,
} itemflag;

typedef enum itemspecial {
	spNone,
	spHeal,
} itemspecial;

#define ITEMS_HEADER_PADDING		26
#define ITEM_PADDING				6

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct item {
	char name[NAME_SIZE];
	item_id id;
	item_type type;
	itemflag_id flags;
	UINT32 value;
	itemspecial_id special;
	INT16 special_argument1;
	INT16 special_argument2;
	unsigned char unused[ITEM_PADDING];
	stat_value stats[NUM_STATS];
} item;

typedef struct items_header {
	VERSION_HEADER;

	UINT16 num_items;

	unsigned char unused[ITEMS_HEADER_PADDING];
} items_header;

typedef struct items {
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
