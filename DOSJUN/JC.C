/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "jc.h"
#include "code.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define JC_LINE_LENGTH	1000

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

bool Tokenize_Code_String(char *source, jc_token *tokens, int *count);

/* F U N C T I O N S ///////////////////////////////////////////////////// */

jc_token_type Parse_Operator(char *string)
{
	if (!strncmp(string, "==", 2)) return ttEquals;
	if (!strncmp(string, "!=", 2)) return ttNotEqual;
	if (!strncmp(string, "<=", 2)) return ttLTE;
	if (!strncmp(string, ">=", 2)) return ttGTE;
	if (!strncmp(string, "<", 1)) return ttLT;
	if (!strncmp(string, ">", 1)) return ttGT;
	if (!strncmp(string, "=", 1)) return ttAssignment;

	return ttUnknown;
}

bool Is_Code_Keyword(char *string)
{
	if (!strcmp(string, "Combat")) return true;
	if (!strcmp(string, "Const")) return true;
	if (!strcmp(string, "Else")) return true;
	if (!strcmp(string, "ElseIf")) return true;
	if (!strcmp(string, "EndIf")) return true;
	if (!strcmp(string, "EndScript")) return true;
	if (!strcmp(string, "EquipItem")) return true;
	if (!strcmp(string, "GiveItem")) return true;
	if (!strcmp(string, "Global")) return true;
	if (!strcmp(string, "If")) return true;
	if (!strcmp(string, "Include")) return true;
	if (!strcmp(string, "Local")) return true;
	if (!strcmp(string, "PcSpeak")) return true;
	if (!strcmp(string, "Return")) return true;
	if (!strcmp(string, "Script")) return true;
	if (!strcmp(string, "Text")) return true;
	if (!strcmp(string, "Unlock")) return true;

	return false;
}

internal_id Get_Internal_Id(char *string)
{
	if (!strcmp(string, "Facing")) return internalFacing;
	if (!strcmp(string, "X")) return internalX;
	if (!strcmp(string, "Y")) return internalY;

	return internalInvalid;
}

char *Get_Token_Name(jc_token_type tt)
{
	switch (tt) {
		case ttUnknown: return "UNK";
		case ttComment: return "COM";
		case ttInternal: return "INT";
		case ttKeyword: return "KEY";
		case ttIdentifier: return "VAR";
		case ttString: return "STR";
		case ttNumber: return "NUM";
		case ttAssignment: return "SET";
		case ttEquals: return "EQU";
		case ttNotEqual: return "NEQ";
		case ttLT: return "LT ";
		case ttLTE: return "LTE";
		case ttGT: return "GT ";
		case ttGTE: return "GTE";

		default: return "???";
	}
}

int Compile_JC(jc_parser *parser, char *filename, bool toplevel)
{
	bool success;
	char line[JC_LINE_LENGTH];
	jc_token *tokens = SzAlloc(MAX_TOKENS_PER_LINE, jc_token, "Compile_JC");
	int count,
		i,
		err = 0,
		line_no = 0;
	FILE *fp;

	printf("%s:\n", filename);
	fp = fopen(filename, "r");
	if (!fp) IO_Error("Could not open JC");

	while (fgets(line, JC_LINE_LENGTH, fp)) {
		line_no++;
		success = Tokenize_Code_String(line, tokens, &count);

		if (success && count > 0) {
			/* special handling for Include */
			if (tokens[0].type == ttKeyword && !strcmp(tokens[0].value, "Include")) {
				success = Compile_JC(parser, tokens[1].value, false) == 0;
			} else {
				success = Parse_Tokens(parser, tokens, count);
			}
		}

		for (i = 0; i < count; i++) {
			Free(tokens[i].value);
		}

		if (!success) {
			printf("--- jcc died on line %d: %s", line_no, line);
			err = line_no;
			break;
		}
	}
	fclose(fp);

	if (toplevel) Dump_Parser_State(parser);
	Free(tokens);
	return err;
}

void Dump_Compiled_JC(jc_parser *p, char *filename)
{
	FILE *fp;
	int i, j, opbytes = 0;

	printf("jcc: dumping progress to %s\n", filename);
	fp = fopen(filename, "w");
	if (!fp) IO_Error("Could not open jcc for writing");

	for (i = 0; i < p->script_count; i++) {
		fprintf(fp, "\n[%s]\n", p->scripts[i].name);

		for (j = 0; j < p->scripts[i].size; j++) {
			fprintf(fp, "  %04x: %02x", j, p->scripts[i].code[j]);

			switch (p->scripts[i].code[j]) {
				case coPushGlobal:
				case coPushLocal:
				case coPushTemp:
				case coPushInternal:
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
