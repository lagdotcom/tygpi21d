#ifndef DOSJUN_STRINGS_H
#define DOSJUN_STRINGS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef int strid;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct stringse {
	strid id;
	char *string;
} stringse;

typedef struct strings {
	strid count;

	stringse *strings;
} strings;
#define STRINGS_HEADER_SZ	sizeof(strid)

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Strings(strings *s);
char *Get_String(strings *s, strid id);
bool Read_Strings(FILE *fp, strings *s);

#endif
