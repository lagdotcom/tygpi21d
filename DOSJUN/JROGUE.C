/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define HIDE_MULTIPLIER		1.5

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Check_Hide(combatant *source)
{
	if (Has_Buff(source, HIDE_BUFF_NAME))
		return false;

	return Has_Skill(source, skHide);
}

noexport void Hidden_Expires(combatant *source, int argument)
{
	if (gState == gsCombat) {
		Combat_Message(source->file, 0, "@n is revealed!");
	}
}

void Hide(combatant *source, combatant *target)
{
	int dexterity = Get_Stat(source, sDexterity);

	if (dexterity > 20) dexterity = 20;
	if (dexterity < 3) dexterity = 3;
	Add_Buff(target, HIDE_BUFF_NAME, exTurnEndChance, 101 - (dexterity * 5), Hidden_Expires, 0);
	Combat_Message(target->file, source->file, "@n is hidden from view.");
}

bool Check_SneakAttack(combatant *source)
{
	return Has_Buff(source, HIDE_BUFF_NAME);
}

noexport void SneakAttack_Inner(combatant *source, combatant *target, item *weapon)
{
	stat_value base = Get_Stat(source, Get_Weapon_Stat(weapon));
	stat_value min, max;
	int roll, potency;

	if (Is_Dead(target)) {
		Combat_Message(source->file, target->file, "@n missed @r chance.");
		return;
	}

	base += Get_Stat(source, sHitBonus);
	/* note: dodge bonus is ignored on enemy! */

	if (randint(1, 20) <= base) {
		Combat_Message(source->file, target->file, "@n strikes @N from the shadows!");

		Get_Weapon_Damage(source, weapon, &min, &max);
		roll = randint(min, max) * HIDE_MULTIPLIER - Get_Stat(target, sArmour);
		if (roll > 0) {
			Damage(target, source, roll);

			if (Is_Dead(target)) {
				return;
			}

			if (Has_Skill(source, skBludgeon)) {
				Try_Stun(source, target, sStrength);
			}

			if (Has_Skill(source, skVenom)) {
				potency = source->stats[sIntelligence] / 3;
				potency = potency < 1 ? 1 : potency;
				Try_Poison(source, target, sDexterity, potency);
			}
		} else {
			Combat_Message(source->file, target->file, "The blow glances off.");
		}
	} else {
		Combat_Message(source->file, target->file, "@n sneakily attacks @N, but misses.");
	}
}

void SneakAttack(combatant *source, combatant *target)
{
	With_Both_Weapons(source, target, SneakAttack_Inner);

	/* TODO: should lose hidden sometimes... */
}
