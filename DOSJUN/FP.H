#ifndef DOSJUN_FP_H
#define DOSJUN_FP_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"
#include "zone.h"
#include "gamelib.h"

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern int current_fp_effect;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Continue_Effect(void);
void Draw_FP(void);
void Free_FP(void);
void Initialise_FP(void);
void Show_Picture(file_id ref);

#endif
