#ifndef DOSJUN_FIGHTER_H
#define DOSJUN_FIGHTER_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "combat.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Check_Cleave(combatant *source);
bool Check_Concentrate(combatant *source);
bool Check_Inspire(combatant *source);
void Cleave(combatant *source, combatant *target);
void Concentrate(combatant *source, combatant *target);
void Inspire(combatant *source, combatant *target);

#endif
