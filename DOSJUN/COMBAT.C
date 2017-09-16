/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "gamelib.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define NUM_ACTIONS	3

/* highlight colours */
#define COLOUR_ACTIVE	14
#define COLOUR_SELECT	11

typedef enum {
	tfAlly = 1,
	tfEnemy = 2,
	tfDead = 4,
	tfSelf = 8
} targetflags;

#define MONSTER(i)		((monster *)List_At(combat_monsters, i))

#define TARGET_PC(i)	(-(i) - 1)
#define TARGET_ENEMY(i)	(i)
#define IS_PC(i)		(i < 0)
#define NAME(i)			(IS_PC(i) ? gSave.characters[TARGET_PC(i)].name : MONSTER(i)->name)

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

noexport pcx_picture combat_bg;

noexport action *combat_actions;
noexport list *combat_monsters;
noexport list *combat_groups[ENCOUNTER_SIZE];
noexport int group_y[ENCOUNTER_SIZE];
noexport int monsters_alive;
noexport int groups_alive;

int randint(int minimum, int maximum);

/* C O M B A T  A C T I O N S //////////////////////////////////////////// */

noexport void Show_Combat_String(char *string, bool wait_for_key)
{
	Draw_Square_DB(0, 96, 144, 223, 191, 1);
	/* TODO: split onto lines a bit more nicely */
	Draw_Bounded_String(100, 148, 15, 5, 15, string, 0);

	if (wait_for_key) {
		Show_Double_Buffer();
		Delay(5);
		Get_Next_Scan_Code();
	} else {
		Show_Double_Buffer();
	}
}

noexport void Combat_Message(char *format, ...)
{
	va_list vargs;
	char message[500];

	va_start(vargs, format);
	vsprintf(message, format, vargs);
	va_end(vargs);

	Show_Combat_String(message, true);
}

noexport void Highlight_Ally(int pc_active, int pc_select)
{
	char buffer[9];
	int i, colour;

	for (i = 0; i < PARTY_SIZE; i++) {
		strncpy(buffer, gSave.characters[i].name, 8);
		buffer[8] = 0;

		colour = 15;
		if (i == pc_active) colour = COLOUR_ACTIVE;
		if (i == pc_select) colour = COLOUR_SELECT;
		Blit_String_DB(248, 8 + i * 32, colour, buffer, 0);
	}
}

noexport void Format_Enemy_Group(int group, char *buffer)
{
	int enemy = (int)List_At(combat_groups[group], 0);
	sprintf(buffer, "%s x%u", MONSTER(enemy)->name, combat_groups[group]->size);
}

#define EPANEL_X	8
#define EPANEL_Y	8
#define EPANEL_W	10
#define EPANEL_H	2

noexport void Show_Enemies(void)
{
	int i;
	int y = EPANEL_Y;
	char buffer[100];

	Draw_Square_DB(0, 8, 8, 87, 135, 1);

	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (combat_groups[i]->size > 0) {
			group_y[i] = y;
			Format_Enemy_Group(i, buffer);
			Draw_Bounded_String(EPANEL_X, y, EPANEL_W, EPANEL_H, 15, buffer, 0);

			y += 16;
		}
	}
}

noexport void Highlight_Enemy_Group(int group, int colour)
{
	int i;
	char buffer[100];

	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (combat_groups[i]->size > 0) {
			Format_Enemy_Group(i, buffer);
			Draw_Bounded_String(EPANEL_X, group_y[i], EPANEL_W, EPANEL_H, (i == group) ? colour : 15, buffer, 0);
		}
	}
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

	return MONSTER(source)->row == rowFront;
}

noexport stat Get_Stat(targ source, stat_id st)
{
	if (IS_PC(source)) {
		return gSave.characters[TARGET_PC(source)].stats[st];
	}

	return MONSTER(source)->stats[st];
}

noexport void Set_Stat(targ source, stat_id st, stat num)
{
	if (IS_PC(source)) {
		gSave.characters[TARGET_PC(source)].stats[st] = num;
	} else {
		MONSTER(source)->stats[st] = num;
	}
}

noexport stat_id Get_Weapon_Stat(item *weapon)
{
	if (weapon == null) return sStrength;
	return (weapon->flags & ifDexterityWeapon) ? sDexterity : sStrength;
}

noexport int Get_Enemy_Group(targ victim)
{
	int i;

	if (IS_PC(victim)) {
		return -1;
	}

	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (In_List(combat_groups[i], (void*)victim)) {
			return i;
		}
	}

	return -1;
}

noexport targ Get_Random_Target(int group)
{
	int i;

	i = randint(0, combat_groups[group]->size - 1);
	return (targ)List_At(combat_groups[group], i);
}

noexport bool Is_Dead(targ victim)
{
	return Get_Stat(victim, sHP) <= 0;
}

noexport void Kill(targ victim)
{
	int group;
	Combat_Message("%s dies!", NAME(victim));

	if (IS_PC(victim)) {
		/* TODO */
	} else {
		monsters_alive--;

		group = Get_Enemy_Group(victim);
		if (group >= 0) {
			Remove_from_List(combat_groups[group], (void*)victim);
			if (combat_groups[group]->size == 0) {
				groups_alive--;
			}
		}
	}
}

noexport void Damage(targ victim, int amount)
{
	stat hp = Get_Stat(victim, sHP);
	hp -= amount;

	Combat_Message("%s takes %d damage.", NAME(victim), amount);
	Set_Stat(victim, sHP, hp);
	if (hp <= 0) {
		Kill(victim);
	}
}

/* C O M B A T  A C T I O N S //////////////////////////////////////////// */

noexport bool Check_Attack(targ source)
{
	bool front;
	item *weapon;

	front = Get_Row(source);
	weapon = Get_Weapon(source);

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

	if (Is_Dead(target)) {
		/* TODO: reassign target? */
		return;
	}

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
	Clear_List(combat_monsters);
}

void Add_Monster(monster* template)
{
	monster *m = SzAlloc(1, monster, "Add_Monster");
	memcpy(m, template, sizeof(monster));
	m->stats[sHP] = m->stats[sMaxHP];
	m->stats[sMP] = m->stats[sMaxMP];

	Add_to_List(combat_monsters, m);
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
	int i;

	combat_monsters = New_List(ltObject, "Initialise_Combat.monsters");
	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		combat_groups[i] = New_List(ltInteger, "Initialise_Combat.group_x");
	}

	combat_actions = SzAlloc(NUM_ACTIONS, action, "Initialise_Combat.actions");
	Add_Combat_Action(0, "Attack", Check_Attack, Attack, tfEnemy);
	Add_Combat_Action(1, "Block", Check_Block, Block, tfSelf);
	Add_Combat_Action(2, "Defend", Check_Defend, Defend, tfAlly);

	PCX_Init(&combat_bg);
	PCX_Load("COMBAT.PCX", &combat_bg, 0);
}

void Free_Combat(void)
{
	int i;

	Free_List(combat_monsters);
	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		Free_List(combat_groups[i]);
	}

	Free(combat_actions);
	PCX_Delete(&combat_bg);
}

noexport act Get_Pc_Action(unsigned char pc)
{
	int i, count = 0;
	act ai;
	int *action_ids = SzAlloc(NUM_ACTIONS, int, "Get_Pc_Action.ids");
	char **action_names = SzAlloc(NUM_ACTIONS, char *, "Get_Pc_Action.names");

	for (i = 0; i < NUM_ACTIONS; i++) {
		if (combat_actions[i].check(TARGET_PC(pc))) {
			action_names[count] = combat_actions[i].name;
			action_ids[count++] = i;
		}
	}

	/* TODO */
	Highlight_Ally(pc, -1);
	Draw_Square_DB(0, 8, 144, 87, 191, true);
	i = Input_Menu(action_names, count, 8, 144);
	ai = action_ids[i];

	Free(action_ids);
	Free(action_names);

	Highlight_Ally(-1, -1);
	return ai;
}

noexport bool Is_Valid_Target(targ source, targ target, targetflags tf)
{
	if (source == target) return tf & tfSelf;
	if (Is_Dead(target) && !(tf & tfDead)) return false;

	if (IS_PC(target)) return tf & tfAlly;
	return tf & tfEnemy;
}

noexport targ Get_Pc_Target(unsigned char pc, act action_id)
{
	bool done = false;
	action *a = &combat_actions[action_id];
	char ch;
	targ me, choice, change;
	int diff;

	me = TARGET_PC(pc);

	/* get the easiest case out of the way */
	if (a->targeting == tfSelf) return TARGET_PC(pc);

	if (a->targeting & tfEnemy) {
		choice = 0;
	} else {
		choice = TARGET_PC(0);
	}

	Highlight_Ally(pc, COLOUR_ACTIVE);
	while (!done) {
		if (IS_PC(choice)) {
			Highlight_Ally(pc, TARGET_PC(choice));
		} else {
			Highlight_Enemy_Group(choice, COLOUR_SELECT);
		}

		Show_Double_Buffer();
		change = choice;
		ch = Get_Next_Scan_Code();

		switch (ch) {
			case SCAN_ENTER:
				done = true;
				break;

			case SCAN_LEFT:
				if (IS_PC(choice) && a->targeting & tfEnemy) {
					change = 0;
				}
				break;

			case SCAN_RIGHT:
				if (!IS_PC(choice) && a->targeting & tfAlly) {
					change = TARGET_PC(0);
				}
				break;

			case SCAN_UP:
				if (IS_PC(choice) && choice != TARGET_PC(0)) {
					change = choice + 1;
				} else if (!IS_PC(choice) && choice > 0) {
					change = choice - 1;
				}
				break;

			case SCAN_DOWN:
				if (IS_PC(choice) && choice != TARGET_PC(PARTY_SIZE - 1)) {
					change = choice - 1;
				}
				else if (!IS_PC(choice) && choice < (groups_alive - 1)) {
					change = choice + 1;
				}
				break;
		}

		if (change != choice || done) {
			if (IS_PC(change)) {
				diff = IS_PC(choice) ? change - choice : -1;
				while (!Is_Valid_Target(me, change, a->targeting)) {
					change += diff;
					if (change == TARGET_PC(PARTY_SIZE)) change = TARGET_PC(0);
					if (change == TARGET_PC(-1)) change = TARGET_PC(PARTY_SIZE - 1);
				}

				Highlight_Ally(pc, -1);
			} else {
				Highlight_Enemy_Group(-1, 0);
			}

			choice = change;
		}
	}

	if (!IS_PC(choice)) {
		choice = Get_Random_Target(choice);
	}

	return choice;
}

noexport void Apply_Pc_Action(unsigned char pc, act action_id, targ target_id)
{
	if (Get_Stat(TARGET_PC(pc), sHP) <= 0) {
		/* TODO: show message? */
		return;
	}

	combat_actions[action_id].act(TARGET_PC(pc), target_id);
}

noexport void Apply_Monster_Action(unsigned char monster, act action_id, targ target_id)
{
	if (action_id >= 0) {
		if (Get_Stat(monster, sHP) <= 0) {
			/* TODO: show message? */
			return;
		}

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

noexport void Show_Combat_Pc_Stats(void)
{
	int i;

	for (i = 0; i < PARTY_SIZE; i++) {
		Draw_Character_Status(i, 232, 8 + i * 32);
	}
}

noexport void Enter_Combat_Loop(void)
{
	int i;
	act pc_actions[PARTY_SIZE];
	targ pc_targets[PARTY_SIZE];
	act *monster_actions;
	targ *monster_targets;

	monster_actions = SzAlloc(combat_monsters->size, act, "Enter_Combat_Loop.actions");
	monster_targets = SzAlloc(combat_monsters->size, targ, "Enter_Combat_Loop.targets");

	while (monsters_alive > 0) {
		Show_Combat_Pc_Stats();
		Show_Enemies();

		for (i = 0; i < PARTY_SIZE; i++) {
			pc_actions[i] = Get_Pc_Action(i);
			pc_targets[i] = Get_Pc_Target(i, pc_actions[i]);
		}

		for (i = 0; i < combat_monsters->size; i++) {
			/* TODO: could have self-res enemies? */
			if (MONSTER(i)->stats[sHP] > 0) {
				/* TODO: change to array lookup? */
				switch (MONSTER(i)->ai) {
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

		for (i = 0; i < combat_monsters->size; i++) {
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
	groups_alive = 0;

	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (!en->monsters[i]) continue;
		m = Lookup_Monster(&gMonsters, en->monsters[i]);
		if (first == null) first = m->image;

		count = randint(en->minimum[i], en->maximum[i]);
		if (count > 0) {
			write += sprintf(write, " %s x%d", m->name, count);

			while (count > 0) {
				Add_to_List(combat_groups[groups_alive], (void*)monsters_alive);
				Add_Monster(m);
				count--;
			}

			groups_alive++;
		}
	}

	/* Briefly show encounter on Dungeon screen */
	Show_Picture(first);
	Show_Game_String(description, true);

	/* Set up Combat screen */
	redraw_everything = true;
	memcpy(double_buffer, combat_bg.buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
	Show_Picture(first);

	/* DO IT */
	Enter_Combat_Loop();
	
	/* TODO: if you lose... shouldn't do this */
	Redraw_Dungeon_Screen(false);
}
