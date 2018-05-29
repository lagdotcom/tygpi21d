/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Read_Monster(FILE *fp, monster *m)
{
	fread(m, MONSTER_SIZE, 1, fp);

	if (m->flags & mHasSkills) {
		m->skills = Read_List(fp, "Load_Monsters");
	} else {
		m->skills = null;
	}

	return true;
}

bool Load_Monster(char *filename, monster *m)
{
	bool result;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		dief("Load_Monster: Could not open for reading: %s\n", filename);
		return false;
	}

	result = Read_Monster(fp, m);
	fclose(fp);
	return result;
}

void Free_Monster(monster *m)
{
	if (m->flags & mHasSkills)
		Free_List(m->skills);
}

bool Save_Monster(char *filename, monster *m)
{
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		dief("Save_Monsters: Could not open for writing: %s\n", filename);
		return false;
	}

	fwrite(m, MONSTER_SIZE, 1, fp);

	if (m->flags & mHasSkills)
		Write_List(m->skills, fp);

	fclose(fp);
	return true;
}
