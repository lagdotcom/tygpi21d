/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	Declare_Array(monster, monsters);
} combat;

/* G L O B A L S ///////////////////////////////////////////////////////// */

combat gCombat;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

int randint(int minimum, int maximum)
{
	return minimum + random(maximum - minimum + 1);
}

noexport void Clear_Encounter(void)
{
	int i;
	Free_Array(i, gCombat.monsters);
}

void Add_Monster(monster_id id)
{
	monster *m = SzAlloc(1, monster, "Add_Monster");
	monster *template = Lookup_Monster(&gMonsters, id);
	memcpy(m, template, sizeof(monster));
	m->stats[sHP] = m->stats[sMaxHP];
	m->stats[sMP] = m->stats[sMaxMP];

	Add_to_Array(gCombat.monsters, m);
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Start_Combat(encounter_id id)
{
	int i, count;
	encounter *en = &gZone.encounters[id];

	Clear_Encounter();

	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (!en->monsters[i]) continue;

		count = randint(en->minimum[i], en->maximum[i]);
		while (count > 0) Add_Monster(en->monsters[i]);
	}

	gState = gsCombat;
}

gamestate Continue_Combat(void)
{
	Clear_Encounter();

	/* TODO */
	return gsDungeon;
}
