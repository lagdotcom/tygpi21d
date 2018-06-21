/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

typedef enum procase {
	pcSubjectPronoun,
	pcObjectPronoun,
	pcPosessiveDeterminer,
	pcPosessivePronoun,
	pcReflexive,
} procase;

#define BUFFER_SIZE 30
#define FMT_CHAR	'@'

/* G L O B A L S ///////////////////////////////////////////////////////// */

noexport const char *pronoun_cases[] = {
	/* proHe */
	"he",
	"him",
	"his",
	"his",
	"himself",

	/* proShe */
	"she",
	"her",
	"her",
	"hers",
	"herself",

	/* proThey */
	"they",
	"them",
	"their",
	"theirs",
	"themself",

	/* proIt */
	"it",
	"it",
	"its",
	"its",
	"itself",
};

noexport char *formatter_buf = null;

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

void Initialise_Formatter()
{
	formatter_buf = SzAlloc(BUFFER_SIZE, char, "Initialise_Formatter");
}

void Free_Formatter()
{
	Free(formatter_buf);
}

noexport pronouns Get_Pronouns(file_id id)
{
	djn_file *f = Lookup_File_Entry(gSave, id, true, true);

	switch (f->type) {
		case ftPC: return ((pc *)f->object)->header.pronouns;
		case ftNPC: return ((npc *)f->object)->pronouns;
	}

	/* TODO */
	return proIt;
}

noexport char *Get_Name(file_id id)
{
	djn_file *f = Lookup_File_Entry(gSave, id, true, true);
	pc *pc;

	switch (f->type) {
		case ftPC:
			pc = f->object;
			if (pc->header.name_id == 0) return pc->name;
			return Resolve_String(pc->header.name_id);

		case ftNPC:
			return Resolve_String(((npc *)f->object)->name_id);
	}

	/* TODO */
	return "(unknown)";
}

noexport const char *Get_Pronoun_Case(pronouns p, procase c)
{
	return pronoun_cases[p * 5 + c];
}

noexport void Show_Word(grf *font, const char *word, point2d *p, const box2d *bounds, colour tint)
{
	size2d sz;

	sz = Measure_String(font, word);
	if (sz.w + p->x > bounds->end.x) {
		p->x = bounds->start.x;
		p->y += sz.h;
	}

	Draw_Font(p->x, p->y, tint, word, font, false);
	p->x += sz.w + Char_Width(font, ' ');
}

noexport int Hex_Digit(char c)
{
	switch (c)
	{
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case 'A': return 0xa;
		case 'b': case 'B': return 0xb;
		case 'c': case 'C': return 0xc;
		case 'd': case 'D': return 0xd;
		case 'e': case 'E': return 0xe;
		case 'f': case 'F': return 0xf;

		default: return 0;
	}
}

/* TODO: bounds.end.y is never used */
/* TODO: tint change */
void Show_Formatted_String(const char *s, file_id speaker, file_id target, const box2d *bounds, grf *font, colour start_tint)
{
	int i, colour_mode;
	char *b;
	const char *word;
	point2d p;
	bool fmt_mode, match;
	procase fmt_case;
	colour tint;
	pronouns
		speaker_p = speaker ? Get_Pronouns(speaker) : proIt,
		target_p = target ? Get_Pronouns(target) : proIt,
		fmt_p;

	tint = start_tint;
	p.x = bounds->start.x;
	p.y = bounds->start.y;
	b = formatter_buf;
	fmt_mode = false;
	colour_mode = false;

	for (i = 0; i < strlen(s); i++) {
		word = null;

		if (fmt_mode) {
			fmt_mode = false;
			match = true;

			switch (s[i]) {
				case 'e':
					fmt_case = pcSubjectPronoun;
					fmt_p = speaker_p;
					break;

				case 'E':
					fmt_case = pcSubjectPronoun;
					fmt_p = target_p;
					break;

				case 'm':
					fmt_case = pcObjectPronoun;
					fmt_p = speaker_p;
					break;

				case 'M':
					fmt_case = pcObjectPronoun;
					fmt_p = target_p;
					break;

				case 'r':
					fmt_case = pcPosessiveDeterminer;
					fmt_p = speaker_p;
					break;

				case 'R':
					fmt_case = pcPosessiveDeterminer;
					fmt_p = target_p;
					break;

				case 's':
					fmt_case = pcPosessivePronoun;
					fmt_p = speaker_p;
					break;

				case 'S':
					fmt_case = pcPosessivePronoun;
					fmt_p = target_p;
					break;

				case 'f':
					fmt_case = pcReflexive;
					fmt_p = speaker_p;
					break;

				case 'F':
					fmt_case = pcReflexive;
					fmt_p = target_p;
					break;

				case 'n':
					word = Get_Name(speaker);
					break;

				case 'N':
					word = Get_Name(target);
					break;

				default:
					*b++ = s[i];
					match = false;
					break;
			}

			if (match) {
				word = Get_Pronoun_Case(fmt_p, fmt_case);
			}

			continue;
		}

		switch (colour_mode) {
			case 2:
				tint += Hex_Digit(s[i]);
				colour_mode = 0;
				continue;

			case 1:
				if (s[i] == 'x') {
					tint = start_tint;
					colour_mode = 0;
				} else {
					tint = Hex_Digit(s[i]) << 4;
					colour_mode = 2;
				}
				continue;
		}

		switch (s[i]) {
			case '\0':
				if (b > formatter_buf) {
					*b = 0;
					word = b = formatter_buf;
				}
				break;

			case FMT_CHAR:
				fmt_mode = true;
				break;

			case ' ':
				*b = 0;
				word = b = formatter_buf;
				break;

			case '^':
				colour_mode = 1;
				break;

			default:
				*b++ = s[i];
				break;
		}

		if (word != null) {
			Show_Word(font, word, &p, bounds, tint);
		}
	}

	if (b > formatter_buf) {
		*b = 0;
		Show_Word(font, formatter_buf, &p, bounds, tint);
	}
}
