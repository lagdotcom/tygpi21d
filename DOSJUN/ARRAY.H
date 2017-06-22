#ifndef DOSJUN_ARRAY_H
#define DOSJUN_ARRAY_H

/* TODO: implement a more sane strategy for inserting */

#define CAT(x,y) x ## y
#define SZ(name) CAT(name, _count)

#define Declare_Array(type, name) type **name; int SZ(name)

#define Free_Array(i, name) { \
	for (i = 0; i < SZ(name); i++) { \
		Free(name[i]); \
	} \
	SZ(name) = 0; \
	Free(name); \
}

#define Add_to_Array(name, obj) { \
	SZ(name)++; \
	name = Reallocate(name, SZ(name), sizeof(void *), "Add_to_Array"); \
	name[SZ(name) - 1] = obj; \
}

#define Null_Array(name) name = null; SZ(name) = 0;

#endif
