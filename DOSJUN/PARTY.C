/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "gamelib.h"
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SX1 8
#define SX2 232
#define SY 8

#define ITEMS_X	160
#define ITEMS_Y	96

#define CONFIRM_X 160
#define CONFIRM_Y 184

#define ITEM_SELECTED YELLOW

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport char damage_range_buf[10];
noexport point2d portrait_pick = { 96, 24 };
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

noexport unsigned char Confirm(char *prompt)
{
	Draw_Font(CONFIRM_X, CONFIRM_Y, WHITE, prompt, gFont, false);
	Show_Double_Buffer();
	return Get_Next_Scan_Code();
}

#define Clear_Confirm() Draw_Square_DB(BLACK, CONFIRM_X, CONFIRM_Y, SCREEN_WIDTH - 1, CONFIRM_Y + 7, true)

void Draw_Character_Status(pcnum index, int x, int y)
{
	char buffer[9];
	pc *pc;
	grf *portrait;
	point2d point;

	pc = Get_PC(index);
	if (!pc) return;

	if (pc->header.portrait_id != 0) {
		portrait = Lookup_File_Chained(gDjn, pc->header.portrait_id);
		point.x = x;
		point.y = y;
		Draw_GRF(&point, portrait, 1, 0);
	}

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

bool Is_Armour(item *it)
{
	switch (it->type)
	{
		case itShield:
		case itHelmet:
		case itBodyArmour:
		case itFootwear:
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

	it = Lookup_File_Chained(gDjn, iv->item);
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

file_id Get_Equipped_Item_Id(pc *pc, itemslot sl)
{
	int i;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		if (pc->header.items[i].slot == sl) {
			return pc->header.items[i].item;
		}
	}

	return 0;
}

item *Get_Equipped_Item(pc *pc, itemslot sl)
{
	return Lookup_File_Chained(gDjn, Get_Equipped_Item_Id(pc, sl));
}

bool PC_Can_Equip(pc *pc, item *it)
{
	if (it->flags & ifHeavy) {
		switch (pc->header.job) {
			case jFighter:
			case jPure:
				return true;

			case jCorrupt:
				return Is_Weapon(it);

			default: return false;
		}
	}

	if (it->flags & ifLight) {
		switch (pc->header.job) {
			case jFighter:
			case jPure:
				return Is_Weapon(it);

			default: return true;
		}
	}

	switch (pc->header.job) {
		case jFighter:
		case jCleric:
		case jRanger:
		case jPure:
		case jCorrupt:
			return true;

		case jRogue:
			return Is_Armour(it);

		default: return false;
	}
}

char *Item_Weight(item *it)
{
	if (it->flags & ifHeavy) return "Heavy";
	if (it->flags & ifLight) return "Light";
	return "Medium";
}

char *Item_Type(item *it)
{
	if (Is_Weapon(it)) return "Weapon";
	if (Is_Armour(it)) return "Armour";
	return "Other";
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

	it = Lookup_File_Chained(gDjn, iv->item);
	if (it == null) return false;

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

	if (!PC_Can_Equip(pc, it)) {
		Log("Equip_Item_At: %s cannot equip %s (%s/%s)", jobspecs[pc->header.job].name, Resolve_String(it->name_id), Item_Weight(it), Item_Type(it));
		Confirm("Can't equip that.");
		Clear_Confirm();
		return false;
	}

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
	item *it = Lookup_File_Chained(gDjn, iid);

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

/* TODO: this kind of sucks */
file_id Get_Equipped_Weapon_Id(pc *pc, bool primary)
{
	file_id id;
	item *it;

	id = Get_Equipped_Item_Id(pc, primary ? slWeapon : slOffHand);
	it = Lookup_File_Chained(gDjn, id);

	/* shields don't count! */
	if (it == null || it->type == itShield) return 0;
	return id;
}

item *Get_Equipped_Weapon(pc *pc, bool primary)
{
	item *it;

	it = Get_Equipped_Item(pc, primary ? slWeapon : slOffHand);

	/* shields don't count! */
	if (it == null || it->type == itShield) return null;
	return it;
}

pc *Get_PC(pcnum index)
{
	file_id ref;

	assert(index >= 0, "Get_PC: pc number too low");
	assert(index < PARTY_SIZE, "Get_PC: pc number too high");

	ref = gParty->members[index];
	return Lookup_File_Chained(gSave, ref);
}

file_id Get_PC_ID(pcnum index)
{
	assert(index >= 0, "Get_PC_ID: pc number too low");
	assert(index < PARTY_SIZE, "Get_PC_ID: pc number too high");
	return gParty->members[index];
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

stat_value Get_PC_Stat(pc *pc, statistic st)
{
	assert(st < NUM_STATS, "Get_PC_Stat: stat number too high");

	return Get_Stat_Base(pc->header.stats, st) + pc->header.stats[st];
}

noexport void Show_Pc_Stat(pc *pc, statistic st, int x, int y)
{
	char *name = Stat_Name(st);
	char temp[5];

	itoa(Get_PC_Stat(pc, st), temp, 10);

	Draw_Font(x, y, WHITE, name, gFont, false);
	Draw_Font(x + 72, y, WHITE, temp, gFont, false);
}

noexport void Show_Pc_Stat_Pair(pc *pc, statistic stc, statistic stm, int x, int y)
{
	char *name = Stat_Name(stm);
	char temp[10];

	Draw_Font(x, y, WHITE, name, gFont, false);

	sprintf(temp, "%3d/%3d", Get_PC_Stat(pc, stc), Get_PC_Stat(pc, stm));
	Draw_Font(x + 72, y, WHITE, temp, gFont, false);
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
			it = Lookup_File_Chained(gDjn, iv->item);
			col = (i == selected) ? ITEM_SELECTED : WHITE;
			Draw_Font(ITEMS_X + 16, y, col, Resolve_String(it->name_id), gFont, false);

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
	int min = Get_PC_Stat(pc, sMinDamage);
	int max = Get_PC_Stat(pc, sMaxDamage);
	int i;
	inventory *iv;
	item *it;

	for (i = 0; i < INVENTORY_SIZE; i++) {
		iv = &pc->header.items[i];
		
		if (iv->flags & vfEquipped) {
			it = Lookup_File_Chained(gDjn, iv->item);

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
	grf *portrait;
	char temp[100];
	point2d pos;

	Fill_Double_Buffer(0);

	Draw_Font(8, 8, WHITE, pc->name, gFont, false);

	sprintf(temp, "Level %u %s", pc->header.job_level[pc->header.job], Job_Name(pc->header.job));
	Draw_Font(8, 16, WHITE, temp, gFont, false);

	if (pc->header.portrait_id != 0) {
		portrait = Lookup_File_Chained(gDjn, pc->header.portrait_id);
		pos.x = 8;
		pos.y = 32;
		Draw_GRF(&pos, portrait, 0, 0);
	}

	Show_Pc_Stat_Pair(pc, sHP, sMaxHP, 8, 168);
	Show_Pc_Stat_Pair(pc, sMP, sMaxMP, 8, 176);

	Show_Pc_Stat(pc, sStrength, 160, 8);
	Show_Pc_Stat(pc, sDexterity, 160, 16);
	Show_Pc_Stat(pc, sIntelligence, 160, 24);

	Show_Pc_Stat(pc, sHitBonus, 160, 40);
	Draw_Font(160, 48, WHITE, "Damage", gFont, false);
	Draw_Font(232, 48, WHITE, Get_Damage_Range(pc), gFont, false);

	Show_Pc_Stat(pc, sDodgeBonus, 160, 64);
	Show_Pc_Stat(pc, sArmour, 160, 72);
	Show_Pc_Stat(pc, sToughness, 160, 80);
}

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
			Add_Item_to_Tile(gParty->x, gParty->y, iv->item);
			redraw_fp = true;

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

noexport bool Confirm_Give_Item(pc *src, int index)
{
	inventory *iv,
		*dest_iv;
	pcnum dest_pc;
	pc *dst;
	int dest_slot;
	assert(index < INVENTORY_SIZE, "Confirm_Give_Item: inventory index too high");

	iv = &src->header.items[index];

	/* Sanity check */
	if (!iv->item) return false;

	dest_pc = Confirm_Pc("To whom? (1-6)");
	if (dest_pc < 0) {
		return false;
	}

	dst = Get_PC(dest_pc);
	if (dst == src || dst == null)
		return false;
	
	/* Equipped? */
	if (iv->flags & vfEquipped) {
		if (!Remove_Item_At(src, index)) {
			/* TODO */
			return false;
		}
	}

	dest_slot = Find_Empty_Inventory_Slot(Get_PC(dest_pc));
	if (dest_slot < 0) {
		Confirm("Inventory is full!");
		Clear_Confirm();
		return false;
	}

	dest_iv = &dst->header.items[dest_slot];
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

	it = Lookup_File_Chained(gDjn, iv->item);
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

	if (Use_Item(it, pc, Get_PC(dest_pc))) {
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
	pcnum index = starting_pc,
		pindex;
	pc *pc;
	int selected = 0,
		scan = 0;
	redraw_everything = true;

	pc = Get_PC(index);
	if (pc == null)
		return;

	while (true) {
		pindex = index;

		Show_Pc_Stats(pc);
		Show_Pc_Items(pc, selected);
		Show_Double_Buffer();

		switch (Get_Next_Scan_Code()) {
			case SCAN_1:
				index = 0;
				break;

			case SCAN_2:
				index = 1;
				break;

			case SCAN_3:
				index = 2;
				break;

			case SCAN_4:
				index = 3;
				break;

			case SCAN_5:
				index = 4;
				break;

			case SCAN_6:
				index = 5;
				break;

			case SCAN_LEFT:
				scan = -1;
				break;

			case SCAN_RIGHT:
				scan = 1;
				break;

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

		if (index != pindex) {
			pc = Get_PC(index);
			if (pc == null) {
				index = pindex;
				pc = Get_PC(index);
			}
		}

		if (scan) {
			while (true) {
				index += scan;
				if (index >= PARTY_SIZE) index = 0;
				if (index < 0) index = PARTY_SIZE - 1;

				pc = Get_PC(index);
				if (pc != null) break;
			}
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

pc *Duplicate_PC(pc *org)
{
	pc *dup = SzAlloc(1, pc, "Duplicate_PC");

	memcpy(&dup->header, &org->header, sizeof(pc_header));

	dup->skills = Duplicate_List(org->skills, "Duplicate_PC.skills");
	dup->buffs = Duplicate_List(org->buffs, "Duplicate_PC.buffs");

	if (org->header.name_id == 0)
		dup->name = Duplicate_String(org->name, "Duplicate_PC.name");
	else
		dup->name = org->name;

	return dup;
}

file_id Pick_Portrait(file_id org)
{
	file_id ref = org;
	file_id *buffer = SzAlloc(PARTY_PORTRAITS, file_id, "Pick_Portrait");
	unsigned char ch;
	grf *g;
	bool done = false;
	int count, i;

	count = Find_Files_of_Type(gDjn, ftGraphic, stPortrait, buffer, 20);
	if (count != 0) {
		/* find current graphic */
		for (i = 0; i < count; i++) {
			if (buffer[i] == org) break;
		}

		if (i == count) i = 0;

		redraw_everything = true;
		Fill_Double_Buffer(0);
		/* TODO: show UI? */

		Draw_Font(8, 8, 0, "Pick a portrait:", gFont, true);
		Draw_Font_Char(80, 84, 0, '<', gFont, true);
		Draw_Font_Char(232, 84, 0, '>', gFont, true);

		while (!done) {
			ref = buffer[i];
			g = Lookup_File_Chained(gDjn, ref);

			Draw_Square_DB(0, portrait_pick.x, portrait_pick.y, portrait_pick.x + 127, portrait_pick.y + 127, true);
			Draw_GRF(&portrait_pick, g, 0, 0);
			Show_Double_Buffer();

			ch = Get_Next_Scan_Code();

			switch (ch) {
				case SCAN_LEFT:
				case SCAN_COMMA:
					i--;
					if (i < 0) i = count - 1;
					break;

				case SCAN_RIGHT:
				case SCAN_PERIOD:
					i++;
					if (i == count) i = 0;
					break;

				case SCAN_ENTER:
					done = true;
					break;
			}
		}
	}

	Free(buffer);
	return ref;
}

bool Add_Item_to_Party(file_id item_id, int qty, pcnum *given_to)
{
	pcnum index;
	pc *pc;

	for (index = 0; index < PARTY_SIZE; index++) {
		pc = Get_PC(index);
		if (!pc) continue;

		if (Add_to_Inventory(pc, item_id, qty)) {
			*given_to = index;
			return true;
		}
	}

	*given_to = -1;
	return false;
}
