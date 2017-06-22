/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdio.h>
#include "dosjun.h"
#include "code.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Initialise_Savefile(save *s)
{
	s->script_globals = null;
	s->script_locals = null;
}

bool Load_Savefile(char *filename, save *s)
{
	int i;
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Could not open for reading: %s\n", filename);
		return false;
	}

	fread(&s->header, sizeof(save_header), 1, fp);
	Check_Version_Header(s->header);

	s->script_globals = SzAlloc(MAX_GLOBALS, int, "Load_Savefile.globals");
	fread(s->script_globals, sizeof(int), MAX_GLOBALS, fp);

	s->script_locals = SzAlloc(s->header.num_zones, int *, "Load_Savefile.locals");
	for (i = 0; i < s->header.num_zones; i++) {
		s->script_locals[i] = SzAlloc(MAX_LOCALS, int, "Load_Savefile.locals[i]");
		fread(s->script_locals[i], sizeof(int), MAX_LOCALS, fp);
	}

	fclose(fp);
	return true;
}

void Free_Savefile(save *s)
{
	int i;
	Free(s->script_globals);

	for (i = 0; i < s->header.num_zones; i++)
		Free(s->script_locals[i]);
	Free(s->script_locals);
	s->header.num_zones = 0;
}

bool Save_Savefile(char *filename, save *s)
{
	int i;
	FILE *fp = fopen(filename, "wb");
	if (!fp) {
		printf("Could not open for reading: %s\n", filename);
		return false;
	}

	Set_Version_Header(s->header);
	fwrite(&s->header, sizeof(save_header), 1, fp);

	fwrite(s->script_globals, sizeof(int), MAX_GLOBALS, fp);

	for (i = 0; i < s->header.num_zones; i++) {
		fwrite(s->script_locals[i], sizeof(int), MAX_LOCALS, fp);
	}

	fclose(fp);
	return true;
}
