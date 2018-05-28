/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport int Flag_Size(UINT16 count)
{
	int size = g->num_flags / 16;
	if (g->num_flags & 0xf) size++;

	return size;
}

void Initialise_Savefile(djn *s)
{
	partystatus *party;
	globals *globs;

	party = SzAlloc(1, partystatus, "Initialise_Savefile.party");
	Add_to_Djn(s, party, 0, ftParty);

	globs = SzAlloc(1, globals, "Initialise_Savefile.globs");
	Add_to_Djn(s, globs, 0, ftGlobals);

	s->next = gDjn;
}

bool Load_Savefile(char *filename, djn *s)
{
	return Load_Djn(filename, s);
}

void Free_Savefile(djn *s)
{
	Free_Djn(s);
}

bool Save_Savefile(char *filename, djn *s)
{
	return Save_Djn(filename, s);
}

void Initialise_Globals(globals *g, campaign *c)
{
	UINT16 size;

	g->num_globals = c->global_count;
	g->globals = SzAlloc(g->num_globals, UINT16, "Initialise_Globals.globals");

	g->num_flags = c->flag_count;
	g->flags = SzAlloc(Flag_Size(g->num_flags), UINT16, "Initialise_Globals.flags");
}

void Initialise_Overlay(zone_overlay *o, zone *z)
{
	int i;
	itempos *p;

	o->num_locals = z->header.num_locals;
	o->locals = SzAlloc(o->num_locals, UINT16, "Initialise_Overlay.locals");

	o->items = New_List_of_Capacity(ltObject, z->header.num_items, "Initialise_Overlay.items");
	for (i = 0; i < z->header.num_items; i++) {
		p = SzAlloc(1, itempos, "Initialise_Overlay.item[i]");
		memcpy(p, &z->items[i], sizeof(itempos));
		Add_to_List(o->items, p);
	}
}

void Free_Globals(globals *g)
{
	Free(g->globals);
	Free(g->flags);
}

void Free_Overlay(zone_overlay *o)
{
	Free(o->locals);
	Clear_List(o->items);
	Free_List(o->items);
}

void Read_Globals(FILE *fp, globals *g)
{
	int size;

	fread(g, GLOBALS_HEADER_SZ, 1, fp);

	g->globals = SzAlloc(g->num_globals, INT16, "Read_Globals.globals");
	fread(g->globals, sizeof(INT16), g->num_globals, fp);

	size = Flag_Size(g->num_flags);
	g->flags = SzAlloc(size, INT16, "Read_Globals.flags");
	fread(g->flags, sizeof(INT16), size, fp);
}

void Read_Overlay(FILE *fp, zone_overlay *o)
{
	fread(o, OVERLAY_HEADER_SZ, 1, fp);

	o->locals = SzAlloc(o->num_locals, INT16, "Read_Overlay.locals");
	fread(o->locals, sizeof(INT16), g->locals, fp);

	o->items = Read_List(fp, "Read_Overlay.items");
}

void Write_Globals(FILE *fp, globals *g)
{
	fwrite(g, GLOBALS_HEADER_SZ, 1, fp);
	fwrite(g->globals, sizeof(INT16), g->num_globals, fp);
	fwrite(g->flags, sizeof(INT16), Flag_Size(g->num_flags), fp);
}

void Write_Overlay(FILE *fp, zone_overlay *o)
{
	fwrite(o, OVERLAY_HEADER_SZ, 1, fp);
	fwrite(o->locals, sizeof(INT16), o->num_locals, fp);
	Write_List(o->items, fp);
}
