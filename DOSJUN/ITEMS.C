/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Read_Item(FILE *fp, item *i)
{
	fread(i, sizeof(item), 1, fp);
	return true;
}

bool Save_Item(char *filename, item *i)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		dief("Save_Item: Could not open for writing: %s\n", filename);
		return false;
	}

	fwrite(i, sizeof(item), 1, fp);
	fclose(fp);
	return true;
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

noexport bool Healing_Item(item *it, pc *pc)
{
	stat_value current, maximum;
	int amount;

	current = Get_PC_Stat(pc, sHP);
	maximum = Get_PC_Stat(pc, sMaxHP);

	if (current >= maximum)
		return false;

	amount = randint(it->special_argument1, it->special_argument2);
	if (current + amount > maximum)
		amount = maximum - current;

	pc->header.stats[sHP] += amount;
	return true;
}

bool Use_Item(item *it, pc *user, pc *targ)
{
	switch (it->special) {
		case spHeal: return Healing_Item(it, targ);
		default: return false;
	}
}

bool Read_DropTable(FILE *fp, droptable *dt)
{
	fread(dt, DROPTABLE_HEADERSZ, 1, fp);
	dt->drops = SzAlloc(dt->count, drop, "Read_DropTable");
	fread(dt->drops, sizeof(drop), dt->count, fp);

	return true;
}

void Free_DropTable(droptable *dt)
{
	Free(dt->drops);
}

file_id Get_Drop(droptable *dt)
{
	drop *d;
	file_id ref;
	int i;

	for (d = dt->drops, i = 0; i < dt->count; d++, i++) {
		if (randint(0, 99) >= d->chance) continue;

		if (d->flags && drfTable) {
			ref = Get_Drop(Lookup_File(gDjn, d->ref, true));
			if (ref) return ref;
		} else {
			return d->ref;
		}
	}

	return 0;
}
