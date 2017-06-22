#ifdef MEMORY_DEBUG

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	char *tag;
	void *address;
	size_t size;
	bool freed;
} entry;

/* G L O B A L S ///////////////////////////////////////////////////////// */

unsigned int entry_count = 0;
unsigned int allocated_entries = 0;
entry *entries = null;

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Add_Entry(void *mem, size_t size, char *tag)
{
	entry *old_entries = entries;
	if (entry_count == allocated_entries) {
		allocated_entries += 20;
		entries = realloc(old_entries, sizeof(entry) * allocated_entries);
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

	entry_count++;
}

noexport void Mark_Entry_Freed(void *mem)
{
	unsigned int i;
	for (i = 0; i < entry_count; i++) {
		if (entries[i].address == mem && entries[i].freed == false) {
			entries[i].freed = true;
			return;
		}
	}

	fprintf(stderr, "WARNING: Cannot free %p - not found.\n", mem);
}

noexport void Update_Entry_Size(void *mem, size_t size)
{
	unsigned int i;
	for (i = 0; i < entry_count; i++) {
		if (entries[i].address == mem && entries[i].freed == false) {
			entries[i].size = size;
			return;
		}
	}

	fprintf(stderr, "WARNING: Cannot update %p - not found.\n", mem);
}

/* M A I N /////////////////////////////////////////////////////////////// */

void *Allocate(size_t count, size_t size, char *tag)
{
	void *mem = calloc(count, size);
	Add_Entry(mem, count * size, tag);
	return mem;
}

void *Reallocate(void *mem, size_t count, size_t size, char *tag)
{
	void *nu = realloc(mem, count * size);
	if (nu == mem) {
		Update_Entry_Size(mem, count * size);
	} else {
		if (mem != null) Mark_Entry_Freed(mem);
		Add_Entry(nu, count * size, tag);
	}

	return nu;
}

char *Duplicate_String(const char *src, char *tag)
{
	void *mem = Allocate(strlen(src) + 1, sizeof(char), tag);
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
		free(mem);

		mem = null;
	}
}

void Stop_Memory_Tracking(void)
{
	unsigned int i;
	for (i = 0; i < entry_count; i++) {
		if (entries[i].freed == false) {
			printf("#%u [%s]: @%p, %u bytes not freed\n", i, entries[i].tag, entries[i].address, entries[i].size);
			free(entries[i].address);
		}
	}

	printf("Tracked %u memory entries overall.\n", entry_count);

	free(entries);
}

#endif
