#ifndef DOSJUN_JC_H
#define DOSJUN_JC_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_TOKENS_PER_LINE		20
#define MAX_SCRIPTS				100
#define MAX_BYTES_PER_SCRIPT	1000
#define MAX_STRINGS				200
#define MAX_STACK				10
#define MAX_STACK_OFFSETS		20

typedef enum {
	ttUnknown,
	ttComment,
	ttKeyword,
	ttIdentifier,
	ttString,
	ttNumber,
	ttAssignment,
	ttEquals,
	ttNotEqual
} jc_token_type;

typedef enum {
	scConst,
	scGlobal,
	scLocal,
	scTemp
} jc_scope;

/* S T R U C T U R E S /////////////////////////////////////////////////// */

typedef struct {
	jc_token_type type;
	char *value;
} jc_token;

typedef struct {
	jc_scope scope;
	char *name;
	int index;
	int value;
} jc_var;

typedef struct {
	char *name;
	bytecode *code;
	int size;
} jc_script;

typedef struct {
	char name[6];
	int start;
	int *offsets;
	int offset_count;
} jc_stack;

typedef struct {
	jc_token *tokens;
	int token_count;
	int position;

	jc_var *vars;
	int global_count;
	int local_count;
	int temp_count;
	int var_count;

	jc_script *scripts;
	int script_count;
	bool in_script;

	char **strings;
	int string_count;

	jc_stack *stack;
	int stack_size;
} jc_parser;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

jc_token_type Parse_Operator(char *string);
bool Is_Keyword(char *string);

#endif
