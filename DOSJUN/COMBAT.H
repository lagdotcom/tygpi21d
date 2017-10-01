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

typedef enum {
	tfAlly = 1,
	tfEnemy = 2,
	tfDead = 4,
	tfSelf = 8
} targetflags;

typedef enum {
	aAttack = 0,
	aBlock,
	aDefend,

	NUM_ACTIONS,
	NO_ACTION = -1
} action_ids;

#define MONSTER(i)		((monster *)List_At(combat_monsters, i))

#define TARGET_PC(i)	(-(i) - 1)
#define TARGET_ENEMY(i)	(i)
#define IS_PC(i)		(i < 0)
#define NAME(i)			(IS_PC(i) ? gSave.characters[TARGET_PC(i)].name : MONSTER(i)->name)

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Combat(void);
void Initialise_Combat(void);
int randint(int minimum, int maximum);
void Start_Combat(encounter_id id);

#endif
