/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SET_VERSION_HEADER(h) strncpy(h.magic, FILE_MAGIC, 3); h.version = VERSION_1

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Save_String(char *string, FILE *fp)
{
	length len;

	len = strlen(string);
	fwrite(&len, sizeof(length), 1, fp);
	fwrite(string, 1, len + 1, fp);
}

void Campaign_Save(char *filename, campaign *c)
{
	int i;
	FILE *fp = fopen(filename, "wb");

	SET_VERSION_HEADER(c->header);
	fwrite(&c->header, sizeof(campaign_header), 1, fp);

	for (i = 0; i < c->header.num_zones; i++) {
		Save_String(c->zones[i], fp);
	}

	fclose(fp);
}

void Savefile_Save(char *filename, save *s)
{
	FILE *fp = fopen(filename, "wb");

	SET_VERSION_HEADER(s->header);
	fwrite(s, sizeof(save), 1, fp);
	fclose(fp);
}

void Zone_Save(char *filename, zone *z)
{
	int i;
	zone_header *h = &z->header;
	FILE *fp = fopen(filename, "wb");

	SET_VERSION_HEADER(z->header);
	fwrite(&z->header, sizeof(zone_header), 1, fp);
	fwrite(z->tiles, sizeof(tile), h->width * h->height, fp);

	for (i = 0; i < h->num_strings; i++) {
		Save_String(z->strings[i], fp);
	}

	for (i = 0; i < h->num_scripts; i++) {
		fwrite(&z->script_lengths[i], sizeof(length), 1, fp);
		fwrite(z->scripts[i], 1, z->script_lengths[i], fp);
	}

	fclose(fp);
}
