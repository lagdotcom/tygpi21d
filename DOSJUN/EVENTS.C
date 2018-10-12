/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct listener {
	event_id ev;
	event_expiry ee;
	event_handler_fn handler;
	file_id script;
} listener;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport listener *listeners;
noexport int count;
noexport int capacity;
noexport int max;
noexport int next;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport char *Event_Name(event_id ev)
{
	switch (ev) {
		case evCombatantDamaged: return "CombatantDamaged";
		case evCombatEntered: return "CombatEntered";
		case evCombatExited: return "CombatExited";
		case evCombatTurnEnded: return "CombatTurnEnded";
		case evItemDropped: return "ItemDropped";
		case evItemTaken: return "ItemTaken";
		case evMonsterDied: return "MonsterDied";
		case evPartyMoved: return "PartyMoved";
		case evPartyTurned: return "PartyTurned";
		case evPCDied: return "PCDied";
		case evPCLevelGained: return "PCLevelGained";
		case evZoneEntered: return "ZoneEntered";
		case evZoneExited: return "ZoneExited";
		default: return "?";
	}
}

noexport char *Event_Expiry(event_expiry ee)
{
	switch (ee) {
		case eeNever: return "Never";
		case eeZone: return "Zone";
		case eeCombat: return "Combat";
		default: return "?";
	}
}

noexport void New_Listener(event_id ev, event_expiry ee, event_handler_fn h, file_id s)
{
	listener *temp;
	listener *l; 
	
	if (count == capacity) {
		capacity += EVENTS_GROW_SIZE;
		temp = Reallocate(listeners, capacity, sizeof(listener), "New_Listener");

		if (temp) {
			listeners = temp;
		} else {
			die("New_Listener: out of memory");
			return;
		}
	}

	Log("New_Listener: %s exp=%s han=%p scr=%d", Event_Name(ev), Event_Expiry(ee), h, s);

	l = &listeners[next];
	l->ev = ev;
	l->ee = ee;
	l->handler = h;
	l->script = s;

	count++;
	if (count > max) max = count;

	while (l->ev) {
		next++;
		if (next >= capacity) next = 0;

		l = &listeners[next];
	}
}

void Add_Listener(event_id ev, event_expiry ee, event_handler_fn h)
{
	New_Listener(ev, ee, h, 0);
}

void Add_Script_Listener(event_id ev, event_expiry ee, file_id s)
{
	New_Listener(ev, ee, null, s);
}

void Expire_Listeners(event_expiry ee)
{
	int i;
	listener *l;

	Log("Expire_Listeners: %s", Event_Expiry(ee));

	for (i = 0, l = listeners; i < capacity; i++, l++) {
		if (l->ee == ee) {
			l->ev = evINVALID;
			l->ee = eeINVALID;
			l->handler = null;
			l->script = 0;

			count--;
		}
	}
}

void Fire_Event(event_id ev, event_data *data)
{
	int i;
	listener *l;
	int handlers = 0,
		scripts = 0;

	for (i = 0, l = listeners; i < capacity; i++, l++) {
		if (l->ev == ev) {
			if (l->script) {
				Run_Event_Code(l->script, data);
				scripts++;
			} else {
				l->handler(data);
				handlers++;
			}
		}
	}

	Log("Fire_Event: %s scr=%d han=%d", Event_Name(ev), scripts, handlers);
}

void Free_Events(void)
{
	Log("Free_Events: cap=%d cnt=%d max=%d nex=%d", capacity, count, max, next);

	Free(listeners);
}

void Initialise_Events(void)
{
	capacity = EVENTS_INITIAL_SIZE;
	count = 0;
	max = 0;
	next = 0;

	listeners = SzAlloc(capacity, listener, "Initialise_Events");
}

/* C O N V E N I E N C E  F U N C T I O N S ////////////////////////////// */

void Fire_Combat_Event(event_id ev, file_id attacker, file_id target)
{
	event_data *data = SzAlloc(1, event_data, "Fire_Combat_Event");

	data->attacker = attacker;
	data->target = target;
	Fire_Event(ev, data);

	Free(data);
}

void Fire_Item_Event(event_id ev, file_id pc, file_id it)
{
	event_data *data = SzAlloc(1, event_data, "Fire_Item_Event");

	data->pc = pc;
	data->item = it;
	Fire_Event(ev, data);

	Free(data);
}

void Fire_PC_Event(event_id ev, file_id pc)
{
	event_data *data = SzAlloc(1, event_data, "Fire_PC_Event");

	data->pc = pc;
	Fire_Event(ev, data);

	Free(data);
}
