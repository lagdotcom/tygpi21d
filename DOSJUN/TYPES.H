#ifndef DOSJUN_TYPES_H
#define DOSJUN_TYPES_H

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef signed char INT8;
typedef unsigned char UINT8;

typedef signed int INT16;
typedef unsigned int UINT16;

typedef signed long INT32;
typedef unsigned long UINT32;

#define true 1
#define false 0
#ifndef bool
typedef unsigned char bool;
#endif

#define noexport static

#ifndef null
#define null 0
#endif

#endif
