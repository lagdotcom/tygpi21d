/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "jc.h"
#include "code.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define JC_LINE_LENGTH	1000

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

jc_token_type JC_Operator(char *string)
{
	if (!strncmp(string, "==", 2)) return IsEqual;
	if (!strncmp(string, "!=", 2)) return IsNotEqual;
	if (!strncmp(string, "=", 1)) return Assignment;

	return Unknown;
}

bool JC_IsKeyword(char *string)
{
	if (!strncmp(string, "Combat", 6)) return true;
	if (!strncmp(string, "Const", 5)) return true;
	if (!strncmp(string, "Else", 4)) return true;
	if (!strncmp(string, "ElseIf", 6)) return true;
	if (!strncmp(string, "EndIf", 5)) return true;
	if (!strncmp(string, "EndScript", 9)) return true;
	if (!strncmp(string, "Global", 6)) return true;
	if (!strncmp(string, "If", 2)) return true;
	if (!strncmp(string, "Include", 7)) return true;
	if (!strncmp(string, "Local", 5)) return true;
	if (!strncmp(string, "PcSpeak", 7)) return true;
	if (!strncmp(string, "Return", 6)) return true;
	if (!strncmp(string, "Script", 6)) return true;
	if (!strncmp(string, "Text", 4)) return true;
	if (!strncmp(string, "Unlock", 6)) return true;

	return false;
}

char *JC_TokenName(jc_token_type tt)
{
	switch (tt) {
		case Unknown: return "UNK";
		case Comment: return "COM";
		case Keyword: return "KEY";
		case Identifier: return "VAR";
		case StringLiteral: return "STR";
		case NumericLiteral: return "NUM";
		case Assignment: return "SET";
		case IsEqual: return "EQU";
		case IsNotEqual: return "NEQ";

		default: return "???";
	}
}

int JC_Compile(jc_parser *parser, char *filename, bool toplevel)
{
	bool success;
	char line[JC_LINE_LENGTH];
	jc_token *tokens = calloc(MAX_TOKENS_PER_LINE, sizeof(jc_token));
	int count,
		i,
		err = 0,
		line_no = 0;
	FILE *fp;

	printf("%s:\n", filename);

	fp = fopen(filename, "r");
	while (fgets(line, JC_LINE_LENGTH, fp)) {
		line_no++;
		success = JC_Lex_String(line, tokens, &count);

		if (success && count > 0) {
			/* special handling for Include */
			if (tokens[0].type == Keyword && !strcmp(tokens[0].value, "Include")) {
				success = JC_Compile(parser, tokens[1].value, false) == 0;
			} else {
				success = JC_Parse(parser, tokens, count);
			}
		}

		for (i = 0; i < count; i++) {
			if (tokens[i].value != null) {
				free(tokens[i].value);
			}
		}

		if (!success) {
			printf("--- jcc died on line %d: %s", line_no, line);
			err = line_no;
			break;
		}
	}
	fclose(fp);

	if (toplevel) Parser_Dump(parser);
	free(tokens);
	return err;
}

void JCC_Dump(jc_parser *p, char *filename)
{
	FILE *fp;
	int i, j, opbytes = 0;

	printf("jcc: dumping progress to %s\n", filename);
	fp = fopen(filename, "w");

	for (i = 0; i < p->script_count; i++) {
		fprintf(fp, "\n[%s]\n", p->scripts[i].name);

		for (j = 0; j < p->scripts[i].size; j++) {
			fprintf(fp, "  %04x: %02x", j, p->scripts[i].code[j]);

			switch (p->scripts[i].code[j]) {
				case coPushGlobal:
				case coPushLocal:
				case coPushTemp:
				case coPopGlobal:
				case coPopLocal:
				case coPopTemp:
					opbytes = 1;
					break;

				case coPushLiteral:
				case coJump:
				case coJumpFalse:
					opbytes = 2;
					break;
			}

			while (opbytes > 0) {
				opbytes--;
				fprintf(fp, " %02x", p->scripts[i].code[++j]);
			}

			fputc('\n', fp);
		}
	}

	fclose(fp);
}
