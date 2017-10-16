/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SING_HITBONUS		1

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Check_Sing(targ source)
{
	int pc;

	if (!IS_PC(source)) {
		/* TODO */
		return false;
	}

	pc = TARGET_PC(source);
	
	/* TODO: check for silence? */
	return Has_Skill(Get_Pc(pc), skSing);
}

noexport void Sing_Expires(targ target, int argument)
{
	character *c;
	int pc;

	if (!IS_PC(target)) {
		/* TODO */
		return;
	}

	for (pc = 0; pc < PARTY_SIZE; pc++) {
		c = Get_Pc(pc);
		c->header.stats[sHitBonus] -= argument;
	}

	if (gState == gsCombat) {
		c = Get_Pc(TARGET_PC(target));
		Combat_Message("%s stops singing.", c->header.name);
	}
}

void Sing(targ source, targ target)
{
	character *c;
	int pc;

	if (!IS_PC(source)) {
		/* TODO */
		return;
	}

	for (pc = 0; pc < PARTY_SIZE; pc++) {
		c = Get_Pc(pc);
		c->header.stats[sHitBonus] += SING_HITBONUS;
	}

	Add_Buff(source, "Singing", exTurns, 1, Sing_Expires, SING_HITBONUS);
	Combat_Message("%s sings, and the party is inspired!", Get_Pc(TARGET_PC(source))->header.name);
}
