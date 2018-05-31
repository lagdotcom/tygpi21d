#ifndef DOSJUN_ITEMS_H
#define DOSJUN_ITEMS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "save.h"

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

#define ITEM_PADDING				6

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct item {
	/* TODO: string_id */
	char name[NAME_SIZE];
	file_id id;
	item_type type;
	itemflag_id flags;
	UINT32 value;
	itemspecial_id special;
	INT16 special_argument1;
	INT16 special_argument2;
	unsigned char unused[ITEM_PADDING];
	stat_value stats[NUM_STATS];
} item;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Item_Has_Use(item *it);
bool Item_Needs_Target(item *it);
bool Read_Item(FILE *fp, item *i);
bool Save_Item(char *filename, item *i);
bool Use_Item(item *it, pc *user, pc *targ);

#endif
