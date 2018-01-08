#ifndef DOSJUN_FP_H
#define DOSJUN_FP_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "zone.h"
#include "gamelib.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Draw_FP(void);
void Delete_Picture(void);
void Free_Textures(void);
bool Load_Picture(char *filename, pcx_picture_ptr image, char *tag);
void Load_Textures(zone *z);
void Show_Picture(char *name);

#endif
