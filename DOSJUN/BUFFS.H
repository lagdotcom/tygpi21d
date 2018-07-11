#ifndef DOSJUN_BUFFS_H
#define DOSJUN_BUFFS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

struct buff;
struct combatant;

typedef void(*buff_expiry_fn)(struct combatant *target, struct buff *b);

typedef enum expiry_type {
	exTurns,
	exSteps,
	exTurnEndChance,
} expiry_type;

typedef enum buff_id {
	bfBlocking,
	bfSinging,
	bfCleaveFatigue,
	bfInspiring,
	bfHidden,
	bfStunned,
	bfLoseNextMove,
	bfPoisoned,
	bfSilenced,

	NUM_BUFFS,
} buff_id;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct buff {
	UINT16 id;
	INT16 duration;
	INT16 arg1;
	INT16 arg2;
} buff;

typedef struct buffspec {
	char *name;
	bool visible;
	expiry_type type;
	buff_expiry_fn expiry;
} buffspec;

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern buffspec *buffspecs;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Buffs(void);
void Initialise_Buffs(void);
buff *Make_Buff(buff_id id, int duration, int arg1, int arg2, char *tag);

#endif
