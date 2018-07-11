#ifndef DOSJUN_ROGUE_H
#define DOSJUN_ROGUE_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "combat.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Check_Hide(combatant *source);
bool Check_SneakAttack(combatant *source);
void Hidden_Expires(combatant *source, buff *b);
void Hide(combatant *source, combatant *target);
void SneakAttack(combatant *source, combatant *target);

#endif
