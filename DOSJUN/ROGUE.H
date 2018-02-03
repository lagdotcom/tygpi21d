#ifndef DOSJUN_ROGUE_H
#define DOSJUN_ROGUE_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "combat.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define HIDE_BUFF_NAME	"Hidden"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Check_Hide(combatant *source);
bool Check_SneakAttack(combatant *source);
void Hide(combatant *source, combatant *target);
void SneakAttack(combatant *source, combatant *target);

#endif
