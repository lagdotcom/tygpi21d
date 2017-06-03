/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "files.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Monsters_Load(char *filename, monsters *m)
{
	FILE *fp = fopen(filename, "rb");
	fread(&m->header, sizeof(monsters_header), 1, fp);
	/* TODO: Check magic/version */

	m->monsters = malloc(sizeof(monster) * m->header.num_monsters);
	fread(m->monsters, sizeof(monster), m->header.num_monsters, fp);

	fclose(fp);
}

void Monsters_Free(monsters *m)
{
	Free_If_Null(m->monsters);
}

void Monsters_Init(monsters *m)
{
	m->monsters = null;
}

void Monsters_Save(char *filename, monsters *m)
{
	FILE *fp = fopen(filename, "wb");

	Set_Version_Header(m->header);
	fwrite(&m->header, sizeof(monsters_header), 1, fp);
	fwrite(m->monsters, sizeof(monster), m->header.num_monsters, fp);
	fclose(fp);
}
