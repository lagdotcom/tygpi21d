/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include "monsters.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Load_Monsters(char *filename, monsters *m)
{
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Could not open for reading: %s\n", filename);
		return false;
	}

	fread(&m->header, sizeof(monsters_header), 1, fp);
	Check_Version_Header(m->header);

	m->monsters = SzAlloc(m->header.num_monsters, monster, "Load_Monsters");
	if (m->monsters == null) die("Load_Monsters: out of memory");
	fread(m->monsters, sizeof(monster), m->header.num_monsters, fp);

	fclose(fp);
	return true;
}

void Free_Monsters(monsters *m)
{
	Free(m->monsters);
}

void Initialise_Monsters(monsters *m)
{
	m->monsters = null;
}

bool Save_Monsters(char *filename, monsters *m)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		printf("Could not open for writing: %s\n", filename);
		return false;
	}

	Set_Version_Header(m->header);
	fwrite(&m->header, sizeof(monsters_header), 1, fp);
	fwrite(m->monsters, sizeof(monster), m->header.num_monsters, fp);
	fclose(fp);
	return true;
}

monster *Lookup_Monster(monsters *lib, monster_id id)
{
	int i;
	for (i = 0; i < lib->header.num_monsters; i++) {
		if (lib->monsters[i].id == id) return &lib->monsters[i];
	}

	return null;
}
