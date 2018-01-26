#ifndef DOSJUN_STATUS_H
#define DOSJUN_STATUS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "combat.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define STUN_BUFF_NAME "Stunned"
#define LOSE_MOVE_BUFF_NAME "Lose Next Move"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

/* Check whether the source beats the target in a roll-off. outcome contains how much beaten by. */
bool Roll(targ source, statistic inflict, targ target, statistic defend, int *outcome);

/* Try to stun the target based on source's stat. Returns success value. */
bool Try_Stun(targ source, targ target, statistic check);

#endif
