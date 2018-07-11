/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

buffspec *buffspecs;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

buff *Make_Buff(buff_id id, int duration, int arg1, int arg2, char *tag)
{
	buff *b = SzAlloc(1, buff, tag);

	b->id = id;
	b->duration = duration;
	b->arg1 = arg1;
	b->arg2 = arg2;

	return b;
}

noexport void Declare_Buff(buff_id id, char *name, bool visible, expiry_type type, buff_expiry_fn expiry)
{
	buffspecs[id].name = name;
	buffspecs[id].type = type;
	buffspecs[id].expiry = expiry;
	buffspecs[id].visible = visible;
}

void Initialise_Buffs(void)
{
	buffspecs = SzAlloc(NUM_BUFFS, buffspec, "Initialise_Buffs");

	Declare_Buff(bfBlocking, "Blocking", true, exTurns, Blocking_Expires);
	Declare_Buff(bfSinging, "Singing", true, exTurns, Singing_Expires);
	Declare_Buff(bfCleaveFatigue, "Cleave Fatigue", false, exTurns, null);
	Declare_Buff(bfInspiring, "Inspiring", true, exTurns, Inspiring_Expires);
	Declare_Buff(bfHidden, "Hidden", true, exTurnEndChance, Hidden_Expires);
	Declare_Buff(bfStunned, "Stunned", true, exTurns, Stunned_Expires);
	Declare_Buff(bfLoseNextMove, "Lose Next Move", false, exTurns, null);
	Declare_Buff(bfPoisoned, "Poisoned", true, exTurns, Poisoned_Expires);
	Declare_Buff(bfSilenced, "Silenced", true, exTurns, null);
}

void Free_Buffs(void)
{
	Free(buffspecs);
}
