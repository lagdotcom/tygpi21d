/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NUM_ACTIONS	3

typedef enum {
	tfAlly = 1,
	tfEnemy = 2,
	tfDead = 4,
	tfSelf = 8
} targetflags;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	bool (*check)(char pc);
	void (*act)(char pc, int target);
	char *name;
	targetflags targeting;
} action;

typedef struct {
	action* actions;
	Declare_Array(monster, monsters);
	int monsters_alive;
} combat;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport combat gCombat;

/* C O M B A T  A C T I O N S //////////////////////////////////////////// */

bool Check_Attack(char pc)
{
	bool front = In_Front_Row(pc);
	item *weapon = Get_Equipped_Weapon(pc);
	if (weapon == null) return front;

	if (weapon->flags & (ifMediumRange | ifLongRange)) return true;
	return front;
}

void Attack(char pc, int target)
{

}

bool Check_Block(char pc)
{
	return true;
}

void Block(char pc, int target)
{

}

bool Check_Defend(char pc)
{
	char i;
	/* There must be an alive ally to Defend */
	for (i = 0; i < PARTY_SIZE; i++) {
		if (i == pc) continue;
		if (gSave.characters[i].stats[sHP] > 0) return true;
	}

	return false;
}

void Defend(char pc, int target)
{

}

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
	gCombat.monsters_alive++;
}

#define Add_Combat_Action(_index, _name, _check, _act, _targeting) { \
	gCombat.actions[_index].check = _check; \
	gCombat.actions[_index].act = _act; \
	gCombat.actions[_index].name = _name; \
	gCombat.actions[_index].targeting = _targeting; \
}

void Initialise_Combat(void)
{
	Null_Array(gCombat.monsters);

	gCombat.actions = SzAlloc(NUM_ACTIONS, action, "Initialise_Combat");
	Add_Combat_Action(0, "Attack", Check_Attack, Attack, tfEnemy);
	Add_Combat_Action(1, "Block", Check_Block, Block, tfSelf);
	Add_Combat_Action(2, "Defend", Check_Defend, Defend, tfAlly);
}

void Free_Combat(void)
{
	Free(gCombat.actions);
}

noexport void Enter_Combat_Loop(void)
{
	int i;

	while (gCombat.monsters_alive > 0) {
		for (i = 0; i < PARTY_SIZE; i++) {
			/* TODO */
		}
	}

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
