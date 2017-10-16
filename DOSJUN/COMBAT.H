#ifndef DOSJUN_COMBAT_H
#define DOSJUN_COMBAT_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef int act;
typedef int targ;
typedef int groupnum;

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
	aBlock,
	aDefend,

	aSing,

	NUM_ACTIONS,
	NO_ACTION = -1
} action_ids;

typedef enum expiry_type {
	exTurns,
	exSteps,
} expiry_type;

#define MONSTER(i)		((monster *)List_At(combat_monsters, i))

#define TARGET_PC(i)	(-(i) - 1)
#define TARGET_ENEMY(i)	(i)
#define IS_PC(i)		((i) < 0)
#define NAME(i)			(IS_PC(i) ? Get_Pc(TARGET_PC(i))->header.name : MONSTER(i)->name)

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	char *name;
	buff_expiry_fn expiry;
	expiry_type type;
	int duration;
	int argument;
} buff;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Add_Buff(targ target, char *name, expiry_type exty, int duration, buff_expiry_fn expiry, int argument);
void Combat_Message(char *format, ...);
void Free_Combat(void);
void Initialise_Combat(void);
int randint(int minimum, int maximum);
void Start_Combat(encounter_id id);

#endif
