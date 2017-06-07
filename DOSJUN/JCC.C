/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "jc.h"
#include "types.h"
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

int JC_Compile(char *filename)
{
	bool lex_success;
	char line[JC_LINE_LENGTH];
	jc_token *tokens = calloc(MAX_TOKENS_PER_LINE, sizeof(jc_token));
	int count,
		i,
		line_no = 0;

	FILE *fp = fopen(filename, "r");
	while (fgets(line, JC_LINE_LENGTH, fp)) {
		line_no++;
		lex_success = JC_Lex_String(line, tokens, &count);

		for (i = 0; i < count; i++) {
			if (tokens[i].value != null) {
				printf("%s %s\n", JC_TokenName(tokens[i].type), tokens[i].value);
				free(tokens[i].value);
			} else {
				printf("%s\n", JC_TokenName(tokens[i].type));
			}
		}

		if (!lex_success) {
			printf("--- jcc died on line %d\n", line_no);
			break;
		}
	}

	fclose(fp);
	free(tokens);
	return line_no;
}

/* M A I N /////////////////////////////////////////////////////////////// */

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Syntax: %s filename.jc\n", argv[0]);
		return 0;
	}

	return JC_Compile(argv[1]);
}
