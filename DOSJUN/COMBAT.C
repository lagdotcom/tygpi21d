/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "gamelib.h"

#include <time.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* highlight colours */
#define COLOUR_ACTIVE	14
#define COLOUR_SELECT	11

#define PRIORITY_WIGGLE	5
#define MAX_PRIORITY	(255 - PRIORITY_WIGGLE)

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	action_check_fn check;
	action_do_fn act;
	char *name;
	targetflags targeting;
	pri priority; /* TODO */
} action;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport pcx_picture combat_bg;

list *combatants;
noexport list *active_combatants;
noexport action *combat_actions;
noexport list *combat_groups[ENCOUNTER_SIZE];
noexport int group_y[ENCOUNTER_SIZE];
noexport int monsters_alive;
noexport int groups_alive;
noexport UINT32 earned_experience;

/* H E L P E R  F U N C T I O N S //////////////////////////////////////// */

noexport void Show_Combat_String(char *string, bool wait_for_key)
{
	Draw_Wrapped_Font(96, 144, 128, 48, 15, string, FNT, true);

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
		Draw_Font(248, 8 + i * 32, colour, buffer, FNT, true);
	}
}

noexport void Format_Enemy_Group(groupnum group, char *buffer)
{
	int enemy = (int)List_At(combat_groups[group], 0);
	sprintf(buffer, "%s x%u", Get_Combatant(enemy)->name, combat_groups[group]->size);
}

#define EPANEL_X	8
#define EPANEL_Y	8
#define EPANEL_W	10
#define EPANEL_H	2
noexport char enemy_panel_buffer[100];

noexport void Show_Enemies(void)
{
	groupnum i;
	int y = EPANEL_Y;

	Draw_Square_DB(0, EPANEL_X, EPANEL_Y, 87, 135, 1);

	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (combat_groups[i]->size > 0) {
			group_y[i] = y;
			Format_Enemy_Group(i, enemy_panel_buffer);
			Draw_Bounded_String(EPANEL_X, y, EPANEL_W, EPANEL_H, 15, enemy_panel_buffer, 0);

			y += 16;
		}
	}
}

noexport void Highlight_Enemy_Group(groupnum active, groupnum select)
{
	groupnum i;
	colour col;

	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		if (combat_groups[i]->size > 0) {
			Format_Enemy_Group(i, enemy_panel_buffer);
			
			col = 15;
			if (i == active) col = COLOUR_ACTIVE;
			if (i == select) col = COLOUR_SELECT;
			Draw_Bounded_String(EPANEL_X, group_y[i], EPANEL_W, EPANEL_H, col, enemy_panel_buffer, 0);
		}
	}
}

item *Get_Weapon(targ source)
{
	combatant *c = Get_Combatant(source);
	return c->weapon ? Lookup_Item(&gItems, c->weapon) : null;
}

stat_value Get_Stat(targ source, statistic st)
{
	combatant *c = Get_Combatant(source);
	return c->stats[st];
}

noexport void Set_Stat(targ source, statistic st, stat_value num)
{
	combatant *c = Get_Combatant(source);
	c->stats[st] = num;
}

statistic Get_Weapon_Stat(item *weapon)
{
	if (weapon == null) return sStrength;
	return (weapon->flags & ifDexterityWeapon) ? sDexterity : sStrength;
}

combatant *Get_Combatant(targ target)
{
	return List_At(combatants, target);
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
	groupnum group;
	combatant *c = Get_Combatant(victim);

	Combat_Message("%s dies!", c->name);

	if (c->is_pc) {
		/* TODO */
	} else {
		monsters_alive--;
		earned_experience += c->monster->experience;

		group = c->group;
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
	bool in_front_row;
	item *weapon;
	combatant *c = Get_Combatant(source);

	/* Sneak Attack is handled separately */
	if (Has_Buff(source, HIDE_BUFF_NAME)) return false;

	in_front_row = c->row == rowFront;
	weapon = Get_Weapon(source);

	if (weapon == null) return in_front_row;

	if (weapon->flags & (ifMediumRange | ifLongRange)) return true;
	return in_front_row;
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

noexport void Block_Expires(targ target, int argument)
{
	combatant *c = Get_Combatant(target);

	c->stats[sArmour] -= argument;
}

noexport void Block(targ source, targ target)
{
	combatant *c = Get_Combatant(target);
	int bonus = c->stats[sStrength];

	c->stats[sArmour] += bonus;
	Add_Buff(target, "Blocking", exTurns, 1, Block_Expires, bonus);

	Combat_Message("%s braces themselves.", c->name);
}

noexport bool Check_Defend(targ source)
{
	/* TODO: fix for enemy */
	unsigned int i;

	/* There must be an alive ally to Defend */
	if (IS_PC(source)) {
		for (i = 0; i < PARTY_SIZE; i++) {
			if (TARGET_PC(i) == source) continue;
			if (!Is_Dead(TARGET_PC(i))) return true;
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
	combatant *c;
	int i;

	Log("Clear_Encounter: %d to remove", combatants->size);

	for (i = 0; i < combatants->size; i++) {
		c = Get_Combatant(i);

		if (!c->is_pc) {
			Free(c->stats);
			Free_List(c->buffs);
		}
	}

	Clear_List(combatants);
}

void Add_Monster(groupnum group, monster* template)
{
	list *buffs;
	combatant *c = SzAlloc(1, combatant, "Add_Monster.combatant");
	stat_value *stats = SzAlloc(NUM_STATS, stat_value, "Add_Monster.stats");
	if (c == null || stats == null)
		die("Add_Monster: out of memory");

	buffs = New_List(ltObject, "Add_Monster.buffs");

	memcpy(stats, template->stats, sizeof(stat_value) * NUM_STATS);
	stats[sHP] = stats[sMaxHP];
	stats[sMP] = stats[sMaxMP];

	c->action = NO_ACTION;
	c->buffs = buffs;
	c->group = group;
	c->is_pc = false;
	c->monster = template;
	c->name = template->name;
	c->pc = null;
	c->row = template->row;
	c->self = combatants->size;
	c->skills = null;	/* TODO */
	c->stats = stats;
	c->weapon = template->weapon;

	Add_to_List(combat_groups[group], (void*)combatants->size);
	Add_to_List(combatants, c);
	monsters_alive++;
}

noexport void Add_Pc(unsigned int pc)
{
	character *ch = Get_Pc(pc);

	combatant *c = SzAlloc(1, combatant, "Add_Pc");
	if (c == null)
		die("Add_Pc: out of memory");

	c->action = NO_ACTION;
	c->buffs = ch->buffs;
	c->group = -1;
	c->is_pc = true;
	c->monster = null;
	c->name = ch->header.name;
	c->pc = ch;
	c->row = In_Front_Row(pc) ? rowFront : rowBack;
	c->self = combatants->size;
	c->skills = ch->skills;
	c->stats = ch->header.stats;
	c->weapon = Get_Equipped_Weapon(pc)->id;

	Add_to_List(combatants, c);
}

void Add_Combat_Action(act id, char *name, action_check_fn check_fn, action_do_fn act_fn, targetflags target, pri priority)
{
	combat_actions[id].check = check_fn;
	combat_actions[id].act = act_fn;
	combat_actions[id].name = name;
	combat_actions[id].targeting = target;
	combat_actions[id].priority = priority > MAX_PRIORITY ? MAX_PRIORITY : priority;
}

void Initialise_Combat(void)
{
	groupnum i;

	combatants = New_List(ltObject, "Initialise_Combat.combatants");
	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		combat_groups[i] = New_List(ltInteger, "Initialise_Combat.group_x");
	}

	/* HACK: ltInteger makes sure Clear_List() doesn't Free() items */
	active_combatants = New_List(ltInteger, "Initialise_Combat.active_combatants");

	combat_actions = SzAlloc(NUM_ACTIONS, action, "Initialise_Combat.actions");
	Add_Combat_Action(aAttack, "Attack", Check_Attack, Attack, tfEnemy, 100);
	Add_Combat_Action(aSneakAttack, "Sneak Attack", Check_SneakAttack, SneakAttack, tfEnemy, 130);
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

	Clear_Encounter();
	Free_List(combatants);
	for (i = 0; i < ENCOUNTER_SIZE; i++) {
		Free_List(combat_groups[i]);
	}

	Free_List(active_combatants);

	Free(combat_actions);
	PCX_Delete(&combat_bg);
}

noexport act Get_Pc_Action(unsigned char pc)
{
	int i, count = 0;
	act ai;
	int *action_ids;
	char **action_names;

	action_ids = SzAlloc(NUM_ACTIONS, int, "Get_Pc_Action.ids");
	action_names = SzAlloc(NUM_ACTIONS, char *, "Get_Pc_Action.names");
	if (action_ids == null || action_names == null)
		die("Get_Pc_Action: out of memory");

	for (i = 0; i < NUM_ACTIONS; i++) {
		if (combat_actions[i].check(TARGET_PC(pc))) {
			action_names[count] = combat_actions[i].name;
			action_ids[count++] = i;
		}
	}

	/* TODO: tiered menu? */
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
	int choice, change;
	int diff;

	/* get the easiest case out of the way */
	if (a->targeting == tfSelf) return pc;

	if (a->targeting & tfEnemy) {
		choice = PARTY_SIZE;
	} else {
		choice = TARGET_PC(0);
	}

	Highlight_Ally(pc, COLOUR_ACTIVE);
	while (!done) {
		if (IS_PC(choice)) {
			Highlight_Ally(pc, choice);
		} else {
			Highlight_Enemy_Group(-1, choice - PARTY_SIZE);
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
					change = PARTY_SIZE;
				}
				break;

			case SCAN_RIGHT:
				if (!IS_PC(choice) && a->targeting & tfAlly) {
					change = TARGET_PC(0);
				}
				break;

			case SCAN_UP:
				if (IS_PC(choice) && choice != TARGET_PC(0)) {
					change = choice - 1;
				}
				else if (!IS_PC(choice) && choice > PARTY_SIZE) {
					change = choice - 1;
				}
				break;

			case SCAN_DOWN:
				if (IS_PC(choice) && choice != TARGET_PC(PARTY_SIZE - 1)) {
					change = choice + 1;
				}
				else if (!IS_PC(choice) && choice < (groups_alive + PARTY_SIZE - 1)) {
					change = choice + 1;
				}
				break;
		}

		if (change != choice || done) {
			if (IS_PC(change)) {
				diff = IS_PC(choice) ? change - choice : -1;
				while (!Is_Valid_Target(pc, change, a->targeting)) {
					change += diff;
					if (change == PARTY_SIZE) change = TARGET_PC(0);
					if (change == -1) change = TARGET_PC(PARTY_SIZE - 1);
				}

				Highlight_Ally(pc, -1);
			} else {
				Highlight_Enemy_Group(-1, -1);
			}

			choice = change;
		}
	}

	if (!IS_PC(choice)) {
		choice = Get_Random_Target(choice - PARTY_SIZE);
	}

	return choice;
}

void Add_Buff(targ target, char *name, expiry_type exty, int duration, buff_expiry_fn expiry, int argument)
{
	buff *b;

	b = Allocate(1, sizeof(buff), name);
	if (b == null)
		die("Add_Buff: out of memory");

	b->name = name;
	b->type = exty;
	b->duration = duration;
	b->expiry = expiry;			/* is this dangerous to Read/Write? */
	b->argument = argument;

	Add_to_List(Get_Combatant(target)->buffs, b);
}

bool Has_Buff(targ target, char *name)
{
	buff *b;
	int i;
	combatant *c = Get_Combatant(target);

	for (i = 0; i < c->buffs->size; i++) {
		b = List_At(c->buffs, i);

		if (strcmp(b->name, name) == 0) {
			return true;
		}
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
	int i;
	combatant *c = Get_Combatant(target);

	for (i = c->buffs->size - 1; i >= 0; i--) {
		b = List_At(c->buffs, i);

		if (strcmp(b->name, name) == 0) {
			Remove_Buff_from_List(target, c->buffs, b);
			return;
		}
	}
}

void Expire_Buffs(void)
{
	buff *b;
	combatant *c;
	int i, j;

	for (i = 0; i < combatants->size; i++) {
		c = Get_Combatant(i);

		for (j = c->buffs->size - 1; j >= 0; j--) {
			b = List_At(c->buffs, j);
			if (b->type == exTurns) {
				b->duration--;

				if (b->duration <= 0) {
					Remove_Buff_from_List(i, c->buffs, b);
				}
			}

			if (b->type == exTurnEndChance) {
				if (randint(0, 100) < b->duration) {
					Remove_Buff_from_List(i, c->buffs, b);
				}
			}
		}
	}
}

void Expire_Combat_Buffs(void)
{
	buff *b;
	combatant *c;
	int i, j;

	for (i = 0; i < combatants->size; i++) {
		c = Get_Combatant(i);
		for (j = c->buffs->size - 1; j >= 0; j--) {
			b = List_At(c->buffs, j);
			if (b->type == exTurns || b->type == exTurnEndChance) {
				Remove_Buff_from_List(i, c->buffs, b);
			}
		}
	}
}

/* A I  R O U T I N E S ////////////////////////////////////////////////// */

noexport void AI_Mindless(int monster, act *action, targ *target)
{
	int t = randint(0, PARTY_SIZE - 1);
	while (Is_Dead(t)) {
		t = randint(0, PARTY_SIZE - 1);
	}

	*action = aAttack;
	*target = t;
}

noexport void Show_Combat_Pc_Stats(void)
{
	int i;

	for (i = 0; i < PARTY_SIZE; i++) {
		Draw_Character_Status(i, 232, 8 + i * 32);
	}
}

noexport void Sort_by_Priority(list *active)
{
	int size = active->size,
		i, j;

	combatant *a, *b, *temp;

	for (i = 0; i < size - 1; i++) {
		a = List_At(active, i);
		for (j = i + 1; j < size; j++) {
			b = List_At(active, j);

			if (b->priority > a->priority) {
				/* Swap place in list */
				active->items[i] = b;
				active->items[j] = a;

				/* Swap place in memory for later comparisons */
				temp = a;
				a = b;
				b = temp;
			}
		}
	}
}

noexport void Do_Combat_Actions(list *active)
{
	int i;
	combatant *c;
	targ pc_self,
		pc_targ;
	groupnum egroup_self,
		egroup_targ;

	for (i = 0; i < active->size; i++) {
		c = List_At(active, i);

		/* TODO: could have self-res enemies? */
		if (c->action != NO_ACTION && !Is_Dead(c->self)) {
			if (c->is_pc) {
				pc_self = c->self;
				egroup_self = -1;
			} else {
				pc_self = -1;
				egroup_self = c->group;
			}

			if (IS_PC(c->target)) {
				pc_targ = c->target;
				egroup_targ = -1;
			} else {
				pc_targ = -1;
				egroup_targ = Get_Combatant(c->target)->group;
			}

			Highlight_Ally(pc_self, pc_targ);
			Highlight_Enemy_Group(egroup_self, egroup_targ);

			/* TODO: retarget dead enemies? */
			combat_actions[c->action].act(c->self, c->target);
		}
	}
}

noexport void Enter_Combat_Loop(void)
{
	int i;
	combatant *c;
	bool first_turn = true;

	while (monsters_alive > 0) {
		Log("Enter_Combat_Loop: begin round");
		if (!first_turn) Expire_Buffs();

		Show_Combat_Pc_Stats();
		Show_Enemies();

		for (i = 0; i < combatants->size; i++) {
			c = Get_Combatant(i);

			/* TODO: could have self-res enemies? */
			if (Is_Dead(i)) {
				c->action = NO_ACTION;
				continue;
			}

			if (c->is_pc) {
				c->action = Get_Pc_Action(i);
				c->target = Get_Pc_Target(i, c->action);
			} else {
				switch (c->monster->ai) {
					case aiMindless:
						AI_Mindless(i, &c->action, &c->target);
						break;
				}
			}

			if (c->action != NO_ACTION) {
				c->priority = combat_actions[c->action].priority + randint(0, PRIORITY_WIGGLE);
				Add_to_List(active_combatants, c);
			}
		}

		Sort_by_Priority(active_combatants);
		Do_Combat_Actions(active_combatants);

		Clear_List(active_combatants);
		first_turn = false;
	}

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

	Log("Start_Combat: #%d", id);

	for (i = 0; i < PARTY_SIZE; i++) {
		Add_Pc(i);
	}

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
				Add_Monster(groups_alive, m);
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
		if (!Is_Dead(count)) {
			Add_Experience(Get_Pc(count), earned_experience);
		}
	}

	Clear_Encounter();

	gState = gsDungeon;
	Expire_Combat_Buffs();
	Redraw_Dungeon_Screen(false);
}
