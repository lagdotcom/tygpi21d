/* I N C L U D E S /////////////////////////////////////////////////////// */

#include "dosjun.h"
#include "code.h"

/* D E F I N E S ///////////////////////////////////////////////////////// */

#define Code_Error printf

/* S T R U C T U R E S /////////////////////////////////////////////////// */

/* G L O B A L S ///////////////////////////////////////////////////////// */

/* F U N C T I O N S ///////////////////////////////////////////////////// */

noexport bytecode Next_Op(host *h)
{
	h->pc = h->next_pc;
	h->next_pc++;
	return h->code[h->pc];
}

noexport int Next_Literal(host *h)
{
	bytecode little = Next_Op(h);
	bytecode big = Next_Op(h);
	return little | (big << 8);
}

noexport void Push_Stack(host *h, int value)
{
	h->stack[h->sp++] = value;
}

noexport int Pop_Stack(host *h)
{
	return h->stack[--h->sp];
}

noexport int Bool(int result)
{
	return result ? 1 : 0;
}

/* O P C O D E  F U N C T I O N S //////////////////////////////////////// */

noexport void Push_Global(host *h)
{
	bytecode index = Next_Op(h);
	Push_Stack(h, h->globals[index]);
}

noexport void Push_Local(host *h)
{
	bytecode index = Next_Op(h);
	Push_Stack(h, h->locals[index]);
}

noexport void Push_Temp(host *h)
{
	bytecode index = Next_Op(h);
	Push_Stack(h, h->temps[index]);
}

noexport void Push_Literal(host *h)
{
	int value = Next_Literal(h);
	Push_Stack(h, value);
}

noexport void Pop_Global(host *h)
{
	bytecode index = Next_Op(h);
	h->globals[index] = Pop_Stack(h);
}

noexport void Pop_Local(host *h)
{
	bytecode index = Next_Op(h);
	h->locals[index] = Pop_Stack(h);
}

noexport void Pop_Temp(host *h)
{
	bytecode index = Next_Op(h);
	h->temps[index] = Pop_Stack(h);
}

noexport void Add(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
	Push_Stack(h, left + right);
}

noexport void Sub(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
	Push_Stack(h, left - right);
}

noexport void Mul(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
	Push_Stack(h, left * right);
}

noexport void Div(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
	Push_Stack(h, left / right);
}

noexport void Neq(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
	Push_Stack(h, Bool(left != right));
}

noexport void Lt(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
	Push_Stack(h, Bool(left < right));
}

noexport void Lte(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
	Push_Stack(h, Bool(left <= right));
}

noexport void Gt(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
	Push_Stack(h, Bool(left > right));
}

noexport void Gte(host *h)
{
	int left = Pop_Stack(h);
	int right = Pop_Stack(h);
	Push_Stack(h, Bool(left >= right));
}

noexport void Jump(host *h)
{
	h->next_pc = Next_Literal(h);
}

noexport void JumpFalse(host *h)
{
	int offset = Next_Literal(h);
	if (Pop_Stack(h)) h->next_pc = offset;
}

noexport void Return(host *h)
{
	h->running = false;
	h->result = 0;
}

noexport void Combat(host *h)
{
	char buffer[300];
	encounter_id combat = Pop_Stack(h);

	/* TODO */
	sprintf(buffer, "-- COMBAT #%d --", combat);
	Show_Game_String(buffer, true);
}

noexport void PcSpeak(host *h)
{
	char buffer[300];
	int pc = Pop_Stack(h);
	string_id string = Pop_Stack(h);
	
	sprintf(buffer, "%s:\n%s", S.characters[pc].name, Z.code_strings[string]);
	Show_Game_String(buffer, true);
}

noexport void Text(host *h)
{
	string_id string = Pop_Stack(h);

	Show_Game_String(Z.code_strings[string], true);
}

noexport void Unlock(host *h)
{
	char buffer[300];
	coord x = Pop_Stack(h);
	coord y = Pop_Stack(h);
	direction dir = Pop_Stack(h);

	/* TODO */
	sprintf(buffer, "-- UNLOCK %d, %d, %d --", x, y, dir);
	Show_Game_String(buffer, true);
}

/* M A I N /////////////////////////////////////////////////////////////// */

noexport void Run_Code_Instruction(host *h, bytecode op)
{
	switch (op) {
		case coPushGlobal:	Push_Global(h); return;
		case coPushLocal:	Push_Local(h); return;
		case coPushTemp:	Push_Temp(h); return;
		case coPushLiteral:	Push_Literal(h); return;
		case coPopGlobal:	Pop_Global(h); return;
		case coPopLocal:	Pop_Local(h); return;
		case coPopTemp:		Pop_Temp(h); return;

		case coAdd:			Add(h); return;
		case coSub:			Sub(h); return;
		case coMul:			Mul(h); return;
		case coDiv:			Div(h); return;

		case coEQ:			Eq(h); return;
		case coNEQ:			Neq(h); return;
		case coLT:			Lt(h); return;
		case coLTE:			Lte(h); return;
		case coGT:			Gt(h); return;
		case coGTE:			Gte(h); return;

		case coJump:		Jump(h); return;
		case coJumpFalse:	JumpFalse(h); return;
		case coReturn:		Return(h); return;

		case coCombat:		Combat(h); return;
		case coPcSpeak:		PcSpeak(h); return;
		case coText:		Text(h); return;
		case coUnlock:		Unlock(h); return;
	}

	h->running = false;
	h->result = -1;
	Code_Error("Unknown opcode: %2x", op);
}

noexport bool Run_Code_Host(host *h)
{
	h->running = true;

	while (h->running) {
		h->next_pc = h->pc + 1;
		Run_Code_Instruction(h, h->code[h->pc]);

		h->pc = h->next_pc;
	}

	return h->result == 0;
}

bool Run_Code(script_id id)
{
	bool result;
	host h;

	h.code = Z.scripts[id];
	h.globals = S.script_globals;
	h.locals = S.script_locals[S.header.zone];
	h.temps = SzAlloc(MAX_TEMPS, int, "Run_Code.temps");
	h.stack = SzAlloc(MAX_STACK, int, "Run_Code.stack");

	h.pc = 0;
	h.result = 0;
	h.sp = 0;

	result = Run_Code_Host(&h);

	Free(h.temps);
	Free(h.stack);

	return result;
}
