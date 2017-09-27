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
	INT16 index;
	INT16 value;
} jc_var;

typedef struct {
	char *name;
	bytecode *code;
	UINT16 size;
} jc_script;

typedef struct {
	char name[6];
	INT16 start;
	INT16 *offsets;
	INT16 offset_count;
} jc_stack;

typedef struct {
	jc_token *tokens;
	UINT16 token_count;
	UINT16 position;

	jc_var *vars;
	UINT16 global_count;
	UINT16 local_count;
	UINT16 temp_count;
	UINT16 var_count;

	jc_script *scripts;
	UINT16 script_count;
	bool in_script;

	char **strings;
	UINT16 string_count;

	jc_stack *stack;
	UINT16 stack_size;
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
