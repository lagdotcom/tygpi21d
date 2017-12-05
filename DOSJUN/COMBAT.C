/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "gamelib.h"

#include <time.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* highlight colours */
#define COLOUR_ACTIVE	14
#define COLOUR_SELECT	11

#define MONSTER(i)		((monster *)List_At(combat_monsters, i))
#define NAME(i)			(IS_PC(i) ? Get_Pc(TARGET_PC(i))->header.name : MONSTER(i)->name)

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	action_check_fn check;
	action_do_fn act;
	char *name;
	targetflags targeting;
	UINT8 priority; /* TODO */
} action;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport pcx_picture combat_bg;

noexport action *combat_actions;
noexport list *combat_monsters;
noexport list *combat_groups[ENCOUNTER_SIZE];
noexport int group_y[ENCOUNTER_SIZE];
noexport int monsters_alive;
noexport int groups_alive;
noexport UINT32 earned_experience;

/* H E L P E R  F U N C T I O N S //////////////////////////////////////// */

noexport void Show_Combat_String(char *string, bool wait_for_key)
{
	Draw_Wrapped_String(96, 144, 128, 48, 15, string, true);

	if (wait_for_key) {
		Show_Double_Buffer();
		Delay(5);
		Get_Next_Scan_Code();
	} else {
		Show_Double_Buffer();
	}
}

void Combat_Message(char *format, ...)
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
		strncpy(buffer, Get_Pc(i)->header.name, 8);
		buffer[8] = 0;

		colour = 15;
		if (i == pc_active) colour = COLOUR_ACTIVE;
		if (i == pc_select) colour = COLOUR_SELECT;
		Blit_String_DB(248, 8 + i * 32, colour, buffer, 0);
	}
}

noexport void Format_Enemy_Group(groupnum group, char *buffer)
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
	groupnum i;
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

noexport void Highlight_Enemy_Group(groupnum group, int colour)
{
	groupnum i;
	char buffer[100];

	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (combat_groups[i]->size > 0) {
			Format_Enemy_Group(i, buffer);
			Draw_Bounded_String(EPANEL_X, group_y[i], EPANEL_W, EPANEL_H, (i == group) ? colour : 15, buffer, 0);
		}
	}
}

item *Get_Weapon(targ source)
{
	item_id iid;

	if (IS_PC(source)) {
		return Get_Equipped_Weapon(TARGET_PC(source));
	}

	iid = MONSTER(source)->weapon;
	return iid ? Lookup_Item(iid) : null;
}

noexport bool Get_Row(targ source)
{
	if (IS_PC(source)) {
		return In_Front_Row(TARGET_PC(source));
	}

	return MONSTER(source)->row == rowFront;
}

stat_value Get_Stat(targ source, statistic st)
{
	if (IS_PC(source)) {
		return Get_Pc(TARGET_PC(source))->header.stats[st];
	}

	return MONSTER(source)->stats[st];
}

noexport void Set_Stat(targ source, statistic st, stat_value num)
{
	if (IS_PC(source)) {
		Get_Pc(TARGET_PC(source))->header.stats[st] = num;
	} else {
		MONSTER(source)->stats[st] = num;
	}
}

statistic Get_Weapon_Stat(item *weapon)
{
	if (weapon == null) return sStrength;
	return (weapon->flags & ifDexterityWeapon) ? sDexterity : sStrength;
}

noexport groupnum Get_Enemy_Group(targ victim)
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

monster *Get_Monster(targ target)
{
	return MONSTER(target);
}

char *Get_Target_Name(targ target)
{
	return NAME(target);
}

noexport targ Get_Random_Target(groupnum group)
{
	int i;

	i = randint(0, combat_groups[group]->size - 1);
	return (targ)List_At(combat_groups[group], i);
}

bool Is_Dead(targ victim)
{
	return Get_Stat(victim, sHP) <= 0;
}

noexport void Kill(targ victim)
{
	monster *m;
	groupnum group;
	Combat_Message("%s dies!", NAME(victim));

	if (IS_PC(victim)) {
		/* TODO */
	} else {
		m = List_At(combat_monsters, victim);
		monsters_alive--;
		earned_experience += m->experience;

		group = Get_Enemy_Group(victim);
		if (group >= 0) {
			Remove_from_List(combat_groups[group], (void*)victim);
			if (combat_groups[group]->size == 0) {
				groups_alive--;
			}
		}
	}
}

void Damage(targ victim, int amount)
{
	stat_value hp = Get_Stat(victim, sHP);
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

	/* Sneak Attack is handled separately */
	if (Has_Buff(source, HIDE_BUFF_NAME)) return false;

	front = Get_Row(source);
	weapon = Get_Weapon(source);

	if (weapon == null) return front;

	if (weapon->flags & (ifMediumRange | ifLongRange)) return true;
	return front;
}

noexport void Attack(targ source, targ target)
{
	item *weapon = Get_Weapon(source);
	stat_value base = Get_Stat(source, Get_Weapon_Stat(weapon));
	stat_value min, max;
	int roll;

	if (Is_Dead(target)) {
		/* TODO: reassign target? */
		return;
	}

	base += Get_Stat(source, sHitBonus);
	base -= Get_Stat(target, sDodgeBonus);

	if (randint(1, 20) <= base) {
		Combat_Message("%s hits %s!", NAME(source), NAME(target));

		min = Get_Stat(source, sMinDamage);
		max = Get_Stat(source, sMaxDamage);

		if (weapon != null) {
			min += weapon->stats[sMinDamage];
			max += weapon->stats[sMaxDamage];
		}

		roll = randint(min, max) - Get_Stat(target, sArmour);
		if (roll > 0) {
			Damage(target, roll);
		} else {
			Combat_Message("The blow glances off.");
		}
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
			if (Get_Pc(i)->header.stats[sHP] > 0) return true;
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
	if (m == null)
		die("Add_Monster: out of memory");

	memcpy(m, template, sizeof(monster));
	m->stats[sHP] = m->stats[sMaxHP];
	m->stats[sMP] = m->stats[sMaxMP];

	Add_to_List(combat_monsters, m);
	monsters_alive++;
}

void Add_Combat_Action(act id, char *name, action_check_fn check_fn, action_do_fn act_fn, targetflags target, UINT8 priority)
{
	combat_actions[id].check = check_fn;
	combat_actions[id].act = act_fn;
	combat_actions[id].name = name;
	combat_actions[id].targeting = target;
	combat_actions[id].priority = priority;
}

void Initialise_Combat(void)
{
	groupnum i;

	combat_monsters = New_List(ltObject, "Initialise_Combat.monsters");
	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		combat_groups[i] = New_List(ltInteger, "Initialise_Combat.group_x");
	}

	combat_actions = SzAlloc(NUM_ACTIONS, action, "Initialise_Combat.actions");
	Add_Combat_Action(aAttack, "Attack", Check_Attack, Attack, tfEnemy, 100);
	Add_Combat_Action(aSneakAttack, "Sneak Attack", Check_SneakAttack, SneakAttack, tfEnemy, 90);
	Add_Combat_Action(aBlock, "Block", Check_Block, Block, tfSelf, 120);
	Add_Combat_Action(aDefend, "Defend", Check_Defend, Defend, tfAlly, 120);
	Add_Combat_Action(aSing, "Sing", Check_Sing, Sing, tfSelf, 200);
	Add_Combat_Action(aHide, "Hide", Check_Hide, Hide, tfSelf, 50);

	PCX_Init(&combat_bg);
	PCX_Load("COMBAT.PCX", &combat_bg, 0);

	randomize();
}

void Free_Combat(void)
{
	groupnum i;

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

	if (action_ids == null || action_names == null)
		die("Get_Pc_Action: out of memory");

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

void Add_Buff(targ target, char *name, expiry_type exty, int duration, buff_expiry_fn expiry, int argument)
{
	buff *b;

	b = Allocate(1, sizeof(buff), name);
	b->name = name;
	b->type = exty;
	b->duration = duration;
	b->expiry = expiry;			/* is this dangerous to Read/Write? */
	b->argument = argument;

	if (IS_PC(target)) {
		Add_to_List(Get_Pc(TARGET_PC(target))->buffs, b);
	} else {
		/* TODO */
	}
}

bool Has_Buff(targ target, char *name)
{
	buff *b;
	monster *m;
	character *c;
	int i;

	if (IS_PC(target)) {
		c = Get_Pc(TARGET_PC(target));
		for (i = 0; i < c->buffs->size; i++) {
			b = List_At(c->buffs, i);

			if (strcmp(b->name, name) == 0) {
				return true;
			}
		}
	} else {
		/* TODO */
	}

	return false;
}

noexport void Remove_Buff_from_List(targ owner, list *buffs, buff *b)
{
	b->expiry(owner, b->argument);
	Remove_from_List(buffs, b);
}

void Remove_Buff(targ target, char *name)
{
	buff *b;
	monster *m;
	character *c;
	int i;

	if (IS_PC(target)) {
		c = Get_Pc(TARGET_PC(target));
		for (i = c->buffs->size - 1; i >= 0; i--) {
			b = List_At(c->buffs, i);

			if (strcmp(b->name, name) == 0) {
				Remove_Buff_from_List(target, c->buffs, b);
				return;
			}
		}
	} else {
		/* TODO */
	}
}

void Expire_Buffs(void)
{
	buff *b;
	character *c;
	int i, j;

	for (i = 0; i < PARTY_SIZE; i++) {
		c = Get_Pc(i);
		for (j = c->buffs->size - 1; j >= 0; j--) {
			b = List_At(c->buffs, j);
			if (b->type == exTurns) {
				b->duration--;

				if (b->duration <= 0) {
					Remove_Buff_from_List(TARGET_PC(i), c->buffs, b);
				}
			}

			if (b->type == exTurnEndChance) {
				if (randint(0, 100) < b->duration) {
					Remove_Buff_from_List(TARGET_PC(i), c->buffs, b);
				}
			}
		}
	}

	/* TODO: enemies */
}

void Expire_Combat_Buffs(void)
{
	buff *b;
	character *c;
	int i, j;

	for (i = 0; i < PARTY_SIZE; i++) {
		c = Get_Pc(i);
		for (j = c->buffs->size - 1; j >= 0; j--) {
			b = List_At(c->buffs, j);
			if (b->type == exTurns || b->type == exTurnEndChance) {
				Remove_Buff_from_List(TARGET_PC(i), c->buffs, b);
			}
		}
	}

	/* TODO: enemies */
}

/* A I  R O U T I N E S ////////////////////////////////////////////////// */

noexport void AI_Mindless(int monster, act *action, targ *target)
{
	int t = randint(0, PARTY_SIZE - 1);
	while (Get_Pc(t)->header.stats[sHP] <= 0) {
		t = randint(0, PARTY_SIZE - 1);
	}

	*action = aAttack;
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
	bool first_turn = true;
	act pc_actions[PARTY_SIZE];
	targ pc_targets[PARTY_SIZE];
	act *monster_actions;
	targ *monster_targets;

	monster_actions = SzAlloc(combat_monsters->size, act, "Enter_Combat_Loop.actions");
	monster_targets = SzAlloc(combat_monsters->size, targ, "Enter_Combat_Loop.targets");
	if (monster_actions == null || monster_targets == null)
		die("Enter_Combat_Loop: out of memory");

	while (monsters_alive > 0) {
		if (!first_turn) Expire_Buffs();

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

		first_turn = false;
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
	groupnum i;
	int count;
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
	earned_experience = 0;
	gState = gsCombat;
	Enter_Combat_Loop();
	
	/* TODO: if you lose... shouldn't do this */
	for (count = 0; count < PARTY_SIZE; count++) {
		if (!Is_Dead(TARGET_PC(count))) {
			Add_Experience(Get_Pc(count), earned_experience);
		}
	}

	gState = gsDungeon;
	Expire_Combat_Buffs();
	Redraw_Dungeon_Screen(false);
}
