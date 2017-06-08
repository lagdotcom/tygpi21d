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

	z->tiles = null;
	z->strings = null;
	z->scripts = null;
	z->script_lengths = null;
	z->encounters = null;
}

void Load_Zone(char *filename, zone *z)
{
	int i;
	zone_header *h = &z->header;
	FILE *fp = fopen(filename, "rb");
	if (!fp) IO_Error("Could not open zone");

	fread(h, sizeof(zone_header), 1, fp);
	/* TODO: Check magic/version */

	z->tiles = szalloc(h->width * h->height, tile);
	fread(z->tiles, sizeof(tile), h->width * h->height, fp);

	if (h->num_strings > 0) {
		z->strings = szalloc(h->num_strings, char *);
		for (i = 0; i < h->num_strings; i++)
			z->strings[i] = Read_LengthString(fp);
	} else {
		z->strings = null;
	}

	if (h->num_scripts > 0) {
		z->scripts = szalloc(h->num_scripts, char *);
		z->script_lengths = szalloc(h->num_scripts, length);
		for (i = 0; i < h->num_scripts; i++) {
			fread(&z->script_lengths[i], sizeof(length), 1, fp);
			z->scripts[i] = szalloc(z->script_lengths[i], bytecode);
			fread(&z->scripts[i], 1, z->script_lengths[i], fp);
		}
	} else {
		z->scripts = null;
	}

	if (h->num_encounters > 0) {
		z->encounters = szalloc(h->num_encounters, encounter);
		fread(z->encounters, sizeof(encounter), h->num_encounters, fp);
	}

	fclose(fp);
}

void Free_Zone(zone *z)
{
	int i;

	nullfree(z->tiles);

	if (z->strings != null) {
		for (i = 0; i < z->header.num_strings; i++) {
			free(z->strings[i]);
		}
		nullfree(z->strings);
	}

	if (z->scripts != null) {
		for (i = 0; i < z->header.num_scripts; i++) {
			free(z->scripts[i]);
		}
		nullfree(z->scripts);
		nullfree(z->script_lengths);
	}

	nullfree(z->encounters);
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

	fclose(fp);
}
