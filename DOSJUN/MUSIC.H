#ifndef DOSJUN_MUSIC_H
#define DOSJUN_MUSIC_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Initialise_Music();
bool Play_Music(char *filename);
void Stop_Music();
void Free_Music();
void Process_Music();

#endif
