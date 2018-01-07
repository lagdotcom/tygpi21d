/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define HIDE_MULTIPLIER		1.5

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Check_Hide(targ source)
{
	if (Has_Buff(source, HIDE_BUFF_NAME))
		return false;

	return Has_Skill(Get_Combatant(source), skHide);
}

noexport void Hidden_Expires(targ source, int argument)
{
	combatant *c;

	if (gState == gsCombat) {
		c = Get_Combatant(source);
		Combat_Message("%s is revealed!", c->name);
	}
}

void Hide(targ source, targ target)
{
	int dexterity = Get_Stat(source, sDexterity);

	if (dexterity > 20) dexterity = 20;
	if (dexterity < 3) dexterity = 3;
	Add_Buff(source, HIDE_BUFF_NAME, exTurnEndChance, 101 - (dexterity * 5), Hidden_Expires, 0);
	Combat_Message("%s is hidden from view.", NAME(source));
}

bool Check_SneakAttack(targ source)
{
	return Has_Buff(source, HIDE_BUFF_NAME);
}

void SneakAttack(targ source, targ target)
{
	combatant *attacker = Get_Combatant(source);
	char *source_name = attacker->name,
		*target_name = NAME(target);
	item *weapon = Get_Weapon(source);
	stat_value base = Get_Stat(source, Get_Weapon_Stat(weapon));
	stat_value min, max;
	int roll;

	if (Is_Dead(target)) {
		Combat_Message("%s missed their chance.", source_name);
		return;
	}

	base += Get_Stat(source, sHitBonus);
	/* note: dodge bonus is ignored on enemy! */

	if (randint(1, 20) <= base) {
		Combat_Message("%s strikes %s from the shadows!", source_name, target_name);

		min = Get_Stat(source, sMinDamage);
		max = Get_Stat(source, sMaxDamage);

		if (weapon != null) {
			min += weapon->stats[sMinDamage];
			max += weapon->stats[sMaxDamage];
		}

		roll = randint(min, max) * HIDE_MULTIPLIER - Get_Stat(target, sArmour);
		if (roll > 0) {
			Damage(target, roll);

			if (Has_Skill(attacker, skBludgeon)) {
				/* TODO: stun */
			}

			if (Has_Skill(attacker, skVenom)) {
				/* TODO: poison */
			}
		} else {
			Combat_Message("The blow glances off.");
		}
	} else {
		Combat_Message("%s sneakily attacks %s, but misses.", source_name, target_name);
	}

	/* TODO: should lose hidden sometimes... */
}
