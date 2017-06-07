#ifndef DOSJUN_JC_H
#define DOSJUN_JC_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "types.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_TOKENS_PER_LINE		20

typedef enum {
	Unknown,
	Comment,
	Keyword,
	Identifier,
	StringLiteral,
	NumericLiteral,
	Assignment,
	IsEqual,
	IsNotEqual
} jc_token_type;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	jc_token_type type;
	char *value;
} jc_token;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

jc_token_type JC_Operator(char *string);
bool JC_IsKeyword(char *string);

#endif
