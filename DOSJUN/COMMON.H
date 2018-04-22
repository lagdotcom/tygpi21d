#ifndef DOSJUN_COMMON_H
#define DOSJUN_COMMON_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include "types.h"
#include "logging.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef enum dir {
	dNorth,
	dEast,
	dSouth,
	dWest,
	dUp,
	dDown
} dir;

typedef enum relative {
	rAhead,
	rLeft,
	rRight
} relative;

typedef enum gamestate {
	gsMainMenu,
	gsDungeon,
	gsQuit,
	gsCombat
} gamestate;

typedef enum job {
	jFighter,	/* Str */
	jCleric,	/* Str, Int */
	jMage,		/* Int */
	jRogue,		/* Dex */
	jRanger,	/* Str, Dex */
	jBard,		/* Int, Dex */

	NUM_JOBS
} job;

typedef UINT8 ai_id;
typedef UINT8 bytecode;
typedef UINT8 charflag_id;
typedef UINT8 colour;
typedef UINT8 coord;
typedef UINT8 dir_id;
typedef UINT16 encounter_id;
typedef UINT16 etable_id;
typedef UINT8 inventoryflag_id;
typedef UINT8 internal_id;
typedef UINT16 item_id;
typedef UINT16 itemflag_id;
typedef UINT8 item_slot;
typedef UINT8 item_type;
typedef UINT8 job_id;
typedef UINT16 length;
typedef UINT16 monster_id;
typedef UINT8 monsterflag_id;
typedef UINT8 row_id;
typedef UINT16 script_id;
typedef UINT8 itemspecial_id;
typedef INT16 stat_value;
typedef UINT16 string_id;
typedef UINT8 thing_id;
typedef UINT16 tileflag_id;
typedef UINT8 walltype_id;
typedef UINT8 zone_id;

#define die(msg) { Stop_Memory_Tracking(); Log("die/%s", msg); printf("%s", msg); exit(1); }

/* Returns 0 if strings and b are equal */
#define streq(a, b) (strcmp(a, b) == 0)

/* Memory Debugging */

#ifdef FAR_MEMORY

    #define PtrDist far
    #define MemSz unsigned long

#else

    #define PtrDist
    #define MemSz size_t

#endif

#ifdef MEMORY_DEBUG

	char PtrDist *Duplicate_String(const char *src, char *tag);
	void PtrDist *Allocate(MemSz count, MemSz size, char *tag);
	void Free(void *mem);
	void PtrDist *Reallocate(void PtrDist *mem, MemSz count, MemSz size, char *tag);
	void Start_Memory_Tracking(void);
	void Stop_Memory_Tracking(void);

#else /* no MEMORY_DEBUG */

	#define Duplicate_String(src, tag) strdup(src)
	#define Start_Memory_Tracking()
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
