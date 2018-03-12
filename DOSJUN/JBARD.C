/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SING_HITBONUS		1
#define SING_PLUS_HITBONUS	4

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Check_Sing(combatant *source)
{
	/* TODO: use Silenced somewhere */
	if (Has_Buff(source, "Silenced")) {
		return false;
	}

	if (Has_Buff(source, SING_BUFF_NAME)) {
		return false;
	}

	return Has_Skill(source, skSing);
}

noexport void Sing_Expires(combatant *target, int argument)
{
	combatant *c;
	int i;
	bool is_pc = target->is_pc;

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);

		if (c->is_pc == is_pc) {
			c->stats[sHitBonus] -= argument;
		}
	}

	if (gState == gsCombat) {
		if (argument == SING_HITBONUS) {
			Combat_Message("%s stops singing.", target->name);
		} else {
			Combat_Message("%s's voice fades away.", target->name);
		}
	}
}

void Sing(combatant *source, combatant *target)
{
	combatant *c;
	int i;
	bool is_pc = target->is_pc;
	stat_value bonus;
	int duration;

	if (Has_Skill(source, skReverberation)) {
		bonus = SING_PLUS_HITBONUS;
		duration = 4;	/* TODO */
	} else {
		bonus = SING_HITBONUS;
		duration = 1;
	}

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);

		if (c->is_pc == is_pc) {
			c->stats[sHitBonus] += bonus;
		}
	}

	Add_Buff(source, SING_BUFF_NAME, exTurns, duration, Sing_Expires, bonus);
	Combat_Message("%s sings, and the party is inspired!", source->name);
}
