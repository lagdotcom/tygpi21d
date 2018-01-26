/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "status.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define STUN_HIT_LOSS	2
#define STUN_DODGE_LOSS	2

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Roll(targ source, statistic inflict, targ target, statistic defense, int *outcome)
{
	int attacking = randint(1, 20) + Get_Stat(source, inflict);
	int defending = randint(1, 20) + Get_Stat(target, defense);

	*outcome = attacking - defending;
	return attacking >= defending;
}

noexport void Stun_Expires(targ target, int argument)
{
	combatant *c = Get_Combatant(target);

	c->stats[sHitBonus] += STUN_HIT_LOSS * argument;
	c->stats[sDodgeBonus] += STUN_DODGE_LOSS * argument;
	Combat_Message("%s shakes it off.", c->name);
}

bool Try_Stun(targ source, targ target, statistic stat)
{
	combatant *c = Get_Combatant(target);
	int outcome, duration;

	if (Roll(source, stat, target, sToughness, &outcome)) {
		Combat_Message("%s sees stars!", NAME(target));

		duration = outcome / 3;
		duration = duration < 2 ? 2 : duration;

		c->stats[sHitBonus] -= STUN_HIT_LOSS;
		c->stats[sDodgeBonus] -= STUN_DODGE_LOSS;
		Add_Buff(target, STUN_BUFF_NAME, exTurns, duration, Stun_Expires, 1);

		Remove_Buff(target, LOSE_MOVE_BUFF_NAME);
		Add_Buff(target, LOSE_MOVE_BUFF_NAME, exTurns, duration, null, 0);
		return true;
	}

	return false;
}
