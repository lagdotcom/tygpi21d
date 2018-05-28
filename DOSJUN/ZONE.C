/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Zone(zone *z)
{
	z->header.num_encounters = 0;
	z->header.num_etables = 0;
	z->header.num_items = 0;

	z->tiles = null;
	z->encounters = null;
	z->etables = null;
	z->items = null;
}

bool Load_Zone(char *filename, zone *z)
{
	unsigned int i;
	zone_header *h = &z->header;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		dief("Load_Zone: Could not open for reading: %s\n", filename);
		return false;
	}

	Log("Load_Zone: %s", filename);

	fread(h, sizeof(zone_header), 1, fp);
	Check_Version_Header(z->header);

	z->tiles = SzAlloc(h->width * h->height, tile, "Load_Zone.tiles");
	if (z->tiles == null) goto _dead;
	fread(z->tiles, sizeof(tile), h->width * h->height, fp);

	if (h->num_encounters > 0) {
		z->encounters = SzAlloc(h->num_encounters, encounter, "Load_Zone.encounters");
		if (z->encounters == null) goto _dead;
		fread(z->encounters, sizeof(encounter), h->num_encounters, fp);
	} else {
		z->encounters = null;
	}

	if (h->num_etables > 0) {
		z->etables = SzAlloc(h->num_etables, etable, "Load_Zone.etables");
		if (z->etables == null) goto _dead;
		fread(z->etables, sizeof(etable), h->num_etables, fp);
	} else {
		z->etables = null;
	}

	if (h->num_items > 0) {
		z->items = SzAlloc(h->num_items, itempos, "Load_Zone.items");
		if (z->items == null) goto _dead;
		fread(z->items, sizeof(itempos), h->num_items, fp);
	} else {
		z->items = null;
	}

	fclose(fp);
	return true;

_dead:
	die("Load_Zone: out of memory");
	return false;
}

void Free_Zone(zone *z)
{
	unsigned int i;

	Log("Free_Zone: %p", z);

	Free(z->tiles);
	Free(z->encounters);
	Free(z->etables);
	Free(z->items);
}

bool Save_Zone(char *filename, zone *z)
{
	unsigned int i;
	zone_header *h = &z->header;
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		dief("Save_Zone: Could not open for writing: %s\n", filename);
		return false;
	}

	Set_Version_Header(z->header);
	fwrite(h, sizeof(zone_header), 1, fp);
	fwrite(z->tiles, sizeof(tile), h->width * h->height, fp);
	fwrite(z->encounters, sizeof(encounter), h->num_encounters, fp);
	fwrite(z->etables, sizeof(etable), h->num_etables, fp);
	fwrite(z->items, sizeof(itempos), h->num_items, fp);

	fclose(fp);
	return true;
}
