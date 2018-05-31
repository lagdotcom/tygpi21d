/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* F U N C T I O N S ///////////////////////////////////////////////////// */

void Free_Strings(strings *s)
{
	str_id i;

	for (i = 0; i < s->count; i++)
		Free(s->strings[i].string);

	Free(s->strings);
}

char *Get_String(strings *s, str_id id)
{
	str_id i;

	for (i = 0; i < s->count; i++)
	{
		if (s->strings[i].id == id)
			return s->strings[i].string;
	}

	Log("Get_String: could not find string id %d", id);
	return null;
}

bool Read_Strings(FILE *fp, strings *s)
{
	str_id i;
	fread(s, STRINGS_HEADER_SZ, 1, fp);

	s->strings = SzAlloc(s->count, stringse, "Read_Strings.list");
	if (!s->strings) {
		Log("%s", "Read_Strings: out of memory");
		return false;
	}

	for (i = 0; i < s->count; i++) {
		fread(&s->strings[i].id, sizeof(str_id), 1, fp);
		s->strings[i].string = Read_LengthString(fp, "Read_Strings[i]");
	}

	return true;
}
