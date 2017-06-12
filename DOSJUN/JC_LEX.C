/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "jc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define LEXER_TOKEN_SIZE	300

typedef enum {
	lsNone,
	lsCommentStart,
	lsNumber,
	lsString,
	lsStringEscape,
	lsKeywordOrIdent,
	lsOperator,
	lsWhitespace,
	lsEndOfLine
} lex_state;

#define caseNumeric \
	case '0': case '1': case '2': case '3': case '4': \
	case '5': case '6': case '7': case '8': case '9'

#define caseAlphabetic \
	case 'a': case 'b': case 'c': case 'd': case 'e': \
	case 'f': case 'g': case 'h': case 'i': case 'j': \
	case 'k': case 'l': case 'm': case 'n': case 'o': \
	case 'p': case 'q': case 'r': case 's': case 't': \
	case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': \
	case 'A': case 'B': case 'C': case 'D': case 'E': \
	case 'F': case 'G': case 'H': case 'I': case 'J': \
	case 'K': case 'L': case 'M': case 'N': case 'O': \
	case 'P': case 'Q': case 'R': case 'S': case 'T': \
	case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': \
	case '_'

#define caseWhitespace \
	case ' ': case '\t': case ','

#define caseEol \
	case '\n': case '\r': case '\0'

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport lex_state Get_Lexer_State(char source)
{
	switch (source) {
		case '#':		return lsCommentStart;
		case '\\':		return lsStringEscape;
		case '"': 		return lsString;
		caseNumeric:	return lsNumber;

		case '=':
		case '!':
		case '>':
		case '<':		return lsOperator;

		caseWhitespace:	return lsWhitespace;
		caseEol:		return lsEndOfLine;

		caseAlphabetic:	return lsKeywordOrIdent;
	}

	return lsNone;
}

noexport void Lexer_Error(char *line, int offset, char *message)
{
	printf("-- LEXER ERROR --\n");
	printf("%s%s at column %d\n", line, message, offset);
}

#define Lex_Error(message) { \
	Lexer_Error(source, ptr - source, message); \
	return false; \
}
#define Lex_Push(c) buffer[buffer_offset++] = c
#define Lex_AddToken(tt) { \
	Add_Lexer_Token(tokens, count, buffer, &buffer_offset, tt); \
	state = lsNone; \
}

noexport void Add_Lexer_Token(jc_token *tokens, int *count, char *buffer, int *buffer_offset, jc_token_type tt)
{
	int index = *count;
	int bo = *buffer_offset;

	tokens[index].type = tt;
	if (bo > 0) {
		buffer[bo] = 0;
		tokens[index].value = Duplicate_String(buffer, "Add_Lexer_Token");
		*buffer_offset = 0;
	} else {
		tokens[index].value = null;
	}

	(*count)++;
}

/* M A I N /////////////////////////////////////////////////////////////// */

bool Tokenize_Code_String(char *source, jc_token *tokens, int *count)
{
	lex_state state = lsNone,
		guess = lsNone;
	jc_token_type token_type;
	unsigned char *ptr = source,
		ch;
	char buffer[LEXER_TOKEN_SIZE];
	int buffer_offset = 0;

	memset(buffer, 0, LEXER_TOKEN_SIZE);
	*count = 0;

	while (guess != lsEndOfLine) {
		ch = *(ptr++);
		guess = Get_Lexer_State(ch);

		switch (state) {
			case lsNone:
				switch (guess) {
					/* ignore rest of line */
					case lsCommentStart:
					case lsEndOfLine:
						return true;

					/* ignore */
					case lsWhitespace:
						continue;

					/* change state */
					case lsString:
						state = guess;
						continue;

					/* set buffer, change state */
					case lsNumber:
					case lsOperator:
					case lsKeywordOrIdent:
						state = guess;
						Lex_Push(ch);
						continue;

					default: Lex_Error("Invalid character");
				}

			case lsString:
				switch (ch) {
					case '\\':
						state = lsStringEscape;
						continue;

					case '"':
						Lex_AddToken(ttString);
						continue;

					caseEol: Lex_Error("Unexpected end of line");

					default:
						Lex_Push(ch);
						continue;
				}

			case lsStringEscape:
				switch (ch) {
					case 'n':
						Lex_Push('\n');
						break;

					caseEol: Lex_Error("Unexpected end of line");

					default:
						Lex_Push(ch);
						break;
				}
				state = lsString;
				continue;

			case lsNumber:
				switch (ch) {
					case '#':
						Lex_AddToken(ttNumber);
						return true;

					caseNumeric:
						Lex_Push(ch);
						continue;

					caseEol:
					caseWhitespace:
						Lex_AddToken(ttNumber);
						continue;

					default: Lex_Error("Invalid character in numeric literal");
				}

			case lsKeywordOrIdent:
				switch (guess) {
					case lsCommentStart:
						Lex_Push('\0');
						Lex_AddToken(Is_Code_Keyword(buffer) ? ttKeyword : ttIdentifier);
						return true;

					case lsKeywordOrIdent:
					case lsNumber:
						Lex_Push(ch);
						continue;

					case lsOperator:
						ptr--;
						/* FALL THROUGH */

					case lsEndOfLine:
					case lsWhitespace:
						Lex_Push('\0');
						Lex_AddToken(Is_Code_Keyword(buffer) ? ttKeyword : ttIdentifier);
						continue;

					default: Lex_Error("Invalid character in keyword/identifier");
				}

			case lsOperator:
				switch (guess) {
					case lsOperator:
						Lex_Push(ch);
						continue;

					case lsKeywordOrIdent:
					case lsNumber:
					case lsString:
						ptr--;
						/* FALL THROUGH */

					case lsEndOfLine:
					case lsWhitespace:
						token_type = Parse_Operator(buffer);
						if (token_type == ttUnknown) Lex_Error("Unknown operator");
						Lex_AddToken(token_type);
						continue;
				}

			default: Lex_Error("Unknown state");
		}
	}

	return true;
}
