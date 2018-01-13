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
