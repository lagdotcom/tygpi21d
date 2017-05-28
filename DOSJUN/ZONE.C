/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Zone_Load(char *filename)
{
	FILE *fp = fopen(filename, "rb");
	fread(&Z, sizeof(zone), 1, fp);
	fclose(fp);
}

void Zone_Save(char *filename)
{
	FILE *fp = fopen(filename, "wb");
	fwrite(&Z, sizeof(zone), 1, fp);
	fclose(fp);
}
