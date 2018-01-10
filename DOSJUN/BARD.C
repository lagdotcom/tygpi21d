/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SING_HITBONUS		1

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Check_Sing(targ source)
{
	/* TODO: use Silenced somewhere */
	if (Has_Buff(source, "Silenced")) {
		return false;
	}

	if (Has_Buff(source, SING_BUFF_NAME)) {
		return false;
	}

	return Has_Skill(Get_Combatant(source), skSing);
}

noexport void Sing_Expires(targ target, int argument)
{
	combatant *c;
	int i;
	bool is_pc = IS_PC(target);

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);

		if (c->is_pc == is_pc) {
			c->stats[sHitBonus] -= argument;
		}
	}

	if (gState == gsCombat) {
		c = Get_Combatant(target);
		Combat_Message("%s stops singing.", c->name);
	}
}

void Sing(targ source, targ target)
{
	combatant *c;
	int i;
	bool is_pc = IS_PC(target);

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);

		if (c->is_pc == is_pc) {
			c->stats[sHitBonus] += SING_HITBONUS;
		}
	}

	Add_Buff(source, SING_BUFF_NAME, exTurns, 1, Sing_Expires, SING_HITBONUS);
	Combat_Message("%s sings, and the party is inspired!", NAME(source));
}
