/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include <string.h>
#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define LIST_START_CAPACITY		10

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport void Resize_List(list *l, unsigned int new_capacity)
{
	void *new_storage = Reallocate(l->items, new_capacity, sizeof(void *), l->tag);
	if (new_storage == null)
		dief("Resize_List(%s) failed: going from %u to %u", l->tag, l->capacity, new_capacity);

	l->items = new_storage;
	l->capacity = new_capacity;
}

noexport bool Item_Matches(list_type type, void *a, void *b)
{
	switch (type) {
		case ltString:
			return streq(a, b);

		case ltInteger:
			return (int)a == (int)b;

		case ltObject:
		case ltReference:
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
		case ltReference:
			return;
	}
}

list *New_List(list_type type, char *tag)
{
	return New_List_of_Capacity(type, LIST_START_CAPACITY, tag);
}

list *New_List_of_Capacity(list_type type, int capacity, char *tag)
{
	list *l = SzAlloc(1, list, tag);
	if (l == null) {
		dief("New_List_with_Capacity(%s): Out of memory", tag);
		return null;
	}

	l->type = type;
	l->object_size = 0;
	l->capacity = 0;
	l->size = 0;
	l->items = null;
	l->tag = tag;

	if (capacity > 0)
		Resize_List(l, capacity);

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
#define LIST_FILE_HEADER_SZ		sizeof(list_type) + sizeof(unsigned int) + sizeof(unsigned int) + sizeof(unsigned int)

list *Read_List(FILE *fp, char *tag)
{
	int i, temp;
	list fake, *l;
	void **obj;
	
	fread(&fake, LIST_FILE_HEADER_SZ, 1, fp);
	l = New_List_of_Capacity(fake.type, fake.capacity, tag);
	l->object_size = fake.object_size;
	l->size = fake.size;

	if (l->size > 0) {
		switch (l->type) {
			case ltInteger:
				for (i = 0; i < l->size; i++) {
					fread(&temp, sizeof(int), 1, fp);
					l->items[i] = (void *)temp;
				}
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
	int i, temp;

	fwrite(l, LIST_FILE_HEADER_SZ, 1, fp);

	if (l->size > 0) {
		switch (l->type) {
			case ltInteger:
				for (i = 0; i < l->size; i++) {
					temp = (int)l->items[i];
					fwrite(&temp, sizeof(int), 1, fp);
				}
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

list *Duplicate_List(list *org, char *tag)
{
	int i;
	list *dup = New_List_of_Capacity(org->type, org->capacity, tag);
	void *temp;
	dup->object_size = org->object_size;

	switch (org->type) {
		case ltString:
			for (i = 0; i < org->size; i++)
				dup->items[i] = Duplicate_String(org->items[i], tag);
			break;

		/* TODO: this could suck on a nested object */
		case ltObject:
			for (i = 0; i < org->size; i++) {
				temp = Allocate(1, org->object_size, tag);
				memcpy(temp, org->items[i], org->object_size);
				dup->items[i] = temp;
			}
			break;

		/* TODO: this is a source of bugs for sure */
		default:
			for (i = 0; i < org->size; i++)
				dup->items[i] = org->items[i];
			break;
	}

	return dup;
}
