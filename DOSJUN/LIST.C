/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "common.h"
#include "list.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define LIST_START_CAPACITY		10

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Resize_List(list *l, int new_capacity)
{
	void *new_storage = Reallocate(l->items, new_capacity, sizeof(void *), "Resize_List");
	if (new_storage == null) {
		printf("Resize_List failed: going from %d to %d\n", l->capacity, new_capacity);
		exit(1);
	}

	l->items = new_storage;
	l->capacity = new_capacity;
}

noexport bool Item_Matches(list_type type, void *a, void *b)
{
	switch (type) {
		case ltString:
			if (strcmp(a, b) == 0)
				return true;
			break;

		case ltInteger:
			return (int)a == (int)b;

		case ltObject:
			/* TODO: ? */
			return a == b;
	}

	return false;
}

noexport void Free_Item(list_type type, void *item)
{
	switch (type) {
		case ltString:
		case ltObject:
			Free(item);
			break;

		case ltInteger:
			return;
	}
}

list *New_List(list_type type, char *tag)
{
	list *l = SzAlloc(1, list, tag);

	l->type = type;
	l->capacity = 0;
	l->size = 0;
	l->items = null;

	Resize_List(l, LIST_START_CAPACITY);
	return l;
}

void Clear_List(list *l)
{
	unsigned int i;

	for (i = 0; i < l->size; i++) {
		Free_Item(l->type, l->items[i]);
	}

	l->size = 0;
}

void Free_List(list *l)
{
	Free(l->items);
	Free(l);
}

void Add_to_List(list *l, void *item)
{
	if (l->size == l->capacity) {
		Resize_List(l, l->capacity * 2);
	}

	l->items[l->size] = item;
	l->size++;
}

void Add_String_to_List(list *l, char *s)
{
	if (l->type != ltString) {
		printf("%s", "Add_String_to_List failed: wrong type of list\n");
		exit(1);
	}

	Add_to_List(l, Duplicate_String(s, "Add_String_to_List"));
}

bool In_List(list *l, void *match)
{
	unsigned int i;

	for (i = 0; i < l->size; i++) {
		void *item = l->items[i];

		if (Item_Matches(l->type, item, match))
			return true;
	}

	return false;
}

bool Remove_from_List(list *l, void *match)
{
	unsigned int i, j;

	for (i = 0; i < l->size; i++) {
		void *item = l->items[i];

		if (Item_Matches(l->type, item, match)) {
			for (j = i + 1; j < l->size; j++) {
				l->items[j - 1] = l->items[j];
			}

			Free_Item(l->type, item);

			l->items[l->size - 1] = null;
			l->size--;

			return true;
		}
	}

	return false;
}

void *List_At(list *l, unsigned int index)
{
	if (index >= l->size) {
		printf("List_At failed: size %d, asked for %d\n", l->size, index);
		return null;
	}

	return l->items[index];
}
