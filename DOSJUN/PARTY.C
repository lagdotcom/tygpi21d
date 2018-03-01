/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SX1 8
#define SX2 232
#define SY 8

#define ITEM_SELECTED YELLOW

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
	Draw_Font(x + 16, y, WHITE, buffer, FNT, false);

	sprintf(buffer, "H%3d/%-3d", ch->header.stats[sHP], ch->header.stats[sMaxHP]);
	Draw_Font(x + 16, y + 8, WHITE, buffer, FNT, false);

	if (ch->header.stats[sMaxMP] > 0) {
		sprintf(buffer, "M%3d/%-3d", ch->header.stats[sMP], ch->header.stats[sMaxMP]);
		Draw_Font(x + 16, y + 16, WHITE, buffer, FNT, false);
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

		/* fixed manually in Equip_Item_At */
		case itTwoHandedWeapon: return slWeapon;

		default: return slInvalid;
	}
}

bool Is_Weapon(item *it)
{
	switch (it->type)
	{
		case itPrimaryWeapon:
		case itTwoHandedWeapon:
		case itSmallWeapon:
			return true;

		default: return false;
	}
}

noexport void Apply_Item_Stats(unsigned char pc, item *it, bool add)
{
	character *ch = &gSave.characters[pc];
	statistic st;
	int multiplier = add ? 1 : -1;

	for (st = 0; st < NUM_STATS; st++) {
		/* don't count weapon damage stats against both weapons */
		if (Is_Weapon(it) && (st == sMinDamage || st == sMaxDamage))
			continue;

		ch->header.stats[st] += multiplier * it->stats[st];
	}
}

bool Remove_Item_At(unsigned char pc, int index)
{
	inventory *iv = &gSave.characters[pc].header.items[index];
	item *it;

	/* Sanity checks */
	if (!iv->item) return false;
	if (!(iv->flags & vfEquipped)) return false;

	it = Lookup_Item(&gItems, iv->item);
	if (it == null) return false;

	/* TODO: curse? */

	iv->flags -= vfEquipped;
	iv->slot = 0;
	Apply_Item_Stats(pc, it, false);

	return true;
}

bool Remove_Item(unsigned char pc, item_id iid)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (gSave.characters[pc].header.items[i].item == iid) {
			return Remove_Item_At(pc, i);
		}
	}

	return false;
}

bool Remove_Equipped_Items(unsigned char pc, itemslot sl)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (gSave.characters[pc].header.items[i].slot == sl) {
			if (!Remove_Item_At(pc, i)) {
				return false;
			}
		}
	}

	return true;
}

item *Get_Equipped_Item(unsigned char pc, itemslot sl)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (gSave.characters[pc].header.items[i].slot == sl) {
			return Lookup_Item(&gItems, gSave.characters[pc].header.items[i].item);
		}
	}
	
	return null;
}

bool Equip_Item_At(unsigned char pc, int index)
{
	inventory *iv = &gSave.characters[pc].header.items[index];
	item *it;
	itemslot sl;
	itemtype ty;

	/* Sanity checks */
	if (!iv->item) return false;
	if (iv->flags & vfEquipped) return false;

	it = Lookup_Item(&gItems, iv->item);
	if (it == null) return false;

	/* TODO: check char can equip item */

	/* Small Weapons can go in either hand */
	ty = it->type;
	if (ty == itSmallWeapon) {
		if (Get_Equipped_Item(pc, slWeapon) == null) {
			sl = slWeapon;
		} else {
			sl = slOffHand;
		}
	} else {
		sl = Get_Slot_for_Type(ty);
	}

	/* Unequippable? */
	if (sl == slInvalid) return false;

	/* Cursed? */
	if (!Remove_Equipped_Items(pc, sl)) return false;
	if (ty == itTwoHandedWeapon && !Remove_Equipped_Items(pc, slOffHand)) return false;

	iv->flags |= vfEquipped;
	iv->slot = sl;
	Apply_Item_Stats(pc, it, true);

	/* TODO: curse? */

	return true;
}

bool Equip_Item(unsigned char pc, item_id iid)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (gSave.characters[pc].header.items[i].item == iid) {
			return Equip_Item_At(pc, i);
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

item *Get_Equipped_Weapon(unsigned char pc, bool primary)
{
	item *it = Get_Equipped_Item(pc, primary ? slWeapon : slOffHand);

	/* shields don't count! */
	if (it->type == itShield) return null;
	return it;
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

	Draw_Font(8, y, WHITE, name, FNT, false);
	Draw_Font(80, y, WHITE, temp, FNT, false);
}

noexport void Show_Pc_Items(int pc, int x, int y, int selected)
{
	character *c = &gSave.characters[pc];
	inventory *iv;
	item *it;
	int i;
	int col;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		iv = &c->header.items[i];

		if (!iv->item) {
			col = (i == selected) ? (YELLOW - 8) : GREY;
			Draw_Font(x + 16, y, col, "--", FNT, false);
		} else {
			it = Lookup_Item(&gItems, iv->item);
			col = (i == selected) ? ITEM_SELECTED : WHITE;
			Draw_Font(x + 16, y, col, it->name, FNT, false);
		}

		if (iv->flags & vfEquipped) {
			Draw_Font(x, y, CYAN, "E", FNT, false);
		} else {
			Draw_Square_DB(BLACK, x, y, x + 15, y + 7, true);
		}

		y += 8;
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

			if (Is_Weapon(it)) {
				min += it->stats[sMinDamage];
				max += it->stats[sMaxDamage];
			}
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

noexport void Show_Pc_Stats(int pc)
{
	character *c = &gSave.characters[pc];
	char temp[100];

	Fill_Double_Buffer(0);

	Draw_Font(8, 8, WHITE, c->header.name, FNT, false);

	sprintf(temp, "Level %d %s", c->header.job_level[c->header.job], Job_Name(c->header.job));
	Draw_Font(8, 16, WHITE, temp, FNT, false);

	Show_Pc_Stat(c, sStrength, 32);
	Show_Pc_Stat(c, sDexterity, 40);
	Show_Pc_Stat(c, sIntelligence, 48);

	Show_Pc_Stat(c, sMaxHP, 64);
	Show_Pc_Stat(c, sMaxMP, 72);

	Show_Pc_Stat(c, sHitBonus, 88);
	Show_Pc_Stat(c, sDodgeBonus, 96);

	Show_Pc_Stat(c, sArmour, 112);
	Show_Pc_Stat(c, sToughness, 120);

	Draw_Font(8, 136, WHITE, "Damage", FNT, false);
	Draw_Font(80, 136, WHITE, Get_Damage_Range(c), FNT, false);
}

void Show_Pc_Screen(int starting_pc)
{
	int pc = starting_pc;
	int selected = 0;
	redraw_everything = true;

	while (true) {
		Show_Pc_Stats(pc);
		Show_Pc_Items(pc, 120, 32, selected);
		Show_Double_Buffer();

		switch (Get_Next_Scan_Code()) {
			case SCAN_1:
				pc = 0;
				selected = 0;
				continue;

			case SCAN_2:
				pc = 1;
				selected = 0;
				continue;

			case SCAN_3:
				pc = 2;
				selected = 0;
				continue;

			case SCAN_4:
				pc = 3;
				selected = 0;
				continue;

			case SCAN_5:
				pc = 4;
				selected = 0;
				continue;

			case SCAN_6:
				pc = 5;
				selected = 0;
				continue;

			case SCAN_LEFT:
				pc--;
				if (pc < 0) pc = PARTY_SIZE - 1;
				continue;

			case SCAN_RIGHT:
				pc++;
				if (pc >= PARTY_SIZE) pc = 0;
				continue;

			case SCAN_UP:
				selected--;
				if (selected < 0) selected = INVENTORY_SIZE - 1;
				continue;

			case SCAN_DOWN:
				selected++;
				if (selected >= INVENTORY_SIZE) selected = 0;
				continue;

			case SCAN_E:
				Equip_Item_At(pc, selected);
				continue;

			case SCAN_R:
				Remove_Item_At(pc, selected);
				continue;

			case SCAN_Q:
			case SCAN_ESC:
				return;
		}
	}
}
