/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Campaign_Init(campaign *c)
{
	c->zones = null;
}

void Campaign_Load(char *filename, campaign *c)
{
	int i;

	FILE *fp = fopen(filename, "rb");
	if (!fp) IO_Error("Could not open campaign");
	fread(&c->header, sizeof(campaign_header), 1, fp);
	/* TODO: Check magic/version */

	c->zones = malloc(sizeof(char*) * c->header.num_zones);
	for (i = 0; i < c->header.num_zones; i++) {
		c->zones[i] = Get_String(fp);
	}

	fclose(fp);
}

void Campaign_Free(campaign *c)
{
	int i;

	if (c->zones != null) {
		for (i = 0; i < c->header.num_zones; i++) {
			free(c->zones[i]);
		}
		Free_If_Null(c->zones);
	}
}

void Campaign_Save(char *filename, campaign *c)
{
	int i;
	FILE *fp = fopen(filename, "wb");

	Set_Version_Header(c->header);
	fwrite(&c->header, sizeof(campaign_header), 1, fp);

	for (i = 0; i < c->header.num_zones; i++) {
		Save_String(c->zones[i], fp);
	}

	fclose(fp);
}
