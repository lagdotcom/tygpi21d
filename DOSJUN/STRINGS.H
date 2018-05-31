#ifndef DOSJUN_STRINGS_H
#define DOSJUN_STRINGS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef UINT16 str_id;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct stringse {
	str_id id;
	char *string;
} stringse;

typedef struct strings {
	str_id count;

	stringse *strings;
} strings;
#define STRINGS_HEADER_SZ	sizeof(str_id)

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

void Free_Strings(strings *s);
char *Get_String(strings *s, str_id id);
bool Read_Strings(FILE *fp, strings *s);

#endif
