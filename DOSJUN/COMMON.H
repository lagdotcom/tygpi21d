#ifndef DOSJUN_COMMON_H
#define DOSJUN_COMMON_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include <stdbool.h>

#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define noexport static

#define null NULL

#define North	0
#define East	1
#define South	2
#define West	3
#define Up		4
#define Down	5

typedef enum {
	rAhead,
	rLeft,
	rRight
} relative;

typedef enum {
	gsMainMenu,
	gsDungeon,
	gsQuit
} gamestate;

typedef enum {
	jFighter,	/* Str */
	jCleric,	/* Str, Int */
	jMage,		/* Int */
	jRogue,		/* Dex */
	jRanger,	/* Str, Dex */
	jBard		/* Int, Dex */
} job_id;
#define NUM_JOBS		6

typedef UINT8 bytecode;
typedef UINT8 char_flag;
typedef UINT8 colour;
typedef UINT8 coord;
typedef UINT8 direction;
typedef UINT16 encounter_id;
typedef UINT16 etable_id;
typedef UINT8 inventory_flag;
typedef UINT8 internal_id;
typedef UINT16 item_id;
typedef UINT16 item_flag;
typedef UINT8 item_type;
typedef UINT8 job_storage;
typedef UINT16 length;
typedef UINT8 monster_ai;
typedef UINT16 monster_id;
typedef UINT8 monster_row;
typedef UINT16 script_id;
typedef UINT8 special_id;
typedef INT16 stat;
typedef UINT16 string_id;
typedef UINT8 thing_id;
typedef UINT16 tile_flag;
typedef UINT8 wall_type;
typedef UINT8 zone_id;

/* Memory Debugging */

#ifdef MEMORY_DEBUG

	char *Duplicate_String(const char *src, char *tag);
	void *Allocate(size_t count, size_t size, char *tag);
	void Free(void *mem);
	void *Reallocate(void *mem, size_t count, size_t size, char *tag);
	void Stop_Memory_Tracking(void);

#else /* no MEMORY_DEBUG */

	#define Duplicate_String(src, tag) strdup(src)
	#define Stop_Memory_Tracking()

	#define Allocate(count, size, tag) calloc(count, size)
	#define Free(mem) { free(mem); mem = null; }
	#define Reallocate(mem, count, size, tag) realloc(mem, (count) * (size))

#endif

#define SzAlloc(count, obj, tag) Allocate(count, sizeof(obj), tag)

#endif /* DOSJUN_COMMON_H */
