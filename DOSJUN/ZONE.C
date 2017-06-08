/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Zone(zone *z)
{
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

	z->tiles = malloc(sizeof(tile) * h->width * h->height);
	fread(z->tiles, sizeof(tile), h->width * h->height, fp);

	if (h->num_strings > 0) {
		z->strings = malloc(sizeof(char*) * h->num_strings);
		for (i = 0; i < h->num_strings; i++)
			z->strings[i] = Read_LengthString(fp);
	} else {
		z->strings = null;
	}

	if (h->num_scripts > 0) {
		z->scripts = malloc(sizeof(char*) * h->num_scripts);
		z->script_lengths = malloc(sizeof(length) * h->num_scripts);
		for (i = 0; i < h->num_scripts; i++) {
			fread(&z->script_lengths[i], sizeof(length), 1, fp);
			z->scripts[i] = malloc(sizeof(bytecode) * z->script_lengths[i]);
			fread(&z->scripts[i], 1, z->script_lengths[i], fp);
		}
	} else {
		z->scripts = null;
	}

	if (h->num_encounters > 0) {
		z->encounters = malloc(sizeof(encounter) * h->num_encounters);
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
	fwrite(&z->header, sizeof(zone_header), 1, fp);
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
