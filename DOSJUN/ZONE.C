/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "zone.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Zone(zone *z)
{
	z->header.num_strings = 0;
	z->header.num_scripts = 0;
	z->header.num_encounters = 0;
	z->header.num_code_strings = 0;
	z->header.num_etables = 0;
	z->header.num_textures = 0;

	z->tiles = null;
	z->strings = null;
	z->scripts = null;
	z->script_lengths = null;
	z->encounters = null;
	z->etables = null;
	z->code_strings = null;
	z->textures = null;
}

bool Load_Zone(char *filename, zone *z)
{
	unsigned int i;
	zone_header *h = &z->header;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Could not open for reading: %s\n", filename);
		return false;
	}

	fread(h, sizeof(zone_header), 1, fp);
	Check_Version_Header(z->header);

	z->tiles = SzAlloc(h->width * h->height, tile, "Load_Zone.tiles");
	if (z->tiles == null) goto _dead;
	fread(z->tiles, sizeof(tile), h->width * h->height, fp);

	if (h->num_strings > 0) {
		z->strings = SzAlloc(h->num_strings, char *, "Load_Zone.strings");
		if (z->strings == null) goto _dead;
		for (i = 0; i < h->num_strings; i++)
			z->strings[i] = Read_LengthString(fp, "Load_Zone.strings.n");
	} else {
		z->strings = null;
	}

	if (h->num_scripts > 0) {
		z->scripts = SzAlloc(h->num_scripts, bytecode *, "Load_Zone.scripts");
		z->script_lengths = SzAlloc(h->num_scripts, length, "Load_Zone.script_lengths");
		if (z->scripts == null || z->script_lengths == null) goto _dead;

		for (i = 0; i < h->num_scripts; i++) {
			fread(&z->script_lengths[i], sizeof(length), 1, fp);
			z->scripts[i] = SzAlloc(z->script_lengths[i], bytecode, "Load_Zone.scripts.i");
			if (z->scripts[i] == null) goto _dead;
			fread(z->scripts[i], sizeof(bytecode), z->script_lengths[i], fp);
		}
	} else {
		z->scripts = null;
		z->script_lengths = null;
	}

	if (h->num_encounters > 0) {
		z->encounters = SzAlloc(h->num_encounters, encounter, "Load_Zone.encounters");
		if (z->encounters == null) goto _dead;
		fread(z->encounters, sizeof(encounter), h->num_encounters, fp);
	} else {
		z->encounters = null;
	}

	if (h->num_code_strings > 0) {
		z->code_strings = SzAlloc(h->num_code_strings, char *, "Load_Zone.code_strings");
		if (z->code_strings == null) goto _dead;
		for (i = 0; i < h->num_code_strings; i++)
			z->code_strings[i] = Read_LengthString(fp, "Load_Zone.code_strings.n");
	} else {
		z->code_strings = null;
	}

	if (h->num_etables > 0) {
		z->etables = SzAlloc(h->num_etables, etable, "Load_Zone.etables");
		if (z->etables == null) goto _dead;
		fread(z->etables, sizeof(etable), h->num_etables, fp);
	} else {
		z->etables = null;
	}

	if (h->num_textures > 0) {
		z->textures = SzAlloc(h->num_textures, char *, "Load_Zone.textures");
		if (z->textures == null) goto _dead;
		for (i = 0; i < h->num_textures; i++)
			z->textures[i] = Read_LengthString(fp, "Load_Zone.textures.n");
	} else {
		z->textures = null;
	}

	fclose(fp);
	return true;

_dead:
	die("Load_Zone: out of memory");
}

void Free_Zone(zone *z)
{
	unsigned int i;

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

	if (z->textures != null) {
		for (i = 0; i < z->header.num_textures; i++) {
			Free(z->textures[i]);
		}
		Free(z->textures);
	}
}

bool Save_Zone(char *filename, zone *z)
{
	unsigned int i;
	zone_header *h = &z->header;
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		printf("Could not open for writing: %s\n", filename);
		return false;
	}

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

	for (i = 0; i < h->num_textures; i++) {
		Write_LengthString(z->textures[i], fp);
	}

	fclose(fp);
	return true;
}
