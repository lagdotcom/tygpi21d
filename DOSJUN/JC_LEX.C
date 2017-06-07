/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "jc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define LEXER_TOKEN_SIZE	300

typedef enum {
	None,
	CommentStart,
	Number,
	String,
	StringEscape,
	KeywordOrIdent,
	Operator,
	Whitespace,
	EndOfLine
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

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

lex_state JC_Lex_Guess(char source)
{
	switch (source) {
		case '#':		return CommentStart;
		case '\\':		return StringEscape;
		case '"': 		return String;
		caseNumeric:	return Number;

		case '=':
		case '!':
		case '>':
		case '<':		return Operator;

		caseWhitespace:	return Whitespace;
		caseEol:		return EndOfLine;

		caseAlphabetic:	return KeywordOrIdent;
	}

	return None;
}

void JC_Lex_Error(char *line, int offset, char *message)
{
	printf("-- LEXER ERROR --\n");
	printf("%s%s at column %d\n", line, message, offset);
}

#define Lex_Error(message) { \
	JC_Lex_Error(source, ptr - source, message); \
	return false; \
}
#define Lex_Push(c) buffer[buffer_offset++] = c
#define Lex_AddToken(tt) { \
	JC_Lex_AddToken(tokens, count, buffer, &buffer_offset, tt); \
	state = None; \
}

void JC_Lex_AddToken(jc_token *tokens, int *count, char *buffer, int *buffer_offset, jc_token_type tt)
{
	int index = *count;
	int bo = *buffer_offset;

	tokens[index].type = tt;
	if (bo > 0) {
		buffer[bo] = 0;
		tokens[index].value = strdup(buffer);
		*buffer_offset = 0;
	} else {
		tokens[index].value = null;
	}

	(*count)++;
}

/* M A I N /////////////////////////////////////////////////////////////// */

bool JC_Lex_String(char *source, jc_token *tokens, int *count)
{
	lex_state state = None,
		guess = None;
	jc_token_type token_type;
	unsigned char *ptr = source,
		ch;
	char buffer[LEXER_TOKEN_SIZE];
	int buffer_offset = 0;

	memset(buffer, 0, LEXER_TOKEN_SIZE);
	*count = 0;

	while (guess != EndOfLine) {
		ch = *(ptr++);
		guess = JC_Lex_Guess(ch);

		switch (state) {
			case None:
				switch (guess) {
					/* ignore rest of line */
					case CommentStart:
					case EndOfLine:
						return true;

					/* ignore */
					case Whitespace:
						continue;

					/* change state */
					case String:
						state = guess;
						continue;

					/* set buffer, change state */
					case Number:
					case Operator:
					case KeywordOrIdent:
						state = guess;
						Lex_Push(ch);
						continue;

					default: Lex_Error("Invalid character");
				}

			case String:
				switch (ch) {
					case '\\':
						state = StringEscape;
						continue;

					case '"':
						Lex_AddToken(StringLiteral);
						continue;

					case EndOfLine: Lex_Error("Unexpected end of line");

					default:
						Lex_Push(ch);
						continue;
				}

			case StringEscape:
				switch (ch) {
					case 'n':
						Lex_Push('\n');
						break;

					case EndOfLine: Lex_Error("Unexpected end of line");

					default:
						Lex_Push(ch);
						break;
				}
				state = String;
				continue;

			case Number:
				switch (ch) {
					case '#':
						Lex_AddToken(NumericLiteral);
						return true;

					caseNumeric:
						Lex_Push(ch);
						continue;

					caseEol:
					caseWhitespace:
						Lex_AddToken(NumericLiteral);
						continue;

					default: Lex_Error("Invalid character in numeric literal");
				}

			case KeywordOrIdent:
				switch (guess) {
					case '#':
						Lex_AddToken(NumericLiteral);
						return true;

					case KeywordOrIdent:
					case Number:
						Lex_Push(ch);
						continue;

					case Operator:
						ptr--;
						/* FALL THROUGH */

					case EndOfLine:
					case Whitespace:
						Lex_Push('\0');
						Lex_AddToken(JC_IsKeyword(buffer) ? Keyword : Identifier);
						continue;

					default: Lex_Error("Invalid character in keyword/identifier");
				}

			case Operator:
				switch (guess) {
					case Operator:
						Lex_Push(ch);
						continue;

					case KeywordOrIdent:
					case Number:
					case '"':
						ptr--;
						/* FALL THROUGH */

					case EndOfLine:
					case Whitespace:
						token_type = JC_Operator(buffer);
						if (token_type == Unknown) Lex_Error("Unknown operator");
						Lex_AddToken(token_type);
						continue;
				}

			default: Lex_Error("Unknown state");
		}
	}

	return true;
}
