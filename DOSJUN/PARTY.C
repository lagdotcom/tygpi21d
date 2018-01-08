/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SX1 8
#define SX2 232
#define SY 8

/* G L O B A L S ///////////////////////////////////////////////////////// */

bool redraw_party;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Pc_Select_Box(bool sel, int x, int y)
{
	Draw_Square_DB(sel ? 14 : 0, x - 2, y - 2, x + 70, y + 30, false);
}

void Pc_Select(int num)
{
	Pc_Select_Box(num == 0, SX1, SY);
	Pc_Select_Box(num == 1, SX1, SY + 34);
	Pc_Select_Box(num == 2, SX1, SY + 68);
	Pc_Select_Box(num == 3, SX2, SY);
	Pc_Select_Box(num == 4, SX2, SY + 34);
	Pc_Select_Box(num == 5, SX2, SY + 68);
}

void Draw_Character_Status(int index, int x, int y)
{
	character* ch = &gSave.characters[index];
	char buffer[9];

	strncpy(buffer, ch->header.name, 8);
	buffer[8] = 0;
	Draw_Font(x + 16, y, 15, buffer, FNT, false);

	sprintf(buffer, "H%3d/%-3d", ch->header.stats[sHP], ch->header.stats[sMaxHP]);
	Draw_Font(x + 16, y + 8, 15, buffer, FNT, false);

	if (ch->header.stats[sMaxMP] > 0) {
		sprintf(buffer, "M%3d/%-3d", ch->header.stats[sMP], ch->header.stats[sMaxMP]);
		Draw_Font(x + 16, y + 16, 15, buffer, FNT, false);
	}
}

void Draw_Party_Status(void)
{
	Draw_Character_Status(0, SX1, SY);
	Draw_Character_Status(1, SX1, SY + 32);
	Draw_Character_Status(2, SX1, SY + 64);
	Draw_Character_Status(3, SX2, SY);
	Draw_Character_Status(4, SX2, SY + 32);
	Draw_Character_Status(5, SX2, SY + 64);

	redraw_party = false;
}

bool Equip_Item(unsigned char pc, item_id iid)
{
	int i;
	item *it = Lookup_Item(&gItems, iid);
	if (it == null) return false;

	/* TODO: check char can equip item */

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (gSave.characters[pc].header.items[i].item == iid) {
			gSave.characters[pc].header.items[i].flags |= vfEquipped;
			/* TODO: remove other items at same time */
			return true;
		}
	}

	return false;
}

bool Add_to_Inventory(unsigned char pc, item_id iid, unsigned char qty)
{
	int i;
	item *it = Lookup_Item(&gItems, iid);
	if (it == null) return false;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (gSave.characters[pc].header.items[i].item == 0) {
			gSave.characters[pc].header.items[i].item = iid;
			gSave.characters[pc].header.items[i].quantity = qty;
			return true;
		}

		/* TODO: stack on existing items */
	}

	return false;
}

bool In_Front_Row(unsigned char pc)
{
	return !(gSave.characters[pc].header.flags & cfBackRow);
}

item *Get_Equipped_Weapon(unsigned char pc)
{
	int i;
	inventory *iv;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		iv = &gSave.characters[pc].header.items[i];
		if (iv->item != 0 && iv->flags & vfEquipped) {
			return Lookup_Item(&gItems, iv->item);
		}
	}

	return null;
}

character *Get_Pc(int pc)
{
	return &gSave.characters[pc];
}
