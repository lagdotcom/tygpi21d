/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Equip_Item(unsigned char pc, item_id iid)
{
	int i;
	item *it = Lookup_Item(&I, iid);
	if (it == null) return false;

	/* TODO: check char can equip item */

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (S.characters[pc].items[i].item == iid) {
			S.characters[pc].items[i].flags |= vfEquipped;
			/* TODO: remove other items at same time */
			return true;
		}
	}

	return false;
}

bool Add_To_Inventory(unsigned char pc, item_id iid, unsigned char qty)
{
	int i;
	item *it = Lookup_Item(&I, iid);
	if (it == null) return false;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (S.characters[pc].items[i].item == 0) {
			S.characters[pc].items[i].item = iid;
			S.characters[pc].items[i].quantity = qty;
			return true;
		}
	}

	return false;
}
