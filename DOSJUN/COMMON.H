#ifndef DOSJUN_COMMON_H
#define DOSJUN_COMMON_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define true 1
#define false 0
#ifndef bool
typedef unsigned char bool;
#endif

#ifndef null
#define null 0
#endif

#define szalloc(n, o) calloc(n, sizeof(o))

#define nullfree(r) if (r != null) { \
	free(r); \
	r = null; \
}

#define North	0
#define East	1
#define South	2
#define West	3

typedef enum {
	rAhead,
	rLeft,
	rRight
} relative;

typedef unsigned char bytecode;
typedef unsigned char colour;
typedef unsigned char coord;
typedef unsigned char direction;
typedef unsigned short encounter_id;
typedef unsigned short item_id;
typedef unsigned short item_flags;
typedef unsigned char item_type;
typedef unsigned char job_id;
typedef unsigned short length;
typedef unsigned short monster_id;
typedef unsigned short script_id;
typedef unsigned short string_id;
typedef unsigned char zone_id;

#endif
