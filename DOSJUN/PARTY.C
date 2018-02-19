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

noexport char damage_range_buf[10];
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

itemslot Get_Slot_for_Type(itemtype ity)
{
	switch (ity)
	{
		case itBodyArmour: return slBody;
		case itFootwear: return slFeet;
		case itHelmet: return slHead;
		case itJewellery: return slAccessory;

		case itShield: return slOffHand;

		case itPrimaryWeapon: return slWeapon;
		case itTwoHandedWeapon: return slWeapon;

		/* TODO */
		case itSmallWeapon: return slOffHand;

		default: return slInvalid;
	}
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

char *Slot_Name(itemslot sl)
{
	switch (sl)
	{
		case slAccessory: return "Accessory";
		case slBody: return "Body";
		case slFeet: return "Feet";
		case slHead: return "Head";
		case slOffHand: return "Off Hand";
		case slWeapon: return "Main Hand";

		default: return "?";
	}
}

stat_value Get_Pc_Stat(character *c, statistic st)
{
	return Get_Stat_Base(c->header.stats, st) + c->header.stats[st];
}

noexport void Show_Pc_Stat(character *c, statistic st, int y)
{
	char *name = Stat_Name(st);
	char temp[10];

	itoa(Get_Pc_Stat(c, st), temp, 10);

	Draw_Font(8, y, 15, name, &font6x8, false);
	Draw_Font(80, y, 15, temp, &font6x8, false);
}

noexport void Show_Pc_Items(character *c, int x, int y)
{
	inventory *iv;
	item *it;
	int i;

	y -= 8;
	for (i = 0; i < INVENTORY_SIZE; i++) {
		y += 8;
		iv = &c->header.items[i];

		if (!iv->item) {
			Draw_Font(x + 16, y, 7, "--", &font6x8, false);
			continue;
		}

		it = Lookup_Item(&gItems, iv->item);
		Draw_Font(x + 16, y, 15, it->name, &font6x8, false);

		if (iv->flags & vfEquipped) {
			Draw_Font(x, y, 14, "E", &font6x8, false);
		}
	}
}

noexport char *Get_Damage_Range(character *c)
{
	int min = Get_Pc_Stat(c, sMinDamage);
	int max = Get_Pc_Stat(c, sMaxDamage);
	int i;
	inventory *iv;
	item *it;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		iv = &c->header.items[i];
		
		if (iv->flags & vfEquipped) {
			it = Lookup_Item(&gItems, iv->item);

			/* TODO: this is temporary */
			if (it->stats[sMinDamage]) min += it->stats[sMinDamage];
			if (it->stats[sMaxDamage]) max += it->stats[sMaxDamage];
		}
	}
	
	if (max < min)
		max = min;

	if (min == max) {
		itoa(min, damage_range_buf, 10);
		return damage_range_buf;
	}

	sprintf(damage_range_buf, "%d - %d", min, max);
	return damage_range_buf;
}

void Show_Pc_Screen(int pc)
{
	character *c = &gSave.characters[pc];
	char temp[100];

	Fill_Double_Buffer(0);

	Draw_Font(8, 8, 15, c->header.name, &font6x8, false);

	sprintf(temp, "Level %d %s", c->header.job_level[c->header.job], Job_Name(c->header.job));
	Draw_Font(8, 16, 15, temp, &font6x8, false);

	Show_Pc_Stat(c, sStrength, 32);
	Show_Pc_Stat(c, sDexterity, 40);
	Show_Pc_Stat(c, sIntelligence, 48);

	Show_Pc_Stat(c, sMaxHP, 64);
	Show_Pc_Stat(c, sMaxMP, 72);

	Show_Pc_Stat(c, sHitBonus, 88);
	Show_Pc_Stat(c, sDodgeBonus, 96);

	Show_Pc_Stat(c, sArmour, 112);
	Show_Pc_Stat(c, sToughness, 120);

	Draw_Font(8, 136, 15, "Damage", &font6x8, false);
	Draw_Font(80, 136, 15, Get_Damage_Range(c), &font6x8, false);

	Show_Pc_Items(c, 120, 32);

	Show_Double_Buffer();
	Get_Next_Scan_Code();

	redraw_everything = true;
}
