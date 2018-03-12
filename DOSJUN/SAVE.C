/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Savefile(save *s)
{
	int i;

	s->header.num_zones = 0;

	s->script_globals = null;
	s->script_locals = null;

	for (i = 0; i < PARTY_SIZE; i++) {
		s->characters[i].skills = null;
		s->characters[i].buffs = null;
	}
}

bool Load_Savefile(char *filename, save *s)
{
	character *c;
	int i;

	FILE *fp = fopen(filename, "rb");
	if (!fp)
		dief("Load_Savefile: Could not open for reading: %s\n", filename);

	Log("Load_Savefile: %s", filename);

	fread(&s->header, sizeof(save_header), 1, fp);
	Check_Version_Header(s->header);

	for (i = 0; i < PARTY_SIZE; i++) {
		c = &s->characters[i];
		fread(&c->header, sizeof(character_header), 1, fp);
		c->skills = Read_List(fp, "Load_Savefile.chars.i.skills");
		c->buffs = Read_List(fp, "Load_Savefile.chars.i.buffs");
	}

	s->script_globals = SzAlloc(MAX_GLOBALS, int, "Load_Savefile.globals");
	if (s->script_globals == null) goto _dead;
	fread(s->script_globals, sizeof(int), MAX_GLOBALS, fp);

	s->script_locals = SzAlloc(s->header.num_zones, int *, "Load_Savefile.locals");
	if (s->script_locals == null) goto _dead;
	for (i = 0; i < s->header.num_zones; i++) {
		s->script_locals[i] = SzAlloc(MAX_LOCALS, int, "Load_Savefile.locals.i");
		if (s->script_locals[i] == null) goto _dead;
		fread(s->script_locals[i], sizeof(int), MAX_LOCALS, fp);
	}

	fclose(fp);
	return true;

_dead:
	die("Load_Savefile: out of memory");
	return false;
}

void Free_Savefile(save *s)
{
	character *c;
	int i;

	Log("Free_Savefile: %p", s);

	Free(s->script_globals);

	for (i = 0; i < s->header.num_zones; i++)
		Free(s->script_locals[i]);
	Free(s->script_locals);
	s->header.num_zones = 0;

	for (i = 0; i < PARTY_SIZE; i++) {
		c = &s->characters[i];
		Free_List(c->skills);
		Clear_List(c->buffs);
		Free_List(c->buffs);
	}
}

bool Save_Savefile(char *filename, save *s)
{
	character *c;
	int i;

	FILE *fp = fopen(filename, "wb");
	if (!fp)
		dief("Save_Savefile: Could not open for reading: %s\n", filename);

	Log("Save_Savefile: %s", filename);

	Set_Version_Header(s->header);
	fwrite(&s->header, sizeof(save_header), 1, fp);

	for (i = 0; i < PARTY_SIZE; i++) {
		c = &s->characters[i];
		fwrite(&c->header, sizeof(character_header), 1, fp);
		Write_List(c->skills, fp);
		Write_List(c->buffs, fp);
	}

	fwrite(s->script_globals, sizeof(int), MAX_GLOBALS, fp);

	for (i = 0; i < s->header.num_zones; i++) {
		fwrite(s->script_locals[i], sizeof(int), MAX_LOCALS, fp);
	}

	fclose(fp);
	return true;
}
