/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define LIST_START_CAPACITY		10

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Resize_List(list *l, unsigned int new_capacity)
{
	void *new_storage = Reallocate(l->items, new_capacity, sizeof(void *), "Resize_List");
	if (new_storage == null)
		dief("Resize_List(%s) failed: going from %u to %u", l->tag, l->capacity, new_capacity);

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
	if (l == null)
		dief("New_List(%s): Out of memory", tag);

	l->type = type;
	l->object_size = 0;
	l->capacity = 0;
	l->size = 0;
	l->items = null;
	l->tag = tag;

	Resize_List(l, LIST_START_CAPACITY);
	return l;
}

list *New_Object_List(size_t size, char *tag)
{
	list *l = New_List(ltObject, tag);
	l->object_size = size;

	return l;
}

void Clear_List(list *l)
{
	unsigned int i;

	if (l == null)
		return;

	for (i = 0; i < l->size; i++) {
		Free_Item(l->type, l->items[i]);
	}

	l->size = 0;
}

void Free_List(list *l)
{
	if (l == null)
		return;

#ifdef LIST_REPORT
	Log("Free_List(%s): max_size %d, capacity %d", l->tag, l->max_size, l->capacity);
#endif

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

#ifdef LIST_REPORT
	if (l->size > l->max_size) {
		l->max_size = l->size;
	}
#endif
}

void Add_String_to_List(list *l, char *s)
{
	if (l->type != ltString)
		dief("Add_String_to_List(%s) failed: wrong type of list", l->tag);

	Add_to_List(l, Duplicate_String(s, "Add_String_to_List"));
}

bool In_List(list *l, void *match)
{
	unsigned int i;

	if (l == null) {
		return false;
	}

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
		Log("List_At(%s): size %u, asked for %u", l->tag, l->size, index);
		return null;
	}

	return l->items[index];
}

/* The size of the list header when written to a file */
#define LIST_FILE_HEADER_SZ		sizeof(list_type) + sizeof(unsigned int) + sizeof(unsigned int)

list *Read_List(FILE *fp, char *tag)
{
	int i;
	list fake, *l;
	void **obj;
	
	fread(&fake, LIST_FILE_HEADER_SZ, 1, fp);
	l = New_List(fake.type, tag);
	l->object_size = fake.object_size;

	if (fake.size > l->capacity)
		Resize_List(l, fake.size);
	else
		l->size = fake.size;

	if (l->size > 0) {
		switch (l->type) {
			case ltInteger:
				fread(l->items, sizeof(int), l->size, fp);
				break;

			case ltString:
				for (i = 0; i < l->size; i++)
					l->items[i] = Read_LengthString(fp, tag);
				break;

			case ltObject:
				obj = Allocate(l->size, l->object_size, tag);
				if (obj == null)
					die("Read_List: out of memory");

				fread(obj, l->object_size, l->size, fp);
				for (i = 0; i < l->size; i++)
					l->items[i] = &obj[i];
				break;

			default:
				dief("Read_List: Cannot read a list of type %d from file.", l->type);
		}
	}

	return l;
}

void Write_List(list *l, FILE *fp)
{
	int i;

	fwrite(l, LIST_FILE_HEADER_SZ, 1, fp);

	if (l->size > 0) {
		switch (l->type) {
			case ltInteger:
				fwrite(l->items, sizeof(int), l->size, fp);
				break;

			case ltString:
				for (i = 0; i < l->size; i++)
					Write_LengthString(l->items[i], fp);
				break;

			case ltObject:
				for (i = 0; i < l->size; i++)
					fwrite(l->items[i], l->object_size, 1, fp);
				break;

			default:
				dief("Write_List: Cannot write a list of type %d to file.", l->type);
		}
	}
}
