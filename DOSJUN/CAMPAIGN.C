/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Campaign(campaign *c)
{
	c->zones = null;
}

void Load_Campaign(char *filename, campaign *c)
{
	int i;

	FILE *fp = fopen(filename, "rb");
	if (!fp) IO_Error("Could not open campaign");
	fread(&c->header, sizeof(campaign_header), 1, fp);
	/* TODO: Check magic/version */

	c->zones = szalloc(c->header.num_zones, char *);
	for (i = 0; i < c->header.num_zones; i++) {
		c->zones[i] = Read_LengthString(fp);
	}

	fclose(fp);
}

void Free_Campaign(campaign *c)
{
	int i;

	if (c->zones != null) {
		for (i = 0; i < c->header.num_zones; i++) {
			free(c->zones[i]);
		}
		nullfree(c->zones);
	}
}

void Save_Campaign(char *filename, campaign *c)
{
	int i;
	FILE *fp = fopen(filename, "wb");

	Set_Version_Header(c->header);
	fwrite(&c->header, sizeof(campaign_header), 1, fp);

	for (i = 0; i < c->header.num_zones; i++) {
		Write_LengthString(c->zones[i], fp);
	}

	fclose(fp);
}
