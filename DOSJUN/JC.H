#ifndef DOSJUN_JC_H
#define DOSJUN_JC_H

/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "common.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define MAX_TOKENS_PER_LINE		20
#define MAX_SCRIPTS				100
#define MAX_BYTES_PER_SCRIPT	1000
#define MAX_STRINGS				200

typedef enum {
	ttUnknown,
	ttComment,
	ttKeyword,
	ttIdentifier,
	ttInternal,
	ttString,
	ttNumber,
	ttAssignment,
	ttEquals,
	ttNotEqual,
	ttLT,
	ttLTE,
	ttGT,
	ttGTE
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
	unsigned int size;
} jc_script;

typedef struct {
	char name[6];
	int start;
	int *offsets;
	int offset_count;
} jc_stack;

typedef struct {
	jc_token *tokens;
	unsigned int token_count;
	unsigned int position;

	jc_var *vars;
	unsigned int global_count;
	unsigned int local_count;
	unsigned int temp_count;
	unsigned int var_count;

	jc_script *scripts;
	unsigned int script_count;
	bool in_script;

	char **strings;
	unsigned int string_count;

	jc_stack *stack;
	unsigned int stack_size;
} jc_parser;

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

int Compile_JC(jc_parser *parser, char *filename, bool toplevel);
bool Dump_Compiled_JC(jc_parser *p, char *filename);
void Dump_Parser_State(jc_parser *p);
void Free_Parser(jc_parser *p);
internal_id Get_Internal_Id(char *string);
void Initialise_Parser(jc_parser *p);
jc_token_type Parse_Operator(char *string);
bool Parse_Tokens(jc_parser *p, jc_token *tokens, int count);

#endif
