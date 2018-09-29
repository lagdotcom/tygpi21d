#ifndef DOSJUN_OPTIONS_H
#define DOSJUN_OPTIONS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define OPTIONS_PADDING		62

#define MAX_VOLUME			10

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct options {
	UINT8 music_vol;
	UINT8 sound_vol;

	char padding[OPTIONS_PADDING];
} options;

/* G L O B A L S ///////////////////////////////////////////////////////// */

extern options *gOptions;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Read_Options(FILE *fp, options *o);
void Write_Options(FILE *fp, options *o);

#endif
