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

#define TARGET_PC(i)	(-i - 1)
#define TARGET_ENEMY(i)	(i)
#define IS_PC(i)		(i < 0)

typedef unsigned int act;
typedef int targ;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	bool (*check)(targ source);
	void (*act)(targ source, targ target);
	char *name;
	targetflags targeting;
} action;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport action *combat_actions;
noexport monster *monsters;
noexport int SZ(monsters);
noexport int monsters_alive;

/* C O M B A T  A C T I O N S //////////////////////////////////////////// */

bool Check_Attack(targ source)
{
	/* TODO: fix for enemy */
	bool front = In_Front_Row(source);
	item *weapon = Get_Equipped_Weapon(source);
	if (weapon == null) return front;

	if (weapon->flags & (ifMediumRange | ifLongRange)) return true;
	return front;
}

void Attack(targ source, targ target)
{
	/* TODO */
}

bool Check_Block(targ source)
{
	return true;
}

void Block(targ source, targ target)
{
	/* TODO */
}

bool Check_Defend(targ source)
{
	/* TODO: fix for enemy */
	char i;

	/* There must be an alive ally to Defend */
	for (i = 0; i < PARTY_SIZE; i++) {
		if (i == pc) continue;
		if (gSave.characters[i].stats[sHP] > 0) return true;
	}

	return false;
}

void Defend(targ source, targ target)
{
	/* TODO */
}

/* F U N C T I O N S ///////////////////////////////////////////////////// */

int randint(int minimum, int maximum)
{
	return minimum + random(maximum - minimum + 1);
}

noexport void Clear_Encounter(void)
{
	int i;
	Free_Array(i, monsters);
}

void Add_Monster(monster* template)
{
	monster *m = SzAlloc(1, monster, "Add_Monster");
	memcpy(m, template, sizeof(monster));
	m->stats[sHP] = m->stats[sMaxHP];
	m->stats[sMP] = m->stats[sMaxMP];

	Add_to_Array(monsters, m);
	monsters_alive++;
}

#define Add_Combat_Action(_index, _name, _check, _act, _targeting) { \
	combat_actions[_index].check = _check; \
	combat_actions[_index].act = _act; \
	combat_actions[_index].name = _name; \
	combat_actions[_index].targeting = _targeting; \
}

void Initialise_Combat(void)
{
	Null_Array(monsters);

	combat_actions = SzAlloc(NUM_ACTIONS, action, "Initialise_Combat");
	Add_Combat_Action(0, "Attack", Check_Attack, Attack, tfEnemy);
	Add_Combat_Action(1, "Block", Check_Block, Block, tfSelf);
	Add_Combat_Action(2, "Defend", Check_Defend, Defend, tfAlly);
}

void Free_Combat(void)
{
	Free(combat_actions);
}

noexport act Get_Pc_Action(unsigned char pc)
{
	int i, count = 0;
	act ai;
	int *action_ids = SzAlloc(NUM_ACTIONS, int, "Get_Pc_Action.ids");
	char **action_names = SzAlloc(NUM_ACTIONS, char *, "Get_Pc_Action.names")

	for (i = 0; i < NUM_ACTIONS; i++) {
		if (combat_actions[i].check(pc)) {
			action_names[count] = combat_actions[i].name;
			action_ids[count++] = i;
		}
	}

	/* TODO */
	i = Input_Menu(action_names, count, 10, 10);
	ai = action_ids[i];

	Free(action_ids);
	Free(action_names);

	return ai;
}

noexport targ Get_Pc_Target(unsigned char pc, act action_id)
{
	char **menu, temp[100];
	int *target_ids;
	int items = 0, max, i;
	targ choice;
	action *a = combat_actions[action_id];

	/* get the easiest case out of the way */
	if (a->targets == tfSelf) return TARGET_PC(pc);

	/* OK, gather all possible targets */
	max = PARTY_SIZE + SZ(monsters);
	menu = SzAlloc(max, char *, "Get_Pc_Target.menu");
	target_ids = SzAlloc(max, int, "Get_Pc_Target.targs");

	if (a->targets & tfSelf) {
		menu[items] = Duplicate_String(gSave.characters[pc].name, "Get_Pc_Target.self");
		target_ids[items++] = TARGET_PC(pc);
	}

	if (a->targets & tfAlly) {
		for (i = 0; i < PARTY_SIZE; i++) {
			if (i == pc) continue;
			if (gSave.characters[i].stats[sHP] > 0 || a->targets & tfDead) {
				menu[items] = Duplicate_String(gSave.characters[i].name, "Get_Pc_Target.ally");
				target_ids[items++] = TARGET_PC(i);
			}
		}
	}

	if (a->targets & tfEnemy) {
		for (i = 0; i < SZ(monsters); i++) {
			if (monsters[i].stats[sHP] > 0 || a->targets & tfDead) {
				/* TODO: ignore if already in menu, use group number */

				if (monsters[i].stats[sHP] > 0) {
					menu[items] = Duplicate_String(monsters[i].name, "Get_Pc_Target.monster");
				} else {
					sprintf(temp, "%s (dead)", monsters[i].name);
					menu[items] = Duplicate_String(temp, "Get_Pc_Target.dead");
				}

				target_ids[items++] = i;
			}
		}
	}

	/* TODO */
	choice = target_ids[Input_Menu(menu, items, 10, 10)];

	for (i = 0; i < items; i++) {
		Free(menu[i]);
	}

	Free(menu);
	Free(target_ids);

	return choice;
}

noexport void Apply_Pc_Action(unsigned char pc, act action_id, targ target_id)
{
	combat_actions[action_id].act(PC_TARGET(pc), target_id);
}

noexport void Enter_Combat_Loop(void)
{
	int i;
	int pc_actions[PARTY_SIZE];
	int pc_targets[PARTY_SIZE];

	while (monsters_alive > 0) {
		for (i = 0; i < PARTY_SIZE; i++) {
			pc_actions[i] = Get_Pc_Action(i);
			pc_targets[i] = Get_Pc_Target(i, pc_actions[i]);
		}

		/* TODO: get monster actions */

		for (i = 0; i < PARTY_SIZE; i++) {
			Apply_Pc_Action(i, pc_actions[i], pc_targets[i]);
		}

		/* TODO: apply monster actions */
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
