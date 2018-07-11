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

void Stunned_Expires(combatant *target, buff *b)
{
	target->stats[sHitBonus] += STUN_HIT_LOSS * b->arg1;
	target->stats[sDodgeBonus] += STUN_DODGE_LOSS * b->arg2;
	Combat_Message(target->file, 0, "@n shakes it off.");
}

bool Try_Stun(combatant *source, combatant *target, statistic stat)
{
	int outcome, duration;

	if (Roll(source, stat, target, sToughness, &outcome)) {
		Combat_Message(source->file, target->file, "@N sees stars!");

		duration = outcome / 3;
		duration = duration < 2 ? 2 : duration;

		target->stats[sHitBonus] -= STUN_HIT_LOSS * 1;
		target->stats[sDodgeBonus] -= STUN_DODGE_LOSS * 1;
		Add_Buff(target, Make_Buff(bfStunned, duration, 1, 1, "Try_Stun.stun"));

		Remove_Buff(target, bfLoseNextMove);
		Add_Buff(target, Make_Buff(bfLoseNextMove, duration, 0, 0, "Try_Stun.lnm"));
		return true;
	}

	return false;
}

void Poisoned_Expires(combatant *target, buff *b)
{
	/* The Poison buff works strangely. It expires every turn, reducing in potency each time.
	When the potency reaches 0, it expires permanently. */
	int damage;
	int argument = b->arg1;

	damage = randint(argument, argument + 1);
	if (damage > 0) {
		Combat_Message(target->file, 0, "@n suffers from poison.");
		Damage(target, null, damage);
	}

	if (argument > 0) {
		Add_Buff(target, Make_Buff(bfPoisoned, 1, argument - 1, 0, "Poison_Expires"));
	} else {
		target->stats[sToughness] += POISON_TOUGHNESS_LOSS;
		Combat_Message(target->file, 0, "@n looks better.");
	}
}

bool Try_Poison(combatant *source, combatant *target, statistic stat, int potency)
{
	int outcome, duration;

	if (Roll(source, stat, target, sToughness, &outcome)) {
		Log("Try_Poison: success %d", outcome);
		Combat_Message(source->file, target->file, "@N looks ill.");

		duration = potency + outcome / 3;
		duration = duration < 2 ? 2 : duration;

		target->stats[sToughness] -= POISON_TOUGHNESS_LOSS;
		Add_Buff(target, Make_Buff(bfPoisoned, 1, duration, 0, "Try_Poison"));
		return true;
	}

	Log("Try_Poison: failure %d", outcome);
	return false;
}
