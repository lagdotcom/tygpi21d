/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Read_Campaign(FILE *fp, campaign *c)
{
	int i;

	fread(&c->header, sizeof(campaign_header), 1, fp);
	Check_Version_Header(c->header);

	return true;
}

bool Load_Campaign(char *filename, campaign *c)
{
	bool res;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		dief("Load_Campaign: Could not open for reading: %s\n", filename);
		return false;
	}

	Log("Load_Campaign: %s", filename);

	res = Read_Campaign(fp, c);
	fclose(fp);
	return res;
}

void Free_Campaign(campaign *c)
{
	int i;

	Log("Free_Campaign: %p", c);
}

bool Save_Campaign(char *filename, campaign *c)
{
	int i;
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		dief("Save_Campaign: Could not open for writing: %s\n", filename);
		return false;
	}

	Set_Version_Header(c->header);
	fwrite(&c->header, sizeof(campaign_header), 1, fp);

	fclose(fp);
	return true;
}
