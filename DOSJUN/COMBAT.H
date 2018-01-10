#ifndef DOSJUN_COMBAT_H
#define DOSJUN_COMBAT_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "monsters.h"
#include "items.h"
#include "save.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef int act;
typedef int targ;
typedef int groupnum;
typedef UINT8 pri;

typedef bool(*action_check_fn)(targ source);
typedef void(*action_do_fn)(targ source, targ target);
typedef void(*buff_expiry_fn)(targ target, int argument);

typedef enum targetflags {
	tfAlly = 1,
	tfEnemy = 2,
	tfDead = 4,
	tfSelf = 8
} targetflags;

typedef enum action_ids {
	aAttack = 0,
	aSneakAttack,
	aBlock,
	aDefend,

	aSing,
	aHide,

	NUM_ACTIONS,
	NO_ACTION = -1
} action_ids;

typedef enum expiry_type {
	exTurns,
	exSteps,
	exTurnEndChance,
} expiry_type;

#define IS_PC(n) ((n) < PARTY_SIZE)
#define NAME(n) (Get_Combatant(n)->name)
#define TARGET_PC(n) (targ)(n)

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct buff {
	char *name;
	buff_expiry_fn expiry;
	expiry_type type;
	int duration;
	int argument;
} buff;

typedef struct combatant {
	char *name;
	stat_value *stats;
	character *pc;
	monster *monster;
	list *buffs;
	list *skills;
	groupnum group;
	item_id weapon;
	row_id row;
	bool is_pc;

	targ self;
	act action;
	targ target;
	pri priority;
} combatant;

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern list *combatants;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Add_Buff(targ target, char *name, expiry_type exty, int duration, buff_expiry_fn expiry, int argument);
void Combat_Message(char *format, ...);
void Damage(targ victim, int amount);
void Free_Combat(void);
combatant *Get_Combatant(targ target);
stat_value Get_Stat(targ source, statistic st);
item *Get_Weapon(targ source);
statistic Get_Weapon_Stat(item *weapon);
bool Has_Buff(targ target, char *name);
bool Is_Dead(targ victim);
void Initialise_Combat(void);
int randint(int minimum, int maximum);
void Remove_Buff(targ target, char *name);
void Start_Combat(encounter_id id);

#endif
