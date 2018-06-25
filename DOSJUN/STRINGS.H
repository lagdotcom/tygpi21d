#ifndef DOSJUN_STRINGS_H
#define DOSJUN_STRINGS_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "grf.h"
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

void Free_Formatter(void);
void Free_Strings(strings *s);
char *Get_String(strings *s, str_id id);
void Initialise_Formatter(void);
bool Read_Strings(FILE *fp, strings *s);
point2d Show_Formatted_String(const char *s, file_id speaker, file_id target, const box2d *bounds, grf *font, colour start_tint);

#endif
