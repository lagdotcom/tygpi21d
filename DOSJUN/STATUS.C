/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "status.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define STUN_HIT_LOSS	2
#define STUN_DODGE_LOSS	2

#define POISON_TOUGHNESS_LOSS	4

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Roll(combatant *source, statistic inflict, combatant *target, statistic defense, int *outcome)
{
	int attacking = randint(1, 20) + Get_Stat(source, inflict);
	int defending = randint(1, 20) + Get_Stat(target, defense);

	*outcome = attacking - defending;
	return attacking >= defending;
}

noexport void Stun_Expires(combatant *target, int argument)
{
	target->stats[sHitBonus] += STUN_HIT_LOSS * argument;
	target->stats[sDodgeBonus] += STUN_DODGE_LOSS * argument;
	Combat_Message("%s shakes it off.", target->name);
}

bool Try_Stun(combatant *source, combatant *target, statistic stat)
{
	int outcome, duration;

	if (Roll(source, stat, target, sToughness, &outcome)) {
		Combat_Message("%s sees stars!", target->name);

		duration = outcome / 3;
		duration = duration < 2 ? 2 : duration;

		target->stats[sHitBonus] -= STUN_HIT_LOSS;
		target->stats[sDodgeBonus] -= STUN_DODGE_LOSS;
		Add_Buff(target, STUN_BUFF_NAME, exTurns, duration, Stun_Expires, 1);

		Remove_Buff(target, LOSE_MOVE_BUFF_NAME);
		Add_Buff(target, LOSE_MOVE_BUFF_NAME, exTurns, duration, null, 0);
		return true;
	}

	return false;
}

noexport void Poison_Expires(combatant *target, int argument)
{
	/* The Poison buff works strangely. It expires every turn, reducing in potency each time.
	When the potency reaches 0, it expires permanently. */
	int damage;

	damage = randint(argument, argument + 1);
	if (damage > 0) {
		Combat_Message("%s suffers from poison.", target->name);
		Damage(target, damage);
	}

	if (argument > 0) {
		Add_Buff(target, POISON_BUFF_NAME, exTurns, 1, Poison_Expires, argument - 1);
	} else {
		target->stats[sToughness] += POISON_TOUGHNESS_LOSS;
		Combat_Message("%s looks better.", target->name);
	}
}

bool Try_Poison(combatant *source, combatant *target, statistic stat, int potency)
{
	int outcome, duration;

	if (Roll(source, stat, target, sToughness, &outcome)) {
		Log("Try_Poison: success %d", outcome);
		Combat_Message("%s looks ill.", target->name);

		duration = potency + outcome / 3;
		duration = duration < 2 ? 2 : duration;

		target->stats[sToughness] -= POISON_TOUGHNESS_LOSS;
		Add_Buff(target, POISON_BUFF_NAME, exTurns, 1, Poison_Expires, duration);
		return true;
	}

	Log("Try_Poison: failure %d", outcome);
	return false;
}
