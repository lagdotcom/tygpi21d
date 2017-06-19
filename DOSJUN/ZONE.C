/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Zone(zone *z)
{
	z->header.num_strings = 0;
	z->header.num_scripts = 0;
	z->header.num_encounters = 0;
	z->header.num_code_strings = 0;
	z->header.num_etables = 0;

	z->tiles = null;
	z->strings = null;
	z->scripts = null;
	z->script_lengths = null;
	z->encounters = null;
	z->etables = null;
	z->code_strings = null;
}

void Load_Zone(char *filename, zone *z)
{
	int i;
	zone_header *h = &z->header;
	FILE *fp = fopen(filename, "rb");
	if (!fp) IO_Error("Could not open zone");

	fread(h, sizeof(zone_header), 1, fp);
	/* TODO: Check magic/version */

	z->tiles = SzAlloc(h->width * h->height, tile, "Load_Zone.tiles");
	fread(z->tiles, sizeof(tile), h->width * h->height, fp);

	if (h->num_strings > 0) {
		z->strings = SzAlloc(h->num_strings, char *, "Load_Zone.strings");
		for (i = 0; i < h->num_strings; i++)
			z->strings[i] = Read_LengthString(fp);
	} else {
		z->strings = null;
	}

	if (h->num_scripts > 0) {
		z->scripts = SzAlloc(h->num_scripts, bytecode *, "Load_Zone.scripts");
		z->script_lengths = SzAlloc(h->num_scripts, length, "Load_Zone.script_lengths");
		for (i = 0; i < h->num_scripts; i++) {
			fread(&z->script_lengths[i], sizeof(length), 1, fp);
			z->scripts[i] = SzAlloc(z->script_lengths[i], bytecode, "Load_Zone.scripts[i]");
			fread(z->scripts[i], sizeof(bytecode), z->script_lengths[i], fp);
		}
	} else {
		z->scripts = null;
		z->script_lengths = null;
	}

	if (h->num_encounters > 0) {
		z->encounters = SzAlloc(h->num_encounters, encounter, "Load_Zone.encounters");
		fread(z->encounters, sizeof(encounter), h->num_encounters, fp);
	} else {
		z->encounters = null;
	}

	if (h->num_code_strings > 0) {
		z->code_strings = SzAlloc(h->num_code_strings, char *, "Load_Zone.code_strings");
		for (i = 0; i < h->num_code_strings; i++)
			z->code_strings[i] = Read_LengthString(fp);
	} else {
		z->code_strings = null;
	}

	if (h->num_etables > 0) {
		z->etables = SzAlloc(h->num_etables, etable, "Load_Zone.etables");
		fread(z->etables, sizeof(etable), h->num_etables, fp);
	} else {
		z->etables = null;
	}

	fclose(fp);
}

void Free_Zone(zone *z)
{
	int i;

	Free(z->tiles);

	if (z->strings != null) {
		for (i = 0; i < z->header.num_strings; i++) {
			Free(z->strings[i]);
		}
		Free(z->strings);
	}

	if (z->scripts != null) {
		for (i = 0; i < z->header.num_scripts; i++) {
			Free(z->scripts[i]);
		}
		Free(z->scripts);
		Free(z->script_lengths);
	}

	Free(z->encounters);

	if (z->code_strings != null) {
		for (i = 0; i < z->header.num_code_strings; i++) {
			Free(z->code_strings[i]);
		}
		Free(z->code_strings);
	}

	Free(z->etables);
}

void Save_Zone(char *filename, zone *z)
{
	int i;
	zone_header *h = &z->header;
	FILE *fp = fopen(filename, "wb");

	Set_Version_Header(z->header);
	fwrite(h, sizeof(zone_header), 1, fp);
	fwrite(z->tiles, sizeof(tile), h->width * h->height, fp);

	for (i = 0; i < h->num_strings; i++) {
		Write_LengthString(z->strings[i], fp);
	}

	for (i = 0; i < h->num_scripts; i++) {
		fwrite(&z->script_lengths[i], sizeof(length), 1, fp);
		fwrite(z->scripts[i], 1, z->script_lengths[i], fp);
	}

	fwrite(z->encounters, sizeof(encounter), h->num_encounters, fp);

	for (i = 0; i < h->num_code_strings; i++) {
		Write_LengthString(z->code_strings[i], fp);
	}

	fwrite(z->etables, sizeof(etable), h->num_etables, fp);

	fclose(fp);
}
