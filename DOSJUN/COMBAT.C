/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "features.h"
#include "gamelib.h"

#include <time.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* highlight colours */
#define COLOUR_ACTIVE	CYAN
#define COLOUR_SELECT	YELLOW

#define PRIORITY_WIGGLE	5
#define MAX_PRIORITY	(255 - PRIORITY_WIGGLE)

#define GROUP_PCFRONT	ENCOUNTER_SIZE
#define GROUP_PCBACK	(ENCOUNTER_SIZE + 1)
#define GROUPS_SIZE		(ENCOUNTER_SIZE + 2)

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct action {
	action_check_fn check;
	action_do_fn act;
	char *name;
	targetflags targeting;
	pri priority;
} action;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport grf *combat_bg;

list *combatants;
noexport list *active_combatants;
noexport action *combat_actions;
noexport combatant **pc_combatants;
noexport list *combat_groups[GROUPS_SIZE];
noexport int group_y[ENCOUNTER_SIZE];
noexport int monsters_alive;
noexport int groups_alive;
noexport UINT32 earned_experience;

noexport box2d combatStringBox = {
	{ 100, 148 },
	{ 220, 188 },
};

/* H E L P E R  F U N C T I O N S //////////////////////////////////////// */

noexport void Show_Combat_String(char *string, bool wait_for_key, file_id speaker, file_id target)
{
	/* Draw_Wrapped_Font(96, 144, 128, 48, WHITE, string, gFont, true); */
	Show_Formatted_String(string, speaker, target, &combatStringBox, gFont, 0, false);

	if (wait_for_key) {
		Show_Double_Buffer();
		Delay(5);
		Get_Next_Scan_Code();
	} else {
		Show_Double_Buffer();
	}
}

void Combat_Message(file_id speaker, file_id target, char *format, ...)
{
	va_list vargs;
#if COMBAT_ALLOC_MSGBUF
	char *message;
	int size2d;
#else
	char message[COMBAT_BUFFER_SIZE];
#endif

	va_start(vargs, format);
#if COMBAT_ALLOC_MSGBUF
	size2d = vsprintf(NULL, format, vargs);
	message = Allocate(size2d+1, 1, "Combat_Message");
#endif
	vsprintf(message, format, vargs);
	va_end(vargs);

	Show_Combat_String(message, true, speaker, target);
#if COMBAT_ALLOC_MSGBUF
	Free(message);
#endif
}

noexport void Highlight_Ally(int pc_active, int pc_select)
{
	char buffer[9];
	int i, colour;

	for (i = 0; i < PARTY_SIZE; i++) {
		if (!pc_combatants[i]) continue;

		strncpy(buffer, Get_PC(i)->name, 8);
		buffer[8] = 0;

		colour = 15;
		if (i == pc_active) colour = COLOUR_ACTIVE;
		if (i == pc_select) colour = COLOUR_SELECT;
		Draw_Font(248, 8 + i * 32, colour, buffer, gFont, true);
	}
}

noexport void Format_Enemy_Group(groupnum group, char *buffer)
{
	combatant *enemy;
	assert(group < GROUPS_SIZE, "Format_Enemy_Group: group number too high");

	enemy = List_At(combat_groups[group], 0);
	sprintf(buffer, "%s x%u", enemy->name, combat_groups[group]->size);
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

item *Get_Weapon(combatant *c, bool primary)
{
	file_id iid = primary ? c->primary : c->secondary;

	return iid ? Lookup_File_Chained(gDjn, iid) : null;
}

range Get_Weapon_Range(item *weapon)
{
	if (weapon == null) return rShort;

	if (weapon->flags & ifLongRange) return rLong;
	if (weapon->flags & ifMediumRange) return rMedium;
	return rShort;
}

range Get_Attack_Range(combatant *c)
{
	item *primary;

	if (Has_Buff(c, bfHidden)) {
		return rLong;
	}

	primary = Get_Weapon(c, true);
	return Get_Weapon_Range(primary);
}

range Get_Combatant_Range(combatant *a, combatant *b)
{
	if (a->row == rowFront) {
		return b->row == rowFront ? rShort : rMedium;
	}

	return b->row == rowFront ? rMedium : rLong;
}

stat_value Get_Stat_Base(stat_value *stats, statistic st)
{
	switch (st)
	{
		case sToughness: return stats[sStrength] / 3;
		case sMinDamage: return stats[sStrength] / 5;
		case sMaxDamage: return stats[sStrength] / 4;
		default: return 0;
	}
}

stat_value Get_Stat(combatant *c, statistic st)
{
	stat_value base;
	assert(st < NUM_STATS, "Get_Stat: stat number too high");

	base = Get_Stat_Base(c->stats, st);
	return base + c->stats[st];
}

noexport void Set_Stat(combatant *c, statistic st, stat_value num)
{
	assert(st < NUM_STATS, "Get_Stat: stat number too high");

	c->stats[st] = num;
}

statistic Get_Weapon_Stat(item *weapon)
{
	if (weapon == null) return sStrength;
	return (weapon->flags & ifDexterityWeapon) ? sDexterity : sStrength;
}

combatant *Get_Random_Target(groupnum group)
{
	int i;
	assert(group < GROUPS_SIZE, "Get_Random_Target: group number too high");

	if (combat_groups[group]->size <= 0)
		return null;

	i = randint(0, combat_groups[group]->size - 1);
	return List_At(combat_groups[group], i);
}

bool Is_Dead(combatant *victim)
{
	return Get_Stat(victim, sHP) <= 0;
}

noexport void Kill(combatant *c, combatant *killer)
{
	groupnum group;

#if COMBAT_DEBUG
	Log("Kill: %s(%d/g%d)", c->name, c->index, c->group);
#endif
	Combat_Message(c->file, killer == null ? 0 : killer->file, "@n dies!");

	if (c->is_pc) {
		/* TODO */
	} else {
		monsters_alive--;
		earned_experience += c->monster->experience;

		group = c->group;
#if COMBAT_DEBUG
		Log("Kill: shrinking group #%d", group);
#endif
		Remove_from_List(combat_groups[group], (void*)c);
		if (combat_groups[group]->size == 0) {
			groups_alive--;
#if COMBAT_DEBUG
			Log("Kill: groups alive=%d", groups_alive);
#endif
		}

		/* On-kill triggered skills */
		if (Check_Cleave(killer)) Cleave(killer, c);
		if (Check_Inspire(killer)) Inspire(killer, c);
	}
}

void Damage(combatant *victim, combatant *attacker, int amount)
{
	stat_value hp = Get_Stat(victim, sHP);
	hp -= amount;

	Combat_Message(victim->file, attacker == null ? 0 : attacker->file, "@n takes %d damage.", amount);
	Set_Stat(victim, sHP, hp);
	if (hp <= 0) {
		Kill(victim, attacker);
	}
}

void Get_Weapon_Damage(combatant *source, item *weapon, stat_value *min, stat_value *max)
{
	*min = Get_Stat(source, sMinDamage);
	*max = Get_Stat(source, sMaxDamage);

	if (weapon != null) {
		*min += weapon->stats[sMinDamage];
		*max += weapon->stats[sMaxDamage];
	}
}

/* C O M B A T  A C T I O N S //////////////////////////////////////////// */

bool Check_Attack(combatant *source)
{
	bool in_front_row;
	item *weapon;

	/* Sneak Attack is handled separately */
	if (Has_Buff(source, bfHidden)) return false;

	in_front_row = source->row == rowFront;
	weapon = Get_Weapon(source, true);

	if (weapon == null) return in_front_row;

	if (weapon->flags & (ifMediumRange | ifLongRange)) return true;
	return in_front_row;
}

noexport void Attack_Inner(combatant *source, combatant *target, item *weapon)
{
	stat_value base = Get_Stat(source, Get_Weapon_Stat(weapon));
	stat_value min, max;
	int roll;

	if (Is_Dead(target)) {
		/* TODO: reassign target? */
		return;
	}

	if (Get_Combatant_Range(source, target) > Get_Weapon_Range(weapon))
		return;

	base += Get_Stat(source, sHitBonus);
	base -= Get_Stat(target, sDodgeBonus);

	if (randint(1, 20) <= base) {
		Combat_Message(source->file, target->file, "@n hits @N!");

		Get_Weapon_Damage(source, weapon, &min, &max);
		roll = randint(min, max) - Get_Stat(target, sArmour);
		if (roll > 0) {
			Damage(target, source, roll);
		} else {
			Combat_Message(source->file, target->file, "The blow glances off.");
		}
	} else {
		Combat_Message(source->file, target->file, "@n attacks @N and misses.");
	}
}

void With_Both_Weapons(combatant *source, combatant *target, weapon_atk_fn func)
{
	item *weapon = Get_Weapon(source, true);
	func(source, target, weapon);

	weapon = Get_Weapon(source, false);
	if (weapon) func(source, target, weapon);
}

void Attack(combatant *source, combatant *target)
{
	With_Both_Weapons(source, target, Attack_Inner);
}

noexport bool Check_Block(combatant *source)
{
	return true;
}

void Blocking_Expires(combatant *c, buff *b)
{
	c->stats[sArmour] -= b->arg1;
}

noexport void Block(combatant *c, combatant *target)
{
	int bonus = c->stats[sStrength];

	c->stats[sArmour] += bonus;
	Add_Buff(target, Make_Buff(bfBlocking, 1, bonus, 0, "Block"));

	Combat_Message(c->file, 0, "@n braces @f.");
}

noexport bool Check_Defend(combatant *source)
{
	/* TODO: fix for enemy */
	unsigned int i;
	combatant *c;

	/* There must be an alive ally to Defend */
	if (source->is_pc) {
		for (i = 0; i < PARTY_SIZE; i++) {
			c = List_At(combatants, i);

			if (c == source) continue;
			if (!Is_Dead(c)) return true;
		}
	} else {
		/* TODO */
	}

	return false;
}

noexport void Defend(combatant *source, combatant *target)
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

#if COMBAT_DEBUG
	Log("Clear_Encounter: %d to remove", combatants->size);
#endif

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);

		if (!c->is_pc) {
			Free(c->stats);
			Free_List(c->buffs);
		}
	}

	Clear_List(combatants);
}

#if COMBAT_DEBUG
noexport void Log_Combatant(combatant *c)
{
	int i;
	buff *b;
	buffspec *spec;
	skill_id sk;

	Log("Log_Combatant: %d - %s (#%d, %s)", c->index, c->name, c->file, c->is_pc ? "PC" : "Monster");
	Log("- Group #%d (%s Row)", c->group, c->row == rowFront ? "Front" : "Back");
	Log("- Weapon #%d/#%d", c->primary, c->secondary);

	if (c->skills != null && c->skills->size > 0) {
		Log("%s", "- Skills:");

		for (i = 0; i < c->skills->size; i++) {
			sk = (skill_id)List_At(c->skills, i);
			Log("-- %s", Skill_Name(sk));
		}
	}

	if (c->buffs != null && c->buffs->size > 0) {
		Log("%s", "- Buffs:");

		for (i = 0; i < c->buffs->size; i++) {
			b = List_At(c->buffs, i);
			spec = &buffspecs[b->id];
			Log("-- %s", spec->name);
		}
	}
}
#endif

combatant *Add_Monster(groupnum group, file_id ref)
{
	list *buffs;
	monster *template;
	combatant *c = SzAlloc(1, combatant, "Add_Monster.combatant");
	stat_value *stats = SzAlloc(NUM_STATS, stat_value, "Add_Monster.stats");
	if (c == null || stats == null)
		die("Add_Monster: out of memory");

	buffs = New_List(ltObject, "Add_Monster.buffs");

	template = Lookup_File_Chained(gDjn, ref);
	memcpy(stats, template->stats, sizeof(stat_value) * NUM_STATS);
	stats[sHP] = stats[sMaxHP];
	stats[sMP] = stats[sMaxMP];

	c->file = ref;
	c->action = NO_ACTION;
	c->buffs = buffs;
	c->group = group;
	c->is_pc = false;
	c->monster = template;
	c->name = Resolve_String(template->name_id);
	c->pc = null;
	c->row = template->row;
	c->index = combatants->size;
	c->skills = template->skills;
	c->stats = stats;
	c->primary = template->weapon;
	c->secondary = 0;

	Add_to_List(combat_groups[group], c);
	Add_to_List(combatants, c);
	monsters_alive++;

#if COMBAT_DEBUG
	Log_Combatant(c);
#endif

	return c;
}

noexport combatant *Add_Pc(pc *pc, file_id ref, pcnum index)
{
	combatant *c;
	if (!pc) return null;

	c = SzAlloc(1, combatant, "Add_Pc");
	if (c == null)
		die("Add_Pc: out of memory");

	c->file = ref;
	c->action = NO_ACTION;
	c->buffs = pc->buffs;
	c->group = In_Front_Row(pc) ? GROUP_PCFRONT : GROUP_PCBACK;
	c->is_pc = true;
	c->monster = null;
	c->name = pc->name;
	c->pc = pc;
	c->row = In_Front_Row(pc) ? rowFront : rowBack;
	c->index = combatants->size;
	c->skills = pc->skills;
	c->stats = pc->header.stats;
	c->primary = Get_Equipped_Weapon_Id(pc, true);
	c->secondary = Get_Equipped_Weapon_Id(pc, false);

	Add_to_List(combat_groups[c->group], c);
	Add_to_List(combatants, c);

	pc_combatants[index] = c;

#if COMBAT_DEBUG
	Log_Combatant(c);
#endif

	return c;
}

noexport combatant *Add_PC_by_Index(pcnum index)
{
	return Add_Pc(Get_PC(index), Get_PC_ID(index), index);
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
	for (i = 0; i < GROUPS_SIZE; i++) {
		combat_groups[i] = New_List(ltReference, "Initialise_Combat.group_x");
	}

	pc_combatants = SzAlloc(PARTY_SIZE, combatant*, "Initialise_Combat.pc_combatants");
	active_combatants = New_List(ltReference, "Initialise_Combat.active_combatants");

	combat_actions = SzAlloc(NUM_ACTIONS, action, "Initialise_Combat.actions");
	Add_Combat_Action(aAttack, "Attack", Check_Attack, Attack, tfEnemy, 100);
	Add_Combat_Action(aSneakAttack, "Sneak Attack", Check_SneakAttack, SneakAttack, tfEnemy, 130);
	Add_Combat_Action(aBlock, "Block", Check_Block, Block, tfSelf, 120);
	Add_Combat_Action(aDefend, "Defend", Check_Defend, Defend, tfAlly, 120);

	Add_Combat_Action(aConcentrate, "Concentrate", Check_Concentrate, Concentrate, tfEnemy, 5);

	Add_Combat_Action(aSing, "Sing", Check_Sing, Sing, tfSelf, 200);
	Add_Combat_Action(aHide, "Hide", Check_Hide, Hide, tfSelf, 50);

	combat_bg = Lookup_File_Chained(gDjn, gCampaign->combatbg_id);

	randomize();
}

void Free_Combat(void)
{
	groupnum i;

	Log("Free_Combat: %p", combatants);

	Clear_Encounter();
	Free_List(combatants);
	for (i = 0; i < GROUPS_SIZE; i++) {
		Free_List(combat_groups[i]);
	}

	Free(pc_combatants);
	Free_List(active_combatants);
	Free(combat_actions);

	if (combat_bg)
		Unload_File(gDjn, gCampaign->combatbg_id);
}

noexport act Get_Pc_Action(pcnum pc)
{
	int i, count = 0;
	act ai;
	int *action_ids;
	char **action_names;
	combatant *self = List_At(combatants, pc);

	action_ids = SzAlloc(NUM_ACTIONS, int, "Get_Pc_Action.ids");
	action_names = SzAlloc(NUM_ACTIONS, char *, "Get_Pc_Action.names");
	if (action_ids == null || action_names == null)
		die("Get_Pc_Action: out of memory");

	for (i = 0; i < NUM_ACTIONS; i++) {
		if (combat_actions[i].check(self)) {
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

noexport bool Is_Valid_Target(combatant *source, combatant *target, targetflags tf)
{
	if (source == target) return tf & tfSelf;
	if (Is_Dead(target) && !(tf & tfDead)) return false;

	if (source->is_pc == target->is_pc) return tf & tfAlly;
	return tf & tfEnemy;
}

noexport combatant *Get_Pc_Target(pcnum pc, act action_id)
{
	bool done = false,
		party = false;
	targetflags tfl = combat_actions[action_id].targeting;
	combatant *self = List_At(combatants, pc),
		*c;
	pcnum targ_pc = 0,
		scan_pc = 0;
	groupnum targ_eg = 0,
		scan_eg = 0;
	char ch;
	
	if (tfl == tfSelf) return self;

	if (tfl & tfAlly) {
		party = true;
	}

	while (!done) {
		if (party) {
			Highlight_Ally(pc, targ_pc);
			Highlight_Enemy_Group(-1, -1);
		} else {
			Highlight_Ally(pc, -1);
			Highlight_Enemy_Group(-1, targ_eg);
		}

		Show_Double_Buffer();
		ch = Get_Next_Scan_Code();

		switch (ch) {
			case SCAN_ENTER:
				done = true;
				break;

			case SCAN_LEFT:
				if (tfl & tfAlly) {
					party = true;
				}
				break;

			case SCAN_RIGHT:
				if (tfl & tfEnemy) {
					party = true;
				}
				break;

			case SCAN_UP:
				if (party) {
					scan_pc = -1;
				} else {
					scan_eg = -1;
				}
				break;

			case SCAN_DOWN:
				if (party) {
					scan_pc = 1;
				} else {
					scan_eg = 1;
				}
				break;
		}

		if (scan_pc) {
			while (true) {
				targ_pc += scan_pc;
				if (targ_pc >= PARTY_SIZE) targ_pc = 0;
				if (targ_pc < 0) targ_pc = PARTY_SIZE - 1;

				if (Is_Valid_Target(self, List_At(combatants, targ_pc), tfl))
					break;
			}

			scan_pc = 0;
		}

		if (scan_eg) {
			while (true) {
				targ_eg += scan_eg;
				if (targ_eg >= ENCOUNTER_SIZE) targ_eg = 0;
				if (targ_eg < 0) targ_eg = ENCOUNTER_SIZE - 1;

				if (combat_groups[targ_eg]->size > 0) {
					c = List_At(combat_groups[targ_eg], 0);

					if (Is_Valid_Target(self, c, tfl))
						break;
				}
			}

			scan_eg = 0;
		}
	}

	Highlight_Ally(-1, -1);
	Highlight_Enemy_Group(-1, -1);

	if (party) {
		return List_At(combatants, targ_pc);
	} else {
		return Get_Random_Target(targ_eg);
	}
}

void Add_Buff(combatant *target, buff *b)
{
#if COMBAT_DEBUG
	Log("Add_Buff: %s(%d/g%d) gains %s", target->name, target->index, target->group, buffspecs[b->id].name);
#endif

	Add_to_List(target->buffs, b);
}

bool Has_Buff(combatant *c, buff_id id)
{
	buff *b;
	int i;

	for (i = 0; i < c->buffs->size; i++) {
		b = List_At(c->buffs, i);

		if (b->id == id) {
			return true;
		}
	}

	return false;
}

noexport void Remove_Buff_from_List(combatant *owner, buff *b)
{
	buffspec *spec = &buffspecs[b->id];

#if COMBAT_DEBUG
	Log("Remove_Buff_from_List: %s(%d/g%d) loses %s", owner->name, owner->index, owner->group, spec->name);
#endif

	if (spec->expiry != null)
		spec->expiry(owner, b);

	Remove_from_List(owner->buffs, b);
}

bool Remove_Buff(combatant *target, buff_id id)
{
	buff *b;
	int i;

	for (i = target->buffs->size - 1; i >= 0; i--) {
		b = List_At(target->buffs, i);

		if (b->id == id) {
			Remove_Buff_from_List(target, b);
			return true;
		}
	}

	return false;
}

bool Remove_Buff_from_PC(file_id ref, buff_id id)
{
	pc *pc;
	buff *b;
	combatant *target;
	int i;

	pc = Lookup_File_Chained(gSave, ref);
	for (i = pc->buffs->size - 1; i >= 0; i--) {
		b = List_At(pc->buffs, i);

		if (b->id == id) {
			/* Temporarily create a combatant! */
			target = Add_Pc(pc, ref, 0);
			Remove_Buff_from_List(target, b);
			Free(target);
			return true;
		}
	}

	return false;
}

void Expire_Buffs(void)
{
	buff *b;
	buffspec *spec;
	combatant *c;
	int i, j;

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);

		for (j = c->buffs->size - 1; j >= 0; j--) {
			b = List_At(c->buffs, j);
			spec = &buffspecs[b->id];

			if (spec->type == exTurns) {
				b->duration--;

				if (b->duration <= 0) {
					Remove_Buff_from_List(c, b);
				}
			}

			if (spec->type == exTurnEndChance) {
				if (randint(0, 100) < b->duration) {
					Remove_Buff_from_List(c, b);
				}
			}
		}
	}
}

void Expire_Combat_Buffs(void)
{
	buff *b;
	buffspec *spec;
	combatant *c;
	int i, j;

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);
		for (j = c->buffs->size - 1; j >= 0; j--) {
			b = List_At(c->buffs, j);
			spec = &buffspecs[b->id];

			if (spec->type == exTurns || spec->type == exTurnEndChance) {
				Remove_Buff_from_List(c, b);
			}
		}
	}
}

/* A I  R O U T I N E S ////////////////////////////////////////////////// */

noexport combatant *Random_Alive_Pc(void)
{
	int t;
	combatant *c;

	/* TODO: make this less awkward */
	do {
		t = randint(0, PARTY_SIZE - 1);
		c = List_At(combatants, t);
	} while (Is_Dead(c));

	return c;
}

noexport void AI_Mindless(combatant *c)
{
	combatant *target = null;
	range weapon_range = Get_Weapon_Range(Get_Weapon(c, true));

	switch (weapon_range) {
		case rLong:
			target = Get_Random_Target(GROUP_PCBACK);
			if (!target) target = Get_Random_Target(GROUP_PCFRONT);
			break;

		default:
			target = Get_Random_Target(GROUP_PCFRONT);
			break;
	}

	if (target) {
		c->action = aAttack;
		c->target = target;
	} else {
		c->action = NO_ACTION;
	}
}

noexport void AI_Rogue(combatant *c)
{
	/* TODO: player row */
	if (Check_Hide(c) && randint(0, 1) == 1) {
		c->action = aHide;
		c->target = c;
	} else {
		c->action = Check_SneakAttack(c) ? aSneakAttack : aAttack;
		c->target = Random_Alive_Pc();
	}
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
	action *a;
	combatant *c, *t;
	int pc_self = -1,
		pc_targ = -1;
	groupnum egroup_self = -1,
		egroup_targ = -1;

	for (i = 0; i < active->size; i++) {
		c = List_At(active, i);
		t = (combatant *)c->target;

		/* TODO: could have self-res enemies? */
		if (c->action != NO_ACTION && !Is_Dead(c)) {
			a = &combat_actions[c->action];

#if COMBAT_DEBUG
			Log("Do_Combat_Actions: %s is doing %s on %s(%d/g%d).", c->name, a->name, t->name, t->index, t->group);
#endif

			if (!Is_Valid_Target(c, t, a->targeting)) {
#if COMBAT_DEBUG
				Log("Do_Combat_Actions: %s(%d/g%d) is an invalid target.", t->name, t->index, t->group);
#endif
				t = Get_Random_Target(t->group);
				if (t == null) {
#if COMBAT_DEBUG
					Log("%s", "Do_Combat_Actions: Could not find another target.");
#endif
					/* only report the failure if we're still fighting */
					if (groups_alive > 0)
						Combat_Message(c->file, 0, "@n misses @r chance.");

					continue;
				}

#if COMBAT_DEBUG
				Log("Do_Combat_Actions: Retargeted to %s(%d/g%d).", t->name, t->index, t->group);
#endif
			}

			if (c->is_pc) {
				pc_self = i;
			} else {
				egroup_self = c->group;
			}

			if (t->is_pc) {
				pc_targ = c->index;
			} else {
				egroup_targ = t->group;
			}

			Highlight_Ally(pc_self, pc_targ);
			Highlight_Enemy_Group(egroup_self, egroup_targ);
			
			combat_actions[c->action].act(c, t);
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
			c = List_At(combatants, i);

			/* TODO: could have self-res enemies? */
			if (Is_Dead(c)) {
				c->action = NO_ACTION;
				continue;
			}

			if (Has_Buff(c, bfLoseNextMove)) {
				Remove_Buff(c, bfLoseNextMove);
				c->action = NO_ACTION;
				continue;
			}

			if (c->is_pc) {
				c->action = Get_Pc_Action(i);
				c->target = Get_Pc_Target(i, c->action);
			} else {
				switch (c->monster->ai) {
					case aiMindless:
						AI_Mindless(c);
						break;

					case aiRogue:
						AI_Rogue(c);
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
	char *description,
		*write,
		*name;
	file_id first_img = 0;
	groupnum group;
	pcnum pc;
	int count;
	encounter *en = &gZone->encounters[id];
	monster *m;
	combatant *c;

	Log("Start_Combat: #%d", id);

	memset(pc_combatants, 0, sizeof(combatant*) * PARTY_SIZE);
	for (pc = 0; pc < PARTY_SIZE; pc++) {
		Add_PC_by_Index(pc);
	}

	description = Allocate(10 + (NAME_SIZE+ENCOUNTER_SIZE)*6, 1, "Start_Combat");

	write = description;
	write += sprintf(write, "You face:");
	groups_alive = 0;

	for (group = 0; group < ENCOUNTER_SIZE; group++) {
		if (!en->monsters[group]) continue;
		m = Lookup_File_Chained(gDjn, en->monsters[group]);
		if (!first_img) first_img = m->image_id;

		count = randint(en->minimum[group], en->maximum[group]);
		if (count > 0) {
			name = Resolve_String(m->name_id);
#if COMBAT_DEBUG
			Log("Start_Combat: adding %dx%s (#%04x)", count, name, en->monsters[group]);
#endif
			write += sprintf(write, " %s x%d", name, count);

			while (count > 0) {
				Add_Monster(groups_alive, en->monsters[group]);
				count--;
			}

			groups_alive++;
		}
	}

	/* Briefly show encounter on Dungeon screen */
	Show_Picture(first_img);
	Show_Game_String(description, true);
	Free(description);
#if COMBAT_RECLAIM_TEXTURES
	Free_Textures();
#endif

	/* Set up Combat screen */
	redraw_everything = true;
	Fill_Double_Buffer(0);
	if (combat_bg)
		Draw_GRF(&gTopLeft, combat_bg, 0, 0);
	Show_Picture(first_img);

	/* DO IT */
	earned_experience = 0;
	gState = gsCombat;
	Enter_Combat_Loop();
	
	/* TODO: if you lose... shouldn't do this */
	for (count = 0; count < PARTY_SIZE; count++) {
		c = List_At(combatants, count);
		if (!Is_Dead(c)) {
			Add_Experience(Get_PC(count), earned_experience);
		}
	}

	gState = gsDungeon;
	Expire_Combat_Buffs();
	Clear_Encounter();

#if COMBAT_RECLAIM_TEXTURES
	Load_Textures(&gZone);
#endif
	Redraw_Dungeon_Screen(false);
}
