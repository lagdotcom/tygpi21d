#ifndef DOSJUN_COMMON_H
#define DOSJUN_COMMON_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define noexport static

#define true 1
#define false 0
#ifndef bool
typedef unsigned char bool;
#endif

#ifndef null
#define null 0
#endif

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

typedef unsigned char bytecode;
typedef unsigned char char_flag;
typedef unsigned char colour;
typedef unsigned char coord;
typedef unsigned char direction;
typedef unsigned int encounter_id;
typedef unsigned int etable_id;
typedef unsigned char inventory_flag;
typedef unsigned char internal_id;
typedef unsigned int item_id;
typedef unsigned int item_flag;
typedef unsigned char item_type;
typedef unsigned char job_storage;
typedef unsigned int length;
typedef unsigned char monster_ai;
typedef unsigned int monster_id;
typedef unsigned char monster_row;
typedef unsigned int script_id;
typedef unsigned char special_id;
typedef int stat;
typedef unsigned int string_id;
typedef unsigned char thing_id;
typedef unsigned int tile_flag;
typedef unsigned char wall_type;
typedef unsigned char zone_id;

/* Memory Debugging */

#ifdef MEMORY_DEBUG

	#ifdef FAR_MEMORY
		
		#define PtrDist far

	#else

		#define PtrDist

	#endif

	char PtrDist *Duplicate_String(const char *src, char *tag);
	void PtrDist *Allocate(size_t count, size_t size, char *tag);
	void Free(void *mem);
	void PtrDist *Reallocate(void PtrDist *mem, size_t count, size_t size, char *tag);
	void Stop_Memory_Tracking(void);

#else /* no MEMORY_DEBUG */

	#define Duplicate_String(src, tag) strdup(src)
	#define Stop_Memory_Tracking()

	#ifdef FAR_MEMORY

		#define Allocate(count, size, tag) farcalloc(count, size)
		#define Free(mem) { farfree(mem); mem = null; }
		#define Reallocate(mem, count, size, tag) farrealloc(mem, (count) * (size))

	#else /* no FAR_MEMORY */

		#define Allocate(count, size, tag) calloc(count, size)
		#define Free(mem) { free(mem); mem = null; }
		#define Reallocate(mem, count, size, tag) realloc(mem, (count) * (size))

	#endif

#endif

#define SzAlloc(count, obj, tag) Allocate(count, sizeof(obj), tag)

#endif /* DOSJUN_COMMON_H */
