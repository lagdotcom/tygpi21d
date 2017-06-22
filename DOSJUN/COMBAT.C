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

void Add_Monster(monster* template)
{
	monster *m = SzAlloc(1, monster, "Add_Monster");
	memcpy(m, template, sizeof(monster));
	m->stats[sHP] = m->stats[sMaxHP];
	m->stats[sMP] = m->stats[sMaxMP];

	Add_to_Array(gCombat.monsters, m);
}

void Initialise_Combat(void)
{
	Null_Array(gCombat.monsters);
}

void Enter_Combat_Loop(void)
{
	/* TODO */

	Clear_Encounter();

	redraw_description = true;
	redraw_fp = true;
	redraw_party = true;
	Draw_FP();
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Start_Combat(encounter_id id)
{
	char description[1000],
		*write,
		*first = null;
	int i, count;
	encounter *en = &gZone.encounters[id];
	monster *m;

	Clear_Encounter();
	description[0] = 0;
	write = description;
	write += sprintf(write, "You face:");

	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (!en->monsters[i]) continue;
		m = Lookup_Monster(&gMonsters, en->monsters[i]);
		if (first == null) first = m->image;

		count = randint(en->minimum[i], en->maximum[i]);
		write += sprintf(write, " %dx %s", count, m->name);
		while (count-- > 0) Add_Monster(m);
	}

	Show_Picture(first);
	Show_Game_String(description, true);

	Enter_Combat_Loop();
}
