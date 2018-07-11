/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define SING_HITBONUS		1
#define SING_PLUS_HITBONUS	4

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Check_Sing(combatant *source)
{
	/* TODO: use Silenced somewhere */
	if (Has_Buff(source, bfSilenced)) {
		return false;
	}

	if (Has_Buff(source, bfSinging)) {
		return false;
	}

	return Has_Skill(source, skSing);
}

void Singing_Expires(combatant *target, buff *b)
{
	combatant *c;
	int i;
	bool is_pc = target->is_pc;

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);

		if (c->is_pc == is_pc) {
			c->stats[sHitBonus] -= b->arg1;
		}
	}

	if (gState == gsCombat) {
		if (b->arg1 == SING_HITBONUS) {
			Combat_Message(target->file, 0, "@n stops singing.");
		} else {
			Combat_Message(target->file, 0, "@n's voice fades away.");
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

	Add_Buff(source, Make_Buff(bfSinging, duration, bonus, 0, "Sing"));
	Combat_Message(source->file, 0, "@n sings, and the party is inspired!");
}
