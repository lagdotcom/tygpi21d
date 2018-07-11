#ifndef DOSJUN_COMBAT_H
#define DOSJUN_COMBAT_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "monsters.h"
#include "items.h"
#include "save.h"
#include "buffs.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef int act;
typedef int groupnum;
typedef UINT8 pri;

typedef enum targetflags {
	tfAlly = 1,
	tfEnemy = 2,
	tfDead = 4,
	tfSelf = 8
} targetflags;

typedef enum range {
	rShort = 1,
	rMedium,
	rLong,
} range;

typedef enum action_ids {
	aAttack = 0,
	aSneakAttack,
	aBlock,
	aDefend,

	aConcentrate,

	aSing,
	aHide,

	NUM_ACTIONS,
	NO_ACTION = -1
} action_ids;

typedef struct combatant {
	char *name;
	stat_value *stats;
	pc *pc;
	monster *monster;
	list *buffs;
	list *skills;
	groupnum group;
	file_id primary, secondary;
	row_id row;
	bool is_pc;

	act action;
	void *target; /* combatant */
	pri priority;
	unsigned int index;
	file_id file;
} combatant;

typedef bool(*action_check_fn)(combatant *source);
typedef void(*action_do_fn)(combatant *source, combatant *target);
typedef void(*weapon_atk_fn)(combatant *source, combatant *target, item *weapon);

#define TARGET_PC(i) (i)

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern list *combatants;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Add_Buff(combatant *target, buff *b);
void Attack(combatant *source, combatant *target);
void Blocking_Expires(combatant *c, buff *b);
bool Check_Attack(combatant *source);
void Combat_Message(file_id speaker, file_id target, char *format, ...);
void Damage(combatant *victim, combatant* attacker, int amount);
void Free_Combat(void);
range Get_Attack_Range(combatant *c);
range Get_Combatant_Range(combatant *a, combatant *b);
combatant *Get_Random_Target(groupnum group);
stat_value Get_Stat_Base(stat_value *stats, statistic st);
stat_value Get_Stat(combatant *source, statistic st);
item *Get_Weapon(combatant *source, bool primary);
void Get_Weapon_Damage(combatant *source, item *weapon, stat_value *min, stat_value *max);
range Get_Weapon_Range(item *weapon);
statistic Get_Weapon_Stat(item *weapon);
bool Has_Buff(combatant *target, buff_id id);
bool Is_Dead(combatant *victim);
void Initialise_Combat(void);
int randint(int minimum, int maximum);
bool Remove_Buff(combatant *target, buff_id id);
bool Remove_Buff_from_PC(file_id ref, buff_id id);
void Start_Combat(encounter_id id);
void With_Both_Weapons(combatant *source, combatant *target, weapon_atk_fn func);

#endif
