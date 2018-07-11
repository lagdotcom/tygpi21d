#ifndef DOSJUN_STATUS_H
#define DOSJUN_STATUS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "combat.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Poisoned_Expires(combatant *target, buff *b);

/* Check whether the source beats the target in a roll-off. outcome contains how much beaten by. */
bool Roll(combatant *source, statistic inflict, combatant *target, statistic defend, int *outcome);

void Stunned_Expires(combatant *target, buff *b);

/* Try to poison the target based on source's stat. Returns success value. */
bool Try_Poison(combatant *source, combatant *target, statistic stat, int potency);

/* Try to stun the target based on source's stat. Returns success value. */
bool Try_Stun(combatant *source, combatant *target, statistic check);

#endif
