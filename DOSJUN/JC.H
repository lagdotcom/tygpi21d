#ifndef DOSJUN_JC_H
#define DOSJUN_JC_H

/* D E F I N E S ///////////////////////////////////////////////////////// */

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

#endif
