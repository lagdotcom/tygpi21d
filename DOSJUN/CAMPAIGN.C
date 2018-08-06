/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Read_Campaign(FILE *fp, campaign *c)
{
	fread(c, sizeof(campaign), 1, fp);
	Check_Version_Header_p(c, "Read_Campaign");

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

bool Save_Campaign(char *filename, campaign *c)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		dief("Save_Campaign: Could not open for writing: %s\n", filename);
		return false;
	}

	Set_Version_Header_p(c);
	fwrite(c, sizeof(campaign), 1, fp);

	fclose(fp);
	return true;
}
