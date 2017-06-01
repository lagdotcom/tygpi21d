/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Savefile_Init(save *s)
{
	/* Nothing! */
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

void Savefile_Save(char *filename, save *s)
{
	FILE *fp = fopen(filename, "wb");

	Set_Version_Header(s->header);
	fwrite(s, sizeof(save), 1, fp);
	fclose(fp);
}
