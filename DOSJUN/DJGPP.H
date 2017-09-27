#ifndef DOSJUN_DJGPP_H
#define DOSJUN_DJGPP_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <sys/nearptr.h>
#include "common.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Enter_Nearptr_Mode(void);
char *Get_Nearptr(unsigned long offset);
#define Leave_Nearptr_Mode __djgpp_nearptr_disable

#endif
