#ifndef DOSJUN_FP_H
#define DOSJUN_FP_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "zone.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Draw_FP(void);
void Delete_Picture(void);
void Free_Textures(void);
void Load_Textures(zone *z);
void Show_Picture(char *name);

#endif
