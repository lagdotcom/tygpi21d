#ifndef DOSJUN_BARD_H
#define DOSJUN_BARD_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "combat.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Check_Sing(combatant *source);
void Sing(combatant *source, combatant *target);
void Singing_Expires(combatant *target, buff *b);

#endif
