/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport int Flag_Size(UINT16 count)
{
	int size = count / 16;
	if (count & 0xf) size++;

	return size;
}

void Initialise_Savefile(djn *s)
{
	party *p;
	globals *g;
	options *o;

	s->files = null;

	p = SzAlloc(1, party, "Initialise_Savefile.party");
	Add_to_Djn(s, p, 0, ftParty);

	g = SzAlloc(1, globals, "Initialise_Savefile.globals");
	Add_to_Djn(s, g, 0, ftGlobals);

	o = SzAlloc(1, options, "Initialise_Savefile.options");
	o->music_vol = MAX_VOLUME;
	o->sound_vol = MAX_VOLUME;
	Add_to_Djn(s, o, 0, ftOptions);

	s->next = gDjn;
}

bool Load_Savefile(char *filename, djn *s)
{
	return Load_Djn(filename, s);
}

void Free_Savefile(djn *s)
{
	Free_Djn(s);
	Free(s);
}

bool Save_Savefile(char *filename, djn *s)
{
	bool result;

	clock_enabled = false;
	result = Save_Djn(filename, s);
	clock_enabled = true;

	return result;
}

void Initialise_Globals(globals *g, campaign *c)
{
	g->num_globals = c->global_count;
	g->globals = SzAlloc(g->num_globals, UINT16, "Initialise_Globals.globals");

	g->num_flags = c->flag_count;
	g->flags = SzAlloc(Flag_Size(g->num_flags), UINT16, "Initialise_Globals.flags");
}

void Initialise_Overlay(overlay *o, zone *z)
{
	int i;
	itempos *p;

	Log("Initialise_Overlay: %d locals, %d items", z->header.num_locals, z->header.num_items);

	o->num_locals = z->header.num_locals;
	o->locals = SzAlloc(o->num_locals, UINT16, "Initialise_Overlay.locals");

	o->items = New_List_of_Capacity(ltObject, z->header.num_items, "Initialise_Overlay.items");
	o->items->object_size = sizeof(itempos);
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

void Free_Overlay(overlay *o)
{
	Free(o->locals);
	Clear_List(o->items);
	Free_List(o->items);
}

bool Read_Globals(FILE *fp, globals *g)
{
	int size;

	fread(g, GLOBALS_HEADER_SZ, 1, fp);

	size = Flag_Size(g->num_flags);

	g->globals = SzAlloc(g->num_globals, INT16, "Read_Globals.globals");
	g->flags = SzAlloc(size, INT16, "Read_Globals.flags");
	if (g->globals == null || g->flags == null) goto _dead;

	fread(g->globals, sizeof(INT16), g->num_globals, fp);
	fread(g->flags, sizeof(INT16), size, fp);

	return true;

_dead:
	die("Read_Globals: out of memory");
	return false;
}

bool Read_Overlay(FILE *fp, overlay *o)
{
	fread(o, OVERLAY_HEADER_SZ, 1, fp);

	o->locals = SzAlloc(o->num_locals, INT16, "Read_Overlay.locals");
	if (o->locals == null) goto _dead;
	fread(o->locals, sizeof(INT16), o->num_locals, fp);

	o->items = Read_List(fp, "Read_Overlay.items");

	return true;

_dead:
	die("Read_Overlay: out of memory");
	return false;
}

bool Read_Party(FILE *fp, party *p)
{
	fread(p, sizeof(party), 1, fp);

	return true;
}

bool Write_Globals(FILE *fp, globals *g)
{
	fwrite(g, GLOBALS_HEADER_SZ, 1, fp);
	fwrite(g->globals, sizeof(INT16), g->num_globals, fp);
	fwrite(g->flags, sizeof(INT16), Flag_Size(g->num_flags), fp);

	return true;
}

bool Write_Overlay(FILE *fp, overlay *o)
{
	fwrite(o, OVERLAY_HEADER_SZ, 1, fp);
	fwrite(o->locals, sizeof(INT16), o->num_locals, fp);
	Write_List(o->items, fp);

	return true;
}

bool Write_Party(FILE *fp, party *p)
{
	fwrite(p, sizeof(party), 1, fp);

	return true;
}

void Add_PC_to_Save(djn *s, pc *org, file_id id)
{
	pc *dup;

	if (In_Djn(s, id, false))
		return;

	dup = Duplicate_PC(org);
	Add_to_Djn(s, dup, id, ftPC);
}

void Add_NPC_to_Save(djn *s, npc *org, file_id id)
{
	npc *dup;

	if (In_Djn(s, id, false))
		return;

	dup = Duplicate_NPC(org);
	Add_to_Djn(s, dup, id, ftNPC);
}

char *Get_Pronoun_Name(pronouns pro)
{
	switch (pro) {
		case proHe: return "he/him/his/himself";
		case proShe: return "she/her/hers/herself";
		case proThey: return "they/them/their/theirs/themself";
		case proIt: return "it/its/itself";

		default: return "?";
	}
}
