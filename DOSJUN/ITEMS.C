/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Items_Load(char *filename, items *i)
{
	FILE *fp = fopen(filename, "rb");
	fread(&i->header, sizeof(items_header), 1, fp);
	/* TODO: Check magic/version */

	i->items = malloc(sizeof(item) * i->header.num_items);
	fread(i->items, sizeof(item), i->header.num_items, fp);

	fclose(fp);
}

void Items_Free(items *i)
{
	Free_If_Null(i->items);
}

void Items_Init(items *i)
{
	i->items = null;
}

void Items_Save(char *filename, items *i)
{
	FILE *fp = fopen(filename, "wb");

	Set_Version_Header(i->header);
	fwrite(&i->header, sizeof(items_header), 1, fp);
	fwrite(i->items, sizeof(item), i->header.num_items, fp);
	fclose(fp);
}
