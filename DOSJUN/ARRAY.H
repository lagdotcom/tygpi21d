#ifndef DOSJUN_ARRAY_H
#define DOSJUN_ARRAY_H

/* TODO: implement a more sane strategy for inserting */

#define CAT(x,y) x ## y

#define Get_Array_Size(name) CAT(name, _count)

#define Declare_Array(type, name) type **name; int Get_Array_Size(name)

#define Free_Array(i, name) { \
	for (i = 0; i < Get_Array_Size(name); i++) { \
		Free(name[i]); \
	} \
	Get_Array_Size(name) = 0; \
	Free(name); \
}

#define Add_to_Array(name, obj) { \
	Get_Array_Size(name)++; \
	name = SzAlloc(Get_Array_Size(name), void *, "Add_to_Array"); \
	name[Get_Array_Size(name) - 1] = obj; \
}

#endif
