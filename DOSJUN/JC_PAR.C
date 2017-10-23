/* I N C L U D E S /////////////////////////////////////////////////////// */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "jc.h"
#include "code.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define UNKNOWN_JUMP	0xFACE

#define STACK_SIZE		10
#define OFFSET_SIZE		20

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

/* P R O T O T Y P E S /////////////////////////////////////////////////// */

char *Get_Token_Name(jc_token_type tt);

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport char Get_Scope_Char(jc_scope sc)
{
	switch (sc) {
		case scGlobal: return 'G';
		case scLocal: return 'L';
		case scTemp: return 'T';
		case scConst: return 'C';
		default: return '?';
	}
}

noexport jc_token *Peek_Token(jc_parser *p)
{
	return &p->tokens[p->position];
}

noexport jc_token *Consume_Token(jc_parser *p)
{
	return &p->tokens[p->position++];
}

noexport bool Token_Matches(jc_parser *p, char *check)
{
	if (p->tokens[p->position].value == null) return false;
	return !strcmp(p->tokens[p->position].value, check);
}

noexport bool Parse_Error(jc_parser *p, char *message)
{
	printf("-- PARSE ERROR @%u/%utk --\n", p->position, p->token_count);
	printf("%s\n", message);

	return false;
}

noexport bool Save_Variable(jc_parser *p, jc_scope sc, char *name)
{
	unsigned int *count;

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

		case scConst: return true;
	}

	p->vars[p->var_count].scope = sc;
	p->vars[p->var_count].name = Duplicate_String(name, "Save_Variable");
	p->vars[p->var_count].index = *count;

	p->var_count++;
	(*count)++;

	return true;
}

noexport jc_var *Resolve_Variable(jc_parser *p, char *name)
{
	unsigned int i = 0;

	for (i = 0; i < p->var_count; i++) {
		if (!strcmp(p->vars[i].name, name))
			return &p->vars[i];
	}

	return null;
}

noexport void Save_Constant(jc_parser *p, char *name, int value)
{
	/* TODO: check var count */
	p->vars[p->var_count].scope = scConst;
	p->vars[p->var_count].name = Duplicate_String(name, "Save_Constant");
	p->vars[p->var_count].index = value;

	p->var_count++;
}

noexport string_id Save_String(jc_parser *p, char *string)
{
	/* TODO: check string count */
	p->strings[p->string_count] = Duplicate_String(string, "Save_String");
	return p->string_count++;
}

noexport void Emit_Code(jc_parser *p, bytecode code)
{
	/* TODO: check code size */
	jc_script *scr = &p->scripts[p->script_count - 1];
	scr->code[scr->size++] = code;
}

noexport void Emit_Int(jc_parser *p, int value)
{
	/* TODO: check code size */
	jc_script *scr = &p->scripts[p->script_count - 1];
	int *dest = (int *)&scr->code[scr->size];

	*dest = value;
	scr->size += 2;
}

/* Scope Stack */

noexport bool Push_Stack(jc_parser *p, char *name, int smod)
{
	/* TODO: check stack size */
	strncpy(p->stack[p->stack_size].name, name, 5);
	p->stack[p->stack_size].start = p->scripts[p->script_count - 1].size + smod;
	p->stack[p->stack_size].offsets = SzAlloc(OFFSET_SIZE, int, "Push_Stack");
	p->stack[p->stack_size].offset_count = 0;

	p->stack_size++;
	return true;
}

noexport void Renew_Stack(jc_parser *p, int smod)
{
	p->stack[p->stack_size - 1].start = p->scripts[p->script_count - 1].size + smod;
}

noexport bool Add_Stack_Offset(jc_parser *p, int omod)
{
	/* TODO: check offsets size */
	jc_stack *s = &p->stack[p->stack_size - 1];
	jc_script *scr = &p->scripts[p->script_count - 1];
	s->offsets[s->offset_count] = scr->size + omod;

	s->offset_count++;
	return true;
}

noexport void Resolve_Stack_Jump(jc_parser *p, int omod)
{
	jc_stack *s = &p->stack[p->stack_size - 1];
	jc_script *scr = &p->scripts[p->script_count - 1];
	int *loc = (int *)&scr->code[s->start];

	*loc = scr->size + omod;
}

noexport void Resolve_Stack_Offsets(jc_parser *p)
{
	jc_stack *s = &p->stack[p->stack_size - 1];
	jc_script *scr = &p->scripts[p->script_count - 1];
	int i, *loc;

	for (i = 0; i < s->offset_count; i++) {
		loc = (int *)&scr->code[s->offsets[i]];
		*loc = scr->size;
	}
}

noexport bool Pop_Stack(jc_parser *p)
{
	if (p->stack_size == 0) return false;
	p->stack_size--;

	Free(p->stack[p->stack_size].offsets);
	return true;
}

noexport bool Emit_Argument(jc_parser *p, jc_token *token)
{
	jc_var *var;
	string_id sindex;
	internal_id iindex;

	switch (token->type) {
		case ttNumber:
			EMIT_ARG(coPushLiteral, atoi(token->value));
			return true;

		case ttString:
			sindex = Save_String(p, token->value);
			EMIT_ARG(coPushLiteral, sindex);
			return true;

		case ttIdentifier:
			var = RESOLVE(token->value);
			if (var == null) return Parse_Error(p, "Unknown identifier");
			EMIT_PUSH_VAR(var);
			return true;

		case ttInternal:
			iindex = Get_Internal_Id(token->value);
			if (iindex == internalInvalid) return Parse_Error(p, "Unknown internal");
			EMIT(coPushInternal);
			EMIT(iindex);
			return true;

		default:
			printf("Emit_Argument() type=%s val=%s\n", Get_Token_Name(token->type), token->value);
			return false;
	}
}

noexport bool Emit_Comparison(jc_parser *p, jc_token *token)
{
	switch (token->type) {
		case ttEquals:
			EMIT(coEQ);
			return true;

		case ttNotEqual:
			EMIT(coNEQ);
			return true;

		case ttLT:
			EMIT(coLT);
			return true;

		case ttLTE:
			EMIT(coLTE);
			return true;

		case ttGT:
			EMIT(coGT);
			return true;

		case ttGTE:
			EMIT(coGTE);
			return true;

		default: return Parse_Error(p, "Unknown comparison");
	}
}

/* P A R S E R  S T A T E S ////////////////////////////////////////////// */

noexport bool Define_Const_State(jc_parser *p)
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

noexport bool Define_Global_State(jc_parser *p)
{
	jc_token *ident;

	CONSUME();
	ident = CONSUME();

	return Save_Variable(p, scGlobal, ident->value);
}

noexport bool Define_Local_State(jc_parser *p)
{
	jc_token *ident;

	CONSUME();
	ident = CONSUME();

	return Save_Variable(p, scLocal, ident->value);
}

noexport bool Begin_Script_State(jc_parser *p)
{
	jc_token *ident;

	CONSUME();
	ident = CONSUME();

	p->scripts[p->script_count].name = Duplicate_String(ident->value, "Begin_Script_State");
	p->scripts[p->script_count].code = SzAlloc(MAX_BYTES_PER_SCRIPT, bytecode, "Begin_Script_State");
	p->scripts[p->script_count].size = 0;
	Save_Constant(p, ident->value, p->script_count++);

	p->in_script = true;
	return true;
}

noexport bool Call_PcSpeak_State(jc_parser *p)
{
	jc_token *pc, *val;

	CONSUME();
	pc = CONSUME();
	val = CONSUME();

	if (!Emit_Argument(p, val)) return false;
	if (!Emit_Argument(p, pc)) return false;
	EMIT(coPcSpeak);
	return true;
}

noexport bool Call_Text_State(jc_parser *p)
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

noexport bool End_Script_State(jc_parser *p)
{
	/* TODO: resize code alloc? */
	if (p->stack_size > 0) return Parse_Error(p, "Unclosed scope");

	EMIT(coReturn);
	p->in_script = false;

	return true;
}

noexport bool Start_If_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *left, *right, *cmp;

	CONSUME();
	left = CONSUME();
	cmp = CONSUME();
	right = CONSUME();

	if (!Emit_Argument(p, right)) return Parse_Error(p, "If right argument invalid");
	if (!Emit_Argument(p, left)) return Parse_Error(p, "If left argument invalid");
	if (!Emit_Comparison(p, cmp)) return false;

	EMIT_ARG(coJumpFalse, UNKNOWN_JUMP);
	Push_Stack(p, "If", -2);
	return true;
}

noexport bool Start_ElseIf_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *left, *right, *cmp;

	if (p->stack_size == 0) return Parse_Error(p, "ElseIf without If");
	/* TODO: check stack top is an If */
	EMIT_ARG(coJump, UNKNOWN_JUMP);
	Add_Stack_Offset(p, -2);
	Resolve_Stack_Jump(p, 0);

	CONSUME();
	left = CONSUME();
	cmp = CONSUME();
	right = CONSUME();

	if (!Emit_Argument(p, right)) return false;
	if (!Emit_Argument(p, left)) return false;
	if (!Emit_Comparison(p, cmp)) return false;

	EMIT_ARG(coJumpFalse, UNKNOWN_JUMP);
	Renew_Stack(p, -2);
	return true;
}

noexport bool End_If_State(jc_parser *p)
{
	if (p->stack_size == 0) return Parse_Error(p, "EndIf without If");
	/* TODO: check stack top is an If */

	CONSUME();
	Resolve_Stack_Jump(p, 0);
	Resolve_Stack_Offsets(p);
	Pop_Stack(p);
	return true;
}

noexport bool Return_State(jc_parser *p)
{
	EMIT(coReturn);
	return true;
}

noexport bool Call_Combat_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *val;

	CONSUME();
	val = CONSUME();

	if (!Emit_Argument(p, val)) return false;

	EMIT(coCombat);
	return true;
}

noexport bool Call_Unlock_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *x, *y, *dir;

	CONSUME();
	x = CONSUME();
	y = CONSUME();
	dir = CONSUME();

	if (!Emit_Argument(p, dir)) return false;
	if (!Emit_Argument(p, y)) return false;
	if (!Emit_Argument(p, x)) return false;

	EMIT(coUnlock);
	return true;
}

noexport bool Call_GiveItem_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *pc, *item, *qty;

	CONSUME();
	pc = CONSUME();
	item = CONSUME();
	qty = CONSUME();

	if (!Emit_Argument(p, qty)) return false;
	if (!Emit_Argument(p, item)) return false;
	if (!Emit_Argument(p, pc)) return false;

	EMIT(coGiveItem);
	return true;
}

noexport bool Call_EquipItem_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *pc, *item;

	CONSUME();
	pc = CONSUME();
	item = CONSUME();

	if (!Emit_Argument(p, item)) return false;
	if (!Emit_Argument(p, pc)) return false;

	EMIT(coEquipItem);
	return true;
}

noexport bool Call_SetTileDescription_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *x, *y, *string;

	CONSUME();
	x = CONSUME();
	y = CONSUME();
	string = CONSUME();

	if (!Emit_Argument(p, string)) return false;
	if (!Emit_Argument(p, y)) return false;
	if (!Emit_Argument(p, x)) return false;

	EMIT(coSetTileDescription);
	return true;
}

noexport bool Call_SetTileColour_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *x, *y, *surface, *colour;

	CONSUME();
	x = CONSUME();
	y = CONSUME();
	surface = CONSUME();
	colour = CONSUME();

	if (!Emit_Argument(p, colour)) return false;
	if (!Emit_Argument(p, surface)) return false;
	if (!Emit_Argument(p, y)) return false;
	if (!Emit_Argument(p, x)) return false;

	EMIT(coSetTileColour);
	return true;
}

noexport bool Call_SetTileThing_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *x, *y, *thing;

	CONSUME();
	x = CONSUME();
	y = CONSUME();
	thing = CONSUME();

	if (!Emit_Argument(p, thing)) return false;
	if (!Emit_Argument(p, y)) return false;
	if (!Emit_Argument(p, x)) return false;

	EMIT(coSetTileThing);
	return true;
}

noexport bool Call_Teleport_State(jc_parser *p)
{
	/* TODO: expressions */
	jc_token *zone, *x, *y, *facing, *transition;

	CONSUME();
	zone = CONSUME();
	x = CONSUME();
	y = CONSUME();
	facing = CONSUME();
	transition = CONSUME();

	if (!Emit_Argument(p, transition)) return false;
	if (!Emit_Argument(p, facing)) return false;
	if (!Emit_Argument(p, y)) return false;
	if (!Emit_Argument(p, x)) return false;
	if (!Emit_Argument(p, zone)) return false;

	EMIT(coTeleport);
	return true;
}

noexport bool Call_SetDanger_State(jc_parser *p)
{
	/* TODO: expression */
	jc_token *danger;

	CONSUME();
	danger = CONSUME();

	if (!Emit_Argument(p, danger)) return false;

	EMIT(coSetDanger);
	return true;
}

noexport bool Keyword_State(jc_parser *p)
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
		if (MATCH("Unlock")) return Call_Unlock_State(p);
		if (MATCH("GiveItem")) return Call_GiveItem_State(p);
		if (MATCH("EquipItem")) return Call_EquipItem_State(p);
		if (MATCH("SetTileDescription")) return Call_SetTileDescription_State(p);
		if (MATCH("SetTileColour")) return Call_SetTileColour_State(p);
		if (MATCH("Teleport")) return Call_Teleport_State(p);
		if (MATCH("SetTileThing")) return Call_SetTileThing_State(p);
		if (MATCH("SetDanger")) return Call_SetDanger_State(p);

		if (MATCH("If")) return Start_If_State(p);
		if (MATCH("ElseIf")) return Start_ElseIf_State(p);
		if (MATCH("EndIf")) return End_If_State(p);

		if (MATCH("Return")) return Return_State(p);
		if (MATCH("EndScript")) return End_Script_State(p);
	}

	return Parse_Error(p, "Unexpected keyword");
}

noexport bool Identifier_State(jc_parser *p)
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
	p->vars = SzAlloc(MAX_GLOBALS + MAX_LOCALS + MAX_TEMPS + MAX_CONSTS, jc_var, "Initialise_Parser.vars");
	p->global_count = 0;
	p->local_count = 0;
	p->temp_count = 0;
	p->var_count = 0;

	p->scripts = SzAlloc(MAX_SCRIPTS, jc_script, "Initialise_Parser.scripts");
	p->script_count = 0;
	p->in_script = false;

	p->strings = SzAlloc(MAX_STRINGS, char *, "Initialise_Parser.strings");
	p->string_count = 0;

	p->stack = SzAlloc(STACK_SIZE, jc_stack, "Initialise_Parser.stack");
	p->stack_size = 0;
}

void Free_Parser(jc_parser *p)
{
	unsigned int i;

	for (i = 0; i < p->var_count; i++) {
		Free(p->vars[i].name);
	}
	Free(p->vars);
	p->var_count = 0;

	for (i = 0; i < p->script_count; i++) {
		Free(p->scripts[i].name);
		Free(p->scripts[i].code);
	}
	Free(p->scripts);
	p->script_count = 0;

	for (i = 0; i < p->string_count; i++) {
		Free(p->strings[i]);
	}
	Free(p->strings);
	p->string_count = 0;

	Free(p->stack);
	p->stack_size = 0;
}

void Dump_Parser_State(jc_parser *p)
{
	unsigned int i;

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

	printf("[STRINGS]: %u\n", p->string_count);
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
