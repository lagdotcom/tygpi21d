/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <alloc.h>
#include <stdio.h>
#include "files.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NFREE(r) if (r != null) { \
	free(r); \
	r = null; \
}

/* F U N C T I O N S ///////////////////////////////////////////////////// */

char* Get_String(FILE* fp)
{
	char* string;
	length len;

	fread(&len, sizeof(length), 1, fp);
	string = malloc(len + 1);
	fread(string, 1, len + 1, fp);
	return string;
}

void Savefile_Load(char *filename, save *s)
{
	FILE *fp = fopen(filename, "rb");
	fread(s, sizeof(save), 1, fp);

	/* TODO: Check magic/version */
	fclose(fp);
}

void Savefile_Free(save *s)
{
	/* Nothing! */
}

void Zone_Load(char *filename, zone *z)
{
	int i;
	zone_header *h = &z->header;
	FILE *fp = fopen(filename, "rb");

	fread(h, sizeof(zone_header), 1, fp);
	/* TODO: Check magic/version */

	z->tiles = malloc(sizeof(tile) * h->width * h->height);
	fread(z->tiles, sizeof(tile), h->width * h->height, fp);

	if (h->num_strings > 0) {
		z->strings = malloc(sizeof(char*) * h->num_strings);
		for (i = 0; i < h->num_strings; i++)
			z->strings[i] = Get_String(fp);
	} else {
		z->strings = null;
	}

	if (h->num_scripts > 0) {
		z->scripts = malloc(sizeof(char*) * h->num_scripts);
		z->script_lengths = malloc(sizeof(length) * h->num_scripts);
		for (i = 0; i < h->num_scripts; i++) {
			fread(&z->script_lengths[i], sizeof(length), 1, fp);
			z->scripts[i] = malloc(z->script_lengths[i]);
			fread(&z->scripts[i], 1, z->script_lengths[i], fp);
		}
	} else {
		z->scripts = null;
	}

	fclose(fp);
}

void Zone_Free(zone *z)
{
	int i;

	NFREE(z->tiles);

	if (z->strings != null) {
		for (i = 0; i < z->header.num_strings; i++) {
			free(z->strings[i]);
		}
		free(z->strings);
	}

	if (z->scripts != null) {
		for (i = 0; i < z->header.num_scripts; i++) {
			free(z->scripts[i]);
		}
		free(z->scripts);
		free(z->script_lengths);
	}
}
