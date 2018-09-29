#ifdef MEMORY_DEBUG

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define LOG_FILE "MEMORY.LOG"

#ifdef FAR_MEMORY

#include <alloc.h>

#define _calloc  farcalloc
#define _free    farfree
#define _realloc farrealloc

#else

#define _calloc  calloc
#define _free    free
#define _realloc realloc

#endif

/* S T R U C T U R E S /////////////////////////////////////////////////// */

struct entry {
	UINT32 index;
	char *tag;
	void PtrDist *address;
	MemSz size;
	struct entry *prev, *next;
};

typedef struct entry entry;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport UINT32 peak_use = 0,
	current_use = 0,
	start_free = 0,
	allocated_entries = 0,
	entry_count = 0;
noexport entry PtrDist *first = null, *last = null;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Clear_MLog(void)
{
	FILE *f = fopen(LOG_FILE, "w");
	if (f == NULL) {
		/* TODO */
		return;
	}

	fclose(f);
}

noexport void MLog(entry *e, char *op)
{
	FILE *f = fopen(LOG_FILE, "a");
	if (f == NULL) {
		/* TODO */
		return;
	}

#ifdef FAR_MEMORY
	fprintf(f, "%09lu.%s @%p+%-9lu | %s", e->index, op, e->address, e->size, e->tag);
#else
	fprintf(f, "%09lu.%s @%p+%-5u | %s", e->index, op, e->address, e->size, e->tag);
#endif

	fputc('\n', f);
	fclose(f);
}

noexport void Update_Peak(long change)
{
	current_use += change;
	if (current_use > peak_use) peak_use = current_use;
}

#define Connect_Nodes(p, n) { \
	if (p != null) \
		p->next = n; \
	if (n != null) \
		n->prev = p; \
}

noexport void Add_Entry(void *mem, MemSz size, char *tag)
{
	entry *e;

	if (mem == null) {
#ifdef FAR_MEMORY
		printf("MEM: Could not allocate %lu bytes for %s.\n", size, tag);
#else
		printf("MEM: Could not allocate %u bytes for %s.\n", size, tag);
#endif
		Stop_Memory_Tracking();
		abort();
		return;
	}

	e = _calloc(1, sizeof(entry));
	if (e == null) {
		printf("MEM: Could not allocate another entry (already have %lu).\n", allocated_entries);
		Stop_Memory_Tracking();
		abort();
		return;
	}

	allocated_entries++;
	e->index = entry_count;
	e->tag = tag;
	e->address = mem;
	e->size = size;

	MLog(e, "A");

	e->next = null;
	Connect_Nodes(last, e);
	last = e;

	Update_Peak(size);
	entry_count++;
}

noexport void Mark_Entry_Freed(void *mem)
{
	entry *e = first;

	while (e) {
		if (e->address == mem) {
			MLog(e, "F");

			Update_Peak(-e->size);

			Connect_Nodes(e->prev, e->next);
			if (last == e)
				last = e->prev;

			allocated_entries--;
			_free(e);
			return;
		}

		e = e->next;
	}

	Log("[WARN] Mark_Entry_Freed: %p not found", mem);
}

noexport void Update_Entry_Size(void *mem, MemSz size)
{
	entry *e = first;

	while (e) {
		if (e->address == mem) {
			Update_Peak(size - e->size);
			e->size = size;

			MLog(e, "U");
			return;
		}

		e = e->next;
	}

	Log("[WARN] Update_Entry_Size: %p not found", mem);
}

/* M A I N /////////////////////////////////////////////////////////////// */

void PtrDist *Allocate(MemSz count, MemSz size, char *tag)
{
	void PtrDist *mem;

	/* k, here's your zero memory pointer */
	if (count == 0 || size == 0)
		return null;
	
	mem = _calloc(count, size);
	Add_Entry(mem, count * size, tag);
	return mem;
}

void PtrDist *Reallocate(void PtrDist *mem, MemSz count, MemSz size, char *tag)
{
	void PtrDist *nu = _realloc(mem, count * size);

#ifdef FAR_MEMORY
	Log("Reallocate: %lu*%lu | %p -> %p", count, size, mem, nu);
#else
	Log("Reallocate: %u*%u | %p -> %p", count, size, mem, nu);
#endif

	if (nu == mem) {
		Update_Entry_Size(nu, count * size);
	} else {
		if (mem != null) Mark_Entry_Freed(mem);
		Add_Entry(nu, count * size, tag);
	}

	return nu;
}

char PtrDist *Duplicate_String(const char *src, char *tag)
{
	void PtrDist *mem = Allocate(strlen(src) + 1, sizeof(char), tag);
	if (!mem) {
		printf("MEM: Duplicate_String[%s] failed on: %s", tag, src);
		Stop_Memory_Tracking();
		abort();
		return null;
	}
	strcpy(mem, src);
	return mem;
}

void Free_Inner(void *mem)
{
	if (mem != null) {
		Mark_Entry_Freed(mem);
		_free(mem);
	}
}

void Start_Memory_Tracking(void)
{
	start_free = coreleft();

	first = _calloc(1, sizeof(entry));
	if (!first) {
		printf("%s", "MEM: could not allocate first entry!\n");
		abort();
		return;
	}

	allocated_entries++;
	entry_count++;
	last = first;
	first->tag = "Start_Memory_Tracking";

	Clear_MLog();
}

void Stop_Memory_Tracking(void)
{
	FILE *fp;
	entry *e, *temp;
	unsigned long end_free;

	e = first;
	while (e != null) {
		if (e != first) {
#ifdef FAR_MEMORY
			printf("#%lu [%s]: @%p, %lu bytes not freed\n", e->index, e->tag, e->address, e->size);
#else
			printf("#%lu [%s]: @%p, %u bytes not freed\n", e->index, e->tag, e->address, e->size);
#endif
			_free(e->address);
		}

		e = e->next;
	}

	printf("Tracked %u memory entries overall.\n", entry_count);

	fp = fopen("memory.txt", "w");
	if (fp) fprintf(fp, "Peak use: %lu bytes\n\n", peak_use);

	e = first;
	while (e != null) {
		if (e != first) {
#ifdef FAR_MEMORY
			if (fp) fprintf(fp, "#%lu [%s]: @%p, %lu bytes not freed\n", e->index, e->tag, e->address, e->size);
#else
			if (fp) fprintf(fp, "#%lu [%s]: @%p, %u bytes not freed\n", e->index, e->tag, e->address, e->size);
#endif
		}

		temp = e;
		e = e->next;
		_free(temp);
	}

	end_free = coreleft();

	printf("coreleft: %lu => %lu\n", start_free, end_free);
	if (fp) {
		fprintf(fp, "\ncoreleft: %lu => %lu\n", start_free, end_free);
		fclose(fp);
	}
}

#endif
