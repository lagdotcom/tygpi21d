/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define CONCENTRATE_HITBONUS	4
#define CONCENTRATE_DMGBONUS	4

/* F U N C T I O N S ///////////////////////////////////////////////////// */

bool Check_Concentrate(combatant *source)
{
	if (!Has_Skill(source, skConcentrate))
		return false;

	return Check_Attack(source);
}

noexport void Concentrate_Inner(combatant *source, combatant *target, item *weapon)
{
	stat_value base = Get_Stat(source, Get_Weapon_Stat(weapon));
	stat_value min, max;
	int roll;

	if (Is_Dead(target)) {
		Combat_Message("%s loses their concentration.", source->name);
		return;
	}

	base += Get_Stat(source, sHitBonus) + CONCENTRATE_HITBONUS;
	base -= Get_Stat(target, sDodgeBonus);

	if (randint(1, 20) <= base) {
		Combat_Message("%s strikes %s true.", source->name, target->name);

		Get_Weapon_Damage(source, weapon, &min, &max);
		roll = randint(min, max) - Get_Stat(target, sArmour) + CONCENTRATE_DMGBONUS;
		if (roll > 0) {
			Damage(target, roll);
		} else {
			Combat_Message("The blow glances off.");
		}
	} else {
		Combat_Message("%s narrowly misses %s.", source->name, target->name);
	}
}

void Concentrate(combatant *source, combatant *target)
{
	With_Both_Weapons(source, target, Concentrate_Inner);
}
