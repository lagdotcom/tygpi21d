/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Load_Items(char *filename, items *i)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp)
		dief("Load_Items: Could not open for reading: %s\n", filename);

	fread(&i->header, sizeof(items_header), 1, fp);
	Check_Version_Header(i->header);

	i->items = SzAlloc(i->header.num_items, item, "Load_Items");
	if (i->items == null)
		die("Load_Items: out of memory");

	fread(i->items, sizeof(item), i->header.num_items, fp);

	fclose(fp);
	return true;
}

void Free_Items(items *i)
{
	Log("Free_Items: %p", i);

	Free(i->items);
}

void Initialise_Items(items *i)
{
	i->items = null;
}

bool Save_Items(char *filename, items *i)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		dief("Save_Items: Could not open for writing: %s\n", filename);

	Set_Version_Header(i->header);
	fwrite(&i->header, sizeof(items_header), 1, fp);
	fwrite(i->items, sizeof(item), i->header.num_items, fp);
	fclose(fp);
	return true;
}

item *Lookup_Item(items *lib, item_id id)
{
	int i;
	for (i = 0; i < lib->header.num_items; i++) {
		if (lib->items[i].id == id) return &lib->items[i];
	}

	return null;
}

bool Item_Has_Use(item *it)
{
	return it->special != spNone;
}

bool Item_Needs_Target(item *it)
{
	switch (it->special) {
		case spHeal:
			return true;

		default: return false;
	}
}

noexport bool Healing_Item(item *it, int pc)
{
	character *c = &gSave.characters[pc];
	stat_value current, maximum;
	int amount;

	current = Get_Pc_Stat(c, sHP);
	maximum = Get_Pc_Stat(c, sMaxHP);

	if (current >= maximum)
		return false;

	amount = randint(it->special_argument1, it->special_argument2);
	if (current + amount > maximum)
		amount = maximum - current;

	c->header.stats[sHP] += amount;
	return true;
}

bool Use_Item(item *it, int pc, int pc_targ)
{
	switch (it->special) {
		case spHeal: return Healing_Item(it, pc_targ);
		default: return false;
	}
}
