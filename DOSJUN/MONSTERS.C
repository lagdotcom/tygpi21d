/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Load_Monsters(char *filename, monsters *m)
{
	int i;
	FILE *fp = fopen(filename, "rb");
	monster *mon;
	if (!fp) {
		dief("Load_Monsters: Could not open for reading: %s\n", filename);
		return false;
	}

	fread(&m->header, sizeof(monsters_header), 1, fp);
	Check_Version_Header(m->header);

	m->monsters = SzAlloc(m->header.num_monsters, monster, "Load_Monsters");
	if (m->monsters == null) die("Load_Monsters: out of memory");

	mon = m->monsters;
	for (i = 0; i < m->header.num_monsters; i++) {
		fread(mon, MONSTER_SIZE, 1, fp);

		if (mon->flags & mHasSkills) {
			mon->skills = Read_List(fp, "Load_Monsters");
		} else {
			mon->skills = null;
		}

		mon++;
	}

	fread(m->monsters, sizeof(monster), m->header.num_monsters, fp);

	fclose(fp);
	return true;
}

void Free_Monsters(monsters *m)
{
	int i;
	monster *mon;

	Log("Free_Monsters: %p", m);

	for (i = 0; i < m->header.num_monsters; i++) {
		mon = &m->monsters[i];

		if (mon->flags & mHasSkills) {
			Free_List(mon->skills);
		}
	}

	Free(m->monsters);
}

void Initialise_Monsters(monsters *m)
{
	m->monsters = null;
}

bool Save_Monsters(char *filename, monsters *m)
{
	int i;
	monster *mon;
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		dief("Save_Monsters: Could not open for writing: %s\n", filename);
		return false;
	}

	Set_Version_Header(m->header);
	fwrite(&m->header, sizeof(monsters_header), 1, fp);

	for (i = 0; i < m->header.num_monsters; i++) {
		mon = &m->monsters[i];
		fwrite(mon, MONSTER_SIZE, 1, fp);

		if (mon->flags & mHasSkills) {
			Write_List(mon->skills, fp);
		}
	}

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
