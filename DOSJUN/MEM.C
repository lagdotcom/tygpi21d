#ifdef MEMORY_DEBUG

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

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

typedef struct {
	char *tag;
	void PtrDist *address;
	MemSz size;
	bool freed;
} entry;

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport unsigned long peak_use = 0;
noexport unsigned long current_use = 0;
noexport unsigned long start_free = 0;
noexport unsigned int entry_count = 0;
noexport unsigned int allocated_entries = 0;
noexport entry PtrDist *entries = null;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Update_Peak(long change)
{
	current_use += change;
	if (current_use > peak_use) peak_use = current_use;
}

noexport void Add_Entry(void *mem, MemSz size, char *tag)
{
	entry *old_entries = entries;
	if (entry_count == allocated_entries) {
		allocated_entries += 20;
		entries = _realloc(old_entries, sizeof(entry) * allocated_entries);
		if (!entries) {
			printf("MEM: Could not allocate another entry (already have %u).\n", allocated_entries - 20);
			entries = old_entries;
			Stop_Memory_Tracking();
			abort();
			return;
		}
	}

	entries[entry_count].tag = tag;
	entries[entry_count].address = mem;
	entries[entry_count].size = size;
	entries[entry_count].freed = false;

	Update_Peak(size);
	entry_count++;
}

noexport void Mark_Entry_Freed(void *mem)
{
	unsigned int i;
	for (i = 0; i < entry_count; i++) {
		if (entries[i].address == mem && entries[i].freed == false) {
			Update_Peak(-entries[i].size);
			entries[i].freed = true;
			return;
		}
	}

	fprintf(stderr, "WARNING: Cannot free %p - not found.\n", mem);
}

noexport void Update_Entry_Size(void *mem, MemSz size)
{
	unsigned int i;
	for (i = 0; i < entry_count; i++) {
		if (entries[i].address == mem && entries[i].freed == false) {
			Update_Peak(size - entries[i].size);
			entries[i].size = size;
			return;
		}
	}

	fprintf(stderr, "WARNING: Cannot update %p - not found.\n", mem);
}

/* M A I N /////////////////////////////////////////////////////////////// */

void PtrDist *Allocate(MemSz count, MemSz size, char *tag)
{
	void PtrDist *mem = _calloc(count, size);
	Add_Entry(mem, count * size, tag);
	return mem;
}

void PtrDist *Reallocate(void PtrDist *mem, MemSz count, MemSz size, char *tag)
{
	void PtrDist *nu = _realloc(mem, count * size);
	if (nu == mem) {
		Update_Entry_Size(mem, count * size);
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

void Free(void *mem)
{
	if (mem != null) {
		Mark_Entry_Freed(mem);
		_free(mem);

		mem = null;
	}
}

void Start_Memory_Tracking(void)
{
	start_free = coreleft();
}

void Stop_Memory_Tracking(void)
{
	FILE *fp;
	unsigned int i;
	unsigned long end_free;

	for (i = 0; i < entry_count; i++) {
		if (entries[i].freed == false) {
			printf("#%u [%s]: @%p, %lu bytes not freed\n", i, entries[i].tag, entries[i].address, entries[i].size);
			_free(entries[i].address);
		}
	}

	printf("Tracked %u memory entries overall.\n", entry_count);

	fp = fopen("memory.txt", "w");
	fprintf(fp, "Peak use: %lu bytes\n\n", peak_use);
	for (i = 0; i < entry_count; i++) {
		fprintf(fp, "#%u [%s]: @%p, %lu bytes%s\n", i, entries[i].tag, entries[i].address, entries[i].size, entries[i].freed ? "" : " NOT FREED");
	}

	_free(entries);
	end_free = coreleft();

	printf("coreleft: %lu => %lu\n", start_free, end_free);
	fprintf(fp, "\ncoreleft: %lu => %lu\n", start_free, end_free);

	fclose(fp);
}

#endif
