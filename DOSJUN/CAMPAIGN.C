/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Campaign(campaign *c)
{
	c->zones = null;
}

bool Load_Campaign(char *filename, campaign *c)
{
	int i;

	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Could not open for reading: %s\n", filename);
		return false;
	}

	fread(&c->header, sizeof(campaign_header), 1, fp);
	Check_Version_Header(c->header);

	c->zones = SzAlloc(c->header.num_zones, char *, "Load_Campaign");
	for (i = 0; i < c->header.num_zones; i++) {
		c->zones[i] = Read_LengthString(fp);
	}

	fclose(fp);
	return true;
}

void Free_Campaign(campaign *c)
{
	int i;

	if (c->zones != null) {
		for (i = 0; i < c->header.num_zones; i++) {
			Free(c->zones[i]);
		}
		Free(c->zones);
	}
}

bool Save_Campaign(char *filename, campaign *c)
{
	int i;
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		printf("Could not open for writing: %s\n", filename);
		return false;
	}

	Set_Version_Header(c->header);
	fwrite(&c->header, sizeof(campaign_header), 1, fp);

	for (i = 0; i < c->header.num_zones; i++) {
		Write_LengthString(c->zones[i], fp);
	}

	fclose(fp);
	return true;
}
