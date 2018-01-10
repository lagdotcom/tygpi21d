#ifndef DOSJUN_LIST_H
#define DOSJUN_LIST_H

typedef enum list_type {
	ltString,
	ltInteger,
	ltObject
} list_type;

typedef struct list {
	list_type type;
	unsigned int object_size;
	unsigned int size;
	unsigned int capacity;
	void **items;
	char *tag;

#ifdef LIST_REPORT
	unsigned int max_size;
#endif
} list;

void Add_to_List(list *l, void *item);
void Add_String_to_List(list *l, char *s);
void Clear_List(list *l);
void Free_List(list *l);
bool In_List(list *l, void *match);
void *List_At(list *l, unsigned int index);
list *New_List(list_type type, char *tag);
list *New_Object_List(size_t size, char *tag);
list *Read_List(FILE *fp, char *tag);
bool Remove_from_List(list *l, void *match);
void Write_List(list *l, FILE *fp);

#endif
