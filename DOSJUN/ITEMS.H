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

typedef enum dropflag {
	drfTable = 1,
} dropflag;

#define ITEM_PADDING				16

#define DROPTABLE_PADDING			14
#define DROPTABLE_HEADERSZ			16

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct item {
	str_id name_id;
	item_type type;
	itemflag_id flags;
	UINT32 value;
	itemspecial_id special;
	INT16 special_argument1;
	INT16 special_argument2;
	file_id image_id;
	unsigned char padding[ITEM_PADDING];

	stat_value stats[NUM_STATS];
} item;

typedef struct drop {
	file_id ref;
	UINT8 chance;
	UINT8 flags;
} drop;

typedef struct droptable {
	UINT16 count;
	unsigned char padding[DROPTABLE_PADDING];

	drop *drops;
} droptable;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

file_id Get_Drop(droptable *dt);
bool Item_Has_Use(item *it);
bool Item_Needs_Target(item *it);
void Free_DropTable(droptable *dt);
bool Read_DropTable(FILE *fp, droptable *dt);
bool Read_Item(FILE *fp, item *i);
bool Save_Item(char *filename, item *i);
bool Use_Item(item *it, pc *user, pc *targ);

#endif
