/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Load_Items(char *filename, items *i)
{
	FILE *fp = fopen(filename, "rb");
	fread(&i->header, sizeof(items_header), 1, fp);
	/* TODO: Check magic/version */

	i->items = SzAlloc(i->header.num_items, item, "Load_Items");
	fread(i->items, sizeof(item), i->header.num_items, fp);

	fclose(fp);
}

void Free_Items(items *i)
{
	Free(i->items);
}

void Initialise_Items(items *i)
{
	i->items = null;
}

void Save_Items(char *filename, items *i)
{
	FILE *fp = fopen(filename, "wb");

	Set_Version_Header(i->header);
	fwrite(&i->header, sizeof(items_header), 1, fp);
	fwrite(i->items, sizeof(item), i->header.num_items, fp);
	fclose(fp);
}
