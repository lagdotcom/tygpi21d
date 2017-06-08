/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Savefile(save *s)
{
	/* Nothing! */
}

void Load_Savefile(char *filename, save *s)
{
	FILE *fp = fopen(filename, "rb");
	fread(s, sizeof(save), 1, fp);

	/* TODO: Check magic/version */
	fclose(fp);
}

void Free_Savefile(save *s)
{
	/* Nothing! */
}

void Save_Savefile(char *filename, save *s)
{
	FILE *fp = fopen(filename, "wb");

	Set_Version_Header(s->header);
	fwrite(s, sizeof(save), 1, fp);
	fclose(fp);
}
