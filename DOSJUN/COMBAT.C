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
#define NAME(i)			(IS_PC(i) ? gSave.characters[TARGET_PC(i)].name : combat_monsters[i]->name)

#define NO_ACTION		-1
typedef int act;
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
noexport monster **combat_monsters;
noexport int SZ(combat_monsters);
noexport int monsters_alive;

int randint(int minimum, int maximum);

/* C O M B A T  A C T I O N S //////////////////////////////////////////// */

noexport void Combat_Message(char *format, ...)
{
	va_list vargs;
	char message[500];

	va_start(vargs, format);
	vsprintf(message, format, vargs);
	va_end(vargs);

	/* TODO */
	Show_Game_String(message, true);
}

noexport item *Get_Weapon(targ source)
{
	if (IS_PC(source)) {
		return Get_Equipped_Weapon(TARGET_PC(source));
	}

	return null; /* TODO */
}

noexport bool Get_Row(targ source)
{
	if (IS_PC(source)) {
		return In_Front_Row(TARGET_PC(source));
	}

	return combat_monsters[source]->row == rowFront;
}

noexport stat Get_Stat(targ source, stat_id st)
{
	if (IS_PC(source)) {
		return gSave.characters[TARGET_PC(source)].stats[st];
	}

	return combat_monsters[source]->stats[st];
}

noexport void Set_Stat(targ source, stat_id st, stat num)
{
	if (IS_PC(source)) {
		gSave.characters[TARGET_PC(source)].stats[st] = num;
	} else {
		combat_monsters[source]->stats[st] = num;
	}
}

noexport stat_id Get_Weapon_Stat(item *weapon)
{
	if (weapon == null) return sStrength;
	return (weapon->flags & ifDexterityWeapon) ? sDexterity : sStrength;
}

noexport void Kill(targ victim)
{
	Combat_Message("%s dies!", NAME(victim));

	if (IS_PC(victim)) {
		/* TODO */
	} else {
		monsters_alive--;
	}
}

noexport void Damage(targ victim, int amount)
{
	stat hp = Get_Stat(victim, sHP);
	hp -= amount;

	Combat_Message("%s takes %d damage.", NAME(victim), amount);
	Set_Stat(victim, sHP, hp);
	if (hp < 0) {
		Kill(victim);
	}
}

/* C O M B A T  A C T I O N S //////////////////////////////////////////// */

noexport bool Check_Attack(targ source)
{
	bool front = Get_Row(source);
	item *weapon = Get_Weapon(source);

	if (weapon == null) return front;

	if (weapon->flags & (ifMediumRange | ifLongRange)) return true;
	return front;
}

noexport void Attack(targ source, targ target)
{
	item *weapon = Get_Weapon(source);
	stat base = Get_Stat(source, Get_Weapon_Stat(weapon));
	stat min, max;
	int roll;

	if (randint(1, 20) <= base) {
		Combat_Message("%s hits %s!", NAME(source), NAME(target));

		min = Get_Stat(source, sMinDamage);
		max = Get_Stat(source, sMaxDamage);

		if (weapon) {
			min += weapon->stats[sMinDamage];
			max += weapon->stats[sMaxDamage];
		}

		roll = randint(min, max) - Get_Stat(target, sArmour);
		if (roll > 0) Damage(target, roll);
	} else {
		Combat_Message("%s attacks %s and misses.", NAME(source), NAME(target));
	}
}

noexport bool Check_Block(targ source)
{
	return true;
}

noexport void Block(targ source, targ target)
{
	/* TODO */
}

noexport bool Check_Defend(targ source)
{
	/* TODO: fix for enemy */
	char i;

	/* There must be an alive ally to Defend */
	if (IS_PC(source)) {
		for (i = 0; i < PARTY_SIZE; i++) {
			if (i == TARGET_PC(source)) continue;
			if (gSave.characters[i].stats[sHP] > 0) return true;
		}
	} else {
		/* TODO */
	}

	return false;
}

noexport void Defend(targ source, targ target)
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
	Free_Array(i, combat_monsters);
}

void Add_Monster(monster* template)
{
	monster *m = SzAlloc(1, monster, "Add_Monster");
	memcpy(m, template, sizeof(monster));
	m->stats[sHP] = m->stats[sMaxHP];
	m->stats[sMP] = m->stats[sMaxMP];

	Add_to_Array(combat_monsters, m);
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
	Null_Array(combat_monsters);

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
	char **action_names = SzAlloc(NUM_ACTIONS, char *, "Get_Pc_Action.names");

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
	action *a = &combat_actions[action_id];

	/* get the easiest case out of the way */
	if (a->targeting == tfSelf) return TARGET_PC(pc);

	/* OK, gather all possible targets */
	max = PARTY_SIZE + SZ(combat_monsters);
	menu = SzAlloc(max, char *, "Get_Pc_Target.menu");
	target_ids = SzAlloc(max, int, "Get_Pc_Target.targs");

	if (a->targeting & tfSelf) {
		menu[items] = Duplicate_String(gSave.characters[pc].name, "Get_Pc_Target.self");
		target_ids[items++] = TARGET_PC(pc);
	}

	if (a->targeting & tfAlly) {
		for (i = 0; i < PARTY_SIZE; i++) {
			if (i == pc) continue;
			if (gSave.characters[i].stats[sHP] > 0 || a->targeting & tfDead) {
				menu[items] = Duplicate_String(gSave.characters[i].name, "Get_Pc_Target.ally");
				target_ids[items++] = TARGET_PC(i);
			}
		}
	}

	if (a->targeting & tfEnemy) {
		for (i = 0; i < SZ(combat_monsters); i++) {
			if (combat_monsters[i]->stats[sHP] > 0 || a->targeting & tfDead) {
				/* TODO: ignore if already in menu, use group number */

				if (combat_monsters[i]->stats[sHP] > 0) {
					menu[items] = Duplicate_String(combat_monsters[i]->name, "Get_Pc_Target.monster");
				} else {
					sprintf(temp, "%s (dead)", combat_monsters[i]->name);
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
	combat_actions[action_id].act(TARGET_PC(pc), target_id);
}

noexport void Apply_Monster_Action(unsigned char monster, act action_id, targ target_id)
{
	if (action_id >= 0) {
		combat_actions[action_id].act(monster, target_id);
	}
}

/* A I  R O U T I N E S ////////////////////////////////////////////////// */

noexport void AI_Mindless(int monster, int *action, int *target)
{
	int t = randint(0, PARTY_SIZE - 1);
	while (gSave.characters[t].stats[sHP] <= 0) {
		t = randint(0, PARTY_SIZE - 1);
	}

	*action = 0;	/* Attack */
	*target = TARGET_PC(t);
}

noexport void Enter_Combat_Loop(void)
{
	int i;
	act pc_actions[PARTY_SIZE];
	targ pc_targets[PARTY_SIZE];
	act *monster_actions;
	targ *monster_targets;

	monster_actions = SzAlloc(SZ(combat_monsters), act, "Enter_Combat_Loop.actions");
	monster_targets = SzAlloc(SZ(combat_monsters), targ, "Enter_Combat_Loop.targets");

	while (monsters_alive > 0) {
		for (i = 0; i < PARTY_SIZE; i++) {
			pc_actions[i] = Get_Pc_Action(i);
			pc_targets[i] = Get_Pc_Target(i, pc_actions[i]);
		}

		for (i = 0; i < SZ(combat_monsters); i++) {
			/* TODO: could have self-res enemies? */
			if (combat_monsters[i]->stats[sHP] > 0) {
				/* TODO: change to array lookup? */
				switch (combat_monsters[i]->ai) {
					case aiMindless:
						AI_Mindless(i, &monster_actions[i], &monster_targets[i]);
						break;
				}
			}
			else {
				monster_actions[i] = NO_ACTION;
			}
		}

		for (i = 0; i < PARTY_SIZE; i++) {
			Apply_Pc_Action(i, pc_actions[i], pc_targets[i]);
		}

		for (i = 0; i < SZ(combat_monsters); i++) {
			Apply_Monster_Action(i, monster_actions[i], monster_targets[i]);
		}
	}

	Clear_Encounter();
	Free(monster_actions);
	Free(monster_targets);

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
