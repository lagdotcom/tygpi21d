#ifndef DOSJUN_STATUS_H
#define DOSJUN_STATUS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "combat.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define LOSE_MOVE_BUFF_NAME "Lose Next Move"
#define POISON_BUFF_NAME "Poisoned"
#define STUN_BUFF_NAME "Stunned"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* Check whether the source beats the target in a roll-off. outcome contains how much beaten by. */
bool Roll(combatant *source, statistic inflict, combatant *target, statistic defend, int *outcome);

/* Try to poison the target based on source's stat. Returns success value. */
bool Try_Poison(combatant *source, combatant *target, statistic stat, int potency);

/* Try to stun the target based on source's stat. Returns success value. */
bool Try_Stun(combatant *source, combatant *target, statistic check);

#endif
