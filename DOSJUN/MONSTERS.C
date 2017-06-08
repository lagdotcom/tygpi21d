/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Load_Monsters(char *filename, monsters *m)
{
	FILE *fp = fopen(filename, "rb");
	fread(&m->header, sizeof(monsters_header), 1, fp);
	/* TODO: Check magic/version */

	m->monsters = SzAlloc(m->header.num_monsters, monster, "Load_Monsters");
	fread(m->monsters, sizeof(monster), m->header.num_monsters, fp);

	fclose(fp);
}

void Free_Monsters(monsters *m)
{
	Free(m->monsters);
}

void Initialise_Monsters(monsters *m)
{
	m->monsters = null;
}

void Save_Monsters(char *filename, monsters *m)
{
	FILE *fp = fopen(filename, "wb");

	Set_Version_Header(m->header);
	fwrite(&m->header, sizeof(monsters_header), 1, fp);
	fwrite(m->monsters, sizeof(monster), m->header.num_monsters, fp);
	fclose(fp);
}
