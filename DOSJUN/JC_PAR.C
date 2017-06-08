/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "jc.h"
#include "code.h"
#include <stdlib.h>
#include <string.h>

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define UNKNOWN_JUMP	0xFACE

#define PEEK()		Peek_Token(p)
#define CONSUME()	Consume_Token(p)
#define MATCH(c)	Token_Matches(p, c)

#define EMIT(co)	Emit_Code(p, co)
#define EMIT_ARG(co, a) { \
	Emit_Code(p, co); \
	Emit_Int(p, a); \
}

#define EMIT_PUSH_VAR(v) { \
	switch (v->scope) { \
		case scGlobal: \
			Emit_Code(p, coPushGlobal); \
			Emit_Code(p, v->index); \
			break; \
\
		case scLocal: \
			Emit_Code(p, coPushLocal); \
			Emit_Code(p, v->index); \
			break; \
\
		case scTemp: \
			Emit_Code(p, coPushTemp); \
			Emit_Code(p, v->index); \
			break; \
\
		case scConst: \
			EMIT_ARG(coPushLiteral, v->index); \
			break; \
\
		default: return Parse_Error(p, "Unknown variable scope"); \
	} \
}

#define EMIT_POP_VAR(v) { \
	switch (v->scope) { \
		case scGlobal: \
			Emit_Code(p, coPopGlobal); \
			Emit_Code(p, v->index); \
			break; \
\
		case scLocal: \
			Emit_Code(p, coPopLocal); \
			Emit_Code(p, v->index); \
			break; \
\
		case scTemp: \
			Emit_Code(p, coPopTemp); \
			Emit_Code(p, v->index); \
			break; \
\
		default: return Parse_Error(p, "Unknown variable scope"); \
	} \
}

#define RESOLVE(n)	Resolve_Variable(p, n)

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

char Get_Scope_Char(jc_scope sc)
{
	switch (sc) {
		case scGlobal: return 'G';
		case scLocal: return 'L';
		case scTemp: return 'T';
		case scConst: return 'C';
		default: return '?';
	}
}

jc_token *Peek_Token(jc_parser *p)
{
	return &p->tokens[p->position];
}

jc_token *Consume_Token(jc_parser *p)
{
	return &p->tokens[p->position++];
}

bool Token_Matches(jc_parser *p, char *check)
{
	if (p->tokens[p->position].value == null) return false;
	return !strcmp(p->tokens[p->position].value, check);
}

bool Parse_Error(jc_parser *p, char *message)
{
	printf("-- PARSE ERROR --\n");
	printf("%s\n", message);

	return false;
}

bool Save_Variable(jc_parser *p, jc_scope sc, char *name)
{
	int *count;

	switch (sc) {
		case scGlobal:
			count = &p->global_count;
			if (*count == MAX_GLOBALS) return Parse_Error(p, "Too many globals");
			break;

		case scLocal:
			count = &p->local_count;
			if (*count == MAX_LOCALS) return Parse_Error(p, "Too many locals");
			break;

		case scTemp:
			count = &p->temp_count;
			if (*count == MAX_TEMPS) return Parse_Error(p, "Too many temps");
			break;
	}

	p->vars[p->var_count].scope = sc;
	p->vars[p->var_count].name = strdup(name);
	p->vars[p->var_count].index = *count;

	p->var_count++;
	*count++;

	return true;
}

jc_var *Resolve_Variable(jc_parser *p, char *name)
{
	int i = 0;

	for (i = 0; i < p->var_count; i++) {
		if (!strcmp(p->vars[i].name, name))
			return &p->vars[i];
	}

	return null;
}

void Save_Constant(jc_parser *p, char *name, int value)
{
	/* TODO: check var count */
	p->vars[p->var_count].scope = scConst;
	p->vars[p->var_count].name = strdup(name);
	p->vars[p->var_count].index = value;

	p->var_count++;
}

int Save_String(jc_parser *p, char *string)
{
	/* TODO: check string count */
	p->strings[p->string_count] = strdup(string);
	return p->string_offset + p->string_count++;
}

void Emit_Code(jc_parser *p, bytecode code)
{
	/* TODO: check code size */
	jc_script *scr = &p->scripts[p->script_count - 1];
	scr->code[scr->size++] = code;
}

void Emit_Int(jc_parser *p, int value)
{
	/* TODO: check code size */
	jc_script *scr = &p->scripts[p->script_count - 1];
	int *dest = &scr->code[scr->size];

	*dest = value;
	scr->size += 2;
}

/* Scope Stack */

bool Push_Stack(jc_parser *p, char *name, int smod)
{
	/* TODO: check stack size */
	strncpy(p->stack[p->stack_size].name, name, 5);
	p->stack[p->stack_size].start = p->scripts[p->script_count - 1].size + smod;
	p->stack[p->stack_size].offsets = szalloc(MAX_STACK_OFFSETS, int);
	p->stack[p->stack_size].offset_count = 0;

	p->stack_size++;
	return true;
}

void Renew_Stack(jc_parser *p, int smod)
{
	p->stack[p->stack_size - 1].start = p->scripts[p->script_count - 1].size + smod;
}

bool Add_Stack_Offset(jc_parser *p, int omod)
{
	/* TODO: check offsets size */
	jc_stack *s = &p->stack[p->stack_size - 1];
	jc_script *scr = &p->scripts[p->script_count - 1];
	s->offsets[s->offset_count] = scr->size + omod;

	s->offset_count++;
	return true;
}

void Resolve_Stack_Jump(jc_parser *p, int omod)
{
	jc_stack *s = &p->stack[p->stack_size - 1];
	jc_script *scr = &p->scripts[p->script_count - 1];
	int *loc = &scr->code[s->start];

	*loc = scr->size + omod;
}

void Resolve_Stack_Offsets(jc_parser *p)
{
	jc_stack *s = &p->stack[p->stack_size - 1];
	jc_script *scr = &p->scripts[p->script_count - 1];
	int i, *loc;

	for (i = 0; i < s->offset_count; i++) {
		loc = &scr->code[s->offsets[i]];
		*loc = scr->size;
	}
}

bool Pop_Stack(jc_parser *p)
{
	if (p->stack_size == 0) return false;
	p->stack_size--;

	free(p->stack[p->stack_size].offsets);
	return true;
}

/* P A R S E R  S T A T E S ////////////////////////////////////////////// */

bool Define_Const_State(jc_parser *p)
{
	jc_token *ident;
	jc_token *val;

	CONSUME();
	ident = CONSUME();

	if (!MATCH("=")) return Parse_Error(p, "Expected =");
	CONSUME();

	val = CONSUME();
	if (val->type != ttNumber) return Parse_Error(p, "Expected number");

	Save_Constant(p, ident->value, atoi(val->value));
	return true;
}

bool Define_Global_State(jc_parser *p)
{
	jc_token *ident;

	CONSUME();
	ident = CONSUME();

	return Save_Variable(p, scGlobal, ident->value);
}

bool Define_Local_State(jc_parser *p)
{
	jc_token *ident;

	CONSUME();
	ident = CONSUME();

	return Save_Variable(p, scLocal, ident->value);
}

bool Begin_Script_State(jc_parser *p)
{
	jc_token *ident;

	CONSUME();
	ident = CONSUME();

	p->scripts[p->script_count].name = strdup(ident->value);
	p->scripts[p->script_count].code = malloc(MAX_BYTES_PER_SCRIPT);
	p->scripts[p->script_count].size = 0;
	Save_Constant(p, ident->value, p->script_count++);

	p->in_script = true;
	return true;
}

bool Call_PcSpeak_State(jc_parser *p)
{
	jc_token *pc, *val;
	jc_var *pc_var;
	int sindex;

	CONSUME();
	pc = CONSUME();
	val = CONSUME();

	if (val->type == ttNumber) {
		EMIT_ARG(coPushLiteral, atoi(val->value));
	} else if (val->type == ttString) {
		sindex = Save_String(p, val->value);
		EMIT_ARG(coPushLiteral, sindex);
	}

	if (pc->type == ttNumber) {
		EMIT_ARG(coPushLiteral, atoi(pc->value));
	} else if (pc->type == ttIdentifier) {
		pc_var = RESOLVE(pc->value);
		if (pc_var == null) return Parse_Error(p, "Unknown identifier");
		EMIT_PUSH_VAR(pc_var);
	} else {
		return Parse_Error(p, "Invalid first argument to PcSpeak");
	}

	EMIT(coPcSpeak);
	return true;
}

bool Call_Text_State(jc_parser *p)
{
	jc_token *val;
	int sindex;

	CONSUME();
	val = CONSUME();

	if (val->type == ttNumber) {
		EMIT_ARG(coPushLiteral, atoi(val->value));
	} else if (val->type == ttString) {
		sindex = Save_String(p, val->value);
		EMIT_ARG(coPushLiteral, sindex);
	} else {
		return Parse_Error(p, "Invalid argument to Text");
	}

	EMIT(coText);
	return true;
}

bool End_Script_State(jc_parser *p)
{
	/* TODO: resize code alloc? */
	if (p->stack_size > 0) return Parse_Error(p, "Unclosed scope");

	EMIT(coReturn);
	p->in_script = false;

	return true;
}

bool Start_If_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *left, *right, *cmp;
	jc_var *left_v, *right_v;

	CONSUME();
	left = CONSUME();
	cmp = CONSUME();
	right = CONSUME();

	if (left->type == ttNumber) {
		EMIT_ARG(coPushLiteral, atoi(left->value));
	} else if (left->type == ttIdentifier) {
		left_v = RESOLVE(left->value);
		if (left_v == null) return Parse_Error(p, "Unknown left identifier");
		EMIT_PUSH_VAR(left_v);
	} else {
		return Parse_Error(p, "Invalid left argument to If");
	}

	if (right->type == ttNumber) {
		EMIT_ARG(coPushLiteral, atoi(right->value));
	} else if (right->type == ttIdentifier) {
		right_v = RESOLVE(right->value);
		if (right_v == null) return Parse_Error(p, "Unknown right identifier");
		EMIT_PUSH_VAR(right_v);
	} else {
		return Parse_Error(p, "Invalid right argument to If");
	}

	if (cmp->type == ttEquals) {
		EMIT(coEQ);
	} else if (cmp->type == ttNotEqual) {
		EMIT(coNEQ);
	} else {
		return Parse_Error(p, "Unknown comparison");
	}

	EMIT_ARG(coJumpFalse, UNKNOWN_JUMP);
	Push_Stack(p, "If", -2);
	return true;
}

bool Start_ElseIf_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *left, *right, *cmp;
	jc_var *left_v, *right_v;
	int *preserved;

	if (p->stack_size == 0) return Parse_Error(p, "ElseIf without If");
	/* TODO: check stack top is an If */
	EMIT_ARG(coJump, UNKNOWN_JUMP);
	Add_Stack_Offset(p, -2);
	Resolve_Stack_Jump(p, 0);

	CONSUME();
	left = CONSUME();
	cmp = CONSUME();
	right = CONSUME();

	if (left->type == ttNumber) {
		EMIT_ARG(coPushLiteral, atoi(left->value));
	} else if (left->type == ttIdentifier) {
		left_v = RESOLVE(left->value);
		if (left_v == null) return Parse_Error(p, "Unknown left identifier");
		EMIT_PUSH_VAR(left_v);
	} else {
		return Parse_Error(p, "Invalid left argument to ElseIf");
	}

	if (right->type == ttNumber) {
		EMIT_ARG(coPushLiteral, atoi(right->value));
	} else if (right->type == ttIdentifier) {
		right_v = RESOLVE(right->value);
		if (right_v == null) return Parse_Error(p, "Unknown right identifier");
		EMIT_PUSH_VAR(right_v);
	} else {
		return Parse_Error(p, "Invalid right argument to ElseIf");
	}

	if (cmp->type == ttEquals) {
		EMIT(coEQ);
	} else if (cmp->type == ttNotEqual) {
		EMIT(coNEQ);
	} else {
		return Parse_Error(p, "Unknown comparison");
	}

	EMIT_ARG(coJumpFalse, UNKNOWN_JUMP);
	Renew_Stack(p, -2);
	return true;
}

bool End_If_State(jc_parser *p)
{
	int i;

	if (p->stack_size == 0) return Parse_Error(p, "EndIf without If");
	/* TODO: check stack top is an If */

	CONSUME();
	Resolve_Stack_Jump(p, 0);
	Resolve_Stack_Offsets(p);
	Pop_Stack(p);
	return true;
}

bool Return_State(jc_parser *p)
{
	EMIT(coReturn);
	return true;
}

bool Call_Combat_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *val;
	jc_var *val_v;

	CONSUME();
	val = CONSUME();

	if (val->type == ttNumber) {
		EMIT_ARG(coPushLiteral, atoi(val->value));
	} else if (val->type == ttIdentifier) {
		val_v = RESOLVE(val->value);
		if (val_v == null) return Parse_Error(p, "Unknown identifier");
		EMIT_PUSH_VAR(val_v);
	} else {
		return Parse_Error(p, "Invalid argument to Text");
	}

	EMIT(coCombat);
	return true;
}

bool Keyword_State(jc_parser *p)
{
	if (!p->in_script) {
		if (MATCH("Const")) return Define_Const_State(p);
		if (MATCH("Global")) return Define_Global_State(p);
		if (MATCH("Local")) return Define_Local_State(p);

		if (MATCH("Script")) return Begin_Script_State(p);
	} else {
		if (MATCH("Combat")) return Call_Combat_State(p);
		if (MATCH("PcSpeak")) return Call_PcSpeak_State(p);
		if (MATCH("Text")) return Call_Text_State(p);

		if (MATCH("If")) return Start_If_State(p);
		if (MATCH("ElseIf")) return Start_ElseIf_State(p);
		if (MATCH("EndIf")) return End_If_State(p);

		if (MATCH("Return")) return Return_State(p);
		if (MATCH("EndScript")) return End_Script_State(p);
	}

	return Parse_Error(p, "Unexpected keyword");
}

bool Identifier_State(jc_parser *p)
{
	/* TODO: Expressions */
	jc_token *targ, *value;
	jc_var *targ_var, *value_var;

	targ = CONSUME();
	targ_var = RESOLVE(targ->value);
	if (targ_var == null) {
		/* Create a new Temp variable */
		targ_var = &p->vars[p->var_count];
		if (!Save_Variable(p, scTemp, targ->value)) return false;
	}

	if (targ_var->scope == scConst) return Parse_Error(p, "Cannot assign to const");

	if (!MATCH("=")) return Parse_Error(p, "Expected =");
	CONSUME();

	value = CONSUME();
	if (value->type == ttNumber) {
		EMIT_ARG(coPushLiteral, atoi(value->value));
	} else if (value->type == ttIdentifier) {
		value_var = RESOLVE(value->value);
		if (value_var == null) return Parse_Error(p, "Unknown identifier");
		EMIT_PUSH_VAR(value_var);
	}

	EMIT_POP_VAR(targ_var);
	return true;
}

/* M A I N /////////////////////////////////////////////////////////////// */

void Initialise_Parser(jc_parser *p)
{
	p->vars = szalloc(MAX_GLOBALS + MAX_LOCALS + MAX_TEMPS + MAX_CONSTS, jc_var);
	p->global_count = 0;
	p->local_count = 0;
	p->temp_count = 0;
	p->var_count = 0;

	p->scripts = szalloc(MAX_SCRIPTS, jc_script);
	p->script_count = 0;
	p->in_script = false;

	p->strings = szalloc(MAX_STRINGS, char *);
	p->string_count = 0;
	p->string_offset = 0;

	p->stack = szalloc(MAX_STACK, jc_stack);
	p->stack_size = 0;
}

void Free_Parser(jc_parser *p)
{
	int i;

	for (i = 0; i < p->var_count; i++) {
		free(p->vars[i].name);
	}
	nullfree(p->vars);

	for (i = 0; i < p->script_count; i++) {
		free(p->scripts[i].name);
		free(p->scripts[i].code);
	}
	nullfree(p->scripts);

	for (i = 0; i < p->string_count; i++) {
		free(p->strings[i]);
	}
	nullfree(p->strings);

	nullfree(p->stack);
}

void Dump_Parser_State(jc_parser *p)
{
	int i;

	printf("[VARS]");
	for (i = 0; i < p->var_count; i++) {
		printf(" %c:%s=%d", Get_Scope_Char(p->vars[i].scope), p->vars[i].name, p->vars[i].index);
	}
	printf("\n");

	printf("[SCRIPTS]");
	for (i = 0; i < p->script_count; i++) {
		printf(" %s", p->scripts[i].name);
	}
	printf("\n");

	printf("[STRINGS]: %d\n", p->string_count);
}

bool Parse_Tokens(jc_parser *p, jc_token *tokens, int count)
{
	p->tokens = tokens;
	p->token_count = count;
	p->position = 0;

	switch (PEEK()->type) {
		case ttKeyword:
			return Keyword_State(p);

		case ttIdentifier:
			return Identifier_State(p);

		default:
			Parse_Error(p, "Invalid starting token");
			return false;
	}
}
