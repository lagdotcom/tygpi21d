/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SX1 8
#define SX2 232
#define SY 8

#define ITEMS_X	140
#define ITEMS_Y	32

#define CONFIRM_Y (ITEMS_Y + (INVENTORY_SIZE + 1) * 8)

#define ITEM_SELECTED YELLOW

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport char damage_range_buf[10];
bool redraw_party;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Pc_Select_Box(bool sel, int x, int y)
{
	Draw_Square_DB(sel ? 14 : 0, x - 2, y - 2, x + 70, y + 30, false);
}

void Pc_Select(pcnum num)
{
	Pc_Select_Box(num == 0, SX1, SY);
	Pc_Select_Box(num == 1, SX1, SY + 34);
	Pc_Select_Box(num == 2, SX1, SY + 68);
	Pc_Select_Box(num == 3, SX2, SY);
	Pc_Select_Box(num == 4, SX2, SY + 34);
	Pc_Select_Box(num == 5, SX2, SY + 68);
}

void Draw_Character_Status(pcnum index, int x, int y)
{
	char buffer[9];
	pc *pc;

	pc = Get_Pc(index);
	if (!pc) return;

	strncpy(buffer, pc->name, 8);
	buffer[8] = 0;
	Draw_Font(x + 16, y, WHITE, buffer, gFont, false);

	sprintf(buffer, "H%3d/%-3d", pc->header.stats[sHP], pc->header.stats[sMaxHP]);
	Draw_Font(x + 16, y + 8, WHITE, buffer, gFont, false);

	if (pc->header.stats[sMaxMP] > 0) {
		sprintf(buffer, "M%3d/%-3d", pc->header.stats[sMP], pc->header.stats[sMaxMP]);
		Draw_Font(x + 16, y + 16, WHITE, buffer, gFont, false);
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

noexport void Apply_Item_Stats(pc *pc, item *it, bool add)
{
	statistic st;
	int multiplier = add ? 1 : -1;

	for (st = 0; st < NUM_STATS; st++) {
		/* don't count weapon damage stats against both weapons */
		if (Is_Weapon(it) && (st == sMinDamage || st == sMaxDamage))
			continue;

		pc->header.stats[st] += multiplier * it->stats[st];
	}
}

bool Remove_Item_At(pc *pc, int index)
{
	inventory *iv;
	item *it;

	iv = &pc->header.items[index];

	/* Sanity checks */
	if (!iv->item) return false;
	if (!(iv->flags & vfEquipped)) return false;

	it = Lookup_File(gDjn, iv->item);
	if (it == null) return false;

	/* TODO: curse? */

	iv->flags -= vfEquipped;
	iv->slot = 0;
	Apply_Item_Stats(pc, it, false);

	return true;
}

bool Remove_Item(pc *pc, file_id iid)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (pc->header.items[i].item == iid) {
			return Remove_Item_At(pc, i);
		}
	}

	return false;
}

bool Remove_Equipped_Items(pc *pc, itemslot sl)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (pc->header.items[i].slot == sl) {
			if (!Remove_Item_At(pc, i)) {
				return false;
			}
		}
	}

	return true;
}

item *Get_Equipped_Item(pc *pc, itemslot sl)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (pc->header.items[i].slot == sl) {
			return Lookup_File(gDjn, pc->header.items[i].item);
		}
	}
	
	return null;
}

bool Equip_Item_At(pc *pc, int index)
{
	inventory *iv;
	item *it;
	itemslot sl;
	itemtype ty;
	assert(index < INVENTORY_SIZE, "Equip_Item_At: inventory index too high");

	iv = &pc->header.items[index];

	/* Sanity checks */
	if (!iv->item) return false;
	if (iv->flags & vfEquipped) return false;

	it = Lookup_File(gDjn, iv->item);
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

bool Equip_Item(pc *pc, file_id iid)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (pc->header.items[i].item == iid) {
			return Equip_Item_At(pc, i);
		}
	}

	return false;
}

int Find_Empty_Inventory_Slot(pc *pc)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (pc->header.items[i].item == 0) {
			return i;
		}
	}

	return -1;
}

bool Add_to_Inventory(pc *pc, file_id iid, unsigned char qty)
{
	int i;
	inventory *iv;
	item *it = Lookup_File(gDjn, iid);

	if (it == null) return false;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		iv = &pc->header.items[i];
		if (iv->item == 0) {
			iv->flags = vfNone;
			iv->item = iid;
			iv->quantity = qty;
			iv->slot = slNone;
			return true;
		}

		/* TODO: stack on existing items */
	}

	return false;
}

bool In_Front_Row(pc *pc)
{
	return !(pc->header.flags & cfBackRow);
}

item *Get_Equipped_Weapon(pc *pc, bool primary)
{
	item *it;

	it = Get_Equipped_Item(pc, primary ? slWeapon : slOffHand);

	/* shields don't count! */
	if (it == null || it->type == itShield) return null;
	return it;
}

pc *Get_Pc(pcnum index)
{
	file_id ref;

	assert(index < PARTY_SIZE, "Get_Pc: pc number too high");

	ref = gParty->members[index];
	return Lookup_File(gSave, ref);
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

stat_value Get_Pc_Stat(pc *pc, statistic st)
{
	assert(st < NUM_STATS, "Get_Pc_Stat: stat number too high");

	return Get_Stat_Base(pc->header.stats, st) + pc->header.stats[st];
}

noexport void Show_Pc_Stat(pc *pc, statistic st, int y)
{
	char *name = Stat_Name(st);
	char temp[5];

	itoa(Get_Pc_Stat(pc, st), temp, 10);

	Draw_Font(8, y, WHITE, name, gFont, false);
	Draw_Font(80, y, WHITE, temp, gFont, false);
}

noexport void Show_Pc_Stat_Pair(pc *pc, statistic stc, statistic stm, int y)
{
	char *name = Stat_Name(stm);
	char temp[10];

	Draw_Font(8, y, WHITE, name, gFont, false);

	sprintf(temp, "%3d/%3d", Get_Pc_Stat(pc, stc), Get_Pc_Stat(pc, stm));
	Draw_Font(80, y, WHITE, temp, gFont, false);
}

noexport void Show_Pc_Items(pc *pc, int selected)
{
	char temp[4];
	inventory *iv;
	item *it;
	int i;
	int col;
	int y = ITEMS_Y;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		iv = &pc->header.items[i];

		if (!iv->item) {
			col = (i == selected) ? (YELLOW - 8) : GREY;
			Draw_Font(ITEMS_X + 16, y, col, "--", gFont, false);
		} else {
			it = Lookup_File(gDjn, iv->item);
			col = (i == selected) ? ITEM_SELECTED : WHITE;
			Draw_Font(ITEMS_X + 16, y, col, it->name, gFont, false);

			if (iv->quantity > 1) {
				sprintf(temp, "x%d", iv->quantity);
				Draw_Font(ITEMS_X + 150, y, col, temp, gFont, false);
			}
		}

		if (iv->flags & vfEquipped) {
			Draw_Font(ITEMS_X, y, CYAN, "E", gFont, false);
		} else {
			Draw_Square_DB(BLACK, ITEMS_X, y, ITEMS_X + 15, y + 7, true);
		}

		y += 8;
	}
}

noexport char *Get_Damage_Range(pc *pc)
{
	int min = Get_Pc_Stat(pc, sMinDamage);
	int max = Get_Pc_Stat(pc, sMaxDamage);
	int i;
	inventory *iv;
	item *it;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		iv = &pc->header.items[i];
		
		if (iv->flags & vfEquipped) {
			it = Lookup_File(gDjn, iv->item);

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

noexport void Show_Pc_Stats(pc *pc)
{
	char temp[100];

	Fill_Double_Buffer(0);

	Draw_Font(8, 8, WHITE, pc->name, gFont, false);

	sprintf(temp, "Level %d %s", pc->header.job_level[pc->header.job], Job_Name(pc->header.job));
	Draw_Font(8, 16, WHITE, temp, gFont, false);

	Show_Pc_Stat(pc, sStrength, 32);
	Show_Pc_Stat(pc, sDexterity, 40);
	Show_Pc_Stat(pc, sIntelligence, 48);

	Show_Pc_Stat_Pair(pc, sHP, sMaxHP, 64);
	Show_Pc_Stat_Pair(pc, sMP, sMaxMP, 72);

	Show_Pc_Stat(pc, sHitBonus, 88);
	Show_Pc_Stat(pc, sDodgeBonus, 96);

	Show_Pc_Stat(pc, sArmour, 112);
	Show_Pc_Stat(pc, sToughness, 120);

	Draw_Font(8, 136, WHITE, "Damage", gFont, false);
	Draw_Font(80, 136, WHITE, Get_Damage_Range(pc), gFont, false);
}

noexport unsigned char Confirm(char *prompt)
{
	Draw_Font(ITEMS_X, CONFIRM_Y, WHITE, prompt, gFont, false);
	Show_Double_Buffer();
	return Get_Next_Scan_Code();
}

#define Clear_Confirm() Draw_Square_DB(BLACK, ITEMS_X, CONFIRM_Y, SCREEN_WIDTH, CONFIRM_Y + 7, true)

noexport bool Confirm_Drop_Item(pc *pc, int index)
{
	inventory *iv;
	bool result = false;
	bool failed = false;

	iv = &pc->header.items[index];

	/* Sanity check */
	if (!iv->item) return false;

	if (Confirm("Are you sure?") == SCAN_Y) {
		if (iv->flags & vfEquipped) {
			if (!Remove_Item_At(pc, index)) {
				/* TODO */
				failed = true;
			}
		}

		if (!failed) {
			iv->item = 0;
			result = true;
		}
	}

	Clear_Confirm();
	return result;
}

noexport int Confirm_Pc(char *prompt)
{
	unsigned char scan;

	scan = Confirm(prompt);
	Clear_Confirm();

	switch (scan) {
		case SCAN_1: return 0;
		case SCAN_2: return 1;
		case SCAN_3: return 2;
		case SCAN_4: return 3;
		case SCAN_5: return 4;
		case SCAN_6: return 5;

		default: return -1;
	}
}

noexport void Clear_Inventory_Slot(inventory *iv)
{
	iv->flags = vfNone;
	iv->item = 0;
	iv->quantity = 0;
	iv->slot = slNone;
}

noexport bool Confirm_Give_Item(pc *pc, int index)
{
	inventory *iv,
		*dest_iv;
	pcnum dest_pc;
	int dest_slot;
	assert(index < INVENTORY_SIZE, "Confirm_Give_Item: inventory index too high");

	iv = &pc->header.items[index];

	/* Sanity check */
	if (!iv->item) return false;

	dest_pc = Confirm_Pc("To whom? (1-6)");
	if (dest_pc < 0 || pc == Get_Pc(dest_pc)) {
		return false;
	}
	
	/* Equipped? */
	if (iv->flags & vfEquipped) {
		if (!Remove_Item_At(pc, index)) {
			/* TODO */
			return false;
		}
	}

	dest_slot = Find_Empty_Inventory_Slot(Get_Pc(dest_pc));
	if (dest_slot < 0) {
		Confirm("Inventory is full!");
		Clear_Confirm();
		return false;
	}

	dest_iv = &pc->header.items[dest_slot];
	dest_iv->flags = iv->flags;
	dest_iv->item = iv->item;
	dest_iv->quantity = iv->quantity;
	dest_iv->slot = slNone;

	Clear_Inventory_Slot(iv);
	return true;
}

noexport bool Confirm_Use_Item(pc *pc, int index)
{
	inventory *iv;
	item *it;
	int dest_pc = -1;
	assert(index < INVENTORY_SIZE, "Confirm_Use_Item: inventory index too high");

	iv = &pc->header.items[index];

	/* Sanity check */
	if (!iv->item) return false;

	it = Lookup_File(gDjn, iv->item);
	if (!Item_Has_Use(it)) {
		Confirm("You can't use that.");
		Clear_Confirm();
		return false;
	}

	if (Item_Needs_Target(it)) {
		dest_pc = Confirm_Pc("On whom? (1-6)");
		if (dest_pc < 0) {
			return false;
		}
	}

	if (Use_Item(it, pc, Get_Pc(dest_pc))) {
		if (iv->quantity > 1) {
			iv->quantity--;
		} else {
			Clear_Inventory_Slot(iv);
		}
	}

	return false;
}

void Show_Pc_Screen(pcnum starting_pc)
{
	pcnum index = starting_pc;
	pc *pc;
	int selected = 0;
	redraw_everything = true;

	while (true) {
		pc = Get_Pc(index);
		Show_Pc_Stats(pc);
		Show_Pc_Items(pc, selected);
		Show_Double_Buffer();

		switch (Get_Next_Scan_Code()) {
			case SCAN_1:
				index = 0;
				selected = 0;
				continue;

			case SCAN_2:
				index = 1;
				selected = 0;
				continue;

			case SCAN_3:
				index = 2;
				selected = 0;
				continue;

			case SCAN_4:
				index = 3;
				selected = 0;
				continue;

			case SCAN_5:
				index = 4;
				selected = 0;
				continue;

			case SCAN_6:
				index = 5;
				selected = 0;
				continue;

			case SCAN_LEFT:
				index--;
				if (index < 0) index = PARTY_SIZE - 1;
				continue;

			case SCAN_RIGHT:
				index++;
				if (index >= PARTY_SIZE) index = 0;
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

			case SCAN_D:
			case SCAN_DEL:
				Confirm_Drop_Item(pc, selected);
				continue;

			case SCAN_G:
				Confirm_Give_Item(pc, selected);
				continue;

			case SCAN_U:
			case SCAN_ENTER:
				Confirm_Use_Item(pc, selected);
				continue;

			case SCAN_Q:
			case SCAN_ESC:
				return;
		}
	}
}

void Free_PC(pc *pc)
{
	Free_List(pc->skills);

	Clear_List(pc->buffs);
	Free_List(pc->buffs);

	if (pc->header.name_id == 0)
		Free(pc->name);
}

bool Read_PC(FILE *fp, pc *pc)
{
	fread(pc, sizeof(pc_header), 1, fp);

	pc->skills = Read_List(fp, "Read_PC.skills");
	pc->buffs = Read_List(fp, "Read_PC.buffs");

	if (pc->header.name_id)
		pc->name = Resolve_String(pc->header.name_id);
	else
		pc->name = Read_LengthString(fp, "Read_PC.name");

	return true;
}

bool Write_PC(FILE *fp, pc *pc)
{
	fwrite(pc, sizeof(pc_header), 1, fp);

	Write_List(pc->skills, fp);
	Write_List(pc->buffs, fp);

	if (pc->header.name_id == 0)
		Write_LengthString(pc->name, fp);

	return true;
}
