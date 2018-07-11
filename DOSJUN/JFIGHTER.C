/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define CONCENTRATE_HITBONUS	4
#define CONCENTRATE_DMGBONUS	4

#define INSPIRE_HITBONUS		2
#define INSPIRE_DMGBONUS		1

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
		Combat_Message(source->file, 0, "@n loses @r concentration.");
		return;
	}

	if (Get_Combatant_Range(source, target) > Get_Weapon_Range(weapon))
		return;

	base += Get_Stat(source, sHitBonus) + CONCENTRATE_HITBONUS;
	base -= Get_Stat(target, sDodgeBonus);

	if (randint(1, 20) <= base) {
		Combat_Message(source->file, target->file, "@n strikes @N true.");

		Get_Weapon_Damage(source, weapon, &min, &max);
		roll = randint(min, max) - Get_Stat(target, sArmour) + CONCENTRATE_DMGBONUS;
		if (roll > 0) {
			Damage(target, source, roll);
		} else {
			Combat_Message(source->file, target->file, "The blow glances off.");
		}
	} else {
		Combat_Message(source->file, target->file, "@n narrowly misses @N.");
	}
}

void Concentrate(combatant *source, combatant *target)
{
	With_Both_Weapons(source, target, Concentrate_Inner);
}

bool Check_Cleave(combatant *source)
{
	if (!Has_Skill(source, skCleave))
		return false;

	if (Has_Buff(source, bfCleaveFatigue))
		return false;

	return true;
}

void Cleave(combatant *source, combatant *target)
{
	combatant *retarget;

	retarget = Get_Random_Target(target->group);
	if (retarget != null) {
		Combat_Message(source->file, retarget->file, "@n swings again!");
		Add_Buff(source, Make_Buff(bfCleaveFatigue, 1, 0, 0, "Cleave"));

		Attack(source, retarget);
	}
}

bool Check_Inspire(combatant *source)
{
	if (!Has_Skill(source, skInspire))
		return false;

	if (Has_Buff(source, bfInspiring))
		return false;

	return true;
}

void Inspiring_Expires(combatant *target, buff *b)
{
	combatant *c;
	int i;

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);

		if (c->is_pc == target->is_pc) {
			c->stats[sHitBonus] -= b->arg1;
			c->stats[sMaxDamage] -= b->arg2;
		}
	}

	if (gState == gsCombat) {
		Combat_Message(target->file, 0, "@n's inspiration fades.");
	}
}

void Inspire(combatant *source, combatant *target)
{
	combatant *c;
	int i;

	/* Uh... that's not very inspiring! */
	if (source->is_pc == target->is_pc) return;

	for (i = 0; i < combatants->size; i++) {
		c = List_At(combatants, i);

		if (c->is_pc == source->is_pc) {
			c->stats[sHitBonus] += INSPIRE_HITBONUS;
			c->stats[sMaxDamage] += INSPIRE_DMGBONUS;
		}
	}

	Combat_Message(source->file, 0, "@n inspires the party to victory!");
	Add_Buff(source, Make_Buff(bfInspiring, 2, INSPIRE_HITBONUS, INSPIRE_DMGBONUS, "Inspire"));
}
