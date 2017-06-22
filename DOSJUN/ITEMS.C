/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"
#include "items.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Load_Items(char *filename, items *i)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Could not open for reading: %s\n", filename);
		return false;
	}

	fread(&i->header, sizeof(items_header), 1, fp);
	Check_Version_Header(i->header);

	i->items = SzAlloc(i->header.num_items, item, "Load_Items");
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
	if (!fp) {
		printf("Could not open for writing: %s\n", filename);
		return false;
	}

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
