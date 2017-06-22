/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SX 148
#define SY 12

/* G L O B A L S ///////////////////////////////////////////////////////// */

bool redraw_party;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Draw_Character_Status(int index, int x, int y)
{
	character* ch = &gSave.characters[index];
	char buffer[11];

	strncpy(buffer, ch->name, 10);
	buffer[10] = 0;
	Blit_String_DB(x, y, 15, buffer, 0);

	sprintf(buffer, "%4u/%u", ch->stats[sHP], ch->stats[sMaxHP]);
	Blit_String_DB(x, y + 8, 15, buffer, 0);

	if (ch->stats[sMaxMP] > 0) {
		sprintf(buffer, "%4u/%u", ch->stats[sMP], ch->stats[sMaxMP]);
		Blit_String_DB(x, y + 16, 15, buffer, 0);
	}
}

void Draw_Party_Status(void)
{
	Draw_Character_Status(0, SX, SY);
	Draw_Character_Status(1, SX, SY + 34);
	Draw_Character_Status(2, SX, SY + 68);
	Draw_Character_Status(3, SX + 80, SY);
	Draw_Character_Status(4, SX + 80, SY + 34);
	Draw_Character_Status(5, SX + 80, SY + 68);

	redraw_party = false;
}

bool Equip_Item(unsigned char pc, item_id iid)
{
	int i;
	item *it = Lookup_Item(&gItems, iid);
	if (it == null) return false;

	/* TODO: check char can equip item */

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (gSave.characters[pc].items[i].item == iid) {
			gSave.characters[pc].items[i].flags |= vfEquipped;
			/* TODO: remove other items at same time */
			return true;
		}
	}

	return false;
}

bool Add_To_Inventory(unsigned char pc, item_id iid, unsigned char qty)
{
	int i;
	item *it = Lookup_Item(&gItems, iid);
	if (it == null) return false;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (gSave.characters[pc].items[i].item == 0) {
			gSave.characters[pc].items[i].item = iid;
			gSave.characters[pc].items[i].quantity = qty;
			return true;
		}
	}

	return false;
}
