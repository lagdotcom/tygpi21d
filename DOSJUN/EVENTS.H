#ifndef DOSJUN_EVENTS_H
#define DOSJUN_EVENTS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "types.h"
#include "combat.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef enum event_id {
	evINVALID,

	evPartyMoved = 10,
	evPartyTurned,
	evZoneEntered,
	evZoneExited,
	evItemDropped,
	evItemTaken,

	evCombatantDamaged = 100,
	evCombatEntered,
	evCombatExited,
	evCombatTurnEnded,
	evMonsterDied,

	evPCDied = 200,
	evPCLevelGained,
} event_id;

typedef enum event_expiry {
	eeINVALID,
	eeNever,
	eeZone,
	eeCombat,
} event_expiry;

typedef struct event_data {
	combatant* attacker;
	combatant* target;
	pc* pc;
	item* item;
} event_data;

typedef void(*event_handler_fn)(event_data *data);

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Add_Listener(event_id ev, event_expiry ee, event_handler_fn h);
void Add_Script_Listener(event_id ev, event_expiry ee, file_id s);
void Expire_Listeners(event_expiry ee);
void Fire_Event(event_id ev, event_data *data);
void Fire_Combat_Event(event_id ev, combatant *attacker, combatant *target);
void Fire_Item_Event(event_id ev, pc *pc, item *it);
void Fire_PC_Event(event_id ev, pc *pc);
void Free_Events(void);
void Initialise_Events(void);

#endif
