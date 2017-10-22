#ifndef DOSJUN_ROGUE_H
#define DOSJUN_ROGUE_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "combat.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define HIDE_BUFF_NAME	"Hidden"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Check_Hide(targ source);
bool Check_SneakAttack(targ source);
void Hide(targ source, targ target);
void SneakAttack(targ source, targ target);

#endif
